#ifndef SPECIALIZED_CAN_FORMATTER_H_
#define SPECIALIZED_CAN_FORMATTER_H_

#include <stddef.h>

#include "specialized/app_can_message.h"

/* Worst case: 20-digit uint64 timestamp, extended ID, DLC and 8 hex payload
 * bytes with separators, plus the terminating NUL.
 */
#define CAN_FORMATTER_LINE_LEN 96

enum can_formatter_result {
	CAN_FORMATTER_OK = 0,
	CAN_FORMATTER_ERR_NULL_ARG,
	CAN_FORMATTER_ERR_INVALID_DLC,
	CAN_FORMATTER_ERR_BUFFER_TOO_SMALL,
};

/* Formats message into buf, NUL-terminated on CAN_FORMATTER_OK.
 *
 * out_len, if non-NULL, receives the number of characters the formatted
 * line occupies excluding the NUL: written into buf on
 * CAN_FORMATTER_OK, or the length that would have been required on
 * CAN_FORMATTER_ERR_BUFFER_TOO_SMALL (buf is left truncated, like
 * snprintf()), so callers can size a retry buffer.
 *
 * buf may be NULL only if buf_size is 0, to only query the required length.
 */
enum can_formatter_result can_formatter_format(const struct app_can_message *message, char *buf,
						size_t buf_size, size_t *out_len);

#endif /* SPECIALIZED_CAN_FORMATTER_H_ */
