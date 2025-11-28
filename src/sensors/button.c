// src/sensors/button.c

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "button.h"
#include "plantcare_config.h"

/*
 * We use the board's user button alias "sw0".
 * On most Nucleo boards, devicetree has:
 *   aliases { sw0 = &user_button; };
 */

#define BUTTON_NODE DT_ALIAS(sw0)

BUILD_ASSERT(DT_NODE_HAS_STATUS(BUTTON_NODE, okay),
             "sw0 alias (user button) not found in devicetree");

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
static struct gpio_callback button_cb_data;

/* ISR: called on button edge */
static void button_isr(const struct device *dev,
                       struct gpio_callback *cb,
                       uint32_t pins)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    /* Just set a global event flag, keep ISR tiny. */
    g_button_pressed_event = true;
}

int button_init(void)
{
    if (!device_is_ready(button.port)) {
        printk("button: GPIO device not ready\n");
        return -ENODEV;
    }

    int ret;

    /* Configure pin as input. ACTIVE_LOW/HIGH is handled by DT. */
    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret != 0) {
        printk("button: gpio_pin_configure_dt failed, err=%d\n", ret);
        return ret;
    }

    /* Configure interrupt on edge-to-active.
     * If the button is ACTIVE_LOW, "active" means low level.
     */
    ret = gpio_pin_interrupt_configure_dt(&button,
                                          GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
        printk("button: interrupt config failed, err=%d\n", ret);
        return ret;
    }

    /* Register callback */
    gpio_init_callback(&button_cb_data, button_isr, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);

    printk("button: init OK (interrupt mode)\n");
    return 0;
}

bool button_is_pressed(void)
{
    int level = gpio_pin_get_dt(&button);
    if (level < 0) {
        return false;
    }

    return (level != 0);
}