# gigi_bare_metal.md
## Bare-metal staging notes (Stage2 ➜ Stage3 / Stage3b)

Date: 2025-12-26  
Target (current): STM32L432KC (NUCLEO-L432KC)  
Goal: keep `main.c` as a storyboard; keep hardware bring-up isolated; preserve portability.

---

## Design north star
**Platform init brings the chip to a known contract; runtime/app implements behavior using only that contract.**

This keeps Stage3 changes localized and makes later MCU swaps mostly a “platform layer” rewrite.

---

## Stage2 status (keep as-is)
Stage2 is a transitional refactor: runtime moved out of `main.c`.  
We keep Stage2 intact and introduce Stage3 as a new axis: a platform bring-up layer.

Current files (Stage2 baseline):
- `build_id.c`
- `linker.ld`
- `main.c`
- `Makefile`
- `openocd_l432_stlink.cfg`
- `README_stage2.md`
- `runtime.c`, `runtime.h`
- `startup.s`

---

## Stage3 plan: add a platform bring-up layer

### 1) Keep `main.c` orchestration-only
`main.c` should read like a storyboard:
- emit FWID / build banner (if you do that)
- `platform_init();`
- `runtime_init();`
- `while (1) { runtime_step(); }`

No “register soup” in `main.c`.

### 2) Add one public entry point for bring-up
Add:
- `platform_init.h`
- `platform_init_<flavor>.c`

Expose **one** public function:
- `void platform_init(void);`

Optionally expose:
- `uint32_t platform_core_clock_hz(void);`
- `uint32_t platform_tick_hz(void);` (only if runtime needs an explicit tick rate)

Keep everything else private inside the platform file(s).

### 3) Avoid a generic `init.c`
Avoid `init.c` as a filename. It becomes a junk drawer.

Prefer names that force boundaries:
- `platform_init_*`
- or `board_init_*` (if you prefer “board” semantics)

### 4) Flavor naming conventions
Use flavor names that encode policy and stay readable over time:

Examples:
- `platform_init_msi.c` (simple baseline)
- `platform_init_pll_80mhz.c` (explicit performance claim)
- `platform_init_systick_1khz.c` (explicit timebase)

Start with one flavor. Naming should allow adding a second without a redesign.

### 5) Don’t split too early
Stage3 should not immediately explode into many files.

Heuristic:
- If `platform_init_<flavor>.c` grows past ~200–250 lines, or you need two flavors, then split:
  - `clock.c/.h`
  - `timebase.c/.h`
  - `gpio.c/.h`

Keep `platform_init_<flavor>.c` as a short “sequence file” that calls those pieces in order.

---

## Platform contract: keep it small and explicit
If you want portability across STM32 parts (and beyond), runtime should not touch RCC/GPIO directly.

A thin platform API that preserves portability:
- `platform_init()`
- timing: `platform_delay_us()` **or** `platform_ticks_now()` + `platform_tick_hz()`
- LED: `platform_led_set(bool)` or `platform_led_toggle()`
- optional debug: `platform_log_putc()` (UART)

If runtime only depends on this contract, most MCU changes become confined to the platform layer.

---

## Portability expectations

### Within STM32 (device/family switches)
This split is the main enabler. Expect to keep:
- `runtime.c/.h`
- `build_id.c`
- most or all of `main.c`

Expect to change:
- `startup.s` (vector table / handlers)
- `linker.ld` (memory map: flash/RAM sizes, regions)
- `platform_init_<flavor>.c` (clock tree + GPIO + time base details)
- `openocd_*.cfg` (board/interface specifics)
- pin mapping (LED, UART pins, etc.)

### Across vendors (e.g., MSP430, RP2040)
The architecture still helps, but startup/linker/init will be rewritten.  
Runtime can still remain stable if the platform contract remains stable.

---

## Suggested future directory split (optional Stage4+)
If/when you want cleaner multi-MCU structure:

- `app/`  
  - `main.c`, `runtime.c/.h`, `build_id.c`
- `platform/<mcu_or_board>/`  
  - `platform_init_<flavor>.c/.h`, `startup.s`, `linker.ld`, `openocd_*.cfg`

Then “switch MCU” becomes “swap platform folder + Makefile target,” not “hunt the whole repo.”

---

## Stage3b note
We can treat “original Stage3 plan” as **Stage3b** if it is a separate objective from introducing the platform layer.
- **Stage3**: platform contract + platform init flavors + keep `main.c` clean
- **Stage3b**: original Stage3 behavior/measurement plan, implemented on top of the new contract

This keeps architectural work separate from experimental/feature work.

---
