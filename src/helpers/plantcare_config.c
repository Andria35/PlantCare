#include <stdbool.h>
#include <stdint.h>
#include "plantcare_config.h"

/* Default values */
volatile bool g_sensors_ready        = false;
volatile uint32_t g_sampling_period_ms = 2000;

/* Start in TEST mode by default */
volatile plantcare_mode_t g_current_mode = PLANTCARE_MODE_TEST;

/* Button event flag, set by button ISR */
volatile bool g_button_pressed_event = false;