#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

/* Initialize the user button GPIO + interrupt.
 * Returns 0 on success, negative errno on error.
 */
int button_init(void);

/* Read current button state (optional helper).
 * true  -> pressed
 * false -> not pressed
 */
bool button_is_pressed(void);

#endif /* BUTTON_H */