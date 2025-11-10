#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2c.h>
#include <stdint.h>
#include "i2c_helpers.h"
#include "accelerometer_sensor.h"

/* Under the hood: MMA8451 */
static const struct i2c_dt_spec accel_i2c =
    I2C_DT_SPEC_GET(DT_NODELABEL(mma8451));

#define REG_CTRL_REG1     0x2A
#define REG_XYZ_DATA_CFG  0x0E
#define REG_OUT_X_MSB     0x01

int accelerometer_sensor_init(void)
{
    uint8_t val;
    int ret = i2c_read_u8_dt(&accel_i2c, REG_CTRL_REG1, &val);
    if (ret < 0) return ret;

    /* standby */
    i2c_write_u8_dt(&accel_i2c, REG_CTRL_REG1, val & ~0x01);
    /* ±2g range */
    i2c_write_u8_dt(&accel_i2c, REG_XYZ_DATA_CFG, 0x00);
    /* active */
    i2c_write_u8_dt(&accel_i2c, REG_CTRL_REG1, val | 0x01);

    printk("Accelerometer sensor initialized\n");
    return 0;
}

int accelerometer_sensor_read(int32_t *x_g100, int32_t *y_g100, int32_t *z_g100)
{
    uint8_t buf[6];
    int ret = i2c_burst_read_dt_checked(&accel_i2c, REG_OUT_X_MSB, buf, sizeof(buf));
    if (ret < 0) return ret;

    int16_t raw_x = (int16_t)((buf[0] << 8) | buf[1]) >> 2;
    int16_t raw_y = (int16_t)((buf[2] << 8) | buf[3]) >> 2;
    int16_t raw_z = (int16_t)((buf[4] << 8) | buf[5]) >> 2;

    *x_g100 = (int32_t)raw_x * 100 / 4096;
    *y_g100 = (int32_t)raw_y * 100 / 4096;
    *z_g100 = (int32_t)raw_z * 100 / 4096;

    return 0;
}