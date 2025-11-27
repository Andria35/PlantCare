#ifndef LED1_H
#define LED1_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdbool.h>

/* Initialize LED1 (built-in board LED). Must be called once at startup. */
int led1_init(void);

/* Turn LED1 on (true) or off (false). */
void led1_set(bool on);

/* Toggle LED1 state. */
void led1_toggle(void);

#endif /* LED1_H */