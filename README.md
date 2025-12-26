# L432_bare_metal

Bare-metal programming experiments on the **STM32L432KC (Cortex-M4F)**, focused on understanding
toolchain behavior, runtime cost, and architectural tradeoffs when writing *truly freestanding*
embedded code.

This repository is organized as a sequence of staged projects. Each stage has its own
`README.md` with detailed design notes, build instructions, and results.

## Project intent

- Build **minimal, auditable bare-metal firmware** without vendor HALs or hidden runtimes
- Compare **C vs Rust** memory footprint, startup cost, and control
- Develop a **portable mental model** for Cortex-M startup, linking, and execution
- Enable repeatable measurement of flash/RAM usage across stages
- Serve as a foundation for later TinyML and fault-aware edge experiments

## Stages

- **Stage 0 — Bare minimum bring-up**
  - First LED blink on bare metal
  - Establish toolchain, linker script, startup path
  - C and Rust variants  
  → `stage0_c_stml432kc_project/`  
  → `stage0_rust_stml432kc_proj/`

- **Stage 1 — Structured bare metal**
  - Clean separation of startup, main, and build metadata
  - Explicit control of clocks, GPIO, and reset behavior
  - Still no runtime or syscalls  
  → `stage1_c_stml432kc_project/`

- **Stage 2 — Runtime refactor**
  - Refactor initialization logic out of `main()`
  - Prepare for multiple runtime “flavors”
  - Maintain freestanding constraints  
  → `stage2_c_stml432kc_project/`

- **Stage 3 — Modular initialization (in progress)**
  - Introduce explicit `init_*` components
  - Improve portability across STM32 devices
  - Foundation for feature growth without runtime creep  
  → `stage3_c_stml432kc_project/`

## Supporting material

- `STM32L432KC_C_vs_Rust_Experiment.md` — experiment overview
- `stm_32_l_432_kc_bare_metal_c_vs_rust_experiment_design.md` — design rationale
- `CvsRust.txt` — notes and comparisons
- `STLINK_WSL_Flashing_Checklist.md` — flashing/debug workflow
- `bare_metal_default_ioc.png` — reference CubeMX configuration (for comparison only)

---

**Status:** active development  
**Philosophy:** clarity over convenience, control over abstraction
