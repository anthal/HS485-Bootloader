#define DEV_ADDR 0x102F
//#define DEV_ADDR 0x1029

#include <avr/pgmspace.h>

/*
Write device Address

https://www.mikrocontroller.net/topic/142704
https://www.mikrocontroller.net/articles/Bin%C3%A4re_Daten_zum_Programm_hinzuf%C3%BCgen
*/

// Device Adresse schreiben:
const uint32_t prg_version __attribute__ ((section (".devaddr"))) = { DEV_ADDR };


/*
Device Addresse schreiben in Inline-Assembler:

#define MAKESTR(a) #a
#define MAKEVER(a) MAKESTR(a)
asm("  .section .devaddr\n"
    "device_address:  .word " MAKEVER(DEV_ADDR) "\n"
    "  .section .text\n");
*/


/*
===== WICHTIG! ===== 
Section muss im Makefile unten stehen:

all: LDSECTIONS  = -Wl,--section-start=.text=0x1800 -Wl,--section-start=.devaddr=0x1FFC

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -j .devaddr --set-section-flags .devaddr=alloc,load -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -j .devaddr --set-section-flags .devaddr=alloc,load -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -j .devaddr --set-section-flags .devaddr=alloc,load -O binary $< $@

*/



