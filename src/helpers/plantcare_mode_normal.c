// src/helpers/plantcare_mode_normal.c

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

#include "plantcare_config.h"
#include "plantcare_state.h"
#include "plantcare_units.h"

#include "sensors/button.h"
#include "sensors/leds.h"
#include "sensors/led1.h"
#include "sensors/led2.h"

// TODO: implement LED2 driver similar to LED1 and include it:
// #include "sensors/led2.h"

/* ---------- Configuration / thresholds for NORMAL MODE ---------- */

/* NM1/NM2: 30-second cadence */
#define NM_SAMPLE_PERIOD_MS    30000   /* 30 s */

/* How many samples in 1 hour at 30 s cadence */
#define NM_SAMPLES_PER_HOUR    (3600 / 30)    /* 120 */

/* Comfortable ranges (you can tune these) */

/* Temperature in x100 °C (15.0°C – 30.0°C) */
#define TEMP_MIN_X100          1500
#define TEMP_MAX_X100          3000

/* Relative humidity in x100 % (30.0% – 70.0%) */
#define HUM_MIN_X100           3000
#define HUM_MAX_X100           7000

/* Ambient light: percentage * 10 (10.0% – 80.0%) */
#define LIGHT_MIN_PCT_X10      100
#define LIGHT_MAX_PCT_X10      800

/* Soil moisture: percentage * 10 (20.0% – 80.0%) */
#define SOIL_MIN_PCT_X10       200
#define SOIL_MAX_PCT_X10       800

/* Acceleration: allowed absolute value in g*100.
 * Example: 200 -> 2.00 g (tune as you like).
 */
#define ACC_ABS_LIMIT_G100     200

/* For colour: we assume a healthy leaf is mostly GREEN.
 * If dominant colour is not GREEN, we consider it an "out-of-limit" colour.
 */

/* ---------- Simple stats helpers ---------- */

struct nm_scalar_stats {
    int64_t sum;        /* sum of all samples */
    int32_t min;        /* minimum value */
    int32_t max;        /* maximum value */
};

static void nm_scalar_stats_reset(struct nm_scalar_stats *st)
{
    st->sum = 0;
    st->min = INT32_MAX;
    st->max = INT32_MIN;
}

static void nm_scalar_stats_add(struct nm_scalar_stats *st, int32_t value)
{
    st->sum += value;
    if (value < st->min) st->min = value;
    if (value > st->max) st->max = value;
}

/* ---------- Hourly stats storage ---------- */

/* NM3: mean, min, max for temp, humidity, light, soil */
static struct nm_scalar_stats temp_stats;
static struct nm_scalar_stats hum_stats;
static struct nm_scalar_stats light_stats;   /* % *10 */
static struct nm_scalar_stats soil_stats;    /* % *10 */

/* NM5: accel axis stats (store in g*100, convert when printing) */
static struct nm_scalar_stats ax_stats;
static struct nm_scalar_stats ay_stats;
static struct nm_scalar_stats az_stats;

/* NM4: dominant colour counts in the last hour */
static uint32_t dom_red_count   = 0;
static uint32_t dom_green_count = 0;
static uint32_t dom_blue_count  = 0;

/* How many samples accumulated in current hour window */
static uint32_t nm_sample_count = 0;

static void nm_reset_hour_window(void)
{
    nm_scalar_stats_reset(&temp_stats);
    nm_scalar_stats_reset(&hum_stats);
    nm_scalar_stats_reset(&light_stats);
    nm_scalar_stats_reset(&soil_stats);
    nm_scalar_stats_reset(&ax_stats);
    nm_scalar_stats_reset(&ay_stats);
    nm_scalar_stats_reset(&az_stats);

    dom_red_count   = 0;
    dom_green_count = 0;
    dom_blue_count  = 0;

    nm_sample_count = 0;
}

/* Accumulate one new sample into hourly stats */
static void nm_accumulate_sample(const struct plantcare_data *s,
                                 int32_t temp_x100,
                                 int32_t hum_x100,
                                 int32_t light_pct_x10,
                                 int32_t soil_pct_x10)
{
    /* Scalars */
    nm_scalar_stats_add(&temp_stats,  temp_x100);
    nm_scalar_stats_add(&hum_stats,   hum_x100);
    nm_scalar_stats_add(&light_stats, light_pct_x10);
    nm_scalar_stats_add(&soil_stats,  soil_pct_x10);

    /* Acceleration in g*100 */
    nm_scalar_stats_add(&ax_stats, s->acc_x_g100);
    nm_scalar_stats_add(&ay_stats, s->acc_y_g100);
    nm_scalar_stats_add(&az_stats, s->acc_z_g100);

    /* Dominant colour counts */
    switch (s->dom_color) {
    case DOM_COLOR_RED:
        dom_red_count++;
        break;
    case DOM_COLOR_GREEN:
        dom_green_count++;
        break;
    case DOM_COLOR_BLUE:
        dom_blue_count++;
        break;
    default:
        break;
    }

    nm_sample_count++;
}

/* Decide which colour was dominant most often in the last hour */
static enum plantcare_dom_color nm_hourly_dominant_color(void)
{
    if (dom_red_count == 0 && dom_green_count == 0 && dom_blue_count == 0) {
        return DOM_COLOR_UNKNOWN;
    }

    if (dom_red_count >= dom_green_count && dom_red_count >= dom_blue_count) {
        return DOM_COLOR_RED;
    } else if (dom_green_count >= dom_red_count && dom_green_count >= dom_blue_count) {
        return DOM_COLOR_GREEN;
    } else {
        return DOM_COLOR_BLUE;
    }
}

/* Print hourly statistics once we have NM_SAMPLES_PER_HOUR samples */
static void nm_print_hourly_stats(void)
{
    if (nm_sample_count == 0) {
        return;
    }

    printk("\n----- NORMAL MODE: HOURLY STATISTICS ----\n");

    int32_t temp_mean_x100  = (int32_t)(temp_stats.sum  / nm_sample_count);
    int32_t hum_mean_x100   = (int32_t)(hum_stats.sum   / nm_sample_count);
    int32_t light_mean_x10  = (int32_t)(light_stats.sum / nm_sample_count);
    int32_t soil_mean_x10   = (int32_t)(soil_stats.sum  / nm_sample_count);

    /* Temperature */
    printk("TEMP: mean=%d.%02d C, min=%d.%02d C, max=%d.%02d C\n",
           temp_mean_x100 / 100, temp_mean_x100 % 100,
           temp_stats.min / 100,  temp_stats.min % 100,
           temp_stats.max / 100,  temp_stats.max % 100);

    /* Humidity */
    printk("HUMIDITY: mean=%d.%02d %%, min=%d.%02d %%, max=%d.%02d %%\n",
           hum_mean_x100 / 100, hum_mean_x100 % 100,
           hum_stats.min  / 100, hum_stats.min  % 100,
           hum_stats.max  / 100, hum_stats.max  % 100);

    /* Light (percent) */
    printk("LIGHT: mean=%d.%01d %%, min=%d.%01d %%, max=%d.%01d %%\n",
           light_mean_x10 / 10, light_mean_x10 % 10,
           light_stats.min / 10, light_stats.min % 10,
           light_stats.max / 10, light_stats.max % 10);

    /* Soil moisture (percent) */
    printk("SOIL: mean=%d.%01d %%, min=%d.%01d %%, max=%d.%01d %%\n",
           soil_mean_x10 / 10, soil_mean_x10 % 10,
           soil_stats.min / 10, soil_stats.min % 10,
           soil_stats.max / 10, soil_stats.max % 10);

    /* Accel: convert g*100 min/max/mean to m/s^2*100 and print */
    int32_t ax_mean_g100 = (int32_t)(ax_stats.sum / nm_sample_count);
    int32_t ay_mean_g100 = (int32_t)(ay_stats.sum / nm_sample_count);
    int32_t az_mean_g100 = (int32_t)(az_stats.sum / nm_sample_count);

    int32_t ax_mean_ms2_x100 = accel_g100_to_ms2_x100(ax_mean_g100);
    int32_t ay_mean_ms2_x100 = accel_g100_to_ms2_x100(ay_mean_g100);
    int32_t az_mean_ms2_x100 = accel_g100_to_ms2_x100(az_mean_g100);

    int32_t ax_min_ms2_x100  = accel_g100_to_ms2_x100(ax_stats.min);
    int32_t ay_min_ms2_x100  = accel_g100_to_ms2_x100(ay_stats.min);
    int32_t az_min_ms2_x100  = accel_g100_to_ms2_x100(az_stats.min);

    int32_t ax_max_ms2_x100  = accel_g100_to_ms2_x100(ax_stats.max);
    int32_t ay_max_ms2_x100  = accel_g100_to_ms2_x100(ay_stats.max);
    int32_t az_max_ms2_x100  = accel_g100_to_ms2_x100(az_stats.max);

    printk("ACCEL X: mean=%d.%02d, min=%d.%02d, max=%d.%02d m/s^2\n",
           ax_mean_ms2_x100 / 100, ax_mean_ms2_x100 % 100,
           ax_min_ms2_x100  / 100, ax_min_ms2_x100  % 100,
           ax_max_ms2_x100  / 100, ax_max_ms2_x100  % 100);

    printk("ACCEL Y: mean=%d.%02d, min=%d.%02d, max=%d.%02d m/s^2\n",
           ay_mean_ms2_x100 / 100, ay_mean_ms2_x100 % 100,
           ay_min_ms2_x100  / 100, ay_min_ms2_x100  % 100,
           ay_max_ms2_x100  / 100, ay_max_ms2_x100  % 100);

    printk("ACCEL Z: mean=%d.%02d, min=%d.%02d, max=%d.%02d m/s^2\n",
           az_mean_ms2_x100 / 100, az_mean_ms2_x100 % 100,
           az_min_ms2_x100  / 100, az_min_ms2_x100  % 100,
           az_max_ms2_x100  / 100, az_max_ms2_x100  % 100);

    /* NM4: dominant colour over last hour */
    enum plantcare_dom_color hour_dom = nm_hourly_dominant_color();
    printk("HOURLY DOMINANT COLOUR: ");
    switch (hour_dom) {
    case DOM_COLOR_RED:   printk("RED\n");   break;
    case DOM_COLOR_GREEN: printk("GREEN\n"); break;
    case DOM_COLOR_BLUE:  printk("BLUE\n");  break;
    default:              printk("UNKNOWN\n"); break;
    }

    printk("----- END OF HOURLY STATISTICS -----\n");
}

/* ---------- NM7: Limit check + RGB LED indication ---------- */

static void nm_update_alarm_led(int32_t temp_x100,
                                int32_t hum_x100,
                                int32_t light_pct_x10,
                                int32_t soil_pct_x10,
                                const struct plantcare_data *s)
{
    bool alarm_temp = (temp_x100 < TEMP_MIN_X100 || temp_x100 > TEMP_MAX_X100);
    bool alarm_hum  = (hum_x100  < HUM_MIN_X100  || hum_x100  > HUM_MAX_X100);
    bool alarm_light= (light_pct_x10 < LIGHT_MIN_PCT_X10 ||
                       light_pct_x10 > LIGHT_MAX_PCT_X10);
    bool alarm_soil = (soil_pct_x10 < SOIL_MIN_PCT_X10 ||
                       soil_pct_x10 > SOIL_MAX_PCT_X10);

    bool alarm_accel = false;
    if (s) {
        int32_t ax = s->acc_x_g100;
        int32_t ay = s->acc_y_g100;
        int32_t az = s->acc_z_g100;
        if (ax < 0) ax = -ax;
        if (ay < 0) ay = -ay;
        if (az < 0) az = -az;

        if (ax > ACC_ABS_LIMIT_G100 ||
            ay > ACC_ABS_LIMIT_G100 ||
            az > ACC_ABS_LIMIT_G100) {
            alarm_accel = true;
        }
    }

    /* Colour alarm: assume healthy leaf is mostly GREEN */
    bool alarm_color = (s && s->dom_color != DOM_COLOR_GREEN);

    /* Priority: TEMP > HUM > LIGHT > SOIL > ACCEL > COLOUR
     * Each parameter uses a different RGB colour code.
     */
    bool r = false, g = false, b = false;

    if (alarm_temp) {
        /* TEMPERATURE -> RED */
        r = true;
    } else if (alarm_hum) {
        /* HUMIDITY -> BLUE */
        b = true;
    } else if (alarm_light) {
        /* LIGHT -> GREEN */
        g = true;
    } else if (alarm_soil) {
        /* SOIL -> YELLOW (RED + GREEN) */
        r = true;
        g = true;
    } else if (alarm_accel) {
        /* ACCEL -> CYAN (GREEN + BLUE) */
        g = true;
        b = true;
    } else if (alarm_color) {
        /* LEAF COLOUR -> MAGENTA (RED + BLUE) */
        r = true;
        b = true;
    } else {
        /* No alarms: turn RGB LED OFF (or you could show leaf colour) */
        r = g = b = false;
    }

    rgb_set(r, g, b);
}

/* ---------- NORMAL MODE main function ---------- */

void plantcare_run_normal_mode(void)
{
    struct plantcare_data s;

    printk("\n===== ENTERING NORMAL MODE =====\n");
    printk("Press button to switch back to TEST MODE.\n");

    /* NM1: 30-second monitoring cadence (background thread uses this) */
    g_current_mode       = PLANTCARE_MODE_NORMAL;
    g_sampling_period_ms = NM_SAMPLE_PERIOD_MS;

    /* NM8: LED2 (green LED) ON, LED1 OFF in Normal Mode */
    led1_set(false);

    led2_set(true);

    /* Reset hourly window */
    nm_reset_hour_window();

    /* For NM1/NM2/NM6: 30-second periodic sending using uptime */
    int64_t last_sample_ms = k_uptime_get();

    while (g_current_mode == PLANTCARE_MODE_NORMAL) {

        /* Handle button event (set by ISR) */
        if (g_button_pressed_event) {
            g_button_pressed_event = false;  /* consume event */

            printk("Button pressed -> switching to TEST MODE\n");
            g_current_mode = PLANTCARE_MODE_TEST;
            break;
        }

        int64_t now = k_uptime_get();

        /* NM1/NM2/NM6: every 30 seconds, take snapshot and send values */
        if ((now - last_sample_ms) >= NM_SAMPLE_PERIOD_MS) {
            last_sample_ms = now;

            plantcare_state_get_snapshot(&s);

            /* Convert raw to more readable units */
            int32_t temp_x100 = s.temp_x100;
            int32_t hum_x100  = s.hum_x100;

            int32_t light_pct_x10 = light_raw_to_pct_x10(s.light_raw);
            int32_t soil_pct_x10  = soil_raw_to_pct_x10(s.soil_raw);

            /* Accumulate into hourly stats (NM3, NM4, NM5) */
            nm_accumulate_sample(&s,
                                 temp_x100,
                                 hum_x100,
                                 light_pct_x10,
                                 soil_pct_x10);

            /* NM2: Send all measured values (print every 30 seconds) */
            printk("\n================ NORMAL MODE =================\n");

            printk("TEMP: %d.%02d C\n",
                   temp_x100 / 100, temp_x100 % 100);

            printk("HUMIDITY: %d.%02d %%\n",
                   hum_x100 / 100, hum_x100 % 100);

            printk("LIGHT: %d.%01d %%\n",
                   light_pct_x10 / 10, light_pct_x10 % 10);

            printk("SOIL: %d.%01d %%\n",
                   soil_pct_x10 / 10, soil_pct_x10 % 10);

            /* Accel instant values in m/s^2 (using helper) */
            int32_t ax_ms2_x100 = accel_g100_to_ms2_x100(s.acc_x_g100);
            int32_t ay_ms2_x100 = accel_g100_to_ms2_x100(s.acc_y_g100);
            int32_t az_ms2_x100 = accel_g100_to_ms2_x100(s.acc_z_g100);

            int32_t ax_abs = (ax_ms2_x100 >= 0) ? ax_ms2_x100 : -ax_ms2_x100;
            int32_t ay_abs = (ay_ms2_x100 >= 0) ? ay_ms2_x100 : -ay_ms2_x100;
            int32_t az_abs = (az_ms2_x100 >= 0) ? az_ms2_x100 : -az_ms2_x100;

            printk("ACCEL: X_axis: %s%d.%02d m/s^2, "
                   "Y_axis: %s%d.%02d m/s^2, "
                   "Z_axis: %s%d.%02d m/s^2\n",
                   (ax_ms2_x100 < 0) ? "-" : "", ax_abs / 100, ax_abs % 100,
                   (ay_ms2_x100 < 0) ? "-" : "", ay_abs / 100, ay_abs % 100,
                   (az_ms2_x100 < 0) ? "-" : "", az_abs / 100, az_abs % 100);

            /* Colour sensor instant values */
            printk("COLOUR SENSOR: Clear=%u, Red=%u, Green=%u, Blue=%u, Dominant=",
                   s.clr, s.red, s.green, s.blue);
            switch (s.dom_color) {
            case DOM_COLOR_RED:   printk("RED\n");   break;
            case DOM_COLOR_GREEN: printk("GREEN\n"); break;
            case DOM_COLOR_BLUE:  printk("BLUE\n");  break;
            default:              printk("UNKNOWN\n"); break;
            }

            /* NM6: GPS position + time every 30 seconds.
             * For now we forward last NMEA sentence, which includes UTC time
             * and coordinates. Full parsing + local-time conversion could be
             * added here later.
             */
            if (s.gps_last_sentence[0] != '\0') {
                printk("GPS (NMEA, UTC): %s\n", s.gps_last_sentence);
            } else {
                printk("GPS: (no data yet)\n");
            }

            /* NM7: Check limits and set RGB LED accordingly */
            nm_update_alarm_led(temp_x100,
                                hum_x100,
                                light_pct_x10,
                                soil_pct_x10,
                                &s);

            /* NM3/NM4/NM5: if one hour worth of samples passed, print stats */
            if (nm_sample_count >= NM_SAMPLES_PER_HOUR) {
                nm_print_hourly_stats();
                nm_reset_hour_window();
            }
        }

        /* Small sleep so we don't busy-loop. Button events are latched
         * by the ISR in g_button_pressed_event, so we won't miss them.
         */
        k_sleep(K_MSEC(50));
    }

    /* Leaving NORMAL MODE: optional cleanup.
     * Turn off LED2 here if you turned it on at entry.
     */
    // led2_set(false);
}