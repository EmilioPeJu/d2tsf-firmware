TOP := $(CURDIR)
PROJECT = evg-ts-feeder
BUILD_DIR = $(TOP)/build
CROSS_COMPILE = arm-none-eabi-

-include $(TOP)/CONFIG

CC = $(CROSS_COMPILE)gcc

ifdef BINUTILS_DIR
    PATH := $(BINUTILS_DIR):$(PATH)
endif

RELEASE = $(shell git describe --tags --dirty)

ifeq ($(RELEASE),)
	OUTFILE_PREFIX := $(PROJECT)
else
	OUTFILE_PREFIX := $(PROJECT)-$(RELEASE)
endif

CPPFLAGS += -DRELEASE='"$(RELEASE)"'

CFLAGS += -mcpu=cortex-m4
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb
CFLAGS += -std=gnu11
CFLAGS += -g3
CFLAGS += -O0
CFLAGS += -DUSE_HAL_DRIVER
CFLAGS += -DSTM32F411xE
CFLAGS += -I$(TOP)/Core/Inc
CFLAGS += -I$(TOP)/Drivers/CMSIS/Include
CFLAGS += -I$(TOP)/Drivers/STM32F4xx_HAL_Driver/Inc
CFLAGS += -I$(TOP)/Drivers/CMSIS/Device/ST/STM32F4xx/Include
CFLAGS += -I$(TOP)/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
CFLAGS += -I$(TOP)/Src
CFLAGS += -fstack-usage
CFLAGS += -ffunction-sections
CFLAGS += -Wall
CFLAGS += -Werror
CFLAGS += -Wextra
#CFLAGS += -Wmissing-declarations
#CFLAGS += -Wmissing-prototypes
#CFLAGS += -Wstrict-prototypes
CFLAGS += -Wno-unused-parameter
CFLAGS += -fdata-sections
CFLAGS += -MMD -MP
CFLAGS += --specs=nano.specs

ASFLAGS += -mcpu=cortex-m4
ASFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb
ASFLAGS += --specs=nano.specs
ASFLAGS += -g3

LINKER_SCRIPT = $(TOP)/STM32F411RETX_FLASH.ld

LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb
LDFLAGS += --specs=nano.specs
LDFLAGS += -T"$(LINKER_SCRIPT)"
LDFLAGS += --specs=nosys.specs
LDFLAGS += -Wl,-Map="$@.map"
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -static
LDFLAGS += -Wl,--start-group -lc -lm -Wl,--end-group
LDFLAGS += -Wl,--print-memory-usage

ifdef DEBUG
    CFLAGS += -DDEBUG
    CFLAGS += -O0
endif

RM := rm -rf
EXECUTABLES = $(BUILD_DIR)/$(OUTFILE_PREFIX).elf
OBJCOPY_BIN += $(BUILD_DIR)/$(OUTFILE_PREFIX).bin
OBJCOPY_HEX += $(BUILD_DIR)/$(OUTFILE_PREFIX).hex
OBJDUMP_LIST += $(BUILD_DIR)/$(OUTFILE_PREFIX).list

SRC_DIRS += Core/Src
SRC_DIRS += Drivers/STM32F4xx_HAL_Driver/Src
SRC_DIRS += Src

SRCS += $(foreach dir,$(SRC_DIRS),$(wildcard $(TOP)/$(dir)/*.c))
OBJS = $(patsubst $(TOP)/%,$(BUILD_DIR)/%,$(SRCS:.c=.o))
DEPS = $(OBJS:.o=.d)

STARTUP_SRC = $(TOP)/Core/Startup/startup_stm32f411retx.s
STARTUP_OBJ = $(patsubst $(TOP)/%,$(BUILD_DIR)/%,$(STARTUP_SRC:.s=.o))
OBJS += $(STARTUP_OBJ)

all: $(EXECUTABLES) $(OBJDUMP_LIST) $(OBJCOPY_BIN) $(OBJCOPY_HEX)

$(BUILD_DIR)/%.o: $(TOP)/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o "$@" "$<"

# Include dependencies
-include $(DEPS)

$(STARTUP_OBJ): $(STARTUP_SRC)
	$(CC) -c -x assembler-with-cpp $(ASFLAGS) -o "$@" "$<"

$(BUILD_DIR)/$(OUTFILE_PREFIX).elf: directories $(OBJS) $(LINKER_SCRIPT)
	$(CC) $(LIBS) $(LDFLAGS) -o "$@" $(OBJS)

$(BUILD_DIR)/$(OUTFILE_PREFIX).list: $(EXECUTABLES)
	$(CROSS_COMPILE)objdump -h -S $(EXECUTABLES) > "$@"

$(BUILD_DIR)/$(OUTFILE_PREFIX).hex: $(EXECUTABLES)
	$(CROSS_COMPILE)objcopy  -O ihex "$<" "$@"

$(BUILD_DIR)/$(OUTFILE_PREFIX).bin: $(EXECUTABLES)
	$(CROSS_COMPILE)objcopy  -O binary "$<" "$@"

directories:
	mkdir -p $(foreach dir,$(SRC_DIRS),$(BUILD_DIR)/$(dir))
	mkdir -p $(BUILD_DIR)/Core/Startup

clean:
	-$(RM) $(BUILD_DIR)

.PHONY: all clean directories
.SECONDARY:
