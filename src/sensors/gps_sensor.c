#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>
#include <errno.h>

#include "gps_sensor.h"

/* Using USART1 (D0/D1) as set in your overlay */
#define GPS_UART_NODE DT_NODELABEL(usart1)

static const struct device *gps_uart;

int gps_sensor_init(void)
{
    gps_uart = DEVICE_DT_GET(GPS_UART_NODE);

    if (!device_is_ready(gps_uart)) {
        printk("gps_sensor: UART not ready!\n");
        return -ENODEV;
    }

    printk("gps_sensor: init OK\n");
    return 0;
}

int gps_sensor_read_char(uint8_t *out_char)
{
    if (!gps_uart || !device_is_ready(gps_uart)) {
        /* GPS not initialized or device not ready yet */
        return -EAGAIN;
    }

    uint8_t c;
    int ret = uart_poll_in(gps_uart, &c);

    if (ret == 0) {
        if (out_char) {
            *out_char = c;
        }
        return 0;               /* got a byte */
    }

    /* uart_poll_in returns -1 when no data; we map that to -EAGAIN */
    return -EAGAIN;
}