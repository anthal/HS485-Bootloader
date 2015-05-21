/*
HS485 Applikation

*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "uart.h"
#include "pin_defs.h"
#include <stdbool.h> 
#include "uart.h"
#include "main.h"
#include "suart.h"


#define DEBUG 


#define UART_BAUD_RATE	19200
 
#define LED_PORT_B PORTB
#define LED_DDR_B  DDRB

#define LED_PORT_C PORTC
#define LED_DDR_C  DDRC

#define LED_DDR  DDRD
#define LED_PORT PORTD

#define LED1     PIND4
#define LED2     PIND5
#define RS485    PIND2

#define LED_red   PINB0
#define LED_blue  PINB2
#define LED_green PINB5

#define Seg_A	PINC0
#define Seg_B	PINC1
#define Seg_C	PINC2
#define Seg_D	PINC3
 
/* define various device id's */
#define CRC16_POLYGON 				0x1002
#define FRAME_START_LONG 	0xFD
#define FRAME_START_SHORT	0xFE
#define ESCAPE_CHAR			  0xFC
#define MAX_RX_FRAME_LENGTH      255
#define CONTAINS_SENDER(x)  (((x) & (1<<3)) !=0)

/* function prototypes */
void byte_response(uint8_t);
void nothing_response(void);
void flash_led(uint8_t);
void error_led(uint8_t);
void rgb_led(uint8_t red, uint8_t green, uint8_t blue);
void crc16_init(void);
void crc16_shift(unsigned char);
void SendAck(int, unsigned char);
void SendByte(unsigned char);
void SendDataByte(unsigned char);
void setup(void);
void AddressCharToHex(unsigned char *p_ucAddress, unsigned long *p_ulAddress);

/* some variables */
union address_union {
	uint16_t word;
	uint8_t  byte[2];
} address;
// register uint16_t address = 0;

union to_address_union {
	uint16_t word[2];
	uint8_t  byte[4];
} to_address;

union length_union {
	uint16_t word;
	uint8_t  byte[2];
} length;

uint8_t ControlByte;
unsigned char Escape = 0;
unsigned char AddressLen;
unsigned char AddressPointer;
unsigned char DataLength;
unsigned char FramePointer;
// unsigned int  LocalCRCRegister;
unsigned int  crc16_register;				// Register mit CRC16 Code
//unsigned uint16_t  crc16_register;				// Register mit CRC16 Code
unsigned char FrameData[MAX_RX_FRAME_LENGTH];
uint8_t SenderAddress[4];
//uint8_t buff[256];
//uint8_t i;
//uint8_t bootuart = 0;
//uint8_t error_count = 0;
 
//void (*bootloader)( void ) = 0x0C00;  // Achtung Falle: Hier Word-Adresse
typedef void (*boot_reset_fptr_t)(void);
boot_reset_fptr_t bootloader = (boot_reset_fptr_t) 0x0C00;

/*********************************************************************************** 
 Hauptprogramm
************************************************************************************/
int main()
{
	unsigned int 	ch = 0;     /* Empfangenes Zeichen + Statuscode */
	unsigned long ulAddress1;
	bool address_ok;
  
	setup();
  
	sei();

	sputs("\n\rHier ist das Anwendungsprogramm...");
	rgb_led(0,1,0);  /* LED gruen ==> ON */
	//bootloader();
	
	while (1) 
	{
		ch = uart_getc();
		
		if ( ch & UART_NO_DATA )
		{
			#ifdef DEBUG 
			  //sputs("\nUART no data!");
			#endif	
			//Segment_led(0);
 		}
		else
		{
		    
			#ifdef DEBUG 
			  // output of receives character:
			  //sputchar(ch);
			  //sputs(":0x%2x ", ch);
			#endif	
				
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
				#ifdef DEBUG 
				  sputs("\n\rStartzeichen empfangen");
				#endif	
				Segment_led(1);
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
					#ifdef DEBUG 
						sputs("\nZiel-Adressbytes empfangen");
						
					#endif	
					Segment_led(2);

				}
				
				// Controllbyte empfangen
				else if (AddressPointer == AddressLen)	
				{
					ControlByte = ch;
					AddressPointer++;
					crc16_shift(ch);
					AddressCharToHex(to_address.byte, &ulAddress1);
					#ifdef DEBUG 
  					sputs("\nControllbyte empfangen");
					#endif	
					Segment_led(3);
					// Adresse mit der Eigenen vergleichen:
					//if (ulAddress1 == 0x1029 )		
					if (ulAddress1 == 0x1028 )		
					{
						address_ok = true;
						#ifdef DEBUG 
							sputs("\nAdresse OK");
						#endif	
						Segment_led(4);
					}
				}
				
				// Absenderadresse empfangen & Controllbyte auswerten:
				else if (CONTAINS_SENDER(ControlByte) && (AddressPointer < (2 * AddressLen + 1)))
				{
					SenderAddress[AddressPointer - AddressLen - 1] = ch;
					AddressPointer++;
					crc16_shift(ch);
					#ifdef DEBUG 
						sputs("\nControllbyte auswerten");
					#endif	
					Segment_led(5);
				}
				
				// Daten-Länge empfangen
				else if (AddressPointer != 0xFF)		
				{
					// Markierung "DataLength Byte" erreicht setzen: 0xFF
					AddressPointer = 0xFF;
					DataLength = ch;
					crc16_shift(ch);
					#ifdef DEBUG 
						sputs("\nDatenlaenge empfangen");
					#endif	
					Segment_led(6);
				}
				
				else // Daten empfangen
				{
					FrameData[FramePointer] = ch;
					crc16_shift(ch);
					
					// Daten komplett empfangen
					if (FramePointer == (DataLength - 1))		
					{
						crc16_shift(0);
						crc16_shift(0);
						FramePointer = 0;
						AddressPointer = 0;
						#ifdef DEBUG 
							sputs("\nDaten komplett empfangen");
						#endif	
						Segment_led(7);
						
						// Checksumme überprüfen
						if (crc16_register == 0)
						{	
						    Segment_led(8);
							// Flash Block size:
							if (FrameData[0] == 0x70)
							{
								// Send ACK
								SendAck(2, (ControlByte >> 1) & 0x03);
								#ifdef DEBUG 
									sputs("\nSprung zum Bootloader");
								#endif	
								Segment_led(9);
								// kurz warten
								_delay_ms(4);
								// LED Blau ==> ON	
								rgb_led(0,0,1);	
								// Sprung zum Bootloader:
								bootloader();
							}	
							
							// Programming fertig:	
							if ( FrameData[0] == 0x67 )
							{
								SendAck(1, (ControlByte >> 1) & 0x03);
							}
						}
						else
						{
							// Prüfsumme falsch
							// LED rot ==> ON
							rgb_led(1,0,0);
							continue;
						}
					}
					if (FramePointer >= MAX_RX_FRAME_LENGTH){
						// Maximale Framelänge überschritten!
						// LED rot ==> ON
						rgb_led(1,0,0);
						continue;
					}
					FramePointer++;
				}
				
			}
			//flash_led(2);
			
		} /* end of forever loop */			
		//return 0;
	}	
}

/******************************************************************************************** 
 Funktionen
*********************************************************************************************/
/*********************************************************************************** 
 setup
************************************************************************************/
void setup(void)
{
	/* set LED pin as output */
	LED_DDR |= _BV(LED1);
	LED_DDR |= _BV(LED2);
	LED_DDR |= _BV(RS485);
	
	/* set RGB LED pin as output */
	LED_DDR_B |= _BV(LED_red);
	LED_DDR_B |= _BV(LED_green);
	LED_DDR_B |= _BV(LED_blue);
	
	/* set 7 Seg. pins as output */
	LED_DDR_C |= _BV(Seg_A);
	LED_DDR_C |= _BV(Seg_B);
	LED_DDR_C |= _BV(Seg_C);
	LED_DDR_C |= _BV(Seg_D);

	// 7 Seg. ==> 0:
	Segment_led(0);

	// Error LED aus:
	LED_PORT |= _BV(LED2);
	
	// Nicht Senden:
	LED_PORT &= ~_BV(RS485);

	// RGB LED an:
	rgb_led(1,1,1);
	
	uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) ); 
	suart_init();
	sei();
	//sputs("Setup ATmega8 from Software UART\n\r" );
}

/*********************************************************************************** 
 Send ACK
 
 SendAck(2, (ControlByte >> 1) & 0x03);
************************************************************************************/
void SendAck(int typ, unsigned char Empfangsfolgenummer)
{
	int i;
	unsigned char StartByte;
	
		// Sende:
	_delay_ms(8);	
	LED_PORT |= _BV(RS485);
	//_delay_ms(1);
	
	ControlByte = (( Empfangsfolgenummer & 0x03 ) << 5 ) | 0x11;
	
	// NUR zum TESTEN!!! Bitte fixen und dann entfernen!!
	ControlByte = 0x52;
	
	#ifdef DEBUG 
	  //sputs("Controlbyte:0x%02x ", ControlByte);
	  sputs("\nSend Controlbyte: ");
	  // printf("Controlbyte:%02x ", stAckData.ucControlByte);
	#endif	

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
/*
	if (typ == 4)
	{
	  DataLength = 4;
		// Data Bytes:
		FrameData[0] = 0x00;	
		FrameData[1] = 0x40;	
	}
*/	
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
	_delay_ms(1);
	SendDataByte((crc16_register) & 0xff);
	
	//_delay_ms(2);
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
 7 Segment Display
************************************************************************************/
void Segment_led( uint8_t value )
{
	// 7 Seg. ==> 0:
	if ( value == 0)
	{
		LED_PORT_C &= ~_BV(Seg_A);
		LED_PORT_C &= ~_BV(Seg_B);
		LED_PORT_C &= ~_BV(Seg_C);
		LED_PORT_C &= ~_BV(Seg_D);
	}
	else if ( value == 1)
	{
		LED_PORT_C |= _BV(Seg_A);
		LED_PORT_C &= ~_BV(Seg_B);
		LED_PORT_C &= ~_BV(Seg_C);
		LED_PORT_C &= ~_BV(Seg_D);
	}	
	else if ( value == 2)
	{
		LED_PORT_C &= ~_BV(Seg_A);
		LED_PORT_C |= _BV(Seg_B);
		LED_PORT_C &= ~_BV(Seg_C);
		LED_PORT_C &= ~_BV(Seg_D);
	}	
	else if ( value == 3)
	{
		LED_PORT_C |= _BV(Seg_A);
		LED_PORT_C |= _BV(Seg_B);
		LED_PORT_C &= ~_BV(Seg_C);
		LED_PORT_C &= ~_BV(Seg_D);
	}	
	else if ( value == 4)
	{
		LED_PORT_C &= ~_BV(Seg_A);
		LED_PORT_C &= ~_BV(Seg_B);
		LED_PORT_C |= _BV(Seg_C);
		LED_PORT_C &= ~_BV(Seg_D);
	}	
	else if ( value == 5)
	{
		LED_PORT_C |= _BV(Seg_A);
		LED_PORT_C &= ~_BV(Seg_B);
		LED_PORT_C |= _BV(Seg_C);
		LED_PORT_C &= ~_BV(Seg_D);
	}	
	else if ( value == 6)
	{
		LED_PORT_C &= ~_BV(Seg_A);
		LED_PORT_C |= _BV(Seg_B);
		LED_PORT_C |= _BV(Seg_C);
		LED_PORT_C &= ~_BV(Seg_D);
	}	
	else if ( value == 7)
	{
		LED_PORT_C |= _BV(Seg_A);
		LED_PORT_C |= _BV(Seg_B);
		LED_PORT_C |= _BV(Seg_C);
		LED_PORT_C &= ~_BV(Seg_D);
	}	
	else if ( value == 8)
	{
		LED_PORT_C &= ~_BV(Seg_A);
		LED_PORT_C &= ~_BV(Seg_B);
		LED_PORT_C &= ~_BV(Seg_C);
		LED_PORT_C |= _BV(Seg_D);
	}	
	else if ( value == 9)
	{
		LED_PORT_C |= _BV(Seg_A);
		LED_PORT_C &= ~_BV(Seg_B);
		LED_PORT_C &= ~_BV(Seg_C);
		LED_PORT_C |= _BV(Seg_D);
	}	

}

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
