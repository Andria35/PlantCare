// src/helpers/plantcare_mode_test.c

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <stdbool.h>

#include "plantcare_state.h"
#include "plantcare_config.h"
#include "plantcare_units.h"

#include "sensors/led1.h"
#include "sensors/leds.h"
#include "sensors/button.h"

void plantcare_run_test_mode(void)
{
    struct plantcare_data s;

    /* Mark current mode + sampling period for background thread */
    g_current_mode       = PLANTCARE_MODE_TEST;
    g_sampling_period_ms = 2000;      /* sensor thread: sample every 2 seconds */

    /* TM5: LED1 (blue LED) ON in Test Mode */
    led1_set(true);

    printk("\n===== ENTERING TEST MODE =====\n");
    printk("Press button to switch to NORMAL MODE.\n");

    /* For "every 2 seconds" behavior using uptime */
    int64_t last_print_ms = k_uptime_get();

    while (g_current_mode == PLANTCARE_MODE_TEST) {

        /* 1) Handle button event if ISR has set the flag */
        if (g_button_pressed_event) {
            g_button_pressed_event = false;  /* consume event */

            printk("Button pressed -> switching to NORMAL MODE\n");
            g_current_mode = PLANTCARE_MODE_NORMAL;
            break;  /* leave TEST MODE function */
        }

        /* 2) Check if it’s time to print (every ~2000 ms) */
        int64_t now = k_uptime_get();
        if ((now - last_print_ms) >= 2000) {
            last_print_ms = now;

            /* Take snapshot from background sensor thread */
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

            /* Convert soil + light RAW readings to percentage *10 */
            int32_t soil_pct_x10  = soil_raw_to_pct_x10(s.soil_raw);
            int32_t light_pct_x10 = light_raw_to_pct_x10(s.light_raw);

            printk("\n================ TEST MODE =================\n");

            printk("SOIL MOISTURE:  %d.%01d%%\n",
                   soil_pct_x10 / 10,
                   soil_pct_x10 % 10);

            printk("LIGHT:          %d.%01d%%\n",
                   light_pct_x10 / 10,
                   light_pct_x10 % 10);

            printk("TEMP/HUM: Temperature: %d.%02d C,  "
                   "Relative Humidity: %d.%02d%%\n",
                   s.temp_x100 / 100, s.temp_x100 % 100,
                   s.hum_x100  / 100, s.hum_x100  % 100);

            /* Accelerometer: g*100 -> (m/s^2)*100 via helper */
            int32_t ax_ms2_x100 = accel_g100_to_ms2_x100(s.acc_x_g100);
            int32_t ay_ms2_x100 = accel_g100_to_ms2_x100(s.acc_y_g100);
            int32_t az_ms2_x100 = accel_g100_to_ms2_x100(s.acc_z_g100);

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

            if (s.gps_last_sentence[0] != '\0') {
                printk("GPS LAST NMEA: %s\n", s.gps_last_sentence);
            } else {
                printk("GPS LAST NMEA: (no data yet)\n");
            }

            /* That completes one "TM2/TM3" cycle (every ~2 seconds). */
        }

        /* 3) Small sleep so we don't spin at 100% CPU.
         *    Button detection does NOT depend on this anymore,
         *    the ISR already latched the event in g_button_pressed_event.
         */
        k_sleep(K_MSEC(50));
    }

    /* Leaving TEST MODE: optional cleanup (e.g. turn off LED1) */
    if (g_current_mode != PLANTCARE_MODE_TEST) {
        led1_set(false);
    }
}