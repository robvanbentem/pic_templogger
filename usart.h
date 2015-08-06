#ifndef USART_H
#define	USART_H

#ifdef	__cplusplus
extern "C" {
#endif

#define BUFLEN 64
    
volatile unsigned char t;

unsigned char *rxbuf[BUFLEN];
unsigned char rxcnt = 0;

volatile unsigned char rxto = 0;
volatile unsigned char rxtoc = 3;


// TIMER FUNC.
inline void tmr1_reset();
void tmr1_start();


// USART FUNC.

void setup_usart();

void USART_putc(unsigned char c);
void USART_puts(unsigned char *s);
inline void USART_put_eol();

void USART_read_byte();
void read_usart();

#ifdef	__cplusplus
}
#endif

#endif	/* USART_H */

