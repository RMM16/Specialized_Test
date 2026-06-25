#ifndef SPECIALIZED_APP_LOGIC_H_
#define SPECIALIZED_APP_LOGIC_H_

#include <stdbool.h>

#include "specialized/app_can_message.h"
#include "specialized/app_config_model.h"

enum app_logic_action {
	APP_LOGIC_ACTION_NONE = 0,
	APP_LOGIC_ACTION_PRINT,
	APP_LOGIC_ACTION_HELLO,
};

struct app_logic_state {
	bool printing_enabled;
	struct app_trigger_config start_trigger;
	struct app_trigger_config stop_trigger;
	struct app_trigger_config hello_trigger;
};

/* Sets the initial printing_enabled state: enabled from boot unless a start
 * trigger is configured, in which case printing stays blocked until that
 * trigger ID is received.
 */
void app_logic_init(struct app_logic_state *state, const struct app_trigger_config *start_trigger,
		     const struct app_trigger_config *stop_trigger,
		     const struct app_trigger_config *hello_trigger);

enum app_logic_action app_logic_handle_message(struct app_logic_state *state,
						const struct app_can_message *message);

#endif /* SPECIALIZED_APP_LOGIC_H_ */
