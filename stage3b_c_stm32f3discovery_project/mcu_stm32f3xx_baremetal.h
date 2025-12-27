#ifndef MCU_STM32F3XX_BAREMETAL_H
#define MCU_STM32F3XX_BAREMETAL_H

/* Minimal STM32F3 (STM32F303) register subset for Stage3b
 * Project-owned bare-metal defines (no CMSIS/HAL).
 */

#include <stdint.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

/* --- Base addresses (STM32F303) --- */
#define PERIPH_BASE        (0x40000000u)
#define AHBPERIPH_BASE     (0x48000000u)
#define RCC_BASE           (0x40021000u)

/* GPIO ports */
#define GPIOA_BASE         (0x48000000u)
#define GPIOE_BASE         (0x48001000u)

/* --- RCC registers --- */
#define RCC_AHBENR         REG32(RCC_BASE + 0x14u)
#define RCC_APB2ENR        REG32(RCC_BASE + 0x18u)
#define RCC_APB1ENR        REG32(RCC_BASE + 0x1Cu)

/* RCC bits */
#define RCC_AHBENR_GPIOEEN (1u << 21)

/* --- GPIO registers (common offsets) --- */
#define GPIO_MODER(base)   REG32((base) + 0x00u)
#define GPIO_ODR(base)     REG32((base) + 0x14u)
#define GPIO_BSRR(base)    REG32((base) + 0x18u)

/* --- Board LED mapping (STM32F3DISCOVERY) --- */
/* LED Pins range from 8 thru 15; choose one as our single "heartbeat" LED. */
#define BOARD_LED_GPIO_BASE  GPIOE_BASE
#define BOARD_LED_PIN        (12u)

#endif /* MCU_STM32F3XX_BAREMETAL_H */

