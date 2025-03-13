PROJECT = bench_client
BUILD_DIR = bin/bench_client/
BUILD_ROOT = bin

KEM ?= ml-kem-512
IMPL ?= m4fstack
PROTO ?= bench_OCAKE
OPT ?= -O3
CSTD ?= -std=gnu99

DEVICE=stm32l4r5zi
OOCD_FILE = board/st_nucleo_l4.cfg
OOCD = openocd -s tcl
OPENCM3_DIR=./libopencm3

# Tool paths
PREFIX	?= arm-none-eabi-
CC 		= $(PREFIX)gcc
CXX		= $(PREFIX)g++
LD		= $(PREFIX)gcc
OBJCOPY	= $(PREFIX)objcopy
OBJDUMP	= $(PREFIX)objdump

# Sources
rwildcard =$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
CFILES := $(wildcard src/*.c)
CFILES += $(wildcard src/proto/$(PROTO)/*.c)

COMMOM_CFILES := $(wildcard src/crypto_kem/common/*.c)
COMMOM_SFILES := $(wildcard src/crypto_kem/common/*.S)

KEM_CFILES := $(wildcard src/crypto_kem/$(KEM)/$(IMPL)/*.c)
KEM_SFILES := $(wildcard src/crypto_kem/$(KEM)/$(IMPL)/*.S) 
KEM_sFILES := $(wildcard src/crypto_kem/$(KEM)/$(IMPL)/*.s)

# Includes
PROJ_INC 	:= ./src
COMMOM_INC 	:= ./src/crypto_kem/common
KEM_INC 	:= ./src/crypto_kem/$(KEM)
PROTO_INC 	:= ./src/proto/$(PROTO)
INCLUDES 	+= $(patsubst %,-I%, $(PROJ_INC) $(COMMOM_INC) $(KEM_INC)/$(IMPL) $(PROTO_INC) /usr/local/include)

# Objects
OBJS = $(CFILES:%.c=$(BUILD_DIR)/%.o)
OBJS += $(COMMOM_CFILES:%.c=$(BUILD_DIR)/%.o)
OBJS += $(COMMOM_SFILES:%.S=$(BUILD_DIR)/%.o)
OBJS += $(KEM_CFILES:%.c=$(BUILD_DIR)/%.o)
OBJS += $(KEM_SFILES:%.S=$(BUILD_DIR)/%.o)
OBJS += $(KEM_sFILES:%.s=$(BUILD_DIR)/%.o)

# Linker
LIBOPENCM3 = opencm3_stm32l4
LDSCRIPT = generated.stm32l4r5zi.ld

# Bins
GENERATED_BINS = $(PROJECT).elf

#####
# You shouldn't have to edit anything below here.
#####

Q		:= @
NULL	:= 2>/dev/null

# Flags
ARCH_FLAGS 	= -mcpu=cortex-m4 -mthumb \
			  -mfloat-abi=hard -mfpu=fpv4-sp-d16

CPPFLAGS += -MD -DSTM32L4 -DSTM32L4R5ZI $(INCLUDES) 

CFLAGS += -g3 $(OPT) $(CSTD) $(ARCH_FLAGS)

LDFLAGS += -T$(LDSCRIPT) -L/usr/local/lib -nostartfiles
LDFLAGS += $(ARCH_FLAGS)
LDFLAGS += -specs=nano.specs
LDFLAGS += -Wl,--gc-sections

LDLIBS += -l$(LIBOPENCM3)
LDLIBS += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

all: $(PROJECT).elf
flash: $(PROJECT).flash
debug: $(PROJECT).debug

# Need a special rule to have a bin dir
$(BUILD_DIR)/%.o: %.c
	@printf "  CC\t$<\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

$(BUILD_DIR)/%.o: %.S
	@printf "  AS\t$<\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

$(BUILD_DIR)/%.o: %.s
	@printf "  AS\t$<\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

$(PROJECT).elf: $(OBJS)
	@printf "  LD\t$@\n"
	$(Q)$(LD) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

%.flash: %.elf
	@printf "  FLASH\t$<\n"
	$(Q)(echo "halt; program $(realpath $(*).elf) verify reset" | nc -4 localhost 4444 2>/dev/null) || \
		$(OOCD) -f $(OOCD_FILE) \
		-c "program $(*).elf verify reset exit" \
		$(NULL)

%.debug: %.elf
	@printf "  FLASH\t$<\n"
	$(Q)(echo "halt; program $(realpath $(*).elf) verify reset" | nc -4 localhost 4444 2>/dev/null) || \
		$(OOCD) -f $(OOCD_FILE) \
		-c "program $(*).elf verify reset" \
		$(NULL)

clean:
	rm -rf $(BUILD_ROOT) $(GENERATED_BINS)

.PHONY: all clean flash debug
-include $(OBJS:.o=.d)
