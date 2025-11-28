#include "led2.h"

#define LED2_NODE DT_ALIAS(led1)

BUILD_ASSERT(DT_NODE_HAS_STATUS(LED2_NODE, okay),
             "LED2 alias (led1) is missing or disabled in devicetree");

static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

int led2_init(void)
{
    if (!device_is_ready(led2.port)) {
        printk("led2: GPIO device not ready\n");
        return -ENODEV;
    }

    /* Configure as output, start inactive (LED off). */
    int ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
    if (ret != 0) {
        printk("led2: gpio_pin_configure_dt failed, err=%d\n", ret);
    }
    return ret;
}

void led2_set(bool on)
{
    /* Logical on/off – Zephyr handles active-high / active-low for us. */
    gpio_pin_set_dt(&led2, on ? 1 : 0);
}

void led2_toggle(void)
{
    gpio_pin_toggle_dt(&led2);
}