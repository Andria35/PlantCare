#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "plantcare_modes.h"
#include "plantcare_config.h"
#include "plantcare_state.h"

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

        printk("SOIL MOISTURE: raw=%d, approx=%.1f%%\n",
               s.soil_raw,
               100.0f * (float)s.soil_mv / 3300.0f);

        printk("AMBIENT LIGHT: raw=%d, approx=%.1f%%\n",
               s.light_raw,
               100.0f * (float)s.light_mv / 3300.0f);

        printk("TEMP/HUM: Temperature: %d.%02d C, Relative Humidity: %d.%02d%%\n",
               s.temp_x100 / 100, s.temp_x100 % 100,
               s.hum_x100  / 100, s.hum_x100  % 100);

        printk("ACCELEROMETERS: X: %d.%02d, Y: %d.%02d, Z: %d.%02d (g*100)\n",
               s.acc_x_g100 / 100, s.acc_x_g100 % 100,
               s.acc_y_g100 / 100, s.acc_y_g100 % 100,
               s.acc_z_g100 / 100, s.acc_z_g100 % 100);

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