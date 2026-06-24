#include <zephyr/sys/printk.h>

#include "specialized/app_config_model.h"
#include "specialized/zephyr_app_config.h"

int main(void)
{
	struct app_config config;

	printk("Specialized Test booted on native_sim\n");

	zephyr_app_config_load(&config);

	if (app_config_validate(&config) != APP_CONFIG_OK) {
		printk("Invalid build-time configuration, halting\n");
		return -1;
	}

	printk("Build-time configuration validated\n");

	return 0;
}
