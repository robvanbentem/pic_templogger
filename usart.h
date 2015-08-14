#ifndef USART_H
#define	USART_H

#ifndef USART_BUFLEN
#define USART_BUFLEN 64
#endif

#ifndef USART_MAX_TIMEOUT_COUNT
#define USART_MAX_TIMEOUT_COUNT 3
#endif

volatile unsigned char cmd = 0;

volatile unsigned char rx_byte; // usart received byte

volatile unsigned char rx_buf[USART_BUFLEN]; // store received bytes in the rx buffer
volatile unsigned char rx_buf_index = 0; // rx buffer index

// tmr1 timeout countdown, will trigger rxto = 1 after 3 tmr1 overflows
volatile unsigned char rx_timeout_cnt_reset = USART_MAX_TIMEOUT_COUNT;
volatile unsigned char rx_timeout_cnt = USART_MAX_TIMEOUT_COUNT;
volatile unsigned char rx_timeout = 0; // rx receive timeout flag


// TIMER FUNC.

inline void tmr1_begin();
inline void tmr1_end();
inline void tmr1_reset();


// USART FUNC.

void setup_usart();

// writing
void USART_putc(unsigned char c);
void USART_puts(unsigned char *s);
void USART_put_eol();

//reading
void USART_read_byte();
void USART_read_to_buf();
char USART_search(char *s);
char USART_search_chr(char s);

//misc
inline void USART_interrupt();
inline void USART_clear_buf();
void USART_store_buf();
void USART_dump_buf();

#endif	/* USART_H */
