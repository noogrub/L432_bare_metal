# Hand-rolled STM32L432KC Stage 0 blink (no HAL)
# John/Gigi experiment scaffolding — keep it simple and inspectable.

TARGET     := blink
BUILD_DIR  := build

CC         := arm-none-eabi-gcc
OBJCOPY    := arm-none-eabi-objcopy
SIZE       := arm-none-eabi-size

CPUFLAGS   := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

CFLAGS     := $(CPUFLAGS) -std=c11 -O2 -g3 -ffreestanding -fno-builtin \
              -Wall -Wextra -Werror -Wno-unused-parameter

LDFLAGS    := $(CPUFLAGS) -nostartfiles -Wl,--gc-sections \
              -Wl,-Map=$(BUILD_DIR)/$(TARGET).map \
              -T linker.ld

SRCS       := startup.s main.c

OBJS       := $(addprefix $(BUILD_DIR)/,$(SRCS:.c=.o))
OBJS       := $(OBJS:.s=.o)

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin size

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	$(CC) $(CPUFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJS) linker.ld
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

size: $(BUILD_DIR)/$(TARGET).elf
	$(SIZE) $<
	@echo
	$(SIZE) -A $<

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean size
