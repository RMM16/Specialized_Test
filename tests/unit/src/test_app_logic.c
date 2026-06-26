#include <zephyr/ztest.h>

#include "specialized/app_logic.h"

static const struct app_trigger_config disabled_trigger = {
	.enabled = false,
	.id = 0,
	.extended_id = false,
};

ZTEST_SUITE(app_logic_tests, NULL, NULL, NULL, NULL, NULL);

ZTEST(app_logic_tests, test_starts_enabled_without_start_trigger)
{
	struct app_logic_state state;

	zassert_true(
		app_logic_init(&state, &disabled_trigger, &disabled_trigger, &disabled_trigger));
	zassert_true(state.printing_enabled);
}

ZTEST(app_logic_tests, test_starts_disabled_with_start_trigger)
{
	struct app_logic_state state;
	struct app_trigger_config start_trigger = {
		.enabled = true, .id = 0x200, .extended_id = false};

	app_logic_init(&state, &start_trigger, &disabled_trigger, &disabled_trigger);

	zassert_false(state.printing_enabled);
}

ZTEST(app_logic_tests, test_init_rejects_null)
{
	struct app_logic_state state;

	zassert_false(
		app_logic_init(NULL, &disabled_trigger, &disabled_trigger, &disabled_trigger));
	zassert_false(app_logic_init(&state, NULL, &disabled_trigger, &disabled_trigger));
}

ZTEST(app_logic_tests, test_start_trigger_enables_printing)
{
	struct app_logic_state state;
	struct app_trigger_config start_trigger = {
		.enabled = true, .id = 0x200, .extended_id = false};
	struct app_can_message msg = {.id = 0x200, .extended_id = false, .dlc = 0};

	app_logic_init(&state, &start_trigger, &disabled_trigger, &disabled_trigger);
	zassert_false(state.printing_enabled);

	zassert_equal(app_logic_handle_message(&state, &msg), APP_LOGIC_ACTION_NONE);
	zassert_true(state.printing_enabled);
}

ZTEST(app_logic_tests, test_stop_trigger_disables_printing)
{
	struct app_logic_state state;
	struct app_trigger_config stop_trigger = {
		.enabled = true, .id = 0x201, .extended_id = false};
	struct app_can_message msg = {.id = 0x201, .extended_id = false, .dlc = 0};

	app_logic_init(&state, &disabled_trigger, &stop_trigger, &disabled_trigger);
	zassert_true(state.printing_enabled);

	zassert_equal(app_logic_handle_message(&state, &msg), APP_LOGIC_ACTION_NONE);
	zassert_false(state.printing_enabled);
}

ZTEST(app_logic_tests, test_hello_trigger_sets_action_without_changing_state)
{
	struct app_logic_state state;
	struct app_trigger_config hello_trigger = {
		.enabled = true, .id = 0x202, .extended_id = false};
	struct app_can_message msg = {.id = 0x202, .extended_id = false, .dlc = 0};

	app_logic_init(&state, &disabled_trigger, &disabled_trigger, &hello_trigger);

	zassert_equal(app_logic_handle_message(&state, &msg), APP_LOGIC_ACTION_HELLO);
	zassert_true(state.printing_enabled);
}

ZTEST(app_logic_tests, test_normal_message_prints_when_enabled)
{
	struct app_logic_state state;
	struct app_can_message msg = {.id = 0x300, .extended_id = false, .dlc = 0};

	app_logic_init(&state, &disabled_trigger, &disabled_trigger, &disabled_trigger);

	zassert_equal(app_logic_handle_message(&state, &msg), APP_LOGIC_ACTION_PRINT);
}

ZTEST(app_logic_tests, test_normal_message_skips_when_disabled)
{
	struct app_logic_state state;
	struct app_trigger_config start_trigger = {
		.enabled = true, .id = 0x200, .extended_id = false};
	struct app_can_message msg = {.id = 0x300, .extended_id = false, .dlc = 0};

	app_logic_init(&state, &start_trigger, &disabled_trigger, &disabled_trigger);

	zassert_equal(app_logic_handle_message(&state, &msg), APP_LOGIC_ACTION_NONE);
}

ZTEST(app_logic_tests, test_handle_message_rejects_null)
{
	struct app_logic_state state;

	app_logic_init(&state, &disabled_trigger, &disabled_trigger, &disabled_trigger);

	zassert_equal(app_logic_handle_message(&state, NULL), APP_LOGIC_ACTION_INVALID_ARGS);
	zassert_equal(app_logic_handle_message(NULL, NULL), APP_LOGIC_ACTION_INVALID_ARGS);
}

ZTEST(app_logic_tests, test_std_and_ext_same_id_dont_match_trigger)
{
	struct app_logic_state state;
	struct app_trigger_config stop_trigger = {
		.enabled = true, .id = 0x201, .extended_id = false};
	struct app_can_message msg = {.id = 0x201, .extended_id = true, .dlc = 0};

	app_logic_init(&state, &disabled_trigger, &stop_trigger, &disabled_trigger);
	zassert_true(state.printing_enabled);

	zassert_equal(app_logic_handle_message(&state, &msg), APP_LOGIC_ACTION_PRINT);
	zassert_true(state.printing_enabled);
}
