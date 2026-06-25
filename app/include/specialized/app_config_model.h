#ifndef SPECIALIZED_APP_CONFIG_MODEL_H_
#define SPECIALIZED_APP_CONFIG_MODEL_H_

#include <stdbool.h>
#include <stdint.h>

#define APP_CONFIG_TX_COUNT 3
#define APP_CONFIG_MAX_STD_CAN_ID 0x7FFu
#define APP_CONFIG_MAX_EXT_CAN_ID 0x1FFFFFFFu

/* Mirrors the ranges enforced in app/Kconfig, since this model can also be
 * built directly (e.g. by tests) without going through Kconfig.
 */
#define APP_CONFIG_MIN_BITRATE 10000u
#define APP_CONFIG_MAX_BITRATE 1000000u
#define APP_CONFIG_MIN_SAMPLE_POINT_PERMILLE 500u
#define APP_CONFIG_MAX_SAMPLE_POINT_PERMILLE 950u
#define APP_CONFIG_MIN_RX_QUEUE_DEPTH 1u
#define APP_CONFIG_MAX_RX_QUEUE_DEPTH 64u

struct app_can_bus_config {
	uint32_t bitrate;
	uint16_t sample_point_permille;
	bool loopback;
	uint16_t rx_queue_depth;
};

struct app_tx_message_config {
	uint32_t id;
	bool extended_id;
	uint32_t period_ms;
};

struct app_trigger_config {
	bool enabled;
	uint32_t id;
	bool extended_id;
};

struct app_config {
	struct app_can_bus_config can;
	struct app_tx_message_config tx[APP_CONFIG_TX_COUNT];
	struct app_trigger_config start_trigger;
	struct app_trigger_config stop_trigger;
	struct app_trigger_config hello_trigger;
};

enum app_config_validation_result {
	APP_CONFIG_OK = 0,
	APP_CONFIG_ERR_NULL_CONFIG,
	APP_CONFIG_ERR_INVALID_CAN_BUS,
	APP_CONFIG_ERR_ZERO_PERIOD,
	APP_CONFIG_ERR_INVALID_ID,
	APP_CONFIG_ERR_TRIGGER_COLLISION,
	APP_CONFIG_ERR_TX_TRIGGER_COLLISION,
};

enum app_config_validation_result app_config_validate(const struct app_config *config);

const char *app_config_validation_result_to_str(enum app_config_validation_result result);

#endif /* SPECIALIZED_APP_CONFIG_MODEL_H_ */
