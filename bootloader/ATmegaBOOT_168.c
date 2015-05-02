#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/wdt.h>
//#include <avr/boot.h>
#include "boot.h"
#include <util/delay.h>
#include "uart.h"
#include "main.h"
#include "suart.h"

#include <stdbool.h> 
//#include <avr/pgmspace.h>
//#include "pin_defs.h"
 
#define UART_BAUD_RATE     19200     /* Baudrate */

#define LED_PORT_B PORTB
#define LED_DDR    DDRD
#define LED_DDR_B  DDRB

#define LED_PORT PORTD

#define LED1     PIND4  /* LED rot-unten */
#define LED2     PIND5  /* LED rot-rechts */
#define RS485    PIND2  /* LED rot-oben */

#define LED_red   PINB0 /* RGB-LED rot  ==> Error ?? */
#define LED_blue  PINB2 /* RGB-LED blau ==> Bootloader */
#define LED_green PINB5 /* RGB-LED grün ==> Anwendungsprogramm */

/* define various device id's */
#define CRC16_POLYGON 		0x1002
#define FRAME_START_LONG 	0xFD
#define FRAME_START_SHORT	0xFE
#define ESCAPE_CHAR			  0xFC
#define MAX_RX_FRAME_LENGTH      255
#define CONTAINS_SENDER(x)  (((x) & (1<<3)) !=0)

// #define DEBUG

/* function prototypes */
void byte_response(uint8_t);
void nothing_response(void);
void flash_led(uint8_t);
void error_led(uint8_t);
void rgb_led(uint8_t red, uint8_t green, uint8_t blue );
void crc16_init(void);
void crc16_shift(unsigned char);
void SendAck(int, unsigned char);
void SendByte(unsigned char);
void SendDataByte(unsigned char);
void setup(void);
void AddressCharToHex(unsigned char *p_ucAddress, unsigned long *p_ulAddress);

unsigned char	temp;              /* Variable */
unsigned int  crc16_register;				// Register mit CRC16 Code	
// Start pointer NUR deklarieren
static void (*start_app) (void);									
	
/*********************************************************************************** 
 Hauptprogramm
***********************************************************************************/
int main()
{
	unsigned int ch = 0;     /* Empfangenes Zeichen + Statuscode */
	uint16_t v, w;
	uint8_t *bufPtr;
	uint16_t addrPtr;
	
	unsigned long ulAddress1;
	bool address_ok;
	
	uint8_t ControlByte=0;
	unsigned char Escape = 0;
	unsigned char AddressLen;
	unsigned char AddressPointer=0;
	unsigned char DataLength=0;
	unsigned char FramePointer=0;
	
	unsigned char FrameData[MAX_RX_FRAME_LENGTH];
	
	uint8_t SenderAddress[4];
	
	uint8_t buff[256];
	
	union address_union {
	  uint16_t word;
	  uint8_t  byte[2];
  } address;

	union to_address_union {
		uint16_t word[2];
		uint8_t  byte[4];
  } to_address;

	union length_union {
		uint16_t word;
		uint8_t  byte[2];
	} length;	
	
	// pointer HIER initialisieren:
	start_app = ( void *) 0x0000;    /* Pointer auf 0x0000 */
	
	setup();	
	
	while (1)
	{
		ch = uart_getc();
		// break;
		if ( ch & UART_NO_DATA )
		{
			//sputs("\n\r Daten nicht empfangen" );
			//rgb_led(1,0,0);
			;
		}
		else
		{		
		  // sputs("\n\r Daten empfangen" );
			//rgb_led(0,1,0);
			//sputchar(ch);
			if (ch == ESCAPE_CHAR && Escape == 0)
			{
				Escape = 1;
				continue;		// zurueck zum Beginn der while Schleife, folgendes auslassen
			}
			
			if (ch == 0xFD) // Startzeichen
			{
				Escape = 0;
				AddressPointer = 0;
				AddressLen = 4;
				FramePointer = 0;
				crc16_init();
				crc16_shift(ch);
				// Flash LED off:
				LED_PORT |= _BV(LED1);
			}
			else
			{
				if (Escape == 1)
				{
					ch |= 0x80;
					Escape = 0;
				}
				
				// Ziel-Adressbytes empfangen:
				if (AddressPointer < AddressLen)		
				{
					to_address.byte[AddressPointer] = ch;
					AddressPointer++;
					crc16_shift(ch);
				}
				
				// Controllbyte empfangen
				else if (AddressPointer == AddressLen)	
				{
					ControlByte = ch;
					AddressPointer++;
					crc16_shift(ch);
					AddressCharToHex(to_address.byte, &ulAddress1);
					// Adresse mit der Eigenen vergleichen:
					if (ulAddress1 == 0x1029 )		
					{
						address_ok = true;
					}
				}
				
				// Absenderadresse empfangen & Controllbyte auswerten:
				else if (CONTAINS_SENDER(ControlByte) && (AddressPointer < (2 * AddressLen + 1)))
				{
					SenderAddress[AddressPointer - AddressLen - 1] = ch;
					AddressPointer++;
					crc16_shift(ch);
				}
				
				// Daten-Länge empfangen
				else if (AddressPointer != 0xFF)		
				{
					// Markierung "DataLength Byte" erreicht setzen: 0xFF
					AddressPointer = 0xFF;
					DataLength = ch;
					crc16_shift(ch);
				}
				else // Daten empfangen
				{
					FrameData[FramePointer] = ch;
					crc16_shift(ch);
					// sputs("\n\r Daten empfangen" );
					
					// Daten komplett empfangen
					if (FramePointer == (DataLength - 1))		
					{
						crc16_shift(0);
						crc16_shift(0);
						FramePointer = 0;
						AddressPointer = 0;
						// sputs("\n\r Daten komplett empfangen" );
						
						// Checksumme überprüfen
						if (crc16_register == 0)
						{	
							// Programming fertig:	
							if ( FrameData[0] == 0x67 )
							{
								//EIND = 0;
								SendAck(2, (ControlByte >> 1) & 0x03);
								sputs("\n\r 0x67 empfangen ==> Programming fertig" );
								//appStart();
								break;
							}
							if (FrameData[0] == 0x70)
							{
								// Send ACK
								SendAck(2, (ControlByte >> 1) & 0x03);
								#ifdef DEBUG 
									sputs("\n\r 0x70 empfangen ==> Vorbereitung Programming" );
								#endif	
							}								
							// Program Flash:	
							if (FrameData[0] == 0x77 && address_ok == true)
							{
								// high byte of address
								address.byte[1] = FrameData[1];
								// low byte of address
								address.byte[0] = FrameData[2];
								// High byte of data block size of word:
								length.byte[1] = 0x00;
								// Low byte of data block size of words:
								length.byte[0] = FrameData[3];
								// for (v = 0; v < length.word; v++) 
								for (v = 0; v < 64; v++) 
								{
									// Store data in buffer, can't keep up with serial data stream whilst programming pages
									buff[w] = FrameData[v + 4];   
									w++;
								}
								w = 0;
								//flash_led(1);
								
								length.word = length.word << 1;	        // length * 2 -> byte location
								
								if ((length.byte[0] & 0x01)) length.word++;	//Even up an odd number of bytes
								
								cli();	//Disable interrupts, just to be sure
								
								// Copy buffer into programming buffer
								bufPtr = buff;
								addrPtr = (uint16_t)(void*)address.word;
								// ATmega328: SPM_PAGESIZE = 128
								ch = SPM_PAGESIZE / 2;
								do 
								{
									uint16_t a;
									a = *bufPtr++;
									a |= (*bufPtr++) << 8;
									__boot_page_fill_short((uint16_t) (void*)addrPtr, a);
									addrPtr += 2;
								} 
								while (--ch);

								// Write from programming buffer
								__boot_page_write_short((uint16_t) (void*)address.word);
								boot_spm_busy_wait();

								sei();	// Enable interrupts, just to be sure
								
								//if (++error_count == MAX_ERROR_COUNT)
								//  appStart();
									
								// Send ACK
								SendAck(1, (ControlByte >> 1) & 0x03);
							}
						}
						else
						{
							// Prüfsumme falsch
							//error_led(3);
							rgb_led(1,0,0);
							continue;
						}
					}
					if (FramePointer >= MAX_RX_FRAME_LENGTH){
						// Maximale Framelänge überschritten!
						//error_led(3);
						rgb_led(1,0,0);
						continue;
					}
					FramePointer++;
				}
			}
		}
		//_delay_ms(1000);
	}	
	
	sputs("\n\r Springe zur Adresse 0x0000!");
	_delay_ms(1000);

	/* vor Rücksprung eventuell benutzte Hardware deaktivieren
		 und Interrupts global deaktivieren, da kein "echter" Reset erfolgt */

	/* Interrupt Vektoren wieder gerade biegen */
	cli();  // disable global interrupts
	temp = GICR;
	GICR = temp | (1<<IVCE);
	GICR = temp & ~(1<<IVSEL);

	/* Rücksprung zur Adresse 0x0000 */
	start_app(); 
	return 0;
}

/******************************************************************************************** 
 Funktionen
*********************************************************************************************/
/*********************************************************************************** 
 Send ACK
************************************************************************************/
void SendAck(int typ, unsigned char Empfangsfolgenummer)
{
	int i;
	unsigned char StartByte;
	uint8_t ControlByte;
	unsigned char DataLength;
	unsigned char FrameData[MAX_RX_FRAME_LENGTH];
	
	// Sende:
	_delay_ms(8);	
	LED_PORT |= _BV(RS485);
	//_delay_ms(1);
	
	ControlByte = (( Empfangsfolgenummer & 0x03 ) << 5 ) | 0x11;
	// printf("Controlbyte:%02x ", stAckData.ucControlByte);
	DataLength = 0;
	StartByte = FRAME_START_SHORT;
	// Frame prüfen (== FE):
	
	crc16_init();
	// Startzeichen:
	SendByte(StartByte);			
	crc16_shift(StartByte);
	// Null-byte:
	SendByte(0x00);	
	crc16_shift(0x00);

	// Controllbyte:
	SendDataByte(ControlByte);	
	crc16_shift(ControlByte);

	if (typ == 2)
	{
	  DataLength = 2;
		// Data Bytes:
		FrameData[0] = 0x00;	
		FrameData[1] = 0x40;	
	}
	
	// Framelänge:
	SendDataByte(DataLength+2);	
	crc16_shift(DataLength+2);
	// Framedaten:
	for ( i = 0; i < DataLength; i++ )				
	{
		SendDataByte(FrameData[i]);
		crc16_shift(FrameData[i]);
	}	
	crc16_shift(0);
	crc16_shift(0);
	// CRC16-Checksumme:
	SendDataByte((crc16_register >> 8) & 0xff);	
	SendDataByte((crc16_register) & 0xff);
	
	// _delay_ms(2);
	_delay_ms(4);
	// Nicht Senden:
 	LED_PORT &= ~_BV(RS485);

	// return 0;
}

/*********************************************************************************** 
 Sendet ein Byte über die Schnittstelle
************************************************************************************/
void SendByte(unsigned char ucByte)
{
	// Senden:
	uart_putc(ucByte);
	//putch(ucByte);
	// return p_cCom->write(&p_ucByte,1);
}

/*********************************************************************************** 
 Sendet ein Byte über die Schnittstelle, prüft jedoch auf Sonderzeichen und wandelt diese entsprechend um
************************************************************************************/
void SendDataByte(unsigned char ucByte)
{
	unsigned char c;
	
	if ((ucByte==FRAME_START_LONG) || (ucByte==FRAME_START_SHORT) || (ucByte==ESCAPE_CHAR))
	{
		c = ESCAPE_CHAR;
		SendByte(c);
		// if (!SendByte(c))
			// return false;
		ucByte &= 0x7f;
	}
	SendByte(ucByte);
	// return SendByte(p_cCom,p_ucByte);
}

/*********************************************************************************** 
 crc16_init 
************************************************************************************/
void crc16_init(void)
{
	crc16_register = 0xffff;
}

/*********************************************************************************** 
 crc16_shift
************************************************************************************/
void crc16_shift(unsigned char w_byte)
{
	unsigned char q;
	unsigned char flag;
	
	for ( q = 0; q < 8; q++)
	{
		flag = (crc16_register & 0x8000) != 0;
		crc16_register <<= 1;
		if (w_byte & 0x80)
		{
			crc16_register |= 1;
		}
		w_byte <<= 1;
		if (flag)
		{
			crc16_register ^= CRC16_POLYGON;
		}
	}
}

/*********************************************************************************** 
 rgb_led
************************************************************************************/
void rgb_led(uint8_t red, uint8_t green, uint8_t blue )
{
  // LED rot ON
  if (red == 1)
	{
	  LED_PORT_B &= ~_BV(LED_red);
	}
  else
  {
	  LED_PORT_B |= _BV(LED_red);
	}	

  if (green == 1)
	{ 
    LED_PORT_B &= ~_BV(LED_green); 
  }		
	else
	{
		LED_PORT_B |= _BV(LED_green);
	}

	if (blue == 1)
	{
		LED_PORT_B &= ~_BV(LED_blue);
	}
	else
	{
	  LED_PORT_B |= _BV(LED_blue);
	}
}

/*********************************************************************************** 
 setup
************************************************************************************/
void setup(void)
{
	// set LED pin as output 
	LED_DDR |= _BV(LED1);
	LED_DDR |= _BV(LED2);
	LED_DDR |= _BV(RS485);
	
	// set RGB LED pin as output 
	LED_DDR_B |= _BV(LED_red);
	LED_DDR_B |= _BV(LED_green);
	LED_DDR_B |= _BV(LED_blue);

	// Error LED aus:
	LED_PORT |= _BV(LED2);
	
	// Nicht Senden:
	LED_PORT &= ~_BV(RS485);

	// RGB LED an:
  //rgb_led(0,0,1);
	
	/* Interrupt Vektoren verbiegen */
	char sregtemp = SREG;
	cli();   // disable global interrupts
	temp = GICR;
	GICR = temp | (1<<IVCE);
	GICR = temp | (1<<IVSEL);
	SREG = sregtemp;
	
	/* Einstellen der Baudrate und aktivieren der Interrupts */
	uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) ); 
	
	suart_init();
	sei();  // enable global interrupts

 	//rgb_led(0,0,0);
	sputs("\n\rBootloader Setup" );
	//_delay_ms(1000);
	// RGB LED an:
  rgb_led(0,0,1);

}

/*********************************************************************************** 
 appStart
************************************************************************************/
/*
void appStart(void) 
{
  uart_puts("Ausloesung Watchdog!\n\r");
	_delay_ms(100);
	// Watchdog ausloesen:
	WDTCSR |= _BV(WDCE) | _BV(WDE);  // Watch Dog Change Enable | Watch Dog Enable
	while (1); // 16 ms
}
*/

/*********************************************************************************** 
 Wandelt ein 4-Byte char array in eine 32-Bit Adresse um
************************************************************************************/
void AddressCharToHex(unsigned char *p_ucAddress, unsigned long *p_ulAddress)
{
	int i;
	char *x=(char*)p_ulAddress;
	for(i=0;i<4;i++)
		x[i]=p_ucAddress[3-i];
}


