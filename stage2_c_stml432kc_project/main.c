/* ------------------------------------
   Stage 1 add SysTick to Stage0 blinky
------------------------------------ */
#include <stdint.h>
#include "runtime.h"
/* -----------------------------
   Minimal register access
----------------------------- */
#define REG32(addr) (*(volatile uint32_t *)(addr))

/* -----------------------------
   STM32L432 RCC + GPIOB + SYSCFG
----------------------------- */
#define RCC_BASE           (0x40021000u)
#define RCC_CR             REG32(RCC_BASE + 0x00u)
#define RCC_ICSCR          REG32(RCC_BASE + 0x04u)
#define RCC_CFGR           REG32(RCC_BASE + 0x08u)
#define RCC_AHB2ENR        REG32(RCC_BASE + 0x4Cu)
#define RCC_APB2ENR        REG32(RCC_BASE + 0x60u)

/* RCC bits (STM32L4) */
#define RCC_CR_MSION       (1u << 0)
#define RCC_CR_MSIRDY      (1u << 1)
#define RCC_CR_PLLON       (1u << 24)
#define RCC_CR_PLLRDY      (1u << 25)

/* CFGR SW[1:0] system clock switch: 00=MSI */
#define RCC_CFGR_SW_MASK   (3u << 0)
#define RCC_CFGR_SWS_MASK  (3u << 2)

/* ICSCR MSI range: MSIRANGE[7:4]. Range 6 = 4 MHz on STM32L4. */
#define RCC_ICSCR_MSIRANGE_SHIFT (4u)
#define RCC_ICSCR_MSIRANGE_MASK  (0xFu << RCC_ICSCR_MSIRANGE_SHIFT)
#define RCC_ICSCR_MSIRANGE_4MHZ  (6u << RCC_ICSCR_MSIRANGE_SHIFT)

/* Enable clocks */
#define RCC_AHB2ENR_GPIOBEN (1u << 1)
#define RCC_APB2ENR_SYSCFGEN (1u << 0)

/* GPIOB */
//#define GPIOB_BASE         (0x48000400u)
#define GPIOB_BASE         0x48000400UL
#define GPIOB_ODR          (*(volatile unsigned int *)(GPIOB_BASE + 0x14))

#define GPIOB_MODER        REG32(GPIOB_BASE + 0x00u)
#define GPIOB_BSRR         REG32(GPIOB_BASE + 0x18u)
#define PB3_PIN            (3u)

/* SYSCFG for TRACESWO disable */
#define SYSCFG_BASE        (0x40010000u)
#define SYSCFG_CFGR1       REG32(SYSCFG_BASE + 0x00u)
#define SYSCFG_CFGR1_TRACESWO_DISABLE (1u << 24)

/* -----------------------------
   System clock contract
   SYSCLK = MSI @ 4 MHz
----------------------------- */
#define SYSCLK_HZ 4000000u

/* -----------------------------
   Deterministic clock bring-up
   Force SYSCLK = MSI @ 4 MHz
----------------------------- */
static void clock_init_msi_4mhz(void)
{
    /* Ensure MSI on */
    RCC_CR |= RCC_CR_MSION;
    while ((RCC_CR & RCC_CR_MSIRDY) == 0u) { }

    /* Set MSI range to 4 MHz */
    RCC_ICSCR = (RCC_ICSCR & ~RCC_ICSCR_MSIRANGE_MASK) | RCC_ICSCR_MSIRANGE_4MHZ;

    /* Force SYSCLK source to MSI (SW=00) */
    RCC_CFGR &= ~RCC_CFGR_SW_MASK;
    while ((RCC_CFGR & RCC_CFGR_SWS_MASK) != 0u) { /* wait SWS==00 */ }

    /* Disable PLL if it was left on (optional but helps determinism) */
    if (RCC_CR & RCC_CR_PLLON) {
        RCC_CR &= ~RCC_CR_PLLON;
        while (RCC_CR & RCC_CR_PLLRDY) { }
    }
}

static void gpio_init_pb3_led(void)
{
    /* Clocks */
    RCC_AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    RCC_APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /* Disable SWO on PB3 so it becomes a normal GPIO */
    SYSCFG_CFGR1 |= SYSCFG_CFGR1_TRACESWO_DISABLE;

    /* PB3 output: MODER3 = 01 */
    GPIOB_MODER &= ~(3u << (PB3_PIN * 2u));
    GPIOB_MODER |=  (1u << (PB3_PIN * 2u));

    /* Start LED OFF */
    GPIOB_BSRR = (1u << (PB3_PIN + 16u));
}


int main(void)
{
	/* EARLY MAIN SIGNATURE: PB3 on immediately */
	RCC_AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	RCC_APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	SYSCFG_CFGR1 |= SYSCFG_CFGR1_TRACESWO_DISABLE;
	GPIOB_MODER &= ~(3u << (PB3_PIN * 2u));
	GPIOB_MODER |=  (1u << (PB3_PIN * 2u));
	GPIOB_BSRR = (1u << PB3_PIN);
		
    runtime_irq_disable();

    clock_init_msi_4mhz();
    gpio_init_pb3_led();

    runtime_init(SYSCLK_HZ);
    runtime_irq_enable();

    /* guard window: prove SysTick and IRQs are alive */
    GPIOB_BSRR = (1u << PB3_PIN);
    runtime_delay_ms(150u);
    GPIOB_BSRR = (1u << (PB3_PIN + 16u));
    runtime_delay_ms(150u);


    /* Signature: solid ON 500s, 250ms off, then blink every 500ms */
    GPIOB_BSRR = (1u << PB3_PIN);      /* ON */
    runtime_delay_ms(500u);
    GPIOB_BSRR = (1u << (PB3_PIN + 16u)); /* OFF */
    runtime_delay_ms(250u);

    while (1) {
        GPIOB_ODR ^= (1u << 3);
        //delay_ms(50);
        runtime_delay_ms(500);
    }
}
