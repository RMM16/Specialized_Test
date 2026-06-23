#ifndef SPECIALIZED_APP_CONFIG_MODEL_H_
#define SPECIALIZED_APP_CONFIG_MODEL_H_

#include <stdbool.h>
#include <stdint.h>

#define APP_CONFIG_TX_COUNT 3
#define APP_CONFIG_MAX_CAN_ID 0x1FFFFFFFu

struct app_can_bus_config {
	uint32_t bitrate;
	uint16_t sample_point_permille;
	bool loopback;
	uint16_t rx_queue_depth;
};

struct app_tx_message_config {
	uint32_t id;
	uint32_t period_ms;
};

struct app_trigger_config {
	bool enabled;
	uint32_t id;
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
	APP_CONFIG_ERR_ZERO_PERIOD,
	APP_CONFIG_ERR_INVALID_ID,
	APP_CONFIG_ERR_TRIGGER_COLLISION,
	APP_CONFIG_ERR_TX_TRIGGER_COLLISION,
};

enum app_config_validation_result app_config_validate(const struct app_config *config);

#endif /* SPECIALIZED_APP_CONFIG_MODEL_H_ */
