# L432_bare_metal — System Setup (Elonius / Debian-derived)

This repo is intentionally bare-metal: no STM32 HAL, no CMSIS device headers, no libc runtime.
Tooling is CLI-first and reproducible.

## Required packages

Install the cross toolchain, OpenOCD, and STLink tools:

```bash
sudo apt update
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi openocd stlink-tools gdb-multiarch
```

Verify:

```bash
arm-none-eabi-gcc --version
openocd --version
st-flash --version
gdb-multiarch --version
```

## GDB: project-local defaults (recommended)

We use `gdb-multiarch` (Ubuntu/Debian packaging). We do NOT rely on `arm-none-eabi-gdb`.

Each stage directory may include a project-local `.gdbinit` to:
- set `architecture arm`
- connect to OpenOCD
- optionally `monitor reset run`

GDB blocks auto-loading local `.gdbinit` unless explicitly trusted.

Trust only this repo tree:

Edit (or create):

`~/.config/gdb/gdbinit`

Add:

```gdb
add-auto-load-safe-path /home/jwb/STM32CubeIDE/L432/L432_bare_metal
```

Do NOT set a global `set auto-load safe-path /` unless you understand the security implications.

## Canonical build (stage directory)

```bash
make clean
make
```

Artifacts:
- `build/blink.elf`
- `build/blink.bin`
- `build/blink.map`

## Flashing options

### Option A: st-flash (simple)
```bash
st-flash write build/blink.bin 0x08000000
```

### Option B: OpenOCD program + verify + reset + run (recommended)
```bash
openocd -f interface/stlink.cfg -f target/stm32l4x.cfg   -c "program build/blink.elf verify reset run exit"
```

## Debugging with OpenOCD + GDB

Terminal 1:
```bash
openocd -f interface/stlink.cfg -f target/stm32l4x.cfg
```

Terminal 2 (stage directory):
```bash
gdb-multiarch build/blink.elf
```

If your stage `.gdbinit` is configured, GDB will auto-connect and may auto-run.

Clean exit:
- quit GDB: `quit`
- stop OpenOCD: `Ctrl+C`

## Notes

- If firmware does not auto-start after flashing, prefer the OpenOCD `reset run` workflow.
- This repo keeps MCU- and architecture-specific definitions in explicit headers (e.g. `mcu_*.h`, `arch_*.h`) for inspectability.
