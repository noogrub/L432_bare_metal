# Bare-Metal Bring‑Up on a Mystery STM32 Board
*The “hard way” methodology: from unknown board → verified blink, using only openocd + gdb and read/write proofs.*

This document describes a repeatable, silicon‑first workflow for bringing up a new/unknown STM32 board without CMSIS/HAL, and without trusting documentation as your first line of defense. Documentation is still used—**after** the chip starts telling the truth.

The core principle:

> **When a definition is suspect, read the silicon.**  
> Treat every address/bit as a hypothesis until the chip confirms it.

---

## Tooling assumptions
- `openocd` with ST‑LINK (`interface/stlink.cfg`)
- `gdb-multiarch` (or `arm-none-eabi-gdb`)
- A minimal ELF that you can flash and debug
- You can edit/rebuild/flash rapidly (`make clean && make && make flash`)

---

## Quick mental model (what we proved in Stage3b)
A “blink” requires *all* of these to be true:

1. **Vector table is correct** (SP and Reset_Handler)
2. **Stack pointer points to real SRAM** (no stacking bus faults)
3. **Reset_Handler runs to `main()`**
4. **The GPIO register block is real and writable**
5. **The GPIO peripheral clock gate is enabled (RCC)**
6. **The pin mode is output (MODER)**
7. **The pin is driven (BSRR/ODR)**
8. **Timing works** (SysTick interrupt + handler increments a counter, or a calibrated busy loop)

If any one is wrong, you may see: “LED on but no blink,” “fault loops,” “writes don’t stick,” etc.

---

## Phase 0 — Establish a clean baseline
**Avoid polluted state.** If you “poke” lots of registers in GDB (e.g., writing `0xFFFFFFFF` to RCC), later experiments become ambiguous.

**Rule:** Between distinct hypotheses, return to baseline with:

```gdb
monitor reset halt
```

Optional: If OpenOCD scripts modify clocks during reset on your setup, consider testing `monitor reset init` as well.

---

## Phase 1 — Prove the vector table is sane
Read the first two words of flash (vector table base).

```gdb
x/2wx 0x08000000
```

Interpretation:
- word[0] = initial MSP
- word[1] = Reset_Handler address (Thumb bit set => LSB = 1 in memory; GDB may show PC as even)

Example:
- `0x2000A000` = stack top
- `0x08000195` = Reset_Handler + Thumb bit

If Reset_Handler symbol lookup is confusing, verify with `nm`:

```bash
arm-none-eabi-nm -n build/blink.elf | grep -E '\bReset_Handler\b|\bg_pfnVectors\b'
```

---

## Phase 2 — If you fault early, decode the fault (don’t guess)
If you land in a fault handler, capture **why**.

### 2.1 Break in the fault handler
```gdb
monitor reset halt
break UsageFault_Handler
break HardFault_Handler
break BusFault_Handler
continue
```

### 2.2 Read fault status registers
Useful SCB registers (ARMv7‑M):
- CFSR: `0xE000ED28` (Configurable Fault Status)
- HFSR: `0xE000ED2C` (HardFault Status)
- SHCSR: `0xE000ED24` (System Handler Control/State)
- BFAR: `0xE000ED38` (BusFault Address) when valid

```gdb
p/x *(unsigned int*)0xE000ED28
p/x *(unsigned int*)0xE000ED2C
p/x *(unsigned int*)0xE000ED24
p/x *(unsigned int*)0xE000ED38
```

### 2.3 Stack frame sanity check (stacking faults)
If fault entry itself fails, you may see BusFault stacking bits (e.g., STKERR/UNSTKERR) and the stacked frame may be garbage/zeroed.

Check MSP and dump a few words:

```gdb
info reg msp
x/8wx $msp
```

**Common root cause we hit:** RAM size mismatch → invalid `_estack` → stacking bus fault during exception entry.

---

## Phase 3 — Confirm SRAM boundary (fix linker first)
If your initial MSP is outside real SRAM, nothing else matters.

### 3.1 Use what you can measure
- The reset MSP value (vector table word[0]) should match your linker’s `_estack`.
- If you suspect wrong SRAM size, adjust linker RAM length and re-test.

### 3.2 Practical fix pattern
In `linker.ld`:

```ld
RAM (rwx) : ORIGIN = 0x20000000, LENGTH = <correct_size>
_estack = ORIGIN(RAM) + LENGTH(RAM);
```

We discovered a board with 40 KiB SRAM where we had assumed 48 KiB; fixing `LENGTH` made faults disappear and allowed `main()` to run.

---

## Phase 4 — Prove you can reach `main()`
Simple check:

```gdb
monitor reset halt
break main
continue
```

If you hit `main()`, you’ve proven:
- vector table correct enough to start
- stack correct enough to run C
- Reset_Handler copies/zeros OK (if you implemented that)

---

## Phase 5 — Prove GPIO address blocks are real (read/write “stickiness”)
### 5.1 Don’t use IDR=0 as “port doesn’t exist”
`IDR` can be legitimately zero if pins read low.

Instead, use **write/readback** on a configuration register like **MODER**.

Example probe for a candidate GPIO base:
```gdb
p/x *(unsigned int*)0x48001000
set *(unsigned int*)0x48001000 = 0xAAAAAAAA
p/x *(unsigned int*)0x48001000
```

Interpretation:
- If the value reads back as written (or changes plausibly), the block is real and writable.
- If it remains fixed (often `0x0`) or bus-faults, the block is not writable in the current state.

**Note:** GPIO blocks often become writable only after the clock gate is enabled (next phase).

---

## Phase 6 — Find the RCC clock gate that makes GPIO writable
This is the “mystery device” heart of the method.

### 6.1 Establish baseline
```gdb
monitor reset halt
set $r = *(unsigned int*)0x40021014
p/x $r
```

(Addresses differ by family; adjust RCC base/offset once verified.)

### 6.2 Hypothesis test
Enable one candidate bit and test whether GPIO MODER becomes writable:

```gdb
set *(unsigned int*)0x40021014 = $r | <bitmask>
p/x *(unsigned int*)0x40021014
set *(unsigned int*)0x48001000 = 0xAAAAAAAA
p/x *(unsigned int*)0x48001000
```

If the MODER write **starts sticking**, you found a working gate.

### 6.3 Important experimental hygiene
- **Reset between tests** (`monitor reset halt`) so a previous “enable” doesn’t make later tests look successful.
- Avoid global writes like `0xFFFFFFFF` to RCC unless you’re deliberately exploring masks; it contaminates later gating experiments.

### 6.4 Encode the result
Once the silicon confirms the register + bit:
- encode it in your project header (`mcu_stm32f3xx_baremetal.h`, etc.)
- stop relying on GDB pokes

---

## Phase 7 — Prove pin mux + output mode + drive
Once the port is writable:
1. set MODER for the pin (`01` = output)
2. drive with BSRR or ODR

**Recommended drive primitives**
- **BSRR** for deterministic set/reset:
  - set: `BSRR = (1<<pin)`
  - reset: `BSRR = (1<<(pin+16))`
- **ODR XOR** for quick toggling (works once configured output):
  - `ODR ^= (1<<pin)`

In code, keep `board_led_on/off/toggle()` as the only places that touch registers.

---

## Phase 8 — If LED turns on but doesn’t blink: timing path diagnosis
This exact symptom usually means:

- your code turns LED on
- then it **blocks forever** in a delay function

### 8.1 Confirm the millisecond tick advances
In GDB:

```gdb
p g_systick_ms
# wait a couple seconds (or continue then Ctrl-C)
p g_systick_ms
```

If unchanged, SysTick interrupt isn’t firing.

### 8.2 Common fixes
- Ensure `SysTick_Handler` exists and increments the counter
- Ensure interrupts are enabled (`cpsie i` or equivalent) after setup
- Ensure SysTick is enabled with:
  - CLKSOURCE
  - TICKINT
  - ENABLE

Minimal handler (project-owned, no CMSIS required):
```c
void SysTick_Handler(void)
{
    g_systick_ms++;
}
```

---

## Phase 9 — Minimal “bring-up script” you can reuse
When a new board arrives:

1. **Flash** your minimal ELF
2. `monitor reset halt`
3. `x/2wx 0x08000000` (vector sanity)
4. `break main; continue` (reach `main`)
5. If faults: read CFSR/HFSR/SHCSR/BFAR + check MSP
6. If LED on/no blink: confirm `g_systick_ms` increments
7. If GPIO writes don’t stick:
   - baseline reset
   - test RCC gating bit by bit using MODER write-stickiness
8. Once proven, encode addresses/bits in `mcu_*.h` and delete GDB pokes

---

## What documentation is for (and when to use it)
Documentation is essential for:
- the canonical memory map (RCC/GPIO base addresses, offsets)
- pin mux details (alternate function mapping)
- board-level LED mappings (which pin drives which LED)
- reserved-bit behavior and register semantics

But this method protects you from:
- wrong board revision notes
- doc snippets for a different MCU variant
- accidental debug clock enabling
- stale assumptions copied across STM32 families

**Use docs to propose hypotheses. Use silicon to confirm them.**

---

## Appendix: Helpful device identity reads
Example reads (family-dependent, but often useful):

```gdb
# Flash size (commonly KB)
p/x *(unsigned short*)0x1FFFF7CC

# DBGMCU IDCODE
p/x *(unsigned int*)0xE0042000
```

---

## Appendix: “Golden rules” we learned the hard way
- **Reset is part of the experiment.** Reset between hypotheses.
- **IDR=0 proves nothing.** Use MODER write/readback.
- **Faults are data.** Decode them; don’t guess.
- **Stack first.** Wrong SRAM size makes everything look haunted.
- **One change at a time.** Make the smallest test that can falsify your hypothesis.
