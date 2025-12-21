# STM32L432KC Bare-Metal Metrics Experiment (C vs Rust)
_Last updated: 2025-12-20_

This document is the reproducible “design notes” for our C vs Rust comparison on an STM32L432KC (NUCLEO-L432KC).

Goal: compile “the same” bare-metal program in C and Rust, then record memory section sizes and totals.

---

## Hardware + baseline assumptions

### Board
- Board: NUCLEO-L432KC
- On-board user LED (LD3) is on **PB3** (Arduino Nano D13). fileciteturn14file14L55-L56

### MCU memory envelope
- STM32L432KC: **up to 256 KB Flash** and **64 KB SRAM**. fileciteturn14file8L4-L6

### Clock gating reminder
Peripheral registers are not supported when the peripheral clock is not active. fileciteturn14file1L17-L18  
So even “blink an LED” implies: enable GPIO clock in RCC, then configure GPIO registers.

---

## Toolchain + environment

### Host environment
- Windows host
- WSL2: Debian (minimal)
- USB forwarded into WSL via `usbipd-win` (host side) + `usbipd attach --wsl`.

### Tools we will use (CLI, minimal “magic”)
- `arm-none-eabi-gcc` + `arm-none-eabi-objcopy` + `arm-none-eabi-size`
- `stlink-tools` (`st-info`, `st-flash`)
- `make`
- Optional but useful: `binutils-arm-none-eabi` (usually already present with gcc)
- Optional: `openocd` + `gdb-multiarch` (for later, not required to start)

### Known-good versions (John’s machine, 2025-12-20)
Record these every time you rerun the experiment:
- `arm-none-eabi-gcc --version`  (already captured in terminal)
- `arm-none-eabi-size --version` (already captured in terminal)
- `st-info --version`, `st-flash --version` (already captured in terminal)

---

## Critical register facts (for the “hand-rolled” code)

### Enabling GPIOB clock
RCC AHB2 peripheral clock enable register:
- Register: `RCC_AHB2ENR`
- Address offset: `0x4C` fileciteturn14file1L13-L16
- Bit 1: `GPIOBEN` (IO port B clock enable) fileciteturn14file4L45-L49

We will use this for the minimum viable program so the LED pin can be driven.

---

## What counts as “minimum viable code”

Minimum viable means:
1. A complete, bootable image: vector table + reset handler + stack + minimal linker script.
2. No HAL, no CMSIS, no vendor startup files.
3. One observable behavior on real hardware: **blink LD3 (PB3)**.

Why blink before “read temperature sensor”:
- Blink proves the flash image is correct and the core is executing our code.
- Temperature read pulls in ADC + calibration data + clock setup sooner than we need.
- We can add “read internal temperature sensor” as Feature A later.

---

## Metrics and how we will record them

We will record these from the final `.elf`:
- `.text`  (code in Flash)
- `.rodata` (const data in Flash)
- `.data`  (init data stored in Flash; copied to RAM at boot)
- `.bss`   (zero-init RAM)
- **total Flash used** = `.text + .rodata + .data + vector table + anything else in Flash`
- **total RAM used at runtime baseline** = `.data + .bss + stack + heap (if any)`

Notes:
- `arm-none-eabi-size -A firmware.elf` gives per-section sizes.
- We will keep stack size fixed (e.g., 2 KB) across C and Rust builds.
- We will not enable LTO at first. Add it later as an explicit feature, because it can change the story.

---

## Feature ladder (the experiment plan)

Each step adds one feature. We record a new row in the results table after each step.

0. **Base (MV)**: blink LD3 (PB3) using direct register writes.
1. **Feature A**: time base using SysTick (still no interrupts beyond SysTick).
2. **Feature B**: ring buffer for “samples” (pure RAM structure).
3. **Feature C**: fake ADC loop (generate samples in software; feed ring buffer).
4. **Feature D**: real ADC read (temperature sensor or external analog pin).
5. **Feature E**: UART “printf-lite” (send a few numbers; no stdio).

Rationale:
- A/B/C isolate “language + runtime” costs from peripheral-driver costs.
- D/E introduce real peripheral drivers in a controlled order.

---

## Directory layout (John’s `metrics/` folder)

Suggested layout (keep C and Rust parallel):

```
metrics/
  docs/
    STM32L432KC_C_vs_Rust_Experiment.md  (this file)
    manuals/                             (PDFs you uploaded, and anything else)
    results/                             (tables you fill in by hand)
  common/
    linker/                              (linker scripts, memory map notes)
  c/
    step00_blink/
    step01_systick/
    ...
  rust/
    step00_blink/
    step01_systick/
    ...
```

Rule: every step directory has:
- `README.md` (what changed, build command, flash command)
- source files
- `Makefile` (or a single build script)
- build artifacts in `build/` (ignored by git if desired)

---

## USB / ST-LINK sanity checks

Inside WSL:
- `lsusb | grep -i st` should show `0483:374b ST-LINK/V2.1` (John already sees this).
- `st-info --probe` should find the ST-LINK once attached into WSL.

If `st-info --probe` finds 0:
- Re-check `usbipd list` on Windows host.
- Re-attach to WSL: `usbipd attach --wsl --busid <BUSID>`.
- Confirm WSL sees it via `lsusb`.

---

## Next action (small, reliable chunk)

1) Install the one missing quality-of-life tool (inside WSL Debian):

- `sudo apt install usbutils`

This provides `lsusb`, which we already used.

2) Confirm probe (inside WSL):

- `st-info --probe`

Once that works, we will create **Step 00** (C blink) with:
- a tiny hand-rolled startup + linker script
- a 10-line `main.c` that toggles PB3

Then we’ll measure `.text/.data/.bss` and only then mirror it in Rust.

---

## Notes on “bare metal” phrasing

If someone hears “edge device” and assumes “Linux,” try:

- “This is **bare metal**. No OS. It’s a single ELF image mapped into Flash, with a vector table at address 0.”

That framing tends to snap people out of the PC mental model.

---

## Source documents used

- UM1956 (Nucleo-32 board manual): user LED LD3 mapping to PB3. fileciteturn14file14L55-L56
- RM0394 (Reference manual): RCC_AHB2ENR offset and GPIOBEN bit. fileciteturn14file1L13-L16 fileciteturn14file4L45-L49
- DS11451 (STM32L432KB/STM32L432KC datasheet): flash/SRAM sizes. fileciteturn14file8L4-L6
