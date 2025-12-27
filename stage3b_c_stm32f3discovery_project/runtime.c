/* runtime.c — minimal explicit runtime services */

#include "runtime.h"
#include "arch_cortexm_baremetal.h"

/* SysTick registers */
#define SYST_CSR           REG32(0xE000E010u)
#define SYST_RVR           REG32(0xE000E014u)
#define SYST_CVR           REG32(0xE000E018u)
#define SYST_CSR_ENABLE    (1u << 0)
#define SYST_CSR_TICKINT   (1u << 1)
#define SYST_CSR_CLKSOURCE (1u << 2)

/* Global tick counter (incremented in SysTick_Handler) */
volatile uint32_t g_systick_ms = 0;

/* SysTick interrupt handler
 * Owns system millisecond timebase.
 * Must be linked into vector table.
 */
void SysTick_Handler(void)
{
    g_systick_ms++;
}


void runtime_irq_disable(void)
{
    arch_irq_disable();
}

void runtime_irq_enable(void)
{
    arch_irq_enable();
}

void runtime_init(uint32_t sysclk_hz)
{
    /* Disable SysTick during setup */
    SYST_CSR = 0;
    SYST_RVR = 0;
    SYST_CVR = 0;

    /* Configure SysTick for 1 kHz */
    SYST_RVR = (sysclk_hz / 1000u) - 1u;
    SYST_CVR = 0;

    SYST_CSR = SYST_CSR_CLKSOURCE |
               SYST_CSR_TICKINT   |
               SYST_CSR_ENABLE;
}

uint32_t runtime_millis(void)
{
    return g_systick_ms;
}

void runtime_delay_ms(uint32_t ms)
{
    uint32_t start = g_systick_ms;
    while ((g_systick_ms - start) < ms) {
        __asm__ volatile ("nop");
    }
}

