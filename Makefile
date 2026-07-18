
# Toolchain: you need arm-none-eabi-gcc installed (a cross-compiler that
# targets ARM chips, not your host machine).
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

# -mcpu=cortex-m4      : target the M4 core (STM32F4 series)
# -mthumb              : use Thumb instruction set (all Cortex-M code does)
# -mfloat-abi=soft     : no hardware float in this tiny program
# -ffreestanding        : no hosted environment (no OS, no libc startup)
# -nostdlib            : don't link the standard library or its _start
# -Wl,--gc-sections     : strip unused code/data to keep binary small
# -Wl,-T,link.ld        : use our custom linker script
CFLAGS  = -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -ffreestanding \
          -nostdlib -O2 -Wall -Wextra
LDFLAGS = -Wl,-T,link.ld -Wl,--gc-sections -Wl,-Map=blink.map

SRCS = main.c startup.c
ELF  = blink.elf
BIN  = blink.bin

all: $(BIN)

$(ELF): $(SRCS) link.ld
	$(CC) $(CFLAGS) $(SRCS) $(LDFLAGS) -o $(ELF)
	$(SIZE) $(ELF)

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $(ELF) $(BIN)

# Flash over ST-Link using st-flash (part of the stlink-tools package).
# If you use OpenOCD or STM32CubeProgrammer instead, swap this line.
flash: $(BIN)
	st-flash write $(BIN) 0x08000000

clean:
	rm -f $(ELF) $(BIN) blink.map

.PHONY: all flash clean
