#ifndef INIT_BOARD_H
#define INIT_BOARD_H

#include <stdint.h>

/* Early diagnostic signature.
 * May be called before clocks/runtime are initialized.
 */
void board_early_signature(void);

/* Initialize board-level hardware (GPIO, pins, LEDs). */
void init_board(void);

/* LED helpers (PB3 on NUCLEO-L432KC) */
void board_led_on(void);
void board_led_off(void);
void board_led_toggle(void);

#endif /* INIT_BOARD_H */

