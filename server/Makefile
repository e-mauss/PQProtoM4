PROJECT = bench_server
BUILD_DIR = bin/bench_server/
BUILD_ROOT = bin

KEM ?= ml-kem-512
PROTO ?= bench_OCAKE
OPT ?= -O2
CSTD ?= -std=gnu99
V?=0

# Tool paths
CC	= gcc
CXX	= g++
LD	= gcc
OBJCOPY	= objcopy
OBJDUMP	= objdump

# Sources
rwildcard =$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
CFILES := $(wildcard src/*.c)
CFILES += $(wildcard src/proto/$(PROTO)/*.c)
CFILES += $(wildcard src/crypto_kem/$(KEM)/*.c)
CFILES += $(wildcard src/crypto_kem/common/*.c)



# Includes
INCLUDES += $(patsubst %,-I%, . ./src ./src/proto/$(PROTO) src/crypto_kem/$(KEM) src/crypto_kem/common)

# Objects
OBJS = $(CFILES:%.c=$(BUILD_DIR)/%.o)

# Bins
GENERATED_BINS = $(PROJECT)

#####
# You shouldn't have to edit anything below here.
#####

ifeq ($(V),0)
Q	:= @
NULL	:= 2>/dev/null
endif

# Flags
TGT_CFLAGS += -g -MMD

default: $(PROJECT)
all: default

$(BUILD_DIR)/%.o: %.c
	@printf "  CC\t$<\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(TGT_CFLAGS) $(INCLUDES) -o $@ -c $<

.PRECIOUS: $(PROJECT) $(OBJS)

$(PROJECT): $(OBJS)
	@printf "  LD\t$@\n"
	$(Q)$(LD) $(OBJS) -Wall -o $@

clean:
	rm -rf $(BUILD_ROOT) $(GENERATED_BINS)

.PHONY: all clean
-include $(OBJS:.o=.d)
