#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2c.h>
#include <stdint.h>
#include "i2c_helpers.h"
#include "humidity_sensor.h"

/* Under the hood: Si7021 */
static const struct i2c_dt_spec humidity_sensor_i2c =
    I2C_DT_SPEC_GET(DT_NODELABEL(si7021));

#define CMD_MEAS_RH_HOLD   0xE5
#define CMD_MEAS_TEMP_HOLD 0xE3

int humidity_sensor_init(void)
{
    printk("Humidity sensor ready\n");
    return 0;
}

static int read_raw(uint8_t cmd, uint16_t *raw)
{
    uint8_t buf[2];
    int ret = i2c_burst_read_dt_checked(&humidity_sensor_i2c, cmd, buf, sizeof(buf));
    if (ret < 0) return ret;
    *raw = ((uint16_t)buf[0] << 8) | buf[1];
    return 0;
}

int humidity_sensor_read(int32_t *humidity_x100, int32_t *temp_x100)
{
    int ret;
    uint16_t raw_rh, raw_t;

    ret = read_raw(CMD_MEAS_RH_HOLD, &raw_rh);
    if (ret < 0) return ret;

    *humidity_x100 = ((int32_t)12500 * raw_rh) / 65536 - 600;

    ret = read_raw(CMD_MEAS_TEMP_HOLD, &raw_t);
    if (ret < 0) return ret;

    *temp_x100 = ((int32_t)17572 * raw_t) / 65536 - 4685;
    return 0;
}