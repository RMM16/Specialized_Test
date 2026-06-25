#ifndef SPECIALIZED_CAN_FORMATTER_H_
#define SPECIALIZED_CAN_FORMATTER_H_

#include <stddef.h>

#include "specialized/app_can_message.h"

/* Long enough for the worst case: timestamp, extended ID, DLC and 8 hex
 * payload bytes with separators.
 */
#define CAN_FORMATTER_LINE_LEN 80

/* Formats message into buf (always NUL-terminated if buf_size > 0), the
 * same way snprintf() does. Returns the number of characters that would
 * have been written excluding the terminating NUL, even if buf was too
 * small to hold them all, so callers can detect truncation.
 */
int can_formatter_format(const struct app_can_message *message, char *buf, size_t buf_size);

#endif /* SPECIALIZED_CAN_FORMATTER_H_ */
