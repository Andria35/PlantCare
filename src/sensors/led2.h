#ifndef LED2_H
#define LED2_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdbool.h>

/* Initialize LED2 (built-in board LED). Must be called once at startup. */
int led2_init(void);

/* Turn LED2 on (true) or off (false). */
void led2_set(bool on);

/* Toggle LED2 state. */
void led2_toggle(void);

#endif /* LED2_H */