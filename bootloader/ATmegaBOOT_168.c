/************************
*  HS485 Bootloader
*************************/

/************************
 ToDo
 - Device Adresse in spezielle Flash-Speicher Adresse (0x1FFC) ==> OK: 14.6.2015
 - in SendAck den Hack ersetzen
 
*************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "boot.h"
#include <util/delay.h>
#include "uart.h"
#include "main.h"
#include "suart.h"
#include <avr/eeprom.h>
#include <stdbool.h> 
#include <avr/pgmspace.h>

// #define DEBUG

//#define OWN_ADDRESS	  	0x1029
#define UART_BAUD_RATE  19200     /* Baudrate */

// define ports 
#define LED_PORT_B PORTB
#define LED_DDR_B  DDRB
#define LED_PORT_D PORTD
#define LED_DDR_D  DDRD
#define RS485_Send    PIND2 /* Send */
#define RS485_Recv    PIND3 /* Recv */
#define LED_red   PINB3 	/* RGB-LED rot  ==> Error */
#define LED_blue  PIND6 	/* RGB-LED blau ==> Bootloader */
#define LED_green PIND4 	/* RGB-LED grün ==> Anwendungsprogramm */

/* define various device id's */
#define CRC16_POLYGON 			0x1002
#define FRAME_START_LONG 		0xFD
#define FRAME_START_SHORT		0xFE
#define ESCAPE_CHAR			  	0xFC
#define MAX_RX_FRAME_LENGTH   	255
#define CONTAINS_SENDER(x)  	(((x) & (1<<3)) !=0)

/* function prototypes */
void byte_response(uint8_t);
void nothing_response(void);
void flash_led(uint8_t);
void error_led(uint8_t);
void rgb_led(uint8_t red, uint8_t green, uint8_t blue );
void crc16_init(void);
void crc16_shift(unsigned char);
void SendAck(int, unsigned char, unsigned char);
void SendByte(unsigned char);
void SendDataByte(unsigned char);
void setup(void);
void AddressCharToHex(unsigned char *p_ucAddress, unsigned long *p_ulAddress);

unsigned char	temp;           /* Variable */
unsigned int  	crc16_register;	// Register mit CRC16 Code	

/*********************************************************************************** 
 Program Page of Controller
***********************************************************************************/
void program_page (uint32_t page, uint8_t *buf)
{
    uint16_t i;
    uint8_t sreg;
 
    /* Disable interrupts */
    sreg = SREG;
    cli();
 
    eeprom_busy_wait ();
 
    boot_page_erase (page);
    boot_spm_busy_wait ();      /* Wait until the memory is erased. */
 
    for (i=0; i<SPM_PAGESIZE; i+=2)
    {
        /* Set up little-endian word. */
        uint16_t w = *buf++;
        w += (*buf++) << 8;
 
        boot_page_fill (page + i, w);
    }
 
    boot_page_write (page);     /* Store buffer in flash page.		*/
    boot_spm_busy_wait();       /* Wait until the memory is written.*/
 
    /* Reenable RWW-section again. We need this if we want to jump back */
    /* to the application after bootloading. */
    boot_rww_enable ();
 
    /* Re-enable interrupts (if they were ever enabled). */
    SREG = sreg;
}
	
/*********************************************************************************** 
 Hauptprogramm
***********************************************************************************/
int main()
{
	unsigned int ch = 0;     /* Empfangenes Zeichen + Statuscode */
	uint16_t v, w;
	
	unsigned long ulAddress1;
	bool address_ok = false;
	
	uint8_t ControlByte=0;
	unsigned char Escape = 0;
	unsigned char AddressLen;
	unsigned char AddressPointer=0;
	unsigned char DataLength=0;
	unsigned char FramePointer=0;
	
	unsigned char FrameData[MAX_RX_FRAME_LENGTH];
	
	bool START_CHAR_SHORT = false;
	
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
	void (*start)( void ) = 0x0000;        /* Funktionspointer auf 0x0000 */
	
	// Device Addresse aus Flash lesen: 
	uint32_t OWN_ADDRESS = pgm_read_dword(0x1FFC);
	
	setup();	
	
	while ( 1 )
	{
		ch = uart_getc();
		// break;
		if ( ch & UART_NO_DATA )
		{
			// Keine Daten empfangen
			;
		}
		else
		{		
			// Daten empfangen
			if (ch == ESCAPE_CHAR && Escape == 0)
			{
				Escape = 1;
				continue;		// zurueck zum Beginn der while Schleife, folgendes auslassen
			}

			if (ch == FRAME_START_LONG ) // Startzeichen 0xFD
			{
				Escape = 0;
				AddressPointer = 0;
				AddressLen = 4;
				FramePointer = 0;
				crc16_init();
				crc16_shift(ch);
				// Flash LED off:
				//LED_PORT |= _BV(LED1);
				START_CHAR_SHORT = 0;
			}
			else if (ch == FRAME_START_SHORT ) // Startzeichen 0xFE
			{
				//sputs("0xFE\n");
				START_CHAR_SHORT = 1;
				
			}
			else if ( START_CHAR_SHORT == 0 )
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
					#ifdef DEBUG 
						sputchar(ch + 0x30);
					#endif
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
					if (ulAddress1 == OWN_ADDRESS )		
					{
						address_ok = true;
						#ifdef DEBUG 
							sputs("\nAdr. OK\n");
						#endif	
					}
					else
					{
						#ifdef DEBUG 
							sputs("\nAdr. Falsch\n");
							address_ok = false;
						#endif
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
				    if ( address_ok == true )
					{
				
						FrameData[FramePointer] = ch;
						crc16_shift(ch);
						//sputs("\n\r Daten empfangen" );
						
						// Daten komplett empfangen
						if (FramePointer == (DataLength - 1))		
						{
							crc16_shift(0);
							crc16_shift(0);
							FramePointer = 0;
							AddressPointer = 0;
							//sputs("\n\r Daten komplett empfangen" );
							
							// Checksumme überprüfen
							if (crc16_register == 0)
							{	
							    //sputs("\n\r CRC OK" );
								// Programming fertig:	
								if ( FrameData[0] == 0x67 )
								{
									//EIND = 0;
									#ifdef DEBUG 
										sputs("revc 0x67\n" );
									#endif	
										
									SendAck(2, (ControlByte >> 1) & 0x03, 0);
									break;
								}
								// Vorbereitung Programming:
								if (FrameData[0] == 0x70)
								{
									// Send ACK
									SendAck(2, (ControlByte >> 1) & 0x03, 0x52);
									//SendAck(2, ControlByte);
									#ifdef DEBUG 
										sputs("recv 0x70\n" );
									#endif	
									// kurz warten
									_delay_ms(4);
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
									
									program_page(address.word, buff);
									
									//if (++error_count == MAX_ERROR_COUNT)
									//  appStart();
										
									// Send ACK
									SendAck(1, (ControlByte >> 1) & 0x03, 0);
								}
							}
							else
							{
								// Prüfsumme falsch
								#ifdef DEBUG 
									sputs("CRC ERROR\n");
								#endif	
								//error_led(3);
								rgb_led(1,0,0);
								continue;
							}
						}
						if (FramePointer >= MAX_RX_FRAME_LENGTH){
							// Maximale Framelänge überschritten!
							#ifdef DEBUG 
								sputs("ERROR 2\n");
							#endif	
							//error_led(3);
							rgb_led(1,0,0);
							continue;
						}
						FramePointer++;
					}	
				}
			}
		}
		//_delay_ms(1000);
	}	
	
	sputs("Jump to 0x0000\n");
	_delay_ms(1000);

	/* vor Rücksprung eventuell benutzte Hardware deaktivieren
		 und Interrupts global deaktivieren, da kein "echter" Reset erfolgt */

	/* Interrupt Vektoren wieder gerade biegen */
	cli();  // disable global interrupts
	
	/* ATmega8  */
	temp = GICR;
	GICR = temp | (1<<IVCE);
	GICR = temp & ~(1<<IVSEL);
 
    /* ATmega88  */
	/*
    temp = MCUCR;
    MCUCR = temp | (1<<IVCE);
    MCUCR = temp & ~(1<<IVSEL);
    */
	
    /* Reset */
    /* Rücksprung zur Adresse 0x0000 */
	start();
	
	return 0;
}

/******************************************************************************************** 
 Funktionen
*********************************************************************************************/
/*********************************************************************************** 
 Send ACK
************************************************************************************/
void SendAck(int typ, unsigned char Empfangsfolgenummer, uint8_t ControlByte)
{
	int i;
	unsigned char StartByte;
	//uint8_t ControlByte;
	unsigned char DataLength;
	unsigned char FrameData[MAX_RX_FRAME_LENGTH];
	
	// Sende:
	_delay_ms(8);	
	LED_PORT_D |= _BV(RS485_Send);
	//_delay_ms(1);

	// Hack ==> ToDo: saubere Loesung!	
	if ( ControlByte == 0 )
	{
		ControlByte = (( Empfangsfolgenummer & 0x03 ) << 5 ) | 0x11;
	}
	
	#ifdef DEBUG 
	  sputs("Send Controlbyte\n");
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
	
	// _delay_ms(2);
	_delay_ms(4);
	// Nicht Senden:
 	LED_PORT_D &= ~_BV(RS485_Send);

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
void rgb_led( uint8_t red, uint8_t green, uint8_t blue )
{
  // LED rot ON
  if ( red == 1 )
	{
	  LED_PORT_B &= ~_BV(LED_red);
	}
  else
  {
	  LED_PORT_B |= _BV(LED_red);
	}	

  if ( green == 1 )
	{ 
		//LED_PORT_B &= ~_BV(LED_green); 
		LED_PORT_D |= _BV(LED_green);
	}		
	else
	{
		//LED_PORT_B |= _BV(LED_green);
		LED_PORT_D &= ~_BV(LED_green); 
		
	}

	if ( blue == 1 )
	{
		//LED_PORT_B &= ~_BV(LED_blue);
		LED_PORT_D |= _BV(LED_blue);
	}
	else
	{
		//LED_PORT_B |= _BV(LED_blue);
		LED_PORT_D &= ~_BV(LED_blue);
		
	}
}

/*********************************************************************************** 
 setup
************************************************************************************/
void setup(void)
{
	// set LED pin as output 
	//LED_DDR_D |= _BV(LED1);
	//LED_DDR_D |= _BV(LED2);
	LED_DDR_D |= _BV(RS485_Send);
	
	// set RGB LED pin as output 
	LED_DDR_B |= _BV(LED_red);
	//LED_DDR_B |= _BV(LED_green);
	LED_DDR_D |= _BV(LED_green);
	//LED_DDR_B |= _BV(LED_blue);
	LED_DDR_D |= _BV(LED_blue);

	// Error LED aus:
	//LED_PORT_D |= _BV(LED2);
	
	// Nicht Senden (LOW):
	LED_PORT_D &= ~_BV(RS485_Send);
	// Immer empfangen (LOW):
	LED_PORT_D &= ~_BV(RS485_Recv);

	/* Interrupt Vektoren verbiegen */
	char sregtemp = SREG;
	cli();   // disable global interrupts
	/* ATmega8  */
	temp = GICR;
	GICR = temp | (1<<IVCE);
	GICR = temp | (1<<IVSEL);
	/* ATmega88  */
	/*
	temp = MCUCR;
    MCUCR = temp | (1<<IVCE);
    MCUCR = temp | (1<<IVSEL);
	*/
	
	SREG = sregtemp;
	
	/* Einstellen der Baudrate und aktivieren der Interrupts */
	uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) ); 
	suart_init();
	sei();  // enable global interrupts

	sputs("\n\rBootloader Setup\n" );
	//_delay_ms(1000);
	// RGB LED Blau an:
    rgb_led(0,0,1);

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

