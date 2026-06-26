#include <errno.h>
#include <stddef.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/can.h>
#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/printk.h>

#include "specialized/app_can_message.h"
#include "specialized/app_logic.h"
#include "specialized/can_formatter.h"
#include "specialized/runtime.h"

#define RX_THREAD_STACK_SIZE 1024
#define RX_THREAD_PRIORITY 5

static const struct device *const can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));

struct tx_context {
	struct k_work_delayable work;
	struct can_frame frame;
	uint32_t period_ms;
};

static struct tx_context tx_contexts[APP_CONFIG_TX_COUNT];
static bool runtime_started;

static void tx_work_handler(struct k_work *work)
{
	struct k_work_delayable *dwork = k_work_delayable_from_work(work);
	struct tx_context *ctx = CONTAINER_OF(dwork, struct tx_context, work);
	int ret;

	sys_rand_get(ctx->frame.data, ctx->frame.dlc);

	ret = can_send(can_dev, &ctx->frame, K_MSEC(100), NULL, NULL);
	if (ret != 0) {
		printk("CAN TX failed for ID 0x%x: %d\n", ctx->frame.id, ret);
	} else {
		printk("CAN TX sent ID 0x%x DLC %u\n", ctx->frame.id, ctx->frame.dlc);
	}

	k_work_reschedule(dwork, K_MSEC(ctx->period_ms));
}

static int configure_can_bus(const struct app_can_bus_config *can)
{
	struct can_timing timing;
	int ret;

	/* can_set_mode()/can_set_timing() must run before can_start(); the
	 * driver rejects them with -EBUSY once the device is started.
	 */
	ret = can_set_mode(can_dev, can->loopback ? CAN_MODE_LOOPBACK : 0);
	if (ret != 0) {
		return ret;
	}

	ret = can_calc_timing(can_dev, &timing, can->bitrate, can->sample_point_permille);
	if (ret < 0) {
		return ret;
	}

	return can_set_timing(can_dev, &timing);
}

int runtime_start_periodic_tx(const struct app_config *config)
{
	int ret;

	if (config == NULL) {
		return -EINVAL;
	}

	/* Not designed to reconfigure a running bus: can_set_mode() and
	 * can_set_timing() reject calls once the device is started, so a
	 * second call would otherwise fail with -EBUSY instead of being a
	 * harmless no-op.
	 */
	if (runtime_started) {
		return 0;
	}

	if (!device_is_ready(can_dev)) {
		return -ENODEV;
	}

	ret = configure_can_bus(&config->can);
	if (ret != 0) {
		return ret;
	}

	ret = can_start(can_dev);
	if (ret != 0 && ret != -EALREADY) {
		return ret;
	}

	for (size_t i = 0; i < APP_CONFIG_TX_COUNT; i++) {
		struct tx_context *ctx = &tx_contexts[i];

		ctx->frame.id = config->tx[i].id;
		ctx->frame.flags = config->tx[i].extended_id ? CAN_FRAME_IDE : 0;
		ctx->frame.dlc = APP_CAN_MESSAGE_MAX_DLC;
		ctx->period_ms = config->tx[i].period_ms;

		k_work_init_delayable(&ctx->work, tx_work_handler);
		k_work_schedule(&ctx->work, K_MSEC(ctx->period_ms));
	}

	runtime_started = true;

	return 0;
}

/* The RX queue depth is a build-time setting, so the msgq uses the Kconfig
 * value directly. The same value is copied into app_config and validated at
 * boot, keeping the documented configuration and runtime storage aligned.
 */
CAN_MSGQ_DEFINE(rx_msgq, CONFIG_APP_CAN_RX_QUEUE_DEPTH);
K_THREAD_STACK_DEFINE(rx_thread_stack, RX_THREAD_STACK_SIZE);

static struct k_thread rx_thread_data;
static bool rx_printer_started;
static struct app_logic_state rx_logic_state;

static void frame_to_message(const struct can_frame *frame, struct app_can_message *message)
{
	uint8_t dlc = frame->dlc > APP_CAN_MESSAGE_MAX_DLC ? APP_CAN_MESSAGE_MAX_DLC : frame->dlc;

	message->id = frame->id;
	message->extended_id = (frame->flags & CAN_FRAME_IDE) != 0;
	message->dlc = dlc;
	memcpy(message->data, frame->data, dlc);
	message->timestamp_ms = (uint64_t)k_uptime_get();
}

static void rx_thread_entry(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	struct can_frame frame;
	struct app_can_message message;
	char line[CAN_FORMATTER_LINE_LEN];

	while (1) {
		k_msgq_get(&rx_msgq, &frame, K_FOREVER);

		frame_to_message(&frame, &message);

		switch (app_logic_handle_message(&rx_logic_state, &message)) {
		case APP_LOGIC_ACTION_PRINT:
			if (can_formatter_format(&message, line, sizeof(line), NULL) ==
			    CAN_FORMATTER_OK) {
				printk("%s\n", line);
			}
			break;
		case APP_LOGIC_ACTION_HELLO:
			printk("hello specialized\n");
			break;
		case APP_LOGIC_ACTION_NONE:
			break;
		case APP_LOGIC_ACTION_INVALID_ARGS:
		default:
			/* Not expected on this path: rx_logic_state and message are
			 * always valid local objects. Logged rather than treated as
			 * fatal, since dropping one frame is not worth a panic.
			 */
			printk("app_logic_handle_message rejected its arguments\n");
			break;
		}
	}
}

int runtime_start_rx_printer(const struct app_config *config)
{
	/* Separate filters for standard and extended IDs: can_filter's IDE
	 * flag is matched exactly, not masked, so a single filter cannot
	 * catch-all across both ID kinds.
	 */
	const struct can_filter std_filter = {.id = 0, .mask = 0, .flags = 0};
	const struct can_filter ext_filter = {.id = 0, .mask = 0, .flags = CAN_FILTER_IDE};
	int ret;

	if (config == NULL) {
		return -EINVAL;
	}

	if (rx_printer_started) {
		return 0;
	}

	if (!device_is_ready(can_dev)) {
		return -ENODEV;
	}

	if (!app_logic_init(&rx_logic_state, &config->start_trigger, &config->stop_trigger,
			     &config->hello_trigger)) {
		return -EINVAL;
	}

	ret = can_add_rx_filter_msgq(can_dev, &rx_msgq, &std_filter);
	if (ret < 0) {
		return ret;
	}

	ret = can_add_rx_filter_msgq(can_dev, &rx_msgq, &ext_filter);
	if (ret < 0) {
		return ret;
	}

	k_thread_create(&rx_thread_data, rx_thread_stack, K_THREAD_STACK_SIZEOF(rx_thread_stack),
			rx_thread_entry, NULL, NULL, NULL, RX_THREAD_PRIORITY, 0, K_NO_WAIT);

	rx_printer_started = true;

	return 0;
}
