#ifndef ACCELEROMETER_SENSOR_H
#define ACCELEROMETER_SENSOR_H

#include <stdint.h>

int accelerometer_sensor_init(void);
int accelerometer_sensor_read(int32_t *x_g100, int32_t *y_g100, int32_t *z_g100);

#endif /* ACCELEROMETER_SENSOR_H */