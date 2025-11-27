#include "plantcare_state.h"

static struct plantcare_data g_buf[2];
static volatile uint32_t g_version;

void plantcare_state_publish(const struct plantcare_data *src)
{
    uint32_t next = g_version + 1;
    g_buf[next & 1] = *src;   // copy whole struct
    g_version = next;
}

void plantcare_state_get_snapshot(struct plantcare_data *dst)
{
    uint32_t v1, v2;
    do {
        v1 = g_version;
        *dst = g_buf[v1 & 1];
        v2 = g_version;
    } while (v1 != v2);
}