/* startup.s — STM32F303VCT6 (STM32F3x) minimal startup
 * Matches Stage3 linker.ld symbols:
 *   _estack, _sidata, _sdata, _edata, _sbss, _ebss
 */

.syntax unified
.cpu cortex-m4
.thumb

.global  Reset_Handler
.type Reset_Handler, %function
.thumb_func

.global  Default_Handler

/* Provided by linker */
.word _estack

/* Vector table */
.section .isr_vector,"a",%progbits
.type g_pfnVectors, %object
g_pfnVectors:
  .word _estack
  .word Reset_Handler

  /* Core system handlers */
  .word NMI_Handler
  .word HardFault_Handler
  .word MemManage_Handler
  .word BusFault_Handler
  .word UsageFault_Handler
  .word 0
  .word 0
  .word 0
  .word 0
  .word SVC_Handler
  .word DebugMon_Handler
  .word 0
  .word PendSV_Handler
  .word SysTick_Handler

  /* External interrupts (we keep a full table of Default_Handler)
   * STM32F303 has up to 81 external IRQs depending on line; we provide 84 entries
   * to be safely "long enough" for typical stm32f3x parts.
   */
  .rept 84
    .word Default_Handler
  .endr

.size g_pfnVectors, . - g_pfnVectors

/* Reset handler */
.section .text,"ax",%progbits
Reset_Handler:
  /* Copy .data from FLASH to RAM */
  ldr r0, =_sidata
  ldr r1, =_sdata
  ldr r2, =_edata
1:
  cmp r1, r2
  bcc 2f
  b 3f
2:
  ldr r3, [r0], #4
  str r3, [r1], #4
  b 1b

3:
  /* Zero .bss */
  ldr r0, =_sbss
  ldr r1, =_ebss
4:
  cmp r0, r1
  bcc 5f
  b 6f
5:
  movs r2, #0
  str r2, [r0], #4
  b 4b

6:
  /* Call main() */
  bl main

  /* If main returns, loop forever */
7:
  b 7b

/* Weak aliases for handlers */
.section .text.Default_Handler,"ax",%progbits
Default_Handler:
  b .

/* Core exception handlers */
.weak NMI_Handler
.thumb_set NMI_Handler, Default_Handler
.weak HardFault_Handler
.thumb_set HardFault_Handler, Default_Handler
.weak MemManage_Handler
.thumb_set MemManage_Handler, Default_Handler
.weak BusFault_Handler
.thumb_set BusFault_Handler, Default_Handler
.weak UsageFault_Handler
.thumb_set UsageFault_Handler, Default_Handler
.weak SVC_Handler
.thumb_set SVC_Handler, Default_Handler
.weak DebugMon_Handler
.thumb_set DebugMon_Handler, Default_Handler
.weak PendSV_Handler
.thumb_set PendSV_Handler, Default_Handler
.weak SysTick_Handler
.thumb_set SysTick_Handler, Default_Handler

