#include <stdio.h>

#include "specialized/can_formatter.h"

static void format_payload(const struct app_can_message *message, char *data_str,
			    size_t data_str_size)
{
	size_t offset = 0;
	uint8_t dlc = message->dlc > APP_CAN_MESSAGE_MAX_DLC ? APP_CAN_MESSAGE_MAX_DLC
							      : message->dlc;

	data_str[0] = '\0';

	for (uint8_t i = 0; i < dlc; i++) {
		int written = snprintf(&data_str[offset], data_str_size - offset, "%s%02X",
					i == 0 ? "" : " ", message->data[i]);

		if (written < 0 || (size_t)written >= data_str_size - offset) {
			break;
		}
		offset += (size_t)written;
	}
}

int can_formatter_format(const struct app_can_message *message, char *buf, size_t buf_size)
{
	char data_str[3 * APP_CAN_MESSAGE_MAX_DLC];

	format_payload(message, data_str, sizeof(data_str));

	return snprintf(buf, buf_size, "[%u ms] ID=0x%X (%s) DLC=%u DATA=%s",
			 message->timestamp_ms, message->id,
			 message->extended_id ? "EXT" : "STD", message->dlc, data_str);
}
