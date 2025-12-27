# Stage 3 — Explicit Initialization & Architecture Isolation

Target: **STM32L432KC (NUCLEO-L432KC)**  
Language: **C (freestanding, bare metal)**

Stage 3 focuses on *architectural clarity*. No new features are added. No behavior changes are introduced.
The goal is to make hardware ownership explicit, isolate responsibilities, and prepare the codebase for
cross-device experiments (e.g., STM32N7) without refactoring logic.

---

## Stage 3 Goals

- Separate **architecture**, **MCU**, and **board** concerns
- Eliminate implicit hardware dependencies from `main.c`
- Make clock, board, and runtime policy explicit and inspectable
- Preserve deterministic startup and measurable footprints
- Enable painless MCU and board swaps in later stages

---

## Layered Architecture

Stage 3 establishes the following layers, from lowest to highest:

### 1. Architecture layer (`arch_*`)
**Example:** `arch_cortexm_baremetal.h`

Owns:
- Cortex-M core registers (SysTick)
- IRQ enable/disable primitives
- CPU-architecture concerns only

Does **not** know:
- Which MCU is used
- Which board or pins exist

---

### 2. MCU layer (`mcu_*`)
**Example:** `mcu_stm32l4xx_baremetal.h`

Owns:
- STM32L4xx register base addresses
- RCC, GPIO, SYSCFG bit definitions actually used by this project
- Vendor / silicon-specific knowledge

Intentionally:
- Minimal
- Incomplete
- Non-CMSIS
- Non-HAL

---

### 3. Board layer (`board_*`, `init_board.*`)

Owns:
- Board-visible hardware (LEDs, pins, diagnostics)
- Mapping from abstract signals ("board LED") to MCU resources
- Early diagnostic signatures

Examples:
- `board_early_signature()`
- `board_led_on()`, `board_led_off()`, `board_led_toggle()`

---

### 4. Init layer (`init_*`)

Owns:
- One-time hardware bring-up policy

Examples:
- `init_clock()` — system clock policy
- `init_board()` — board GPIO bring-up

Each init unit:
- Does exactly one job
- Has no side effects outside its contract

---

### 5. Runtime layer (`runtime.*`)

Owns:
- Time base (SysTick)
- Millisecond delays
- Interrupt policy wrappers

Does **not**:
- Touch RCC, GPIO, or board details directly

---

### 6. Application layer (`main.c`)

Owns:
- Control flow
- System sequencing
- Intent

Knows nothing about:
- Registers
- Addresses
- Pins
- MCU families

---

## `mcu.h` — Single-Point MCU Selection

All MCU-specific headers are routed through:

```c
#include "mcu.h"
```

Switching MCU families (e.g., STM32L4 → STM32N7) is intended to be a
**one-line change** inside `mcu.h`, followed by updating the corresponding
`mcu_<family>_baremetal.h` file.

---

## What Changed from Stage 2

- Clock bring-up moved out of `main`
- Board GPIO configuration isolated
- Architecture vs MCU responsibilities separated
- Duplicate register definitions eliminated
- `main.c` reduced to orchestration only

Behavior is intentionally identical to Stage 2.

---

## What Stage 3 Enables

- Clean comparison across STM32 families
- Controlled experiments on footprint and startup cost
- Incremental addition of peripherals without architectural drift
- Future Rust/C comparisons on an identical hardware contract

---

## Next Stage Preview (Stage 4)

- Swap to a different STM32 device (e.g., STM32N7)
- Validate architecture portability
- Introduce additional peripherals under the same layering rules

Stage 3 is complete when this structure is stable and documented.
