#ifndef PLANTCARE_CONFIG_H
#define PLANTCARE_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

/* Sampling period for background sensor thread (ms) */
extern volatile uint32_t g_sampling_period_ms;

/* Sensors ready flag */
extern volatile bool g_sensors_ready;

/* ---- Mode enum ---- */
typedef enum {
    PLANTCARE_MODE_TEST = 0,
    PLANTCARE_MODE_NORMAL = 1,
} plantcare_mode_t;

extern volatile plantcare_mode_t g_current_mode;

/* ---- Button event flag (set from ISR) ---- */
extern volatile bool g_button_pressed_event;

#endif /* PLANTCARE_CONFIG_H */