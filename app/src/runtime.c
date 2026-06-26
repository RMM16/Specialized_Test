#include <errno.h>
#include <stddef.h>

#include <zephyr/device.h>
#include <zephyr/drivers/can.h>
#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/printk.h>

#include "specialized/app_can_message.h"
#include "specialized/runtime.h"

static const struct device *const can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));

struct tx_context {
	struct k_work_delayable work;
	struct can_frame frame;
	uint32_t period_ms;
};

static struct tx_context tx_contexts[APP_CONFIG_TX_COUNT];

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

	return 0;
}
