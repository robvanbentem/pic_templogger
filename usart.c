#include "config.h"
#include <xc.h>
#include "usart.h"
#include <stdio.h>
#include <string.h>

// INIT. FUNC.

void setup_usart() {
    TXSTAbits.TXEN = 1; // enable transmitter
    TXSTAbits.BRGH = 1; // high baud rate mode
    RCSTAbits.CREN = 1; // disable continous receiving

    // configure I/O pins
    TRISBbits.RB5 = 1; // RX pin is input
    TRISBbits.RB7 = 1; // TX pin is input (automatically configured)

    SPBRG = 103; //(unsigned char)(_XTAL_FREQ / (16 * 19200) - 1); // set baud rate to 19200 baud (48MHz/(16*baudrate))-1

    PIE1bits.RCIE = 0; // 1/0 enable/disable USART receive interrupt
    RCSTAbits.SPEN = 0; // enable USART


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

void tmr1_begin() {
    tmr1_reset();
    PIR1bits.TMR1IF = 0; // reset overflow
    T1CONbits.TMR1ON = 1; // turn it on
}

void tmr1_end() {
    T1CONbits.TMR1ON = 0; // turn it off
    PIR1bits.TMR1IF = 0; // reset overflow
}

void tmr1_reset() {
    rx_timeout = 0;
    rx_timeout_cnt = rx_timeout_cnt_reset;
    TMR1L = 0;
    TMR1H = 0;
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

    while (!PIR1bits.RCIF && !rx_timeout);

    if (RCSTAbits.FERR == 1) { // framing error
        rx_byte = RCREG;
        rx_byte = 0;
        return;
    }

    rx_byte = RCREG;
}

void USART_put_eol() {
    USART_putc('\r');
    USART_putc('\n');
}

void USART_read_to_buf() {
    USART_clear_buf();
    tmr1_begin();

    while (rx_timeout == 0) {
        USART_read_byte();
        if (rx_timeout == 0) {
            tmr1_reset();
            if (rx_buf_index >= USART_BUFLEN) {
                USART_clear_buf();
            }
            rx_buf[rx_buf_index++] = rx_byte;
        }
    }
}

void USART_interrupt() {
    if (PIR1bits.RCIF == 1) {
        cmd = 0xA1;
        return;
    } else if (PIR1bits.TMR1IF) {
        if (--rx_timeout_cnt == 0) {
            rx_timeout = 1;
            tmr1_end();
        } else {
            //Turn timer off, reset it and turn on again
            T1CONbits.TMR1ON = 0;

            TMR1L = 0;
            TMR1H = 0;
            PIR1bits.TMR1IF = 0;

            T1CONbits.TMR1ON = 1;
        }
    }
}

char USART_search(char *s) {
    USART_read_to_buf();

    char *pos;
    pos = strstr(rx_buf, s);

    return (pos - rx_buf) < rx_buf_index;
}

char USART_search_chr(char c) {
    tmr1_begin();

    while (rx_timeout == 0) {
        USART_read_byte();
        if (rx_byte == c) {
            tmr1_end();

            return 1;
        } else if (rx_timeout == 0) {
            tmr1_reset();
        }
    }

    return 0;
}

inline void USART_clear_buf() {
    rx_buf_index = 0;
}

void USART_store_buf() {
    for (char n = 0; n < USART_BUFLEN; n++) {
        eeprom_write(n, rx_buf[n]);
    }

}

void USART_dump_buf() {
    USART_putc('<');
    for (char n = 0; n < rx_buf_index; n++) {
        USART_putc(rx_buf[n]);
    }
    USART_putc('>');
    USART_put_eol();
}
