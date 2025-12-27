# L432_bare_metal

Bare-metal programming experiments on STM32 microcontrollers, focused on understanding
**what actually happens between reset and `main()`** when writing *truly freestanding*
embedded code — with no vendor HALs, no CMSIS runtime crutches, and no unexamined assumptions.

Although the project began on the **STM32L432KC (Cortex-M4F)**, it now explicitly includes
successful **cross-device bring-up** on other STM32 families (notably STM32F3), validating
the portability of both the code structure *and* the debugging methodology.

---

## Project intent

- Build **minimal, auditable bare-metal firmware** without vendor HALs or hidden runtimes
- Understand **startup, linking, and execution** at the level of actual silicon behavior
- Develop a **portable mental model** for Cortex-M bring-up across STM32 families
- Compare **C vs Rust** memory footprint, startup cost, and control
- Enable repeatable measurement of flash/RAM usage across stages
- Serve as a foundation for later TinyML, fault-aware, and reliability experiments

---

## Repository organization

This repository is organized as a sequence of **staged projects**.
Each stage has its own `README.md` with detailed design notes, build instructions,
and results.

The stages are intentionally incremental: each adds structure or capability
*without hiding behavior*.

---

## Stages

### **Stage 0 — Bare minimum bring-up**
- First LED blink on bare metal
- Establish toolchain, linker script, startup path
- C and Rust variants
- No runtime, no interrupts

→ `stage0_c_stml432kc_project/`  
→ `stage0_rust_stml432kc_proj/`

---

### **Stage 1 — Structured bare metal**
- Clean separation of startup, `main()`, and build metadata
- Explicit control of clocks, GPIO, and reset behavior
- Still no runtime or syscalls

→ `stage1_c_stml432kc_project/`

---

### **Stage 2 — Runtime refactor**
- Refactor initialization logic out of `main()`
- Prepare for multiple runtime “flavors”
- Maintain freestanding constraints
- Clearer boundaries between board, MCU, and runtime logic

→ `stage2_c_stml432kc_project/`

---

### **Stage 3 — Modular initialization & cross-MCU bring-up**
- Explicit `init_*` components (`init_clock`, `init_board`, runtime init)
- Board-level abstraction without HALs
- Verified bring-up on **multiple STM32 families**
- Demonstrated port from STM32L4 → STM32F3 using:
  - empirical RCC clock-gate discovery
  - GPIO base validation via read/write stickiness
  - fault decoding (BusFault / UsageFault)
  - linker SRAM correction from silicon behavior
  - explicit SysTick ownership

→ `stage3_c_stml432kc_project/`  
→ `stage3b_c_stm32f3discovery_project/`

---

## The “hard-way” bare-metal methodology (recommended reading)

During Stage 3 bring-up, we developed and validated a **repeatable, silicon-first
debugging methodology** for unknown or partially-documented STM32 boards.

Rather than trusting documentation blindly, the approach:

- treats register addresses and bitfields as *hypotheses*,
- confirms them by **reading and writing live silicon** via OpenOCD + GDB,
- uses fault status registers, stack inspection, and register write-stickiness
  as primary diagnostic tools,
- resets between experiments to avoid polluted state,
- and only then encodes results into project headers.

This methodology is documented here:

→ **`BareMetal_MysterySTM32_Methodology.md`**

If you ever find yourself thinking *“this should work, but it doesn’t”*, start there.

---

## Supporting material

- `BareMetal_MysterySTM32_Methodology.md` — **silicon-first bring-up workflow**
- `STM32L432KC_C_vs_Rust_Experiment.md` — experiment overview
- `stm_32_l_432_kc_bare_metal_c_vs_rust_experiment_design.md` — design rationale
- `CvsRust.txt` — notes and comparisons
- `STLINK_WSL_Flashing_Checklist.md` — flashing/debug workflow
- `bare_metal_default_ioc.png` — reference CubeMX configuration (for comparison only)

---

## Status

**Active development**

Stage 3 established a stable, portable bare-metal foundation.
Future work will build upward (TinyML, fault awareness, edge reliability)
without sacrificing the clarity achieved here.

---

## Philosophy

**Clarity over convenience.**  
**Control over abstraction.**  
**Trust the silicon — verify everything else.**

