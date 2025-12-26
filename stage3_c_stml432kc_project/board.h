#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

/* Re-export the clock contract (SYSCLK_HZ) */
#include "init_clock.h"

/* Re-export board-visible signals */
#include "init_board.h"

/* One-shot board initialization:
 * - clock policy
 * - board GPIO/pins policy
 */
void board_init(void);

#endif /* BOARD_H */

