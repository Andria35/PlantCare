#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "plantcare_config.h"
#include "sensors/button.h"

/* Later we will also:
 *  - Turn LED2 (green) ON here
 *  - Set sampling to 30s
 *  - Implement NM1..NM8 logic
 */
void plantcare_run_normal_mode(void)
{
    printk("\n===== ENTERING NORMAL MODE (stub) =====\n");
    printk("Normal mode logic not implemented yet.\n");
    printk("Press button to switch back to TEST MODE.\n");

    /* Just sit in NORMAL MODE until button interrupt sets the flag */
    while (g_current_mode == PLANTCARE_MODE_NORMAL) {

        /* Check if ISR reported a button press */
        if (g_button_pressed_event) {
            g_button_pressed_event = false;  /* consume event */

            printk("Button pressed -> switching to TEST MODE\n");
            g_current_mode = PLANTCARE_MODE_TEST;
            break;
        }

        /* Small sleep so we don't busy-loop */
        k_sleep(K_MSEC(50));
    }
}