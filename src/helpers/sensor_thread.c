#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "plantcare_config.h"
#include "plantcare_state.h"

/* Sensors are under sensors/ */
#include "sensors/soil_sensor.h"
#include "sensors/light_sensor.h"
#include "sensors/humidity_sensor.h"
#include "sensors/accelerometer_sensor.h"
#include "sensors/rgb_sensor.h"
#include "sensors/gps_sensor.h"   /* uses gps_sensor_init + gps_sensor_read_char */

#define SENSOR_THREAD_STACK_SIZE 2048
#define SENSOR_THREAD_PRIORITY   5

/* Small helper to drain GPS UART and keep last NMEA line */
static void gps_update(struct plantcare_data *data)
{
    static char line[64];
    static size_t idx = 0;

    uint8_t ch;
    int ret;
    int iterations = 0;

    /* Try to read a limited number of chars, so we don't block forever */
    while (iterations < 200) {
        ret = gps_sensor_read_char(&ch);   /* your existing function */
        if (ret != 0) {
            break;  /* no more data right now */
        }

        iterations++;

        if (ch == '\r') {
            /* ignore CR, wait for '\n' */
            continue;
        }

        if (ch == '\n') {
            /* End of NMEA sentence */
            line[idx] = '\0';

            /* Copy into shared struct (truncated) */
            for (size_t i = 0; i < sizeof(data->gps_last_sentence); i++) {
                data->gps_last_sentence[i] = line[i];
                if (line[i] == '\0') break;
            }

            idx = 0;
        } else {
            /* Normal character: append to line buffer if space */
            if (idx < sizeof(line) - 1) {
                line[idx++] = (char)ch;
            } else {
                /* Overflow: reset and start over */
                idx = 0;
            }
        }
    }
}

static void sensor_thread_entry(void *p1, void *p2, void *p3)
{
    struct plantcare_data data;

    while (1) {
        /* Wait until main() says sensors are initialized */
        if (!g_sensors_ready) {
            k_msleep(100);
            continue;
        }

        /* Now it is safe to talk to sensors */

        data.gps_last_sentence[0] = '\0';
        /* --- Soil + light (ADC) --- */
        soil_sensor_read(&data.soil_raw,  &data.soil_mv);
        light_sensor_read(&data.light_raw, &data.light_mv);

        /* --- Temp / humidity (Si7021) --- */
        humidity_sensor_read(&data.hum_x100, &data.temp_x100);

        /* --- Accelerometer (MMA8451) --- */
        accelerometer_sensor_read(&data.acc_x_g100,
                                  &data.acc_y_g100,
                                  &data.acc_z_g100);

        /* --- Color sensor (TCS34725) --- */
        rgb_sensor_read(&data.clr, &data.red, &data.green, &data.blue);

        /* Compute dominant color */
        data.dom_color = DOM_COLOR_UNKNOWN;
        if (data.red >= data.green && data.red >= data.blue) {
            data.dom_color = DOM_COLOR_RED;
        } else if (data.green >= data.red && data.green >= data.blue) {
            data.dom_color = DOM_COLOR_GREEN;
        } else if (data.blue >= data.red && data.blue >= data.green) {
            data.dom_color = DOM_COLOR_BLUE;
        }

        /* --- GPS: update last NMEA sentence --- */
        gps_update(&data);

        /* If no sentence yet, ensure string is null-terminated */
        if (data.gps_last_sentence[0] == '\0') {
            data.gps_last_sentence[0] = '\0';
        }

        /* Publish to shared state */
        plantcare_state_publish(&data);

        /* Sleep according to current mode (2s Test, 30s Normal later) */
        k_msleep(g_sampling_period_ms);
    }
}

/* Define and auto-start the background thread */
K_THREAD_DEFINE(sensor_thread_id,
                SENSOR_THREAD_STACK_SIZE,
                sensor_thread_entry,
                NULL, NULL, NULL,
                SENSOR_THREAD_PRIORITY, 0, 0);