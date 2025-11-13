#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <stdlib.h>   // for abs()
#include <stdint.h>

#include "sensors/rgb_sensor.h"
#include "sensors/humidity_sensor.h"
#include "sensors/accelerometer_sensor.h"
#include "sensors/light_sensor.h"   // <-- NEW

void main(void)
{
    printk("=== PlantCare: sensor demo ===\n");

    rgb_sensor_init();
    humidity_sensor_init();
    accelerometer_sensor_init();
    light_sensor_init();   // <-- NEW

    while (1) {
        uint16_t c, r, g, b;
        int32_t rh_x100, temp_x100;
        int32_t ax_x100, ay_x100, az_x100;
        int16_t light_raw;     // <-- NEW
        int32_t light_mv;      // <-- NEW

        printk("\n--- Sensor sample ---\n");

        /* RGB sensor (integers only) */
        if (rgb_sensor_read(&c, &r, &g, &b) == 0) {
            printk("RGB: C=%u R=%u G=%u B=%u\n", c, r, g, b);
        }

        /* Humidity + temperature: values are in x100 units */
        if (humidity_sensor_read(&rh_x100, &temp_x100) == 0) {

            /* Clamp RH to 0–100% just in case */
            if (rh_x100 < 0) {
                rh_x100 = 0;
            } else if (rh_x100 > 10000) {
                rh_x100 = 10000;
            }

            int rh_int   = (int)(rh_x100 / 100);
            int rh_frac  = (int)(abs(rh_x100 % 100));

            int t_int    = (int)(temp_x100 / 100);
            int t_frac   = (int)(abs(temp_x100 % 100));

            printk("Humidity: %d.%02d %%  Temp: %d.%02d C\n",
                   rh_int, rh_frac,
                   t_int,  t_frac);
        }

        /* Accelerometer: acceleration in g * 100 */
        if (accelerometer_sensor_read(&ax_x100, &ay_x100, &az_x100) == 0) {

            int ax_int = (int)(ax_x100 / 100);
            int ax_frac = (int)(abs(ax_x100 % 100));

            int ay_int = (int)(ay_x100 / 100);
            int ay_frac = (int)(abs(ay_x100 % 100));

            int az_int = (int)(az_x100 / 100);
            int az_frac = (int)(abs(az_x100 % 100));

            printk("Accel: X=%d.%02d g  Y=%d.%02d g  Z=%d.%02d g\n",
                   ax_int, ax_frac,
                   ay_int, ay_frac,
                   az_int, az_frac);
        }

        /* Light sensor on A0 (PB1 / ADC1_IN5) */
        if (light_sensor_read(&light_raw, &light_mv) == 0) {
            printk("Light: raw=%d (~%d mV)\n", light_raw, light_mv);
        }

        k_sleep(K_SECONDS(1));
    }
}