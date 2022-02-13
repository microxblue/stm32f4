OUTDIR   := out
PRJDIR   := $(shell pwd)
EMDDIR   := $(shell realpath $(PRJDIR)/..)
SHELLDIR := $(EMDDIR)/Shell
SDKDIR   := $(SHELLDIR)/lib
PREFIX   := arm-none-eabi-
CC       := $(PREFIX)gcc
LD       := $(PREFIX)ld
AS       := $(PREFIX)gcc -x assembler-with-cpp
AR       := $(PREFIX)ar
GDB      := $(PREFIX)gdb
OBJCOPY  := $(PREFIX)objcopy
OBJDUMP  := $(PREFIX)objdump
ELFSIZE  := $(PREFIX)size
LINKLD   := $(OUTDIR)/link.ld
PYSTLINK := python "$(EMDDIR)/Tools/pystlink/pystlink.py"
STMTALK  := python "$(EMDDIR)/Tools/stmtalk.py"


MODULE_BASE  := 0x20004000
MODULE_ENTRY := 0x20004001

MODNAME  ?= test
NAME      = cmd_$(MODNAME)

LIBDIR= $(EMDDIR)/Lib
INCDIR= -I. \
        -I$(SHELLDIR) \
        -I$(SHELLDIR)/lib/Cmsis/Include \
        -I$(SHELLDIR)/lib/Cmsis/Device/ST/STM32F4xx/Include \
        -I$(SHELLDIR)/lib/Sys/inc \


# Define optimisation level here
CPU_FLG  = -mthumb -mcpu=cortex-m4 -Os
FPU_ABI  = hard
FPU_FLG  = -mfpu=fpv4-sp-d16 -mfloat-abi=$(FPU_ABI) -ffp-contract=off -fsingle-precision-constant
COM_FLG  = -fdata-sections -ffunction-sections -nostartfiles -Wall -Wno-unused-function
OPT_FLG  = -MMD -Os -fomit-frame-pointer -flto

ASFLAGS  = $(CPU_FLG)
CPFLAGS  = $(CPU_FLG) $(FPU_FLG) $(COM_FLG) $(OPT_FLG) $(INCDIR)
CPFLAGS += -DHSE_VALUE=8000000 -DSTM32F40_41xxx -DSTM32F407xx
LDFLAGS  = $(CPU_FLG) $(FPU_FLG) $(COM_FLG) -T$(LINKLD) -Wl,-Map=$(OUTDIR)/$(NAME).map,--cref,--no-warn-mismatch
LDFLAGS +=

SRC = $(wildcard *.c)

STD_OBJS=\
		$(OUTDIR)/stm32f4xx_rcc.o \
		$(OUTDIR)/stm32f4xx_adc.o \
		$(OUTDIR)/stm32f4xx_dma.o \
		$(OUTDIR)/stm32f4xx_tim.o \
		$(OUTDIR)/stm32f4xx_gpio.o \
		$(OUTDIR)/stm32f4xx_i2c.o \

OBJ=\
		$(OUTDIR)/$(NAME).o \
		$(STD_OBJS)

all: prebuild $(OUTDIR)/$(NAME).bin

-include $(wildcard $(OBJ:.o=.d))

$(OUTDIR)/$(NAME).bin: $(OUTDIR)/$(NAME).elf
	$(OBJDUMP) -h -S -C -r $< > $(OUTDIR)/$(NAME).s
	$(OBJCOPY) -O ihex   $< $(OUTDIR)/$(NAME).hex
	$(OBJCOPY) -O binary $< $(OUTDIR)/$(NAME).bin
	$(ELFSIZE) -A -x --format=Berkeley $(OUTDIR)/$(NAME).elf

$(OUTDIR)/$(NAME).elf : $(LINKLD)
$(OUTDIR)/$(NAME).elf : $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ)

$(OUTDIR)/%.o : $(PRJDIR)/%.c
	$(CC) -c $(CPFLAGS) $(CPPFLAGS) $< -o $@

$(OUTDIR)/%.o : $(SHELLDIR)/lib/Sys/src/%.c
	$(CC) -c $(CPFLAGS) $(CPPFLAGS) $< -o $@


$(LINKLD):  $(PRJDIR)/module.ld Makefile
	@python -c "with open('$(LINKLD)', 'wt') as out: \
             lines = [line.replace('@FLASHBASE', '$(MODULE_BASE)') for line in open('$(PRJDIR)/module.ld').readlines()]; \
             out.writelines(lines)"

run: $(OUTDIR)/$(NAME).bin
	$(STMTALK) $< $(MODULE_BASE) sram p
	$(STMTALK) "-go $(MODULE_ENTRY)"


prebuild:
	@[ -d $(OUTDIR) ] || mkdir $(OUTDIR)

clean:
	@rm -Rf $(OUTDIR)
