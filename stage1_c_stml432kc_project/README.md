# STM32L432KC C vs Rust Metrics — Stage 1 (Bare-metal LED blink + SysTick timebase)

Target: **NUCLEO-L432KC** (STM32L432KC, Cortex-M4F, 256 KB flash, 48 KB SRAM)

## Stage 1 goal
- Preserve Stage0’s fully hand-rolled bare-metal discipline (no HAL, no CMSIS startup).
- Make execution **deterministic**.
- Add a **real timebase** using **SysTick + IRQ**.
- Establish a stable baseline for later stages (UART, faults, sleep) without adding runtime baggage.

## What Stage 1 adds (vs Stage 0)
- Forces **SYSCLK = MSI @ 4 MHz** for repeatable timing.
- Configures **SysTick @ 1 kHz**.
- SysTick IRQ increments a millisecond counter.
- Delay logic driven from the SysTick timebase.
- Persistent firmware breadcrumbs via `build_id.c` kept in flash using linker `KEEP()`.
- Early reset signature (in `startup.s`) plus early main signature.

## LED mapping (board)
- User LED **LD3** is connected to **PB3** (Arduino D13).

## Build prerequisites (Debian / WSL)
- `arm-none-eabi-gcc`
- `make`
- `stlink-tools` (for `st-flash` / `st-info`)
- Optional: `binutils-arm-none-eabi`

## One-time flash / USB setup
Use:
- `STLINK_WSL_Flashing_Checklist.md`

## Build
From this folder:

```bash
make clean
make
```

Outputs:
- `build/blink.elf`
- `build/blink.bin`
- `build/blink.map`

## Flash
```bash
st-flash write build/blink.bin 0x08000000
```

## Runtime behavior (signature)
- Early reset LED pulse (from `startup.s`)
- Early main signature pulse
- Guard window proving SysTick + IRQ are alive
- Steady blink pattern timed via SysTick

## Metrics capture
### Size summary
```bash
arm-none-eabi-size build/blink.elf
```

### Size by section
```bash
arm-none-eabi-size -A build/blink.elf
```

### Confirm section placement
```bash
arm-none-eabi-readelf -S build/blink.elf
arm-none-eabi-objdump -h build/blink.elf
```

## Notes
- Flash footprint typically includes `.isr_vector`, `.text`, `.build_id`, and any `.data` load image.
- Stage1 should be deterministic across resets, flashes, and power cycles.
