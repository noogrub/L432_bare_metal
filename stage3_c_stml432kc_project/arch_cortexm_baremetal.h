#ifndef ARCH_CORTEXM_BAREMETAL_H
#define ARCH_CORTEXM_BAREMETAL_H

#include <stdint.h>

/* Architecture-level register accessor */
#define REG32(addr) (*(volatile uint32_t *)(addr))

/* ============================
   SysTick (Cortex-M)
   ============================ */
#define SYST_CSR           REG32(0xE000E010u)
#define SYST_RVR           REG32(0xE000E014u)
#define SYST_CVR           REG32(0xE000E018u)

/* SysTick CSR bits */
#define SYST_CSR_ENABLE    (1u << 0)
#define SYST_CSR_TICKINT   (1u << 1)
#define SYST_CSR_CLKSOURCE (1u << 2)

/* ============================
   IRQ control (Cortex-M)
   ============================ */
static inline void arch_irq_disable(void)
{
    __asm__ volatile ("cpsid i" ::: "memory");
}

static inline void arch_irq_enable(void)
{
    __asm__ volatile ("cpsie i" ::: "memory");
}

#endif /* ARCH_CORTEXM_BAREMETAL_H */

