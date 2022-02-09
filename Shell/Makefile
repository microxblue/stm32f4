NAME     :=shell
OUTDIR   :=out
BINDIR   :=bin
PRJDIR   := $(shell pwd)
EMDDIR   := $(shell realpath $(PRJDIR)/..)
IMAGE    := $(OUTDIR)/$(NAME).bin

GETPC    := "print(hex(int.from_bytes(open('$(IMAGE)', 'rb').read()[4:8], 'little')))"
GETSP    := "print(hex(int.from_bytes(open('$(IMAGE)', 'rb').read()[0:4], 'little')))"
PYSTLINK := python "$(EMDDIR)/Tools/pystlink/pystlink.py"
STMTALK  := python "$(EMDDIR)/Tools/stmtalk.py"

USBADR  ?= .

PREFIX   = arm-none-eabi-
CC       = $(PREFIX)gcc
LD       = $(PREFIX)ld
AS       = $(PREFIX)gcc -x assembler-with-cpp
AR       = $(PREFIX)ar
GDB      = $(PREFIX)gdb
OBJCOPY  = $(PREFIX)objcopy
OBJDUMP  = $(PREFIX)objdump
ELFSIZE  = $(PREFIX)size
LINKLD   = $(OUTDIR)/link.ld
EMDDIR   = ..


RECOVERY     = 0
SRAM_BASE    = 0x20000000
SRAM_SIZE    = 16K
FLASH_BASE   = 0x08000000
FLASH_SIZE   = 16K

FLASH_BASE_RAM = 0x20004000
run:      FLASH_BASE  = $(FLASH_BASE_RAM)
sram:     FLASH_BASE  = $(FLASH_BASE_RAM)
normal:   FLASH_BASE  = 0x08000000
recovery: FLASH_BASE  = 0x08004000
recovery: RECOVERY = 1

SHELL_BASE   = 0x08000000
SHELL_SIZE   = 0x00004000
FLASH_SIZE   = $(shell python -c 'print ( str($(SHELL_SIZE)>>10) + "K" )')
FLASH_SIZE_F = $(shell python -c 'print ( str($(SHELL_SIZE)>>9)  + "K" )')
FLASH_BASE   = $(SHELL_BASE)
FLASH_BASE_R = $(shell python -c 'print ( hex($(SHELL_BASE)+$(SHELL_SIZE)) )')

LIBDIR=
INCDIR= -I. \
        -Iusb \
        -Ilib/Cmsis/Include \
        -Ilib/Cmsis/Device/ST/STM32F4xx/Include \
        -Ilib/Sys/inc \
        -Ilib/Usb/inc \

# Define optimisation level here
CPU_FLG  = -mthumb -mcpu=cortex-m4
FPU_FLG  = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -ffp-contract=off -fsingle-precision-constant
COM_FLG  = -fdata-sections -ffunction-sections -nostartfiles -Wall
OPT_FLG  = -Os -fomit-frame-pointer -flto
EXT_FLAG = -nostdlib

ASFLAGS  = $(CPU_FLG)
CPFLAGS  = $(CPU_FLG) $(FPU_FLG) $(COM_FLG) $(OPT_FLG) $(FPU_FLG) $(INCDIR) -fdata-sections -ffunction-sections -fno-builtin -nostdlib -Wall
CPFLAGS += -DHSE_VALUE=8000000 -DSTM32F40_41xxx -DSTM32F407xx -DVECT_TAB_SRAM -DSHELL_SIZE=$(SHELL_SIZE) -DRECOVERY=$(RECOVERY)
LDFLAGS  = $(CPU_FLG) $(FPU_FLG) $(COM_FLG) -T$(LINKLD) -Wl,-Map=$(OUTDIR)/$(NAME).map,--cref,--no-warn-mismatch $(LIBDIR)
LDFLAGS +=

SYS_OBJ=\
    $(OUTDIR)/stm32f4xx_rcc.o \
    $(OUTDIR)/stm32f4xx_gpio.o \
    $(OUTDIR)/stm32f4xx_flash.o \
    $(OUTDIR)/stm32f4xx_usart.o \

USB_OBJ=\
		$(OUTDIR)/usbd_core.o \
		$(OUTDIR)/usbd_stm32f429_otgfs.o \
		$(OUTDIR)/usbbuf.o \
		$(OUTDIR)/usbdev.o \
		$(OUTDIR)/ringbuf.o \


OBJ=\
		$(OUTDIR)/startup.o \
		$(OUTDIR)/system.o \
    $(OUTDIR)/main.o \
    $(OUTDIR)/isr.o \
    $(OUTDIR)/shell.o \
    $(OUTDIR)/common.o \
    $(OUTDIR)/console.o \
    $(OUTDIR)/memory.o \
    $(OUTDIR)/iromprog.o \
    $(OUTDIR)/tprintf.o \
    $(USB_OBJ) \
    $(SYS_OBJ)


all: normal

$(OUTDIR)/$(NAME).bin: $(OUTDIR)/$(NAME).elf
	$(OBJDUMP) -h -S -C -r $< > $(OUTDIR)/$(NAME).s
	$(OBJCOPY) -O ihex   $< $(OUTDIR)/$(NAME).hex
	$(OBJCOPY) -O binary $< $(OUTDIR)/$(NAME).bin
	$(ELFSIZE) -A -x --format=Berkeley $(OUTDIR)/$(NAME).elf
	@echo FLASH_BASE @ $(FLASH_BASE)

$(OUTDIR)/$(NAME).elf : $(LINKLD)
$(OUTDIR)/$(NAME).elf : $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ)


$(OUTDIR)/%.o : $(PRJDIR)/%.s
	$(CC) -c $(CPFLAGS) $(CPPFLAGS) $< -o $@

$(OUTDIR)/%.o : $(PRJDIR)/lib/Sys/src/%.c
	$(CC) -c $(CPFLAGS) $(CPPFLAGS) $< -o $@

$(OUTDIR)/%.o : $(PRJDIR)/lib/Usb/src/%.c
	$(CC) -c $(CPFLAGS) $(CPPFLAGS) $< -o $@

$(OUTDIR)/%.o : $(PRJDIR)/usb/%.c
	$(CC) -c $(CPFLAGS) $(CPPFLAGS) $< -o $@

$(OUTDIR)/%.o : $(PRJDIR)/%.c
	$(CC) -c $(CPFLAGS) $(CPPFLAGS) $< -o $@

$(LINKLD):  Template.ld Makefile
	@python -c "with open('$(LINKLD)', 'wt') as out: \
             lines = open('Template.ld').readlines(); \
             lines = [line.replace('@SRAM_BASE',  '$(SRAM_BASE)')  for line in lines]; \
             lines = [line.replace('@SRAM_SIZE',  '$(SRAM_SIZE)')  for line in lines]; \
             lines = [line.replace('@FLASH_BASE', '$(FLASH_BASE)') for line in lines]; \
             lines = [line.replace('@FLASH_SIZE', '$(FLASH_SIZE)') for line in lines]; \
             out.writelines(lines)"


recovery: prebuild $(OUTDIR)/$(NAME).bin

normal: prebuild $(OUTDIR)/$(NAME).bin

sram: prebuild $(OUTDIR)/$(NAME).bin

run: prebuild $(OUTDIR)/$(NAME).bin
	$(PYSTLINK) -q reset:halt write:$(FLASH_BASE):$< set:msp:$(shell python -c $(GETSP)) set:pc:$(shell python -c $(GETPC))

both: prebuild
	make distclean
	make bin/Shell0.bin
	make bin/Shell1.bin

bin/Shell1.bin:
	make clean
	make recovery
	cp   out/shell.bin bin/shell1.bin

bin/Shell0.bin:
	make clean
	make normal
	cp   out/shell.bin bin/shell0.bin

dump:
	$(PYSTLINK) dump:core

burnstlink: bin/Shell0.bin
	$(PYSTLINK) -q flash:erase:0x08000000:$<

burnn: bin/Shell0.bin
	$(STMTALK) $< $(FLASH_BASE) IROM:$(USBADR) ep

burnr: bin/Shell1.bin
	$(STMTALK) $< $(FLASH_BASE_R) IROM:$(USBADR) ep

burnall: bin/Shell0.bin bin/Shell1.bin
	-$(STMTALK) bin/Shell1.bin  $(FLASH_BASE_R) IROM:$(USBADR) ep
	-$(STMTALK) "-rt 0xAA" $(USBADR)
	-ping 2.1.1.1 -n 1 -w 2000 >NUL
	-$(STMTALK)  bin/Shell0.bin  $(FLASH_BASE) IROM:$(USBADR) ep
	-$(STMTALK) "-rt" $(USBADR)



prebuild:
	@[ -d $(OUTDIR) ] || mkdir $(OUTDIR)
	@[ -d $(BINDIR) ] || mkdir $(BINDIR)

clean:
	@rm -Rf $(OUTDIR)

distclean:
	@rm -Rf $(OUTDIR) $(BINDIR)
