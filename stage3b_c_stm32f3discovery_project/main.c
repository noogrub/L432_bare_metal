/* ------------------------------------
   See Stage3 README.md
------------------------------------ */
#include <stdint.h>
#include "runtime.h"
#include "board.h"

int main(void)
{
    /* EARLY MAIN SIGNATURE: prove we reached main() */
    board_early_signature();

    runtime_irq_disable();

    board_init();

    runtime_init(SYSCLK_HZ);
    runtime_irq_enable();

    /* guard window: prove SysTick and IRQs are alive */
    board_led_on();
    runtime_delay_ms(150u);
    board_led_off();
    runtime_delay_ms(150u);

    /* Signature: solid ON 500ms, 250ms off, then blink every 500ms */
    board_led_on();
    runtime_delay_ms(500u);
    board_led_off();
    runtime_delay_ms(250u);

    while (1) {
        board_led_toggle();
        runtime_delay_ms(500u);
    }
}

