// src/sensors/button.h
#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>
#include <zephyr/kernel.h>

/* Initialize the user button GPIO.
 * Returns 0 on success, negative errno on error.
 */
int button_init(void);

/* Return current logical state of the button:
 *   true  -> pressed
 *   false -> not pressed
 *
 * This uses Zephyr's gpio_pin_get_dt(), so ACTIVE_LOW / ACTIVE_HIGH
 * from devicetree is handled for you.
 */
bool button_is_pressed(void);

/* Edge-detection helper:
 * Call this regularly from your main loop.
 * Returns true exactly once when a new "press" is detected
 * (transition from not pressed -> pressed).
 */
bool button_was_pressed(void);

#endif /* BUTTON_H */