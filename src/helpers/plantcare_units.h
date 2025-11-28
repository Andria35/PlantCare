// src/helpers/plantcare_units.h
#ifndef PLANTCARE_UNITS_H
#define PLANTCARE_UNITS_H

#include <stdint.h>

/*
 * All functions return percentage * 10:
 *   0    =>  0.0 %
 *   123  => 12.3 %
 *   1000 => 100.0 %
 */
int32_t light_raw_to_pct_x10(int32_t raw);
int32_t soil_raw_to_pct_x10(int32_t raw);

/*
 * Accelerometer:
 *  Input:  g * 100   (e.g.  981  =>  9.81 g)
 *  Output: (m/s^2) * 100  (e.g. 981  =>  9.81 m/s^2)
 */
int32_t accel_g100_to_ms2_x100(int32_t g100);

#endif /* PLANTCARE_UNITS_H */