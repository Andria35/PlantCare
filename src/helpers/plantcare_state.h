#ifndef PLANTCARE_STATE_H
#define PLANTCARE_STATE_H

#include <zephyr/kernel.h>
#include <stdint.h>
#include <stdbool.h>

enum plantcare_dom_color {
    DOM_COLOR_UNKNOWN = 0,
    DOM_COLOR_RED,
    DOM_COLOR_GREEN,
    DOM_COLOR_BLUE,
};

struct plantcare_data {
    /* Soil + light (ADC) */
    int16_t soil_raw;
    int32_t soil_mv;
    int16_t light_raw;
    int32_t light_mv;

    /* Temp / humidity (Si7021) */
    int32_t temp_x100;   /* °C * 100 */
    int32_t hum_x100;    /* %RH * 100 */

    /* Accelerometer (MMA8451, g * 100) */
    int32_t acc_x_g100;
    int32_t acc_y_g100;
    int32_t acc_z_g100;

    /* Color sensor (TCS34725) */
    uint16_t clr;
    uint16_t red;
    uint16_t green;
    uint16_t blue;
    enum plantcare_dom_color dom_color;

    /* GPS: for Test Mode we just store the last NMEA sentence (truncated) */
    char gps_last_sentence[64];
};

/* Called by the worker thread after computing new values */
void plantcare_state_publish(const struct plantcare_data *src);

/* Called by the UI thread to get a stable snapshot */
void plantcare_state_get_snapshot(struct plantcare_data *dst);

#endif /* PLANTCARE_STATE_H */