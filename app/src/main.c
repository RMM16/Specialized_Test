#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "specialized/app_config_model.h"
#include "specialized/zephyr_app_config.h"

int main(void)
{
	struct app_config config;
	enum app_config_validation_result result;

	printk("Specialized Test booted on %s\n", CONFIG_BOARD);

	zephyr_app_config_load(&config);

	result = app_config_validate(&config);
	if (result != APP_CONFIG_OK) {
		printk("Invalid build-time configuration: %s\n",
		       app_config_validation_result_to_str(result));
		k_panic();
	}

	printk("Build-time configuration validated\n");

	return 0;
}
