#ifndef INIT_CLOCK_H
#define INIT_CLOCK_H

#include <stdint.h>

/* Stage3 clock contract:
 * SYSCLK is MSI @ 4 MHz.
 */
#define SYSCLK_HZ 4000000u

/* Initialize system clock.
 * Must be called before runtime_init().
 */
void init_clock(void);

#endif /* INIT_CLOCK_H */

