# Bare-metal STM32 Cortex-M defaults

set architecture arm
set endian little
set confirm off

# Don’t ask before loading symbols
set auto-load safe-path /

# Useful Cortex-M defaults
set print asm-demangle on
set print pretty on

# automate the connection to openocd
target extended-remote localhost:3333
monitor reset run
