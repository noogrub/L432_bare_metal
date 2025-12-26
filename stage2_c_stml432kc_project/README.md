# STM32L432KC C vs Rust Metrics — Stage 2 (Bare-metal LED blink + minimal runtime boundary)

Target: **NUCLEO-L432KC** (STM32L432KC, Cortex-M4F, 256 KB flash, 48 KB SRAM)

## Stage 2 goal
- Preserve **Stage1 behavior exactly** (timing, LED signatures, determinism).
- Introduce a **minimal, explicit runtime boundary**.
- Ensure application code never touches:
  - SysTick registers
  - interrupt enable/disable instructions
  - timebase internals

Stage2 is about **structure**, not new features.

## What Stage 2 adds (vs Stage 1)
- Adds `runtime.c` / `runtime.h`.
- Runtime layer owns:
  - SysTick configuration
  - millisecond tick counter
  - delay services
  - explicit IRQ enable/disable helpers
- Moves tick-counter ownership out of `main.c` and into the runtime layer.
- Refactors `main.c` to depend only on runtime APIs for time and IRQ policy.

## What Stage 2 does NOT add
- No HAL / CMSIS
- No libc
- No UART, printf, semihosting
- No dynamic allocation
- No new interrupts beyond SysTick
- No behavior change in LED patterns

## LED mapping (board)
- User LED **LD3** is connected to **PB3** (Arduino D13).

## File responsibilities (critical invariant)
- `startup.s`
  - Vector table and reset handler
  - `.data` copy and `.bss` zero
  - Early reset LED signature
  - `SysTick_Handler` increments runtime-owned tick
- `runtime.c` / `runtime.h`
  - Owns timebase and interrupt policy
  - Configures SysTick @ 1 kHz
  - Provides `runtime_delay_ms()`, `runtime_millis()`
- `main.c`
  - Application logic only
  - GPIO init and LED pattern
  - No direct SysTick or IRQ manipulation
- `build_id.c` + `linker.ld`
  - Firmware identity breadcrumbs in `.build_id`
  - Section explicitly kept via linker `KEEP()`

## Build prerequisites (Debian / WSL)
- `arm-none-eabi-gcc`
- `make`
- `stlink-tools`
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
Must match Stage1 exactly:
- Early reset LED pulse (startup)
- Early main signature pulse
- Guard window proving SysTick + IRQ are alive
- Steady blink pattern driven by runtime timebase

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
- Stage2 footprint should remain very close to Stage1.
- Any future change to timing or sleep semantics must occur inside the runtime layer, not in application code.
