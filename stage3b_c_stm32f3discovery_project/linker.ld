/* STM32L432KC memory layout (flash 256K, SRAM 48K)
 * Hand-rolled for inspectability.
 */
ENTRY(Reset_Handler)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 256K
  RAM   (rwx): ORIGIN = 0x20000000, LENGTH = 40K
}

/* Fixed stack reservation (2 KiB) */
_stack_size = 0x800; /* 2048 bytes */

/* Stack grows down from the top of RAM */
_estack  = ORIGIN(RAM) + LENGTH(RAM);
_sstack  = _estack - _stack_size;

SECTIONS
{
  .isr_vector :
  {
    KEEP(*(.isr_vector))
  } > FLASH

  .text :
  {
    *(.text*)
    *(.rodata*)
    *(.glue_7*)
    *(.glue_7t*)
    *(.eh_frame*)
  } > FLASH

  /* Firmware identity breadcrumbs (keep even with --gc-sections) */
  .build_id :
  {
    KEEP(*(.build_id))
  } > FLASH

  /* Initialized data copied from FLASH to RAM at boot */
  /* Initialized data copied from FLASH to RAM at boot */
  .data :
  {
    _sdata = .;
    *(.data*)
    _edata = .;
  } > RAM AT > FLASH
  _sidata = LOADADDR(.data);

  /* Zero-init data in RAM */
  .bss :
  {
    _sbss = .;
    *(.bss*)
    *(COMMON)
    _ebss = .;
  } > RAM

  /* Optional: keep end symbol for debugging */
  _end = .;
}

/* Enforce that .data/.bss do not overlap the reserved 2 KiB stack area */
ASSERT(_ebss <= _sstack, "ERROR: RAM overflow: .bss overlaps reserved stack");	
