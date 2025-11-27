#ifndef PLANTCARE_STATE_H
#define PLANTCARE_STATE_H

#include <zephyr/kernel.h>
#include <stdint.h>

struct plantcare_data {
    int16_t light_raw;
    int32_t light_mv;
    int16_t soil_raw;
    int32_t soil_mv;
    int32_t hum_x100;
    int32_t temp_x100;
    int32_t acc_x_g100, acc_y_g100, acc_z_g100;
};

void plantcare_state_publish(const struct plantcare_data *src);
void plantcare_state_get_snapshot(struct plantcare_data *dst);

#endif