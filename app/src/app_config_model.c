#include <stddef.h>

#include "specialized/app_config_model.h"

#define APP_CONFIG_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

static bool id_in_range(uint32_t id, bool extended_id)
{
	uint32_t max_id = extended_id ? APP_CONFIG_MAX_EXT_CAN_ID : APP_CONFIG_MAX_STD_CAN_ID;

	return id <= max_id;
}

static bool can_bus_valid(const struct app_can_bus_config *can)
{
	if (can->bitrate < APP_CONFIG_MIN_BITRATE || can->bitrate > APP_CONFIG_MAX_BITRATE) {
		return false;
	}

	if (can->sample_point_permille < APP_CONFIG_MIN_SAMPLE_POINT_PERMILLE ||
	    can->sample_point_permille > APP_CONFIG_MAX_SAMPLE_POINT_PERMILLE) {
		return false;
	}

	if (can->rx_queue_depth < APP_CONFIG_MIN_RX_QUEUE_DEPTH ||
	    can->rx_queue_depth > APP_CONFIG_MAX_RX_QUEUE_DEPTH) {
		return false;
	}

	return true;
}

/* A standard ID and an extended ID that share the same numeric value are
 * distinct CAN identifiers, so the extended flag is part of identity here.
 */
static bool same_can_id(uint32_t id_a, bool extended_a, uint32_t id_b, bool extended_b)
{
	return id_a == id_b && extended_a == extended_b;
}

static bool triggers_collide(const struct app_trigger_config *a, const struct app_trigger_config *b)
{
	return a->enabled && b->enabled && same_can_id(a->id, a->extended_id, b->id, b->extended_id);
}

static bool tx_collides_with_trigger(const struct app_tx_message_config *tx,
				      const struct app_trigger_config *trigger)
{
	return trigger->enabled &&
	       same_can_id(tx->id, tx->extended_id, trigger->id, trigger->extended_id);
}

enum app_config_validation_result app_config_validate(const struct app_config *config)
{
	if (config == NULL) {
		return APP_CONFIG_ERR_NULL_CONFIG;
	}

	if (!can_bus_valid(&config->can)) {
		return APP_CONFIG_ERR_INVALID_CAN_BUS;
	}

	const struct app_trigger_config *triggers[] = {
		&config->start_trigger,
		&config->stop_trigger,
		&config->hello_trigger,
	};

	for (size_t i = 0; i < APP_CONFIG_TX_COUNT; i++) {
		const struct app_tx_message_config *tx = &config->tx[i];

		if (tx->period_ms == 0) {
			return APP_CONFIG_ERR_ZERO_PERIOD;
		}

		if (!id_in_range(tx->id, tx->extended_id)) {
			return APP_CONFIG_ERR_INVALID_ID;
		}
	}

	for (size_t i = 0; i < APP_CONFIG_ARRAY_SIZE(triggers); i++) {
		if (triggers[i]->enabled && !id_in_range(triggers[i]->id, triggers[i]->extended_id)) {
			return APP_CONFIG_ERR_INVALID_ID;
		}
	}

	if (triggers_collide(&config->start_trigger, &config->stop_trigger) ||
	    triggers_collide(&config->start_trigger, &config->hello_trigger) ||
	    triggers_collide(&config->stop_trigger, &config->hello_trigger)) {
		return APP_CONFIG_ERR_TRIGGER_COLLISION;
	}

	for (size_t i = 0; i < APP_CONFIG_TX_COUNT; i++) {
		const struct app_tx_message_config *tx = &config->tx[i];

		for (size_t j = 0; j < APP_CONFIG_ARRAY_SIZE(triggers); j++) {
			if (tx_collides_with_trigger(tx, triggers[j])) {
				return APP_CONFIG_ERR_TX_TRIGGER_COLLISION;
			}
		}
	}

	return APP_CONFIG_OK;
}
