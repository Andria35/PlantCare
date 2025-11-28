// src/sensors/button.c

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "button.h"

/*
 * We use the board's user button alias "sw0".
 * Most Zephyr board DTS files for Nucleo have:
 *   aliases { sw0 = &user_button; };
 *
 * If your board uses a different alias, change "sw0" below
 * (e.g. to DT_ALIAS(user_button)).
 */

#define BUTTON_NODE DT_ALIAS(sw0)

BUILD_ASSERT(DT_NODE_HAS_STATUS(BUTTON_NODE, okay),
             "sw0 alias (user button) not found in devicetree");

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

/* For edge detection */
static bool last_state = false;
static bool initialized = false;

int button_init(void)
{
    if (!device_is_ready(button.port)) {
        printk("button: GPIO device not ready\n");
        return -ENODEV;
    }

    int ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret != 0) {
        printk("button: gpio_pin_configure_dt failed, err=%d\n", ret);
        return ret;
    }

    int level = gpio_pin_get_dt(&button);
    if (level < 0) {
        level = 0;
    }
    last_state = (level != 0);
    initialized = true;

    printk("button: init OK (initial state: %s)\n",
           last_state ? "pressed" : "released");
    return 0;
}

bool button_is_pressed(void)
{
    if (!initialized) {
        return false;
    }

    int level = gpio_pin_get_dt(&button);
    if (level < 0) {
        return false;
    }

    return (level != 0);
}

bool button_was_pressed(void)
{
    if (!initialized) {
        return false;
    }

    int level = gpio_pin_get_dt(&button);
    if (level < 0) {
        return false;
    }

    bool curr = (level != 0);
    bool pressed_event = (!last_state && curr);  // rising edge
    last_state = curr;
    return pressed_event;
}