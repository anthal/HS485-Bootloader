/************************************************************************/
/*                                                                      */
/*                      Software UART using T1                          */
/*                                                                      */
/*              Author: P. Dannegger                                    */
/*                      danni@specs.de                                  */
/*                                                                      */
/************************************************************************/
#include "main.h"
#include "suart.h"


#define BIT_TIME	(u16)((XTAL + BAUD/2) / BAUD)


volatile unsigned char stx_count;
unsigned char stx_data;

void suart_init( void )
{
  OCR1A = TCNT1 + 1;			// force first compare
  TCCR1A = 1<<COM1A1^1<<COM1A0;		// set OC1A high, T1 mode 0
  TCCR1B = 1<<ICNC1^1<<CS10;		// noise canceler, 1>0 transition,
					// CLK/1, T1 mode 0
  TIFR = 1<<ICF1;			// clear pending interrupt
  TIMSK = 1<<TICIE1^1<<OCIE1A;		// enable tx and wait for start

  stx_count = 0;			// nothing to sent
  STXDDR |= 1<<STX;			// TX output
}

void sputchar(char val )			// send byte
{
  while( stx_count );	// until last byte finished
  stx_data = ~val;		// invert data for Stop bit generation
  stx_count = 10;			// 10 bits: Start + data + Stop
}


void sputs(const char *txt )			// send string
{
  while( *txt )
    sputchar( *txt++ );
}


SIGNAL( SIG_OUTPUT_COMPARE1A )		// tx bit
{
  u8 dout;
  u8 count;

  OCR1A += BIT_TIME;			// next bit slice
  count = stx_count;

  if( count ){
    stx_count = --count;		// count down
    dout = 1<<COM1A1;			// set low on next compare
    if( count != 9 ){			// no start bit
      if( !(stx_data & 1) )		// test inverted data
	dout = 1<<COM1A1^1<<COM1A0;	// set high on next compare
      stx_data >>= 1;			// shift zero in from left
    }
    TCCR1A = dout;
  }
}
