include config.mk

SRC=$(foreach m, $(MODULES), $(wildcard $(m)/src/*.c))
INCLUDES=$(foreach m, $(MODULES), -I$(m)/inc)
_DEFINES=$(foreach m, $(DEFINES), -D$(m))
OBJECTS=$(SRC:.c=.o)
DEPS=$(SRC:.c=.d)
LDSCRIPT=ciaa_lpc4337.ld

ARCH_FLAGS=-mcpu=cortex-m4 -mthumb

TARGET=$(APP).elf
TARGET_BIN=$(basename $(TARGET)).bin
TARGET_LST=$(basename $(TARGET)).lst
TARGET_MAP=$(basename $(TARGET)).map

ifeq ($(USE_FPU),y)
ARCH_FLAGS+=-mfloat-abi=hard -mfpu=fpv4-sp-d16
endif

CFLAGS=$(ARCH_FLAGS) $(INCLUDES) $(_DEFINES) -ggdb3 -O$(OPT) -ffunction-sections -fdata-sections
LDFLAGS=$(ARCH_FLAGS) -T$(LDSCRIPT) -nostartfiles -Wl,-gc-sections -Wl,-Map=$(TARGET_MAP) -Wl,--cref

ifeq ($(USE_NANO),y)
LDFLAGS+=--specs=nano.specs
endif

ifeq ($(SEMIHOST),y)
LDFLAGS+=--specs=rdimon.specs
endif

CROSS=arm-none-eabi-
CC=$(CROSS)gcc
LD=$(CROSS)gcc
SIZE=$(CROSS)size
LIST=$(CROSS)objdump -xdS
OBJCOPY=$(CROSS)objcopy
GDB=$(CROSS)gdb
OOCD=openocd

OOCD_SCRIPT?=ciaa-nxp.cfg

ifeq ($(VERBOSE),y)
Q=
else
Q=@
endif

all: $(TARGET) $(TARGET_BIN) $(TARGET_LST) size

-include $(DEPS)

%.o: %.c
	@echo CC $<
	$(Q)$(CC) -MMD $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJECTS)
	@echo LD $@
	$(Q)$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

$(TARGET_BIN): $(TARGET)
	@echo BIN
	$(Q)$(OBJCOPY) -v -O binary $< $@

$(TARGET_LST): $(TARGET)
	@echo LIST
	$(Q)$(LIST) $< > $@

size: $(TARGET)
	$(Q)$(SIZE) $<

program: $(TARGET_BIN)
	@echo PROG
	$(Q)$(OOCD) -f $(OOCD_SCRIPT) \
		-c "init" \
		-c "halt 0" \
		-c "flash write_image erase unlock $< 0x1A000000 bin" \
		-c "reset run" \
		-c "shutdown" 2>&1

debug: $(TARGET_BIN)
	@echo DEBUG
	$(Q)$(OOCD) -f $(OOCD_SCRIPT) 2>&1

clean:
	@echo CLEAN
	$(Q)rm -fR $(OBJECTS) $(TARGET) $(TARGET_BIN) $(TARGET_LST) $(DEPS)

.PHONY: all size clean program
