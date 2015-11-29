/***************************
*  HS485 Applikation
* 
* Ansteuerung ELV USB-SI6
****************************/

/************************
 ToDo
 - Interruptsteuerung beim UART-Empfang ==> OK, war bereits realisiert
 - Device Adresse in spezielle Flash-Speicher Adresse (0x1FFC) ==> OK (Bootlodaer File: device_addr.c): 14.6.2015
 - in SendAck2 0x19 ersetzen
 - Abfrage Relais Status ==> OK
 - in SendState 0xA1 ersetzen
 - Programmierung über HS485 nicht möglich (kein Sprung zum Bootloader ?!)
*************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "uart.h"
#include "pin_defs.h"
#include <stdbool.h> 
#include "uart.h"
#include "main.h"
#include "suart.h"
#include <avr/pgmspace.h>


//#define DEBUG 
#define UART_BAUD_RATE	19200
 
#define LED_PORT_B PORTB
#define LED_DDR_B  DDRB

#define LED_PORT_C PORTC
#define LED_DDR_C  DDRC

#define LED_PORT_D PORTD
#define LED_DDR_D  DDRD

// INPUT
#define STATE1      PINC0
#define STATE2      PINC1
#define STATE3      PINC2
#define STATE4      PINC3
#define STATE5      PINC4
#define STATE6      PINC5

// OUTPUT
#define RELAIS1     PIND4
#define RELAIS2     PIND5
#define RELAIS3     PIND6
#define RELAIS4     PIND7
#define RELAIS5     PINB3
#define RELAIS6     PINB4

#define RS485    PIND2

// TX for soft UART in suart.h:
// #define STX     PB1

//#define LED_red   PINB3 /* RGB-LED rot  ==> Error */
//#define LED_blue  PIND6 /* RGB-LED blau ==> Bootloader */
//#define LED_green PIND4 /* RGB-LED grün ==> Anwendungsprogramm */


/*
#define Seg_A	PINC0
#define Seg_B	PINC1
#define Seg_C	PINC2
#define Seg_D	PINC3
*/
 
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
void SendAck2(int, unsigned char);
void SendByte(unsigned char);
void SendDataByte(unsigned char);
void setup(void);
void AddressCharToHex(unsigned char *p_ucAddress, unsigned long *p_ulAddress);
void switch_relais( uint8_t, uint8_t);
int get_relais_state(uint8_t number);
void SendState(unsigned char Empfangsfolgenummer, int actor, int state);


/* some variables */
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

uint8_t ControlByte;
unsigned char Escape = 0;
unsigned char AddressLen;
unsigned char AddressPointer;
unsigned char DataLength;
unsigned char FramePointer;
// unsigned int  LocalCRCRegister;
unsigned int  crc16_register;				// Register mit CRC16 Code
//unsigned uint16_t  crc16_register;		// Register mit CRC16 Code
unsigned char FrameData[MAX_RX_FRAME_LENGTH];
uint8_t SenderAddress[4];
 
typedef void (*boot_reset_fptr_t)(void);
boot_reset_fptr_t bootloader = (boot_reset_fptr_t) 0x0C00;


/*********************************************************************************** 
 Hauptprogramm
************************************************************************************/
int main()
{
	unsigned int 	ch = 0;     /* Empfangenes Zeichen + Statuscode */
	unsigned long   ulAddress1;
	bool            address_ok = false;
    unsigned int    actor, relais_state;
  
	setup();

	sputs("\n\rHier ist die Test-App...");
	// gruen:
	//rgb_led(0,1,0);  /* LED gruen ==> ON */
    unsigned int state_update = 0;
	
	// Device Addresse aus Flash lesen: 
	uint32_t OWN_ADDRESS = pgm_read_dword(0x1FFC);
	// uint32_t OWN_ADDRESS = 0x1029;
	// uint32_t OWN_ADDRESS = 0x1001;
	
	while (1) 
	{
		ch = uart_getc();
		if ( ch & UART_NO_DATA )
		{
		    // Idle: Recv No Datas:
			// ==> Place for Idle code:
            if ( state_update == 0 )
            {
                //rgb_led(0,1,0);  // LED gruen ==> ON 
                //_delay_ms(500);
                //rgb_led(0,0,1);  // LED blau ==> ON 
                //_delay_ms(500);
                //rgb_led(1,0,0);  // LED rot ==> ON 
                //_delay_ms(200);
                #ifdef DEBUG 
                  //sputs("\nUART no data!");
                #endif	
            }
 		}
		else
		{
            state_update = 1;
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
					// Adresse mit der Eigenen vergleichen:
					if (ulAddress1 == OWN_ADDRESS )		
					{
						address_ok = true;
						//#ifdef DEBUG 
							sputs("\nAdresse OK");
						//#endif	
					}
					else
					{
						sputs("\nAdresse Falsch");
                        address_ok = false;
						state_update = 0;
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
				}
				
				else // Daten empfangen
				{
				    if ( address_ok == true )
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
							
							// Checksumme überprüfen
							if (crc16_register == 0)
							{	
								// Get Aktor Status:	
								if ( FrameData[0] == 0x53 )
								{
									// Actor Number:
                                    actor = FrameData[1];
									if (( actor >= 0x01 ) && ( actor <= 0x06 ))
									{
										// Get Relais State:        
                                        relais_state = get_relais_state(actor);  
                                        // SendState(Empfangsfolgenummer, actor, relais_state);
                                        SendState((ControlByte >> 1) & 0x03, actor, relais_state);
									}
								}
								
								// Programming fertig:	
								if ( FrameData[0] == 0x67 )
								{
									SendAck(1, (ControlByte >> 1) & 0x03);
								}
								
								// p: Jump to bootloader (Flash Block size):
								if (FrameData[0] == 0x70)
								{
									// kurz warten
									_delay_ms(4);
									// Send ACK
									SendAck(2, (ControlByte >> 1) & 0x03);
									#ifdef DEBUG 
										sputs("\nJump to bootloader");
									#endif	
                                    
                                    // LOW:
                                    LED_PORT_D &= ~_BV(RELAIS1);
                                    _delay_ms(500);
                                    // HIGH:
                                    LED_PORT_D |= _BV(RELAIS1);
                                    
									// LED Blau ==> ON	
									//rgb_led(0,0,1);	
									// Jump to bootloader:
									bootloader();
								}	
								
								// Set Aktor:	
								if ( FrameData[0] == 0x73 )
								{
									// Aktor Nummer:
									if (( FrameData[2] >= 0x01 ) && ( FrameData[2] <= 0x06 ))
									{
										// Zustand:        
										if ( FrameData[3] == 0x00 )
										{
											switch_relais(FrameData[2],0);  // Relais 1 rot ==> OFF 
										}
										if ( FrameData[3] == 0x01 )
										{
											switch_relais(FrameData[2],1);  // Relais 1 rot ==> ON 
										}
									}
								}
                                
							}
							else
							{
								// Prüfsumme falsch
								// LED rot ==> ON
								//rgb_led(1,0,0);
								continue;
							}
						}
						if (FramePointer >= MAX_RX_FRAME_LENGTH){
							// Maximale Framelänge überschritten!
							// LED rot ==> ON
							//rgb_led(1,0,0);
							continue;
						}
						FramePointer++;
					}
					sputs("*");
				}
					sputs("#");				
				
			}
			//flash_led(2);
			//sputs(".");	
			
		} 	/* else = serial data  */ 		
	}	    /* end of forever loop */
}

/******************************************************************************************** 
 Funktionen
*********************************************************************************************/
/*********************************************************************************** 
 setup
************************************************************************************/
void setup(void)
{

	#define STATE1      PINC0
	#define STATE2      PINC1
	#define STATE3      PINC2
	#define STATE4      PINC3
	#define STATE5      PINC4
	#define STATE6      PINC5

	#define RELAIS1     PIND4
	#define RELAIS2     PIND5
	#define RELAIS3     PIND6
	#define RELAIS4     PIND7
	#define RELAIS5     PINB3
	#define RELAIS6     PINB4	
	
	/* set RS485 pin as output */
	LED_DDR_D |= _BV(RS485);
	
	/* set RELAIS pins as output */
	LED_DDR_D |= _BV(RELAIS1);
	LED_DDR_D |= _BV(RELAIS2);
	LED_DDR_D |= _BV(RELAIS3);
	LED_DDR_D |= _BV(RELAIS4);
	LED_DDR_B |= _BV(RELAIS5);
	LED_DDR_B |= _BV(RELAIS6);
	
	/* set STATE pins as input */
	LED_DDR_C &= ~_BV(STATE1);
	LED_DDR_C &= ~_BV(STATE2);
	LED_DDR_C &= ~_BV(STATE3);
	LED_DDR_C &= ~_BV(STATE4);
	LED_DDR_C &= ~_BV(STATE5);
	LED_DDR_C &= ~_BV(STATE6);

	// Nicht Senden:
	LED_PORT_D &= ~_BV(RS485);
	
	// Relais 1 ==> 1:
	LED_PORT_D |= _BV(RELAIS1);
	// Relais 2 ==> 1:
	LED_PORT_D |= _BV(RELAIS2);
	// Relais 3 ==> 1:
	LED_PORT_D |= _BV(RELAIS3);
	// Relais 4 ==> 1:
	LED_PORT_D |= _BV(RELAIS4);
	// Relais 5 ==> 1:
	LED_PORT_B |= _BV(RELAIS5);
	// Relais 6 ==> 1:
	LED_PORT_B |= _BV(RELAIS6);

	// alle Relais aus
	/*
  if ( red == 1 )
	{
	  LED_PORT_B &= ~_BV(LED_red);
	}
  else
  {
	  LED_PORT_B |= _BV(LED_red);
	}	
	*/

    /*
     *  Initialize UART library, pass baudrate and AVR cpu clock
     *  with the macro 
     *  UART_BAUD_SELECT() (normal speed mode )
     */
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
	// #ifdef DEBUG 
	//  sputs(sprintf("\nSend Controlbyte: 0x%02x", ControlByte));
	// #endif	

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
 Send ACK
 
 SendAck(2, (ControlByte >> 1) & 0x03);
 
************************************************************************************/
void SendAck2(int typ, unsigned char Empfangsfolgenummer)
{
	int i;
	unsigned char StartByte;
	
	// Sende:
	_delay_ms(8);	
	LED_PORT |= _BV(RS485);
	//_delay_ms(1);
	
	ControlByte = (( Empfangsfolgenummer & 0x03 ) << 5 ) | 0x11;
	
	// NUR zum TESTEN!!! Bitte fixen und dann entfernen!!
	ControlByte = 0x19;
	
	// #ifdef DEBUG 
	//  sputs(sprintf("\nSend Controlbyte: 0x%02x", ControlByte));
	// #endif	

	DataLength = 0;
	StartByte = FRAME_START_LONG;
	// Frame prüfen (== FD):
	
	crc16_init();
	// Startzeichen:
	SendByte(StartByte);			
	crc16_shift(StartByte);
	// Master address:
	SendDataByte(0);	
	crc16_shift(0);
	SendDataByte(0);	
	crc16_shift(0);
	SendDataByte(0);	
	crc16_shift(0);
	SendDataByte(0);	
	crc16_shift(0);

	// Controllbyte:
	SendDataByte(ControlByte);	
	crc16_shift(ControlByte);

    // Sender address:
    DataLength = typ;
	for ( i = 0; i < DataLength; i++ )				
	{
		SendDataByte(to_address.byte[i]);
		crc16_shift(to_address.byte[i]);
	}	
    
    // Count of Data Bytes:
	SendByte(0x02);	
	crc16_shift(0x02);
    
	crc16_shift(0);
	crc16_shift(0);
	// CRC16-Checksumme:
	SendDataByte((crc16_register >> 8) & 0xff);	
	_delay_ms(1);
	SendDataByte((crc16_register) & 0xff);
	
	_delay_ms(4);
	// Nicht Senden:
 	LED_PORT &= ~_BV(RS485);
	// return 0;
}

/*********************************************************************************** 
 Send State
 
 SendState(2, (ControlByte >> 1) & 0x03);
 
************************************************************************************/
void SendState(unsigned char Empfangsfolgenummer, int actor, int state)
{
	int i;
	unsigned char StartByte;
	
	// Sende:
	_delay_ms(8);	
	LED_PORT |= _BV(RS485);
	//_delay_ms(1);
	
	ControlByte = (( Empfangsfolgenummer & 0x03 ) << 5 ) | 0x11;
	
	// NUR zum TESTEN!!! Bitte fixen und dann entfernen!!
	ControlByte = 0x1A;
	
	// #ifdef DEBUG 
	//  sputs(sprintf("\nSend Controlbyte: 0x%02x", ControlByte));
	// #endif	

	DataLength = 0;
	StartByte = FRAME_START_LONG;
	// Frame prüfen (== FD):
	
	crc16_init();
	// Startzeichen:
	SendByte(StartByte);			
	crc16_shift(StartByte);
	// Master address:
	SendDataByte(0);	
	crc16_shift(0);
	SendDataByte(0);	
	crc16_shift(0);
	SendDataByte(0);	
	crc16_shift(0);
	SendDataByte(0);	
	crc16_shift(0);

	// Controllbyte:
	SendDataByte(ControlByte);	
	crc16_shift(ControlByte);

    // Sender address:
    DataLength = 4;
	for ( i = 0; i < DataLength; i++ )				
	{
		SendDataByte(to_address.byte[i]);
		crc16_shift(to_address.byte[i]);
	}	
    
    // Count of Data Bytes:
	SendByte(0x04);	
	crc16_shift(0x04);
    
    // Number of Actor:
	SendByte(actor);	
	crc16_shift(actor);
    
    // State
	SendByte(state);	
	crc16_shift(state);
    
	crc16_shift(0);
	crc16_shift(0);
	// CRC16-Checksumme:
	SendDataByte((crc16_register >> 8) & 0xff);	
	_delay_ms(1);
	SendDataByte((crc16_register) & 0xff);
	
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
    _delay_ms(1);
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
        // _delay_ms(1);
		SendByte(c);
		// if (!SendByte(c))
			// return false;
		ucByte &= 0x7f;
	}
    // _delay_ms(1);
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
 switch_relais
************************************************************************************/
void switch_relais( uint8_t number, uint8_t state )
{
	int delay_time = 500;
	
	if ( number == 1 )
	{
		// C0
		if (( state == 1 ) && (!(PINC & (1<<PINC0))))
		{
			// LOW:
			LED_PORT_D &= ~_BV(RELAIS1);
			_delay_ms(delay_time);
			// HIGH:
			LED_PORT_D |= _BV(RELAIS1);
		}	
		if (( state == 0 ) && ((PINC & (1<<PINC0))))
		{
			// LOW:
			LED_PORT_D &= ~_BV(RELAIS1);
			_delay_ms(delay_time);
			// HIGH:
			LED_PORT_D |= _BV(RELAIS1);
		}	
	}	

	if ( number == 2 )
	{
		if (( state == 1 ) && (!(PINC & (1<<PINC1))))
		{
			// LOW:
			LED_PORT_D &= ~_BV(RELAIS2);
			_delay_ms(delay_time);
			// HIGH:
			LED_PORT_D |= _BV(RELAIS2);
		}	
		if (( state == 0 ) && ((PINC & (1<<PINC1))))
		{
			// LOW:
			LED_PORT_D &= ~_BV(RELAIS2);
			_delay_ms(delay_time);
			// HIGH:
			LED_PORT_D |= _BV(RELAIS2);
		}	
	}
	
	if ( number == 3 )
	{
		if (( state == 1 ) && (!(PINC & (1<<PINC2))))
		{
			// LOW:
			LED_PORT_D &= ~_BV(RELAIS3);
			_delay_ms(delay_time);
			// HIGH:
			LED_PORT_D |= _BV(RELAIS3);
		}	
		if (( state == 0 ) && ((PINC & (1<<PINC2))))
		{
			// LOW:
			LED_PORT_D &= ~_BV(RELAIS3);
			_delay_ms(delay_time);
			// HIGH:
			LED_PORT_D |= _BV(RELAIS3);
		}	
	}
	
	if ( number == 4 )
	{
		if (( state == 1 ) && (!(PINC & (1<<PINC3))))
		{
			// LOW:
			LED_PORT_D &= ~_BV(RELAIS4);
			_delay_ms(delay_time);
			// HIGH:
			LED_PORT_D |= _BV(RELAIS4);
		}	
		if (( state == 0 ) && ((PINC & (1<<PINC3))))
		{
			// LOW:
			LED_PORT_D &= ~_BV(RELAIS4);
			_delay_ms(delay_time);
			// HIGH:
			LED_PORT_D |= _BV(RELAIS4);
		}	
	}	

	if ( number == 5 )
	{
		if (( state == 1 ) && (!(PINC & (1<<PINC4))))
		{
			// LOW:
			LED_PORT_B &= ~_BV(RELAIS5);
			_delay_ms(delay_time);
			// HIGH:
			LED_PORT_B |= _BV(RELAIS5);
		}	
		if (( state == 0 ) && ((PINC & (1<<PINC4))))
		{
			// LOW:
			LED_PORT_B &= ~_BV(RELAIS5);
			_delay_ms(delay_time);
			// HIGH:
			LED_PORT_B |= _BV(RELAIS5);
		}	
		
	}	

	if ( number == 6 )
	{
		if (( state == 1 ) && (!(PINC & (1<<PINC5))))
		{
			// LOW:
			LED_PORT_B &= ~_BV(RELAIS6);
			_delay_ms(delay_time);
			// HIGH:
			LED_PORT_B |= _BV(RELAIS6);
		}	
		if (( state == 0 ) && ((PINC & (1<<PINC5))))
		{
			// LOW:
			LED_PORT_B &= ~_BV(RELAIS6);
			_delay_ms(delay_time);
			// HIGH:
			LED_PORT_B |= _BV(RELAIS6);
		}	
		
	}	
	SendAck2(4, (ControlByte >> 1) & 0x03);
}

/*********************************************************************************** 
 get relais_state
************************************************************************************/
int get_relais_state(uint8_t number)
{
	//int delay_time = 500;
	
	if ( number == 1 )
	{
		if (PINC & (1<<PINC0))
		{
            return 1;
		}	
        else
        {
            return 0;
        }
	}	
	else if ( number == 2 )
	{
		if (PINC & (1<<PINC1))
		{
            return 1;
		}	
        else
        {
            return 0;
        }
	}	
	else if ( number == 3 )
	{
		if (PINC & (1<<PINC2))
		{
            return 1;
		}	
        else
        {
            return 0;
        }
	}	
	else if ( number == 4 )
	{
		if (PINC & (1<<PINC3))
		{
            return 1;
		}	
        else
        {
            return 0;
        }
	}	
	else if ( number == 5 )
	{
		if (PINC & (1<<PINC4))
		{
            return 1;
		}	
        else
        {
            return 0;
        }
	}	
	else if ( number == 6 )
	{
		if (PINC & (1<<PINC5))
		{
            return 1;
		}	
        else
        {
            return 0;
        }
	}
	else
	{
		return -1;
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
