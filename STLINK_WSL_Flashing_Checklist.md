# ST-LINK over WSL (Windows ↔ Debian/Linux) — Bare‑Metal Flashing Checklist
_Last verified: 2025‑12‑20_

This checklist documents the **exact, reproducible command‑line steps** required to flash an STM32 Nucleo board (ST‑LINK/V2‑1) from **Linux running under WSL2** using `stlink-tools`.

It is intentionally boring, linear, and explicit so it can be reused on:
- another Windows machine,
- another WSL distro,
- or a native Linux system (Mint, Debian, Ubuntu).

---

## 0. What this enables (mental model)

- **Windows** owns the USB host controller.
- **usbipd‑win** selectively forwards a USB device into WSL.
- **Linux (WSL)** talks to the ST‑LINK as if it were locally attached.
- **stlink-tools** programs flash over SWD.
- No IDEs, no HALs, no hidden automation.

---

## 1. Requirements — Windows side

### 1.1 Operating system
- Windows 11 (recommended) or Windows 10 with WSL2 support.

### 1.2 WSL installed
Verify in PowerShell:
```powershell
wsl --status
```

You should see WSL version 2 enabled.

### 1.3 usbipd‑win installed (one‑time)
Preferred (Winget):
```powershell
winget install usbipd
```

Fallback:
- Download installer from:
  https://github.com/dorssel/usbipd-win/releases
- Install the `.msi`

Verify:
```powershell
usbipd --version
```

---

## 2. Requirements — Linux side (WSL or native Linux)

### 2.1 Required packages
Debian / Ubuntu / Mint:

```bash
sudo apt update
sudo apt install -y   build-essential   gcc-arm-none-eabi   binutils-arm-none-eabi   make   stlink-tools   usbutils
```

### 2.2 Verify tools
```bash
arm-none-eabi-gcc --version
st-info --version || true
st-flash --version || true
```

---

## 3. One‑time USB binding (Windows host)

### 3.1 Plug in the Nucleo board
Connect via USB. The ST‑LINK will enumerate.

### 3.2 List USB devices (normal PowerShell)
```powershell
usbipd list
```

Look for:
```
ST-Link Debug, USB Mass Storage Device, STMicroelectronics
```

Note the **BUSID**, e.g. `5-3`.

### 3.3 Bind device (Administrator PowerShell)
⚠️ This must be run **as Administrator**.

Open **Administrator PowerShell**, then:

```powershell
usbipd bind --busid <BUSID>
```

Example:
```powershell
usbipd bind --busid 5-3
```

Expected result:
- Silent success (no error).

Binding is **persistent** across reboots.

---

## 4. Attach device to WSL (per session)

### 4.1 Attach (Administrator PowerShell)
```powershell
usbipd attach --wsl --busid <BUSID>
```

Expected output:
- Mentions the WSL distribution being used.
- No errors.

The ST‑LINK LED may blink or re‑enumerate. This is normal.

---

## 5. Verify from Linux (WSL)

### 5.1 Confirm USB visibility
```bash
lsusb | grep -i st
```

Expected:
```
STMicroelectronics ST-LINK/V2.1
```

### 5.2 Confirm probe works
```bash
st-info --probe
```

Expected:
- ST‑LINK version
- Chip ID
- Flash size
- SRAM size

Example:
```
Found 1 stlink programmers
flash: 262144
sram: 49152
chipid: 0x0435
descr: L43x/L44x
```

At this point, flashing is ready.

---

## 6. Flashing firmware (Linux side)

### 6.1 Build firmware
Produce a binary or ELF:
```bash
make
```

Typical outputs:
- `firmware.elf`
- `firmware.bin`

### 6.2 Flash binary
```bash
st-flash write firmware.bin 0x08000000
```

### 6.3 Optional: erase flash
```bash
st-flash erase
```

---

## 7. Common failure modes (quick fixes)

### `st-info --probe` finds 0 devices
- Device not attached to WSL:
  ```powershell
  usbipd attach --wsl --busid <BUSID>
  ```
- USB cable unplugged / power issue.

### `usbipd` command not found
- usbipd‑win not installed.
- Reopen PowerShell after installation.

### Permission denied on bind
- PowerShell not running as Administrator.

---

## 8. Notes for native Linux (no Windows)

On Linux Mint / Debian / Ubuntu **without Windows**:
- Skip all `usbipd` steps.
- Ensure user has USB access (often automatic).
- `lsusb` → `st-info --probe` → `st-flash`.

---

## 9. Why this checklist exists

This workflow:
- is deterministic,
- avoids IDE state,
- scales to scripts and CI,
- keeps the mental model aligned with the hardware.

It is intentionally verbose so you do **not** have to remember it.

---

**Status:** Verified on STM32L432KC (NUCLEO‑L432KC), Windows + WSL Debian, Dec 2025.
