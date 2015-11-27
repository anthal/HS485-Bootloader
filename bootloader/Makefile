# Makefile for ATmegaBOOT
# E.Lins, 18.7.2005
# $Id$
#
# Instructions
#
# To make bootloader .hex file:
# make diecimila
# make lilypad
# make ng
# etc...
#
# To burn bootloader .hex file:
# make diecimila_isp
# make lilypad_isp
# make ng_isp
# etc...

# program name should not be changed...
# PROGRAM    = optiboot
PROGRAM    = ATmegaBOOT_168
TARGET_ELF = arduino-bootloader.elf

# The default behavior is to build using tools that are in the users
# current path variables, but we can also build using an installed
# Arduino user IDE setup, or the Arduino source tree.
# Uncomment this next lines to build within the arduino environment,
# using the arduino-included avrgcc toolset (mac and pc)
# ENV ?= arduino
# ENV ?= arduinodev
# OS ?= macosx
# OS ?= windows


# enter the parameters for the avrdude isp tool
ISPTOOL	   = stk500v2
ISPPORT	   = usb
ISPSPEED   = -b 115200

# MCU_TARGET = atmega168
# LDSECTIONS  = -Wl,--section-start=.text=0x3e00 -Wl,--section-start=.version=0x3ffe

# Build environments
# Start of some ugly makefile-isms to allow optiboot to be built
# in several different environments.  See the README.TXT file for
# details.

# default
# fixpath = $(1)
# fixpath = "d:/INSTALL/Arduino/arduino-1.0.5/hardware/tools/"
fixpath = "d:\INSTALL\Arduino\arduino-1.0.5\hardware\tools\"

ifeq ($(ENV), arduino)
	# For Arduino, we assume that we're connected to the optiboot directory
	# included with the arduino distribution, which means that the full set
	# of avr-tools are "right up there" in standard places.
	# TOOLROOT = ../../../tools
	TOOLROOT = $(fixpath)
	GCCROOT = $(TOOLROOT)/avr/bin/
	AVRDUDE_CONF = -C$(TOOLROOT)/avr/etc/avrdude.conf
	ifeq ($(OS), windows)
		# On windows, SOME of the tool paths will need to have backslashes instead
		# of forward slashes (because they use windows cmd.exe for execution instead
		# of a unix/mingw shell?)  We also have to ensure that a consistent shell
		# is used even if a unix shell is installed (ie as part of WINAVR)
		fixpath = $(subst /,\,$1)
		SHELL = cmd.exe
	endif
else ifeq ($(ENV), arduinodev)
	# Arduino IDE source code environment.  Use the unpacked compilers created
	# by the build (you'll need to do "ant build" first.)
	ifeq ($(OS), macosx)
		TOOLROOT = ../../../../build/macosx/work/Arduino.app/Contents/Resources/Java/hardware/tools
	endif
	ifeq ($(OS), windows)
		TOOLROOT = ../../../../build/windows/work/hardware/tools
	endif
	GCCROOT = $(TOOLROOT)/avr/bin/
	AVRDUDE_CONF = -C$(TOOLROOT)/avr/etc/avrdude.conf
else
	GCCROOT =
	AVRDUDE_CONF =
	#TOOLROOT = "d:/INSTALL/Arduino/arduino-1.0.5/hardware/"
	#GCCROOT = $(TOOLROOT)/avr/bin/
endif
#
# End of build environment code.


# the efuse should really be 0xf8; since, however, only the lower
# three bits of that byte are used on the atmega168, avrdude gets
# confused if you specify 1's for the higher bits, see:
# http://tinker.it/now/2007/02/24/the-tale-of-avrdude-atmega168-and-extended-bits-fuses/
#
# similarly, the lock bits should be 0xff instead of 0x3f (to
# unlock the bootloader section) and 0xcf instead of 0x2f (to
# lock it), but since the high two bits of the lock byte are
# unused, avrdude would get confused.

ISPFUSES    = $(GCCROOT)avrdude $(AVRDUDE_CONF) -c $(ISPTOOL) \
              -p $(MCU_TARGET) -P $(ISPPORT) $(ISPSPEED) \
              -e -u -U lock:w:0x3f:m -U efuse:w:0x$(EFUSE):m \
              -U hfuse:w:0x$(HFUSE):m -U lfuse:w:0x$(LFUSE):m
ISPFLASH    = $(GCCROOT)avrdude $(AVRDUDE_CONF) -c $(ISPTOOL) \
              -p $(MCU_TARGET) -P $(ISPPORT) $(ISPSPEED) \
              -U flash:w:$(PROGRAM)_$(TARGET).hex -U lock:w:0x2f:m

STK500 = "C:\Program Files\Atmel\AVR Tools\STK500\Stk500.exe"
STK500-1 = $(STK500) -e -d$(MCU_TARGET) -pf -vf -if$(PROGRAM)_$(TARGET).hex \
-lFF -LFF -f$(HFUSE)$(LFUSE) -EF8 -ms -q -cUSB -I200kHz -s -wt
STK500-2 = $(STK500) -d$(MCU_TARGET) -ms -q -lCF -LCF -cUSB -I200kHz -s -wt

OBJ        = $(PROGRAM).o uart.o suart.o device_addr.o
OPTIMIZE   = -O2
# OPTIMIZE = -Os -fno-inline-small-functions -fno-split-wide-types -mshort-calls

DEFS       = 
LIBS       =

CC         = $(GCCROOT)avr-gcc
# CC = d:/INSTALL/Arduino/arduino-1.0.5/hardware/tools/avr/bin/avr-gcc

# Override is only needed by avr-lib build system.

override CFLAGS        = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) -DF_CPU=$(AVR_FREQ) $(DEFS)
# override LDFLAGS       = $(LDSECTIONS) -Wl,--relax -Wl,--gc-sections -nostartfiles -nostdlib
override LDFLAGS       = $(LDSECTIONS) 

OBJCOPY        = $(GCCROOT)avr-objcopy
OBJDUMP        = $(call fixpath,$(GCCROOT)avr-objdump)
#OBJCOPY        = avr-objcopy
#OBJDUMP        = avr-objdump

SIZE           = $(GCCROOT)avr-size
all: TARGET = atmega8
all: MCU_TARGET = atmega8
all: AVR_FREQ = 16000000L 
all: LDSECTIONS  = -Wl,--section-start=.text=0x1800 -Wl,--section-start=.devaddr=0x1FFC
# all: LDSECTIONS  = -Wl,--section-start=.text=0x1800 -Wl,--section-start=.version=0x1FF0
all: $(PROGRAM)_atmega8.hex
# run 'lst' failed:
# all: $(PROGRAM)_atmega8.lst
all: CFLAGS += '-DMAX_TIME_COUNT=F_CPU>>4' '-DNUM_LED_FLASHES=1' -DBAUD_RATE=19200
all: ${TARGET_ELF}
	@echo
	@avr-size -C --mcu=${MCU_TARGET} ${TARGET_ELF}


#
# Generic build instructions
#
#

isp: $(TARGET)
	$(ISPFUSES)
	$(ISPFLASH)

isp-stk500: $(PROGRAM)_$(TARGET).hex
	$(STK500-1)
	$(STK500-2)

%.elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(SIZE) $@

clean:
	# rm -rf *.o *.elf *.lst *.map *.sym *.lss *.eep *.srec *.bin *.hex
	rm -rf *.o *.elf *.lst *.map *.sym *.lss *.eep *.srec *.bin

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -j .devaddr --set-section-flags .devaddr=alloc,load -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -j .devaddr --set-section-flags .devaddr=alloc,load -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -j .devaddr --set-section-flags .devaddr=alloc,load -O binary $< $@
