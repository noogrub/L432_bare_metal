/* init_clock.c — system clock initialization
 *
 * Stage3: explicit init refactor
 * Contract: configure SYSCLK before any runtime services start.
 */

#include <stdint.h>
#include "mcu.h"

/* -----------------------------
   Public API
----------------------------- */
void init_clock(void)
{
    /* Ensure MSI on */
    RCC_CR |= RCC_CR_MSION;
    while ((RCC_CR & RCC_CR_MSIRDY) == 0u) { }

    /* Set MSI range to 4 MHz */
    RCC_ICSCR =
        (RCC_ICSCR & ~RCC_ICSCR_MSIRANGE_MASK) |
         RCC_ICSCR_MSIRANGE_4MHZ;

    /* Force SYSCLK source to MSI */
    RCC_CFGR &= ~RCC_CFGR_SW_MASK;
    while ((RCC_CFGR & RCC_CFGR_SWS_MASK) != 0u) { }

    /* Disable PLL for determinism */
    if (RCC_CR & RCC_CR_PLLON) {
        RCC_CR &= ~RCC_CR_PLLON;
        while (RCC_CR & RCC_CR_PLLRDY) { }
    }
}

