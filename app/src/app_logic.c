#include "specialized/app_logic.h"

static bool message_matches_trigger(const struct app_can_message *message,
				     const struct app_trigger_config *trigger)
{
	return trigger->enabled && message->id == trigger->id &&
	       message->extended_id == trigger->extended_id;
}

void app_logic_init(struct app_logic_state *state, const struct app_trigger_config *start_trigger,
		     const struct app_trigger_config *stop_trigger,
		     const struct app_trigger_config *hello_trigger)
{
	state->start_trigger = *start_trigger;
	state->stop_trigger = *stop_trigger;
	state->hello_trigger = *hello_trigger;
	state->printing_enabled = !start_trigger->enabled;
}

enum app_logic_action app_logic_handle_message(struct app_logic_state *state,
						const struct app_can_message *message)
{
	if (message_matches_trigger(message, &state->hello_trigger)) {
		return APP_LOGIC_ACTION_HELLO;
	}

	if (message_matches_trigger(message, &state->start_trigger)) {
		state->printing_enabled = true;
		return APP_LOGIC_ACTION_NONE;
	}

	if (message_matches_trigger(message, &state->stop_trigger)) {
		state->printing_enabled = false;
		return APP_LOGIC_ACTION_NONE;
	}

	return state->printing_enabled ? APP_LOGIC_ACTION_PRINT : APP_LOGIC_ACTION_NONE;
}
