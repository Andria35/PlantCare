#include "plantcare_state.h"

static struct plantcare_data g_buf[2];
static volatile uint32_t g_version;  /* starts at 0 by default */

void plantcare_state_publish(const struct plantcare_data *src)
{
    uint32_t next = g_version + 1U;

    /* Copy entire struct into buffer 0 or 1 */
    g_buf[next & 1U] = *src;

    /* Single 32-bit write “publishes” the new data */
    g_version = next;
}

void plantcare_state_get_snapshot(struct plantcare_data *dst)
{
    uint32_t v1, v2;

    do {
        v1 = g_version;
        *dst = g_buf[v1 & 1U];
        v2 = g_version;
    } while (v1 != v2);
}