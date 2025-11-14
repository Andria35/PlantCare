#ifndef GPS_SENSOR_H
#define GPS_SENSOR_H

#include <stdint.h>

/* Init UART for the GPS (USART1 on D0/D1).
 * Returns 0 on success, negative errno on failure.
 */
int gps_sensor_init(void);

/* Non-blocking read of one byte from GPS.
 * Returns:
 *   0        -> one character read, stored in *out_char
 *   -EAGAIN  -> no data available right now
 *   other <0 -> error
 */
int gps_sensor_read_char(uint8_t *out_char);

#endif /* GPS_SENSOR_H */