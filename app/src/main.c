#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "specialized/app_config_model.h"
#include "specialized/zephyr_app_config.h"

int main(void)
{
	struct app_config config;

	printk("Specialized Test booted on %s\n", CONFIG_BOARD);

	zephyr_app_config_load(&config);

	if (app_config_validate(&config) != APP_CONFIG_OK) {
		printk("Invalid build-time configuration\n");
		k_panic();
	}

	printk("Build-time configuration validated\n");

	return 0;
}
