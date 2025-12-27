/* runtime.h — minimal explicit runtime services
 *
 * Owns timebase, interrupt policy, and critical sections.
 * No libc. No allocation. No side effects beyond what is documented.
 */

#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdint.h>

/* Initialize runtime services.
 * Must be called once, after clocks are stable.
 */
void runtime_init(uint32_t sysclk_hz);

/* Millisecond time since runtime_init() */
uint32_t runtime_millis(void);

/* Sleep for at least ms milliseconds */
void runtime_delay_ms(uint32_t ms);

/* Explicit critical section control */
void runtime_irq_disable(void);
void runtime_irq_enable(void);

#endif /* RUNTIME_H */

