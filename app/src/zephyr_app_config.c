#include <zephyr/sys/util.h>

#include "specialized/zephyr_app_config.h"

void zephyr_app_config_load(struct app_config *config)
{
	config->can.bitrate = CONFIG_APP_CAN_BITRATE;
	config->can.sample_point_permille = CONFIG_APP_CAN_SAMPLE_POINT_PERMILLE;
	config->can.loopback = IS_ENABLED(CONFIG_APP_CAN_LOOPBACK);
	config->can.rx_queue_depth = CONFIG_APP_CAN_RX_QUEUE_DEPTH;

	config->tx[0].id = CONFIG_APP_TX1_ID;
	config->tx[0].extended_id = IS_ENABLED(CONFIG_APP_TX1_EXTENDED_ID);
	config->tx[0].period_ms = CONFIG_APP_TX1_PERIOD_MS;

	config->tx[1].id = CONFIG_APP_TX2_ID;
	config->tx[1].extended_id = IS_ENABLED(CONFIG_APP_TX2_EXTENDED_ID);
	config->tx[1].period_ms = CONFIG_APP_TX2_PERIOD_MS;

	config->tx[2].id = CONFIG_APP_TX3_ID;
	config->tx[2].extended_id = IS_ENABLED(CONFIG_APP_TX3_EXTENDED_ID);
	config->tx[2].period_ms = CONFIG_APP_TX3_PERIOD_MS;

	config->start_trigger.enabled = IS_ENABLED(CONFIG_APP_START_TRIGGER_ENABLE);
	config->start_trigger.id = CONFIG_APP_START_TRIGGER_ID;
	config->start_trigger.extended_id = IS_ENABLED(CONFIG_APP_START_TRIGGER_EXTENDED_ID);

	config->stop_trigger.enabled = IS_ENABLED(CONFIG_APP_STOP_TRIGGER_ENABLE);
	config->stop_trigger.id = CONFIG_APP_STOP_TRIGGER_ID;
	config->stop_trigger.extended_id = IS_ENABLED(CONFIG_APP_STOP_TRIGGER_EXTENDED_ID);

	config->hello_trigger.enabled = IS_ENABLED(CONFIG_APP_HELLO_TRIGGER_ENABLE);
	config->hello_trigger.id = CONFIG_APP_HELLO_TRIGGER_ID;
	config->hello_trigger.extended_id = IS_ENABLED(CONFIG_APP_HELLO_TRIGGER_EXTENDED_ID);
}
