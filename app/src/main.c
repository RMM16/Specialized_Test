#include <zephyr/sys/printk.h>

int main(void)
{
	printk("Specialized Test booted on native_sim\n");

	return 0;
}
