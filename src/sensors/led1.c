#include "led1.h"

#define LED1_NODE DT_ALIAS(led0)

BUILD_ASSERT(DT_NODE_HAS_STATUS(LED1_NODE, okay),
             "LED1 alias (led0) is missing or disabled in devicetree");

static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);

int led1_init(void)
{
    if (!device_is_ready(led1.port)) {
        printk("led1: GPIO device not ready\n");
        return -ENODEV;
    }

    /* Configure as output, start inactive (LED off). */
    int ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
    if (ret != 0) {
        printk("led1: gpio_pin_configure_dt failed, err=%d\n", ret);
    }
    return ret;
}

void led1_set(bool on)
{
    /* Logical on/off – Zephyr handles active-high / active-low for us. */
    gpio_pin_set_dt(&led1, on ? 1 : 0);
}

void led1_toggle(void)
{
    gpio_pin_toggle_dt(&led1);
}