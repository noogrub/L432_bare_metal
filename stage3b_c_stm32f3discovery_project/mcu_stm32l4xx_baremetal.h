#ifndef MCU_STM32L4XX_H
#define MCU_STM32L4XX_H

#include <stdint.h>

/* Minimal register accessor */
#define REG32(addr) (*(volatile uint32_t *)(addr))

/* ============================
   RCC (STM32L4xx)
   ============================ */
#define RCC_BASE           (0x40021000u)
#define RCC_CR             REG32(RCC_BASE + 0x00u)
#define RCC_ICSCR          REG32(RCC_BASE + 0x04u)
#define RCC_CFGR           REG32(RCC_BASE + 0x08u)
#define RCC_AHB2ENR        REG32(RCC_BASE + 0x4Cu)
#define RCC_APB2ENR        REG32(RCC_BASE + 0x60u)

/* RCC bits */
#define RCC_CR_MSION       (1u << 0)
#define RCC_CR_MSIRDY      (1u << 1)
#define RCC_CR_PLLON       (1u << 24)
#define RCC_CR_PLLRDY      (1u << 25)

/* CFGR SW[1:0] system clock switch: 00 = MSI */
#define RCC_CFGR_SW_MASK   (3u << 0)
#define RCC_CFGR_SWS_MASK  (3u << 2)

/* ICSCR MSI range: MSIRANGE[7:4], Range 6 = 4 MHz */
#define RCC_ICSCR_MSIRANGE_SHIFT (4u)
#define RCC_ICSCR_MSIRANGE_MASK  (0xFu << RCC_ICSCR_MSIRANGE_SHIFT)
#define RCC_ICSCR_MSIRANGE_4MHZ  (6u << RCC_ICSCR_MSIRANGE_SHIFT)

/* Peripheral clock enables used by this project */
#define RCC_AHB2ENR_GPIOBEN (1u << 1)
#define RCC_APB2ENR_SYSCFGEN (1u << 0)

/* ============================
   GPIOB (STM32L4xx)
   ============================ */
#define GPIOB_BASE         (0x48000400u)
#define GPIOB_MODER        REG32(GPIOB_BASE + 0x00u)
#define GPIOB_ODR          REG32(GPIOB_BASE + 0x14u)
#define GPIOB_BSRR         REG32(GPIOB_BASE + 0x18u)

/* ============================
   SYSCFG (STM32L4xx)
   ============================ */
#define SYSCFG_BASE        (0x40010000u)
#define SYSCFG_CFGR1       REG32(SYSCFG_BASE + 0x00u)
#define SYSCFG_CFGR1_TRACESWO_DISABLE (1u << 24)

#endif /* MCU_STM32L4XX_H */

