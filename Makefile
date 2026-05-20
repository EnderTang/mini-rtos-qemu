PROJECT := mini_rtos
BUILD_DIR := build

CC := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
GDB := gdb-multiarch
QEMU := qemu-system-arm

CPU_FLAGS := -mcpu=cortex-m3 -mthumb
CFLAGS := $(CPU_FLAGS) -std=c99 -Wall -Wextra -Werror -ffreestanding -fno-builtin \
          -fdata-sections -ffunction-sections -O2 -g3 \
          -Isrc
ASFLAGS := $(CPU_FLAGS) -g3
LDFLAGS := $(CPU_FLAGS) -nostdlib -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map \
           -T linker.ld

C_SOURCES := $(wildcard src/*.c)
ASM_SOURCES := $(wildcard asm/*.s)
OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SOURCES)) \
           $(patsubst %.s,$(BUILD_DIR)/%.o,$(ASM_SOURCES))

.PHONY: all run debug clean

all: $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/$(PROJECT).bin

$(BUILD_DIR)/$(PROJECT).elf: $(OBJECTS) linker.ld
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

run: all
	$(QEMU) -M lm3s6965evb -cpu cortex-m3 -kernel $(BUILD_DIR)/$(PROJECT).elf -nographic -serial mon:stdio

debug: all
	$(QEMU) -M lm3s6965evb -cpu cortex-m3 -kernel $(BUILD_DIR)/$(PROJECT).elf -nographic -serial mon:stdio -S -gdb tcp::3333

gdb: all
	$(GDB) $(BUILD_DIR)/$(PROJECT).elf -ex "target remote :3333"

clean:
	rm -rf $(BUILD_DIR)
