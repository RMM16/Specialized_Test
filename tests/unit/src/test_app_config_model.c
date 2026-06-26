#include <zephyr/ztest.h>

#include "specialized/app_config_model.h"

static struct app_config valid_config(void)
{
	struct app_config config = {
		.can = {
			.bitrate = 500000,
			.sample_point_permille = 875,
			.loopback = true,
			.rx_queue_depth = 8,
		},
		.tx = {
			{.id = 0x100, .extended_id = false, .period_ms = 1000},
			{.id = 0x101, .extended_id = false, .period_ms = 500},
			{.id = 0x102, .extended_id = false, .period_ms = 250},
		},
		.start_trigger = {.enabled = false, .id = 0x200, .extended_id = false},
		.stop_trigger = {.enabled = false, .id = 0x201, .extended_id = false},
		.hello_trigger = {.enabled = false, .id = 0x202, .extended_id = false},
	};

	return config;
}

ZTEST_SUITE(app_config_model_tests, NULL, NULL, NULL, NULL, NULL);

ZTEST(app_config_model_tests, test_valid_defaults)
{
	struct app_config config = valid_config();

	zassert_equal(app_config_validate(&config), APP_CONFIG_OK);
}

ZTEST(app_config_model_tests, test_null_config_rejected)
{
	zassert_equal(app_config_validate(NULL), APP_CONFIG_ERR_NULL_CONFIG);
}

ZTEST(app_config_model_tests, test_rejects_zero_period)
{
	struct app_config config = valid_config();

	config.tx[0].period_ms = 0;

	zassert_equal(app_config_validate(&config), APP_CONFIG_ERR_ZERO_PERIOD);
}

ZTEST(app_config_model_tests, test_rejects_invalid_standard_id)
{
	struct app_config config = valid_config();

	config.tx[0].extended_id = false;
	config.tx[0].id = APP_CONFIG_MAX_STD_CAN_ID + 1;

	zassert_equal(app_config_validate(&config), APP_CONFIG_ERR_INVALID_ID);
}

ZTEST(app_config_model_tests, test_rejects_invalid_extended_id)
{
	struct app_config config = valid_config();

	config.tx[0].extended_id = true;
	config.tx[0].id = APP_CONFIG_MAX_EXT_CAN_ID + 1;

	zassert_equal(app_config_validate(&config), APP_CONFIG_ERR_INVALID_ID);
}

ZTEST(app_config_model_tests, test_rejects_overlapping_triggers)
{
	struct app_config config = valid_config();

	config.start_trigger.enabled = true;
	config.stop_trigger.enabled = true;
	config.stop_trigger.id = config.start_trigger.id;
	config.stop_trigger.extended_id = config.start_trigger.extended_id;

	zassert_equal(app_config_validate(&config), APP_CONFIG_ERR_TRIGGER_COLLISION);
}

ZTEST(app_config_model_tests, test_std_and_ext_same_value_dont_collide)
{
	struct app_config config = valid_config();

	config.start_trigger.enabled = true;
	config.start_trigger.id = 0x123;
	config.start_trigger.extended_id = false;
	config.stop_trigger.enabled = true;
	config.stop_trigger.id = 0x123;
	config.stop_trigger.extended_id = true;

	zassert_equal(app_config_validate(&config), APP_CONFIG_OK);
}

ZTEST(app_config_model_tests, test_rejects_tx_trigger_collision)
{
	struct app_config config = valid_config();

	config.start_trigger.enabled = true;
	config.start_trigger.id = config.tx[0].id;
	config.start_trigger.extended_id = config.tx[0].extended_id;

	zassert_equal(app_config_validate(&config), APP_CONFIG_ERR_TX_TRIGGER_COLLISION);
}

ZTEST(app_config_model_tests, test_disabled_triggers_dont_collide_with_tx)
{
	struct app_config config = valid_config();

	config.start_trigger.enabled = false;
	config.start_trigger.id = config.tx[0].id;
	config.stop_trigger.enabled = false;
	config.stop_trigger.id = config.tx[0].id;

	zassert_equal(app_config_validate(&config), APP_CONFIG_OK);
}

ZTEST(app_config_model_tests, test_rejects_invalid_can_bus)
{
	struct app_config config = valid_config();

	config.can.bitrate = APP_CONFIG_MIN_BITRATE - 1;

	zassert_equal(app_config_validate(&config), APP_CONFIG_ERR_INVALID_CAN_BUS);
}
