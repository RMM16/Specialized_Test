#include <string.h>

#include <zephyr/ztest.h>

#include "specialized/can_formatter.h"

ZTEST_SUITE(can_formatter_tests, NULL, NULL, NULL, NULL, NULL);

ZTEST(can_formatter_tests, test_std_id_zero_length_payload)
{
	char buf[CAN_FORMATTER_LINE_LEN];
	size_t out_len = 0;
	struct app_can_message msg = {
		.id = 0x123,
		.extended_id = false,
		.dlc = 0,
		.timestamp_ms = 1000,
	};

	zassert_equal(can_formatter_format(&msg, buf, sizeof(buf), &out_len), CAN_FORMATTER_OK);
	zassert_equal(out_len, strlen(buf));
	zassert_not_null(strstr(buf, "ID=0x123 (STD)"));
	zassert_not_null(strstr(buf, "DLC=0"));
	zassert_not_null(strstr(buf, "DATA="));
}

ZTEST(can_formatter_tests, test_ext_id_full_payload)
{
	char buf[CAN_FORMATTER_LINE_LEN];
	struct app_can_message msg = {
		.id = 0x1FFFFFFF,
		.extended_id = true,
		.dlc = 8,
		.data = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88},
		.timestamp_ms = 1234567,
	};

	zassert_equal(can_formatter_format(&msg, buf, sizeof(buf), NULL), CAN_FORMATTER_OK);
	zassert_not_null(strstr(buf, "ID=0x1FFFFFFF (EXT)"));
	zassert_not_null(strstr(buf, "DLC=8"));
	zassert_not_null(strstr(buf, "DATA=11 22 33 44 55 66 77 88"));
}

ZTEST(can_formatter_tests, test_small_buffer_truncates_safely)
{
	char buf[8];
	size_t out_len = 0;
	struct app_can_message msg = {
		.id = 0x123,
		.extended_id = false,
		.dlc = 8,
		.data = {1, 2, 3, 4, 5, 6, 7, 8},
		.timestamp_ms = 1,
	};

	enum can_formatter_result result = can_formatter_format(&msg, buf, sizeof(buf), &out_len);

	zassert_equal(result, CAN_FORMATTER_ERR_BUFFER_TOO_SMALL);
	zassert_true(out_len >= sizeof(buf));
	zassert_equal(strlen(buf), sizeof(buf) - 1);
}

ZTEST(can_formatter_tests, test_rejects_invalid_dlc)
{
	char buf[CAN_FORMATTER_LINE_LEN];
	struct app_can_message msg = {
		.id = 0x1,
		.extended_id = false,
		.dlc = APP_CAN_MESSAGE_MAX_DLC + 1,
	};

	zassert_equal(can_formatter_format(&msg, buf, sizeof(buf), NULL),
		      CAN_FORMATTER_ERR_INVALID_DLC);
}

ZTEST(can_formatter_tests, test_rejects_null_message)
{
	char buf[CAN_FORMATTER_LINE_LEN];

	zassert_equal(can_formatter_format(NULL, buf, sizeof(buf), NULL),
		      CAN_FORMATTER_ERR_NULL_ARG);
}

ZTEST(can_formatter_tests, test_query_required_length)
{
	size_t out_len = 0;
	struct app_can_message msg = {
		.id = 0x1,
		.extended_id = false,
		.dlc = 0,
		.timestamp_ms = 1,
	};

	enum can_formatter_result result = can_formatter_format(&msg, NULL, 0, &out_len);

	zassert_equal(result, CAN_FORMATTER_ERR_BUFFER_TOO_SMALL);
	zassert_true(out_len > 0);
}
