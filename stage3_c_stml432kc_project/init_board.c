/* init_board.c — board-specific bring-up
 *
 * Stage3: explicit init refactor
 * Owns GPIO, pin mux, and board-visible signals.
 */

#include <stdint.h>
#include "mcu.h"

/* -----------------------------
   Minimal register access
----------------------------- */
#define REG32(addr) (*(volatile uint32_t *)(addr))

#define PB3_PIN            (3u)

/* -----------------------------
   Public API
----------------------------- */
void board_early_signature(void)
{
    /* Enable GPIOB + SYSCFG clocks */
    RCC_AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    RCC_APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /* Disable SWO on PB3 */
    SYSCFG_CFGR1 |= SYSCFG_CFGR1_TRACESWO_DISABLE;

    /* PB3 output */
    GPIOB_MODER &= ~(3u << (PB3_PIN * 2u));
    GPIOB_MODER |=  (1u << (PB3_PIN * 2u));

    /* LED ON immediately */
    GPIOB_BSRR = (1u << PB3_PIN);
}

void init_board(void)
{
    /* Enable GPIOB + SYSCFG clocks */
    RCC_AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    RCC_APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /* Disable SWO on PB3 */
    SYSCFG_CFGR1 |= SYSCFG_CFGR1_TRACESWO_DISABLE;

    /* PB3 output: MODER3 = 01 */
    GPIOB_MODER &= ~(3u << (PB3_PIN * 2u));
    GPIOB_MODER |=  (1u << (PB3_PIN * 2u));

    /* LED OFF */
    GPIOB_BSRR = (1u << (PB3_PIN + 16u));
}

void board_led_on(void)
{
    GPIOB_BSRR = (1u << PB3_PIN);
}

void board_led_off(void)
{
    GPIOB_BSRR = (1u << (PB3_PIN + 16u));
}

void board_led_toggle(void)
{
    /* Use ODR toggle for simplicity */
    GPIOB_ODR ^= (1u << PB3_PIN);
}

