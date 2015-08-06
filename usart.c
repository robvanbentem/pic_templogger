#include "config.h"
#include <xc.h>
#include "usart.h"

// INIT. FUNC.

void setup_usart() {
    TXSTAbits.TXEN = 1; // enable transmitter
    TXSTAbits.BRGH = 1; // high baud rate mode
    RCSTAbits.CREN = 1; // enable continous receiving

    // configure I/O pins
    TRISBbits.RB5 = 1; // RX pin is input
    TRISBbits.RB7 = 1; // TX pin is input (automatically configured)

    SPBRG = 103; // set baud rate to 19200 baud (32MHz/(16*baudrate))-1

    PIE1bits.RCIE = 1; // 1/0 enable/disable USART receive interrupt
    RCSTAbits.SPEN = 1; // enable USART


    /*
     * Configure Timer1 with 1:8 prescaler to use as timeout timer
     */
    T1CONbits.RD16 = 0; // set 16 bit mode in two 8 bit registers
    T1CONbits.TMR1CS = 0; // use internal clock source
    T1CONbits.T1CKPS = 0b11; // set prescaler to 1:8
    PIE1bits.TMR1IE = 1; // enable timer1 interrupts

    T1CONbits.TMR1ON = 0; // dont turn on
    PIR1bits.TMR1IF = 0; // clear overflow bit
}


// TIMER FUNC.

inline void tmr1_reset() {
    /**
     * this will overflow after about ~65ms when running at 32Mhz.
     * calc: 1sec / (32Mhz / 4FOSC / 1:8 prescaler / 2^16 (16 bit reg.)) = 0.065536sec
     */
    TMR1L = 0;
    TMR1H = 0;
}

void tmr1_start() {
    PIR1bits.TMR1IF = 0; // reset overflow

    rxtoc = 3;
    rxto = 0;
    tmr1_reset();

    T1CONbits.TMR1ON = 1; // turn it on
}

// USART FUNC.

void USART_putc(unsigned char c) {
    while (!TXSTAbits.TRMT); // wait until transmit shift register is empty
    TXREG = c; // write character to TXREG and start transmission

}

void USART_puts(unsigned char *s) {
    while (*s) {
        USART_putc(*s); // send character pointed to by s
        s++; // increase pointer location to the next character
    }
}

void USART_read_byte() {
    if (RCSTAbits.OERR == 1) { // overun error
        RCSTAbits.CREN = 0; // clear overrun if yes
        RCSTAbits.CREN = 1;
    }

    while (!PIR1bits.RCIF && !rxto);

    if (RCSTAbits.FERR == 1) { // framing error
        t = RCREG;
        t = 0;
        return;
    }

    t = RCREG;
}

inline void USART_put_eol(){
    USART_putc('\r');
    USART_putc('\n');
}

void read_usart() {
    tmr1_start();

    while (rxto == 0) {
        USART_read_byte();
        *rxbuf[++rxcnt] = t;
        tmr1_reset();
    }

    rxto = 0;
    rxcnt = 0;
    PIE1bits.RCIE = 1;
}