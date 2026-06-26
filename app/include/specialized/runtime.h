#ifndef SPECIALIZED_RUNTIME_H_
#define SPECIALIZED_RUNTIME_H_

#include "specialized/app_config_model.h"

/* Brings up the CAN device and starts the three periodic TX timers
 * described by config->tx[], each sending a random payload at its
 * configured period. Returns 0 on success, a negative errno on failure.
 */
int runtime_start_periodic_tx(const struct app_config *config);

#endif /* SPECIALIZED_RUNTIME_H_ */
