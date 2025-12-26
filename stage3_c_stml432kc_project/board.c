/* board.c — board abstraction layer (Stage3)
 *
 * Goal: main.c should talk to "the board", not to MCU-specific init pieces.
 */

#include "board.h"
#include "init_clock.h"
#include "init_board.h"

void board_init(void)
{
    init_clock();
    init_board();
}

