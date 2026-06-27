#ifndef SPECIALIZED_RUNTIME_H_
#define SPECIALIZED_RUNTIME_H_

#include "specialized/app_config_model.h"

/* Brings up the CAN device and starts the three periodic TX timers
 * described by config->tx[], each sending a random payload at its
 * configured period. Returns 0 on success, a negative errno on failure.
 */
int runtime_start_periodic_tx(const struct app_config *config);

/* Starts a consumer thread that receives every CAN frame (loopback of our
 * own TX for now), converts it to the portable app_can_message and routes
 * it through app_logic_handle_message() using the start/stop/hello triggers
 * from config. Prints the formatted frame, a hello greeting, or nothing,
 * depending on the action app_logic returns. Returns 0 on success, a
 * negative errno on failure.
 */
int runtime_start_rx_printer(const struct app_config *config);

/* Only does something when CONFIG_APP_SMOKE_TEST_INJECT_TRIGGERS=y: sends
 * the enabled start/hello/stop trigger IDs once each, a fixed delay apart,
 * over the loopback CAN bus. This is the only way to exercise the full
 * trigger lifecycle on native_sim without a second bus peer; it is a no-op
 * otherwise. Returns 0 on success, a negative errno on failure.
 */
int runtime_start_smoke_injector(const struct app_config *config);

#endif /* SPECIALIZED_RUNTIME_H_ */
