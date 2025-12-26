.syntax unified
.cpu cortex-m4
.thumb

.global  g_pfnVectors
.global  Reset_Handler
.global  Default_Handler
.global  SysTick_Handler

/* Minimal vector table.
 * We only wire Reset_Handler; everything else loops in Default_Handler.
 */
.section .isr_vector,"a",%progbits
.align 2
.type g_pfnVectors, %object

 
.section .isr_vector,"a",%progbits
.type g_pfnVectors, %object
g_pfnVectors:
  .word  _estack
  .word  Reset_Handler + 1
  .word  Default_Handler + 1 /* NMI */
  .word  Default_Handler + 1 /* HardFault */
  .word  Default_Handler + 1 /* MemManage */
  .word  Default_Handler + 1 /* BusFault */
  .word  Default_Handler + 1 /* UsageFault */
  .word  0
  .word  0
  .word  0
  .word  0
  .word  Default_Handler + 1 /* SVCall */
  .word  Default_Handler + 1 /* DebugMon */
  .word  0
  .word  Default_Handler + 1 /* PendSV */
  .word  SysTick_Handler + 1 /* SysTick */
.size g_pfnVectors, . - g_pfnVectors

/* Reset handler:
 * - Copy .data from FLASH to RAM
 * - Zero .bss
 * - Call main()
 * - If main returns, loop
 */
.section .text.Reset_Handler,"ax",%progbits
Reset_Handler:
  ldr r0, =_estack
  mov sp, r0

  /* Set VTOR = 0x08000000 (vector table in flash) */
  ldr r0, =0xE000ED08     /* SCB_VTOR */
  ldr r1, =0x08000000
  str r1, [r0]

  /* EARLY RESET SIGNATURE: PB3 ON briefly, then OFF */

  /* Enable GPIOB clock: RCC_AHB2ENR |= (1<<1) */
  ldr r0, =0x4002104C     /* RCC_AHB2ENR */
  ldr r1, [r0]
  orr r1, r1, #(1 << 1)
  str r1, [r0]

  /* Enable SYSCFG clock: RCC_APB2ENR |= (1<<0) */
  ldr r0, =0x40021060     /* RCC_APB2ENR */
  ldr r1, [r0]
  orr r1, r1, #(1 << 0)
  str r1, [r0]

  /* Disable TRACESWO on PB3: SYSCFG_CFGR1 |= (1<<24) */
  ldr r0, =0x40010000     /* SYSCFG_CFGR1 */
  ldr r1, [r0]
  orr r1, r1, #(1 << 24)
  str r1, [r0]

  /* PB3 output: GPIOB_MODER set MODER3=01 */
  ldr r0, =0x48000400     /* GPIOB base */
  ldr r1, [r0, #0x00]     /* MODER */
  bic r1, r1, #(3 << (3*2))
  orr r1, r1, #(1 << (3*2))
  str r1, [r0, #0x00]

  /* PB3 ON */
  movw r2, #(1 << 3)
  str r2, [r0, #0x18]     /* BSRR */

  /* short delay */
  movs r3, #0
1: adds r3, r3, #1
   cmp r3, #200
   blt 1b

  /* PB3 OFF */
  ldr r2, =0x00080000
  str r2, [r0, #0x18]     /* BSRR */


  /* Copy .data */
  ldr r0, =_sidata
  ldr r1, =_sdata
  ldr r2, =_edata
1:
  cmp r1, r2
  bcc 2f
  b   3f
2:
  ldr r3, [r0], #4
  str r3, [r1], #4
  b   1b

3:
  /* Zero .bss */
  ldr r0, =_sbss
  ldr r1, =_ebss
4:
  cmp r0, r1
  bcc 5f
  b   6f
5:
  movs r2, #0
  str r2, [r0], #4
  b   4b

6:

  cpsid i
  ldr r0, =0xE000E010    /* SYST_CSR */
  movs r1, #0
  str r1, [r0, #0]       /* CSR = 0 */
  str r1, [r0, #4]       /* RVR = 0 */
  str r1, [r0, #8]       /* CVR = 0 */

  b main
7:
  b 7b

.section .text.Default_Handler,"ax",%progbits
Default_Handler:
8:
  wfe
  b 8b

.section .text.SysTick_Handler,"ax",%progbits
SysTick_Handler:
  ldr r0, =g_systick_ms
  ldr r1, [r0]
  adds r1, r1, #1
  str r1, [r0]
  bx lr


