#![no_std]
#![no_main]

use core::arch::{asm, global_asm};
use core::ptr::{read_volatile, write_volatile};

// -----------------------------
// Panic: abort-style loop (no formatting)
#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {
        unsafe { asm!("nop", options(nomem, nostack, preserves_flags)); }
    }
}

// -----------------------------
// Linker symbols (match linker.ld)
extern "C" {
    static mut _sidata: u32;
    static mut _sdata: u32;
    static mut _edata: u32;
    static mut _sbss: u32;
    static mut _ebss: u32;
    static mut _estack: u32;
}

// Firmware ID breadcrumbs (kept via linker KEEP(*(.build_id))).
const BUILD_ID_BYTES: &[u8] = include_bytes!(concat!(env!("OUT_DIR"), "/build_id.bin"));
const BUILD_ID_LEN: usize = BUILD_ID_BYTES.len();

#[link_section = ".build_id"]
#[used]
static BUILD_ID: [u8; BUILD_ID_LEN] = *include_bytes!(concat!(env!("OUT_DIR"), "/build_id.bin"));

// -----------------------------
// Vector table (minimal: SP + Reset). Section name matches linker KEEP.
global_asm!(
    r#"
    .section .isr_vector,"a",%progbits
    .word _estack
    .word Reset_Handler
"#
);


// -----------------------------
// Reset handler: init .data/.bss, then call main.
#[no_mangle]
unsafe extern "C" fn Reset_Handler() -> ! {
    // Copy .data from FLASH to RAM
    let mut src = core::ptr::addr_of_mut!(_sidata) as *const u32;
    let mut dst = core::ptr::addr_of_mut!(_sdata) as *mut u32;
    while (dst as usize) < (core::ptr::addr_of_mut!(_edata) as usize) {
        write_volatile(dst, read_volatile(src));
        dst = dst.add(1);
        src = src.add(1);
    }

    // Zero .bss
    let mut bss = core::ptr::addr_of_mut!(_sbss) as *mut u32;
    while (bss as usize) < (core::ptr::addr_of_mut!(_ebss) as usize) {
        write_volatile(bss, 0);
        bss = bss.add(1);
    }

    main()
}

// -----------------------------
// Minimal register access
#[inline(always)]
fn reg32(addr: u32) -> *mut u32 {
    addr as *mut u32
}

// Core SysTick (polling, no IRQ)
const SYST_CSR: u32 = 0xE000_E010;
const SYST_RVR: u32 = 0xE000_E014;
const SYST_CVR: u32 = 0xE000_E018;

// SCB VTOR
const SCB_VTOR: u32 = 0xE000_ED08;

// Cortex-M: disable IRQ
#[inline(always)]
unsafe fn irq_off() {
    asm!("cpsid i", options(nomem, nostack, preserves_flags));
}

// -----------------------------
// STM32L432 RCC + GPIOB + SYSCFG
const RCC_BASE: u32 = 0x4002_1000;
const RCC_CR: u32 = RCC_BASE + 0x00;
const RCC_ICSCR: u32 = RCC_BASE + 0x04;
const RCC_CFGR: u32 = RCC_BASE + 0x08;
const RCC_AHB2ENR: u32 = RCC_BASE + 0x4C;
const RCC_APB2ENR: u32 = RCC_BASE + 0x60;

// RCC bits
const RCC_CR_MSION: u32 = 1 << 0;
const RCC_CR_MSIRDY: u32 = 1 << 1;
const RCC_CR_PLLON: u32 = 1 << 24;
const RCC_CR_PLLRDY: u32 = 1 << 25;

// CFGR SW[1:0] system clock switch: 00=MSI
const RCC_CFGR_SW_MASK: u32 = 3 << 0;
const RCC_CFGR_SWS_MASK: u32 = 3 << 2;

// ICSCR MSI range: MSIRANGE[7:4]. Range 6 = 4 MHz on STM32L4.
const RCC_ICSCR_MSIRANGE_SHIFT: u32 = 4;
const RCC_ICSCR_MSIRANGE_MASK: u32 = 0xF << RCC_ICSCR_MSIRANGE_SHIFT;
const RCC_ICSCR_MSIRANGE_4MHZ: u32 = 6 << RCC_ICSCR_MSIRANGE_SHIFT;

// Enable clocks
const RCC_AHB2ENR_GPIOBEN: u32 = 1 << 1;
const RCC_APB2ENR_SYSCFGEN: u32 = 1 << 0;

// GPIOB
const GPIOB_BASE: u32 = 0x4800_0400;
const GPIOB_MODER: u32 = GPIOB_BASE + 0x00;
const GPIOB_ODR: u32 = GPIOB_BASE + 0x14;
const GPIOB_BSRR: u32 = GPIOB_BASE + 0x18;
const PB3_PIN: u32 = 3;

// SYSCFG for TRACESWO disable
const SYSCFG_BASE: u32 = 0x4001_0000;
const SYSCFG_CFGR1: u32 = SYSCFG_BASE + 0x00;
const SYSCFG_CFGR1_TRACESWO_DISABLE: u32 = 1 << 24;

// -----------------------------
// Deterministic clock bring-up: SYSCLK = MSI @ 4 MHz
unsafe fn clock_init_msi_4mhz() {
    // Ensure MSI on
    let cr = reg32(RCC_CR);
    write_volatile(cr, read_volatile(cr) | RCC_CR_MSION);
    while (read_volatile(cr) & RCC_CR_MSIRDY) == 0 {}

    // Set MSI range to 4 MHz
    let icscr = reg32(RCC_ICSCR);
    let v = read_volatile(icscr);
    write_volatile(
        icscr,
        (v & !RCC_ICSCR_MSIRANGE_MASK) | RCC_ICSCR_MSIRANGE_4MHZ,
    );

    // Force SYSCLK source to MSI (SW=00)
    let cfgr = reg32(RCC_CFGR);
    write_volatile(cfgr, read_volatile(cfgr) & !RCC_CFGR_SW_MASK);
    while (read_volatile(cfgr) & RCC_CFGR_SWS_MASK) != 0 {}

    // Disable PLL if left on
    if (read_volatile(cr) & RCC_CR_PLLON) != 0 {
        write_volatile(cr, read_volatile(cr) & !RCC_CR_PLLON);
        while (read_volatile(cr) & RCC_CR_PLLRDY) != 0 {}
    }
}

// -----------------------------
// Busy-wait delay (mirrors your C Stage0 style)
#[inline(never)]
fn delay_cycles(mut cycles: u32) {
    while cycles != 0 {
        unsafe { asm!("nop", options(nomem, nostack, preserves_flags)); }
        cycles -= 1;
    }
}

#[inline(never)]
fn delay_ms(mut ms: u32) {
    while ms != 0 {
        delay_cycles(4000);
        ms -= 1;
    }
}

unsafe fn gpio_init_pb3_led() {
    // Clocks
    write_volatile(reg32(RCC_AHB2ENR), read_volatile(reg32(RCC_AHB2ENR)) | RCC_AHB2ENR_GPIOBEN);
    write_volatile(reg32(RCC_APB2ENR), read_volatile(reg32(RCC_APB2ENR)) | RCC_APB2ENR_SYSCFGEN);

    // Disable SWO on PB3
    write_volatile(reg32(SYSCFG_CFGR1), read_volatile(reg32(SYSCFG_CFGR1)) | SYSCFG_CFGR1_TRACESWO_DISABLE);

    // PB3 output: MODER3 = 01
    let shift = PB3_PIN * 2;
    let moder = reg32(GPIOB_MODER);
    let mut m = read_volatile(moder);
    m &= !(3 << shift);
    m |=  1 << shift;
    write_volatile(moder, m);

    // Start LED OFF
    write_volatile(reg32(GPIOB_BSRR), 1 << (PB3_PIN + 16));
}

#[no_mangle]
pub extern "C" fn main() -> ! {
    unsafe {
        // VTOR explicitly (matches your C habit)
        write_volatile(reg32(SCB_VTOR), 0x0800_0000);

        // Deterministic startup state
        irq_off();
        write_volatile(reg32(SYST_CSR), 0);
        write_volatile(reg32(SYST_RVR), 0);
        write_volatile(reg32(SYST_CVR), 0);

        clock_init_msi_4mhz();
        gpio_init_pb3_led();

        // Signature: ON, then OFF, then blink
        write_volatile(reg32(GPIOB_BSRR), 1 << PB3_PIN);
        delay_ms(500);
        write_volatile(reg32(GPIOB_BSRR), 1 << (PB3_PIN + 16));
        delay_ms(250);

        loop {
            let odr = reg32(GPIOB_ODR);
            let v = read_volatile(odr) ^ (1 << PB3_PIN);
            write_volatile(odr, v);
            delay_ms(50);
        }
    }
}
