#include "plantcare_config.h"

/* Default: 2000 ms = 2 seconds (Test Mode). Normal Mode will set 30000 later. */
volatile uint32_t g_sampling_period_ms = 2000;
volatile bool g_sensors_ready = false;