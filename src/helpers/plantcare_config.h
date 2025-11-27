#ifndef PLANTCARE_CONFIG_H
#define PLANTCARE_CONFIG_H

#include <stdint.h>
#include <stdbool.h>      // <--- add this

/* Sampling period used by the sensor background thread (milliseconds) */
extern volatile uint32_t g_sampling_period_ms;
extern volatile bool g_sensors_ready;

#endif /* PLANTCARE_CONFIG_H */