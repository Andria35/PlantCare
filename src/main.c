#include <zephyr/kernel.h>

int main(void)
{
    while (1) {
        printk("Hello from my_zephyr_app!\n");
        k_sleep(K_SECONDS(1));
    }

    return 0;
}
