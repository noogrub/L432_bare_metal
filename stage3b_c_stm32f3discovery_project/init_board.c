/* init_board.c — STM32F3Discovery bring-up
 *
 * Stage3b: minimal GPIO init for a single heartbeat LED.
 * Uses BOARD_LED_GPIO_BASE / BOARD_LED_PIN from mcu_stm32f3xx_baremetal.h
 */

#include <stdint.h>
#include "mcu.h"
#include "init_board.h"

static inline void led_config_output(void)
{
    /* Enable GPIOE clock */
    RCC_AHBENR |= RCC_AHBENR_GPIOEEN;

    /* MODER: 01 = general purpose output */
    const uint32_t pin = BOARD_LED_PIN;
    uint32_t moder = GPIO_MODER(BOARD_LED_GPIO_BASE);
    moder &= ~(3u << (pin * 2u));
    moder |=  (1u << (pin * 2u));
    GPIO_MODER(BOARD_LED_GPIO_BASE) = moder;
}

void board_early_signature(void)
{
    led_config_output();
    /* LED ON immediately */
    GPIO_BSRR(BOARD_LED_GPIO_BASE) = (1u << BOARD_LED_PIN);
}

void init_board(void)
{
    led_config_output();
    /* LED OFF */
    GPIO_BSRR(BOARD_LED_GPIO_BASE) = (1u << (BOARD_LED_PIN + 16u));
}

void board_led_on(void)
{
    GPIO_BSRR(BOARD_LED_GPIO_BASE) = (1u << BOARD_LED_PIN);
}

void board_led_off(void)
{
    GPIO_BSRR(BOARD_LED_GPIO_BASE) = (1u << (BOARD_LED_PIN + 16u));
}

void board_led_toggle(void)
{
    /* Toggle via ODR */
    GPIO_ODR(BOARD_LED_GPIO_BASE) ^= (1u << BOARD_LED_PIN);
}

