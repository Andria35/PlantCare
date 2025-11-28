// src/helpers/plantcare_units.c

#include <stdbool.h>
#include <stdint.h>
#include "plantcare_units.h"

/*
 * Calibration constants (RAW ADC values).
 *
 * Start simple:
 *  - Map full ADC range 0..4095 to 0.0..100.0 %
 *  - Then, after you see real readings in the terminal,
 *    you can narrow these ranges to better match your sensors.
 *
 * Later you can do, for example:
 *   - LIGHT_RAW_MIN = raw in dark
 *   - LIGHT_RAW_MAX = raw in very bright light
 *   - SOIL_RAW_MIN  = raw in very dry soil
 *   - SOIL_RAW_MAX  = raw in very wet soil
 */

#define LIGHT_RAW_MIN   0    
#define LIGHT_RAW_MAX   400 

#define SOIL_RAW_MIN    0       /* adjust later after measuring */
#define SOIL_RAW_MAX    4095    /* 12-bit ADC max */

/* Map [raw_min, raw_max] -> [0.0, 100.0] %, result scaled *10 */
static int32_t pct_x10_from_range(int32_t raw,
                                  int32_t raw_min,
                                  int32_t raw_max)
{
    /* Clamp into range */
    if (raw < raw_min) raw = raw_min;
    if (raw > raw_max) raw = raw_max;

    int32_t den = raw_max - raw_min;
    if (den <= 0) {
        /* Misconfiguration: avoid division by zero */
        return 0;
    }

    int32_t num = raw - raw_min;

    /* percentage * 10 = 1000 * num / den */
    return (int32_t)((int64_t)num * 1000 / den);
}

int32_t light_raw_to_pct_x10(int32_t raw)
{
    /* Assume higher RAW = more light */
    return pct_x10_from_range(raw, LIGHT_RAW_MIN, LIGHT_RAW_MAX);
}

int32_t soil_raw_to_pct_x10(int32_t raw)
{
    /* Assume higher RAW = "more" of whatever we call 100% (wet or dry).
     * If your sensor is reversed, you will later swap SOIL_RAW_MIN/MAX.
     */
    return pct_x10_from_range(raw, SOIL_RAW_MIN, SOIL_RAW_MAX);
}

// src/helpers/plantcare_units.c
#include <stdbool.h>
#include <stdint.h>
#include "plantcare_units.h"

/* ... existing LIGHT/ SOIL code ... */

int32_t accel_g100_to_ms2_x100(int32_t g100)
{
    /* 1 g ≈ 9.81 m/s^2
     * Input is g*100, output is (m/s^2)*100
     *
     * Example:
     *   g100 = 100   => 1.00 g  => ~9.81 m/s^2 => 981 in return value
     */
    return (g100 * 981) / 100;
}