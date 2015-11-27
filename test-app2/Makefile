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
PROGRAM    = ATmega_app
TARGET_ELF = ATmega-app.elf

# enter the parameters for the avrdude isp tool
ISPTOOL	   = stk500v2
ISPPORT	   = usb
ISPSPEED   = -b 115200

MCU_TARGET = atmega168
# LDSECTION  = --section-start=.text=0x3800

# the efuse should really be 0xf8; since, however, only the lower
# three bits of that byte are used on the atmega168, avrdude gets
# confused if you specify 1's for the higher bits, see:
# http://tinker.it/now/2007/02/24/the-tale-of-avrdude-atmega168-and-extended-bits-fuses/
#
# similarly, the lock bits should be 0xff instead of 0x3f (to
# unlock the bootloader section) and 0xcf instead of 0x0f (to
# lock it), but since the high two bits of the lock byte are
# unused, avrdude would get confused.

ISPFUSES    = avrdude -c $(ISPTOOL) -p $(MCU_TARGET) -P $(ISPPORT) $(ISPSPEED) \
-e -u -U lock:w:0x3f:m -U efuse:w:0x$(EFUSE):m -U hfuse:w:0x$(HFUSE):m -U lfuse:w:0x$(LFUSE):m
ISPFLASH    = avrdude -c $(ISPTOOL) -p $(MCU_TARGET) -P $(ISPPORT) $(ISPSPEED) \
-U flash:w:$(PROGRAM)_$(TARGET).hex -U lock:w:0x0f:m

STK500 = "C:\Program Files\Atmel\AVR Tools\STK500\Stk500.exe"
STK500-1 = $(STK500) -e -d$(MCU_TARGET) -pf -vf -if$(PROGRAM)_$(TARGET).hex \
-lFF -LFF -f$(HFUSE)$(LFUSE) -EF8 -ms -q -cUSB -I200kHz -s -wt
STK500-2 = $(STK500) -d$(MCU_TARGET) -ms -q -lCF -LCF -cUSB -I200kHz -s -wt


# OBJ        = $(PROGRAM).o
OBJ        = $(PROGRAM).o uart.o suart.o
# OPTIMIZE   = -O2
OPTIMIZE   = -Os
# OPTIMIZE = -Os -fno-inline-small-functions -fno-split-wide-types -mshort-calls

DEFS       = 
LIBS       =

# CC         = avr-gcc
CC = d:/INSTALL/Arduino/arduino-1.0.5/hardware/tools/avr/bin/avr-gcc
# REMOVE = rm -rf
REMOVE = d:/INSTALL/Arduino/arduino-1.0.5/hardware/tools/avr/utils/bin/rm -rf

# Override is only needed by avr-lib build system.
override CFLAGS        = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) -DF_CPU=$(AVR_FREQ) $(DEFS)
override LDFLAGS       = -Wl,$(LDSECTION)
##override LDFLAGS       = -Wl,$(LDSECTION) -Wl,$(LDSECTION_DATA)
#override LDFLAGS       = -Wl,-Map,$(PROGRAM).map,$(LDSECTION)

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump

#all: TARGET = atmega328
all: TARGET = atmega8
#all: MCU_TARGET = atmega328p
all: MCU_TARGET = atmega8
#all: $(PROGRAM)_atmega328.hex
all: $(PROGRAM)_atmega8.hex
# Sektion für den Code (.text):
all: LDSECTION  = --section-start=.text=0x0000
# all: LDSECTION_DATA  = --section-start=.progmem.data=0x1F00
# all: LDSECTION_DATA  = --section-start=.progmem.data=0x0000
# all: LDSECTION_DATA  = --section-start=.mydata=0x0000
# all: LDSECTION_DATA  = --section-start=.prg_version=0x1fff
all: AVR_FREQ = 16000000L 
all: CFLAGS += '-DMAX_TIME_COUNT=F_CPU>>4' '-DNUM_LED_FLASHES=1' -DBAUD_RATE=19200
all: ${TARGET_ELF}
	@echo
	@avr-size -C --mcu=${MCU_TARGET} ${TARGET_ELF}


isp: $(TARGET)
	$(ISPFUSES)
	$(ISPFLASH)

isp-stk500: $(PROGRAM)_$(TARGET).hex
	$(STK500-1)
	$(STK500-2)

%.elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
	
clean:
	# rm -rf *.o *.elf *.lst *.map *.sym *.lss *.eep *.srec *.bin *.hex
	# rm -rf *.o *.elf *.lst *.map *.sym *.lss *.eep *.srec *.bin 
	$(REMOVE) *.o *.elf *.lst *.map *.sym *.lss *.eep *.srec *.bin 

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@
	
