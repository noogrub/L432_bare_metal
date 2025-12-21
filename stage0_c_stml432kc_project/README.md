# STM32L432KC C vs Rust Metrics — Stage 0 (Bare-metal LED blink)

Target: **NUCLEO-L432KC** (STM32L432KC, Cortex-M4F, 256 KB flash, 48 KB SRAM)

Stage 0 goal:
- Prove we can build and flash a **hand-rolled bare-metal** program (no HAL, no CMSIS startup files).
- Capture baseline memory sections for the experiment table.

LED mapping (board):
- User LED **LD3** is connected to **PB3** (Arduino D13).  
  Source: Nucleo-32 user manual, Table “Arduino nano connector” / pin mapping.  

## Build prerequisites (Debian / WSL)
- `arm-none-eabi-gcc`
- `make`
- `stlink-tools` (for `st-flash` / `st-info`)
- Optional: `binutils-arm-none-eabi` (for `arm-none-eabi-objdump`, `arm-none-eabi-readelf`)

## One-time flash/USB setup
Use the checklist you already saved:
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

## Metrics capture (for your table)
### Size summary (quick)
```bash
arm-none-eabi-size build/blink.elf
```

### Size by section (more useful)
```bash
arm-none-eabi-size -A build/blink.elf
```

### Confirm section placement and addresses
```bash
arm-none-eabi-readelf -S build/blink.elf
arm-none-eabi-objdump -h build/blink.elf
```

Notes:
- Your “total flash” will typically be `.text + .rodata + .data` plus any fixed overhead you deliberately include
  (vector table, etc.). In this Stage 0 project, the vector table is part of `.isr_vector` which ends up in flash.

