#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "plantcare_modes.h"

/* Sensors in sensors/ */
#include "sensors/soil_sensor.h"
#include "sensors/light_sensor.h"
#include "sensors/humidity_sensor.h"
#include "sensors/accelerometer_sensor.h"
#include "sensors/rgb_sensor.h"
#include "sensors/leds.h"
#include "sensors/gps_sensor.h"
#include "sensors/button.h"
#include "helpers/plantcare_config.h"

#include "sensors/led1.h"

void main(void)
{
    int ret;

    printk("PlantCare booting...\n");

    /* ---- Initialize all sensors and LEDs (TM1) ---- */

    ret = soil_sensor_init();
    if (ret) printk("soil_sensor_init failed: %d\n", ret);

    ret = light_sensor_init();
    if (ret) printk("light_sensor_init failed: %d\n", ret);

    ret = humidity_sensor_init();
    if (ret) printk("humidity_sensor_init failed: %d\n", ret);

    ret = accelerometer_sensor_init();
    if (ret) printk("accelerometer_sensor_init failed: %d\n", ret);

    ret = rgb_sensor_init();
    if (ret) printk("rgb_sensor_init failed: %d\n", ret);

    ret = leds_init();
    if (ret) printk("leds_init failed: %d\n", ret);

    ret = led1_init();
    if (ret) printk("led1_init failed: %d\n", ret);

    ret = gps_sensor_init();
    if (ret) printk("gps_sensor_init failed: %d\n", ret);

    ret = button_init();
    if (ret) printk("buttonr_init failed: %d\n", ret);

    g_sensors_ready = true;
    printk("Initialization done. Entering TEST MODE.\n");

    /* Run Test Mode on the main thread (never returns) */
    // plantcare_run_test_mode();

    while (1) {
    if (button_was_pressed()) {
    printk("BUTTON: pressed!\n");
    // e.g. toggle some flag, later maybe switch mode, etc.
}
    }
}