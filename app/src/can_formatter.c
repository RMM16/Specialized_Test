#include <inttypes.h>
#include <stdio.h>

#include "specialized/can_formatter.h"

static void format_payload(const struct app_can_message *message, char *data_str,
			    size_t data_str_size)
{
	size_t offset = 0;

	data_str[0] = '\0';

	for (uint8_t i = 0; i < message->dlc; i++) {
		int written = snprintf(&data_str[offset], data_str_size - offset, "%s%02X",
					i == 0 ? "" : " ", message->data[i]);

		if (written < 0 || (size_t)written >= data_str_size - offset) {
			break;
		}
		offset += (size_t)written;
	}
}

enum can_formatter_result can_formatter_format(const struct app_can_message *message, char *buf,
						size_t buf_size, size_t *out_len)
{
	char data_str[3 * APP_CAN_MESSAGE_MAX_DLC];
	int written;

	if (message == NULL || (buf == NULL && buf_size > 0)) {
		return CAN_FORMATTER_ERR_NULL_ARG;
	}

	if (message->dlc > APP_CAN_MESSAGE_MAX_DLC) {
		return CAN_FORMATTER_ERR_INVALID_DLC;
	}

	format_payload(message, data_str, sizeof(data_str));

	written = snprintf(buf, buf_size, "[%" PRIu64 " ms] ID=0x%X (%s) DLC=%u DATA=%s",
			    message->timestamp_ms, message->id,
			    message->extended_id ? "EXT" : "STD", message->dlc, data_str);

	if (written < 0) {
		return CAN_FORMATTER_ERR_FORMAT_FAILED;
	}

	if (out_len != NULL) {
		*out_len = (size_t)written;
	}

	if ((size_t)written >= buf_size) {
		return CAN_FORMATTER_ERR_BUFFER_TOO_SMALL;
	}

	return CAN_FORMATTER_OK;
}
