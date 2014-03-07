//#ifdef _AVR_IOM8_H_

#define STX     PB1     // OC1A on Mega8
#define STXDDR  DDRB
/*
#else
#error
#error Please add the defines:
#error
#error STX, STXDDR
#error
#error for the new target !
#error
#endif
*/
void suart_init( void );
void sputchar(char val );
void sputs(const char *txt );

