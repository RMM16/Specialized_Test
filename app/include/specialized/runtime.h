#ifndef SPECIALIZED_RUNTIME_H_
#define SPECIALIZED_RUNTIME_H_

#include "specialized/app_config_model.h"

/* Brings up the CAN device and starts the three periodic TX timers
 * described by config->tx[], each sending a random payload at its
 * configured period. Returns 0 on success, a negative errno on failure.
 */
int runtime_start_periodic_tx(const struct app_config *config);

/* Starts a consumer thread that receives every CAN frame (loopback of our
 * own TX for now), converts it to the portable app_can_message and prints
 * it formatted via can_formatter. Does not yet apply app_logic's
 * start/stop/hello decisions; that lands in Branch 8. Returns 0 on success,
 * a negative errno on failure.
 */
int runtime_start_rx_printer(void);

#endif /* SPECIALIZED_RUNTIME_H_ */
