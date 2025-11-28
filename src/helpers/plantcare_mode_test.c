#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "plantcare_modes.h"
#include "plantcare_config.h"
#include "plantcare_state.h"
#include "plantcare_units.h"

#include "sensors/led1.h"
#include "sensors/leds.h"   /* rgb_set(), leds_init() */

void plantcare_run_test_mode(void)
{
    struct plantcare_data s;

    /* TEST MODE sampling: every 2 seconds */
    g_sampling_period_ms = 2000;

    /* TM5: LED1 (blue LED) ON in Test Mode */
    led1_set(true);

    while (1) {
        plantcare_state_get_snapshot(&s);

        /* TM4: RGB LED colored as dominant color from sensor */
        bool r = false, g = false, b = false;
        switch (s.dom_color) {
        case DOM_COLOR_RED:   r = true; break;
        case DOM_COLOR_GREEN: g = true; break;
        case DOM_COLOR_BLUE:  b = true; break;
        default: break;
        }
        rgb_set(r, g, b);

        /* TM3: send all measured values to PC every 2 seconds (printk) */

        printk("\n================ TEST MODE =================\n");

        int32_t soil_pct_x10  = soil_raw_to_pct_x10(s.soil_raw);
        int32_t light_pct_x10 = light_raw_to_pct_x10(s.light_raw);

        /* ---- TM3: send all measured values to PC every 2 seconds ---- */

        printk("\n================ TEST MODE =================\n");

        printk("SOIL MOISTURE:  %d.%01d%%\n",
               soil_pct_x10 / 10,
               soil_pct_x10 % 10);

        printk("LIGHT:          %d.%01d%%\n",
               light_pct_x10 / 10,
               light_pct_x10 % 10);

        printk("TEMP/HUM: Temperature: %d.%02d C, Relative Humidity: %d.%02d%%\n",
               s.temp_x100 / 100, s.temp_x100 % 100,
               s.hum_x100  / 100, s.hum_x100  % 100);

        /* Convert from g*100 to (m/s^2)*100 using helper */
        int32_t ax_ms2_x100 = accel_g100_to_ms2_x100(s.acc_x_g100);
        int32_t ay_ms2_x100 = accel_g100_to_ms2_x100(s.acc_y_g100);
        int32_t az_ms2_x100 = accel_g100_to_ms2_x100(s.acc_z_g100);

        /* Absolute values for printing fractional part */
        int32_t ax_abs = (ax_ms2_x100 >= 0) ? ax_ms2_x100 : -ax_ms2_x100;
        int32_t ay_abs = (ay_ms2_x100 >= 0) ? ay_ms2_x100 : -ay_ms2_x100;
        int32_t az_abs = (az_ms2_x100 >= 0) ? az_ms2_x100 : -az_ms2_x100;

        printk("ACCELEROMETERS: X_axis: %s%d.%02d m/s^2, "
               "Y_axis: %s%d.%02d m/s^2, "
               "Z_axis: %s%d.%02d m/s^2\n",
               (ax_ms2_x100 < 0) ? "-" : "", ax_abs / 100, ax_abs % 100,
               (ay_ms2_x100 < 0) ? "-" : "", ay_abs / 100, ay_abs % 100,
               (az_ms2_x100 < 0) ? "-" : "", az_abs / 100, az_abs % 100);

        printk("COLOR SENSOR: Clear=%u, Red=%u, Green=%u, Blue=%u, Dominant=",
               s.clr, s.red, s.green, s.blue);
        switch (s.dom_color) {
        case DOM_COLOR_RED:   printk("RED\n");   break;
        case DOM_COLOR_GREEN: printk("GREEN\n"); break;
        case DOM_COLOR_BLUE:  printk("BLUE\n");  break;
        default:              printk("UNKNOWN\n"); break;
        }

        /* GPS: show last NMEA sentence collected by background thread */
        if (s.gps_last_sentence[0] != '\0') {
            printk("GPS LAST NMEA: %s\n", s.gps_last_sentence);
        } else {
            printk("GPS LAST NMEA: (no data yet)\n");
        }

        /* TM2/TM3: UI print every 2 seconds */
        k_sleep(K_SECONDS(2));
    }
}