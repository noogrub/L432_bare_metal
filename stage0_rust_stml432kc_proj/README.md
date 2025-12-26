# Stage0 Rust Blink — STM32L432KC (fair-compare scaffold)

## Build
```bash
rustup target add thumbv7em-none-eabihf
cargo build --release
```

## Artifacts
ELF:
`target/thumbv7em-none-eabihf/release/stage0_rust_blink`

Map:
`target/stage0_rust_blink.map`

## Flash
```bash
arm-none-eabi-objcopy -O binary   target/thumbv7em-none-eabihf/release/stage0_rust_blink stage0.bin

st-flash write stage0.bin 0x08000000
```

## Size
```bash
arm-none-eabi-size target/thumbv7em-none-eabihf/release/stage0_rust_blink
arm-none-eabi-size -A target/thumbv7em-none-eabihf/release/stage0_rust_blink
```
