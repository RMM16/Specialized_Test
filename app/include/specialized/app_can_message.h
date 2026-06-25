#ifndef SPECIALIZED_APP_CAN_MESSAGE_H_
#define SPECIALIZED_APP_CAN_MESSAGE_H_

#include <stdbool.h>
#include <stdint.h>

#define APP_CAN_MESSAGE_MAX_DLC 8

/* Portable representation of a received classic CAN frame, decoupled from
 * Zephyr's struct can_frame so app_logic and can_formatter stay testable on
 * a PC. runtime.c is responsible for filling this in from the real frame.
 */
struct app_can_message {
	uint32_t id;
	bool extended_id;
	uint8_t dlc;
	uint8_t data[APP_CAN_MESSAGE_MAX_DLC];
	uint64_t timestamp_ms;
};

#endif /* SPECIALIZED_APP_CAN_MESSAGE_H_ */
