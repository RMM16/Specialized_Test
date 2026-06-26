#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "specialized/app_config_model.h"
#include "specialized/runtime.h"
#include "specialized/zephyr_app_config.h"

int main(void)
{
	struct app_config config;
	enum app_config_validation_result result;
	int ret;

	printk("Specialized Test booted on %s\n", CONFIG_BOARD);

	zephyr_app_config_load(&config);

	result = app_config_validate(&config);
	if (result != APP_CONFIG_OK) {
		printk("Invalid build-time configuration: %s\n",
		       app_config_validation_result_to_str(result));
		k_panic();
	}

	printk("Build-time configuration validated\n");

	ret = runtime_start_periodic_tx(&config);
	if (ret != 0) {
		printk("Failed to start periodic CAN TX: %d\n", ret);
		k_panic();
	}

	ret = runtime_start_rx_printer();
	if (ret != 0) {
		printk("Failed to start CAN RX printer: %d\n", ret);
		k_panic();
	}

	return 0;
}
