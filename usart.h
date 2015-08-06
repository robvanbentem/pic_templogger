#ifndef USART_H
#define	USART_H

#ifndef USART_BUFLEN
#define USART_BUFLEN 64
#endif

volatile unsigned char cmd = 0;

volatile unsigned char rx_byte; // usart received byte

volatile unsigned char rxbuf[USART_BUFLEN]; // store received bytes in the rx buffer
volatile unsigned char rxcnt = 0; // position of the rx buffer

// tmr1 timeout countdown, will trigger rxto = 1 after 3 tmr1 overflows
volatile unsigned char rxtoc = 3;
volatile unsigned char rxto = 0; // rx receive timeout flag


// TIMER FUNC.

inline void tmr1_reset();
void tmr1_start();


// USART FUNC.

void setup_usart();

// writing
void USART_putc(unsigned char c);
void USART_puts(unsigned char *s);
inline void USART_put_eol();

//reading
void USART_read_byte();
void read_usart();

//misc
void USART_interrupt();

#endif	/* USART_H */
