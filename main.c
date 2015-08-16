/* 
 * File:   main.c
 * Author: rob
 *
 * Created on 23 December 2014, 12:45
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <xc.h>
#include <pic18f13k50.h>

#include "onewire.h"
#include "usart.h"

// AT Commands
#define AT_ECHO_OFF (char*)"ATE0"
#define AT_MUX_ON (char*)"AT+CIPMUX=1"
#define AT_RESET (char*)"AT+RST"
#define AT_SEND_OPEN (char*)"AT+CIPSTART=4,\"TCP\",\"192.168.1.1\",4555"
#define AT_SEND_CLOSE (char*)"AT+CIPCLOSE=4"

#define AT_RESP_OK (char*)"OK"
#define AT_RESP_ACK (char*)"+IPD,4,3:ACK4,CLOSED\r\n"

char temp_sign;
char temp_degr;

uint16_t timer0_reset;
volatile unsigned int report_interval_counter = 0;
unsigned int report_interval = S_DEVICE_REPORT_INTERVAL;

// HELPER FUNC.

void delay(unsigned short ms) {
    unsigned short delay_counter;
    for (delay_counter = 0; delay_counter < ms; delay_counter++) {
        __delay_ms(1);
    }
}


// INITIALIZATION FUNC.

void setup() {
    ANSEL = 0;
    ANSELH = 0;
    TRISAbits.RA5 = 0;
    TRISB = 0;
    TRISC = 0;
    PORTB = 0;
    PORTC = 0;

    OSCCON = 0b11100000; // Set 8Mhz (with PLL x4 it's 32Mhz)

    T0CONbits.T08BIT = 0; // 16 bit timer
    T0CONbits.T0CS = 0; // use main clock
    T0CONbits.PSA = 0; // use a prescaler
    T0CONbits.T0PS = 0b111; // set prescaler to 1:256

    timer0_reset = 2^16 - (_XTAL_FREQ / 4 / 256); // overflow after 1s

    setup_usart();

    WIFI_OFF;

    INTCONbits.PEIE = 1; // enable peripheral interrupts
    INTCONbits.GIE = 1; // enable global interrupt
    RCSTAbits.SPEN = 1; // enable uart
}

inline void tmr0_start() {
    INTCONbits.TMR0IE = 0;
    T0CONbits.TMR0ON = 0;
    TMR0H = timer0_reset >> 8;
    TMR0L = timer0_reset & 0x00ff;

    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1;
    T0CONbits.TMR0ON = 1;
}

void tmr0_stop() {
    INTCONbits.TMR0IE = 0;
    T0CONbits.TMR0ON = 0;
    INTCONbits.TMR0IF = 0;
}



// INTERRUPT

void interrupt rx() {
    USART_interrupt();

    if (TMR0IF == 1) {
        report_interval_counter += 1;
        tmr0_start();
    }
}

// APP FUNC.

char get_temp() {
    if (ow_reset()) {

        ow_write_byte(0xCC);
        ow_write_byte(0x44);

        delay(800);

        if (ow_reset()) {
            ow_write_byte(0xCC);
            ow_write_byte(0xBE);

            temp_degr = ow_read_byte();
            temp_sign = ow_read_byte();

            return 1;
        }
    }

    return 0;
}

void esp_cmd(unsigned char *data) {
    USART_puts(data);
    USART_put_eol();
}

char wifi_setup() {
    esp_cmd(AT_ECHO_OFF);
    if (USART_search(AT_RESP_OK)) {

        USART_clear_buf();
        esp_cmd(AT_MUX_ON);
        if (USART_search(AT_RESP_OK)) {

            USART_clear_buf();
            return 1;
        }
    }

    return 0;
}

char wifi_boot() {
    WIFI_ON;
    RCSTAbits.CREN = 1;
    USART_read_to_buf();

    return wifi_setup();
}

char wifi_reboot() {
    WIFI_OFF;
    delay(10);

    return wifi_boot();
}

char wifi_report_temp() {
    if (get_temp() == 1) {
        esp_cmd(AT_SEND_OPEN);
        if (USART_search(AT_RESP_OK)) {
            uint16_t itemp = (temp_sign << 8) | temp_degr;
            char data[64];
            unsigned char n = sprintf(data, "GET /log?d=%s&a=%s&v=%d HTTP/1.0\r\n\r\n", S_DEVICE_ID, S_DEVICE_ATTR_TEMP, itemp);

            char cmd[18];
            sprintf(cmd, "AT+CIPSEND=4,%d", n);

            esp_cmd((char*) cmd);
            if (USART_search_chr('>')) {
                // Write temperture
                USART_puts((char*) data);
                if (USART_search(AT_RESP_ACK)) {

                    return 1;
                }
            }
        }

    }

    return 0;
}

int main() {
    setup();
    tmr0_start();

    while (1) {
        char result = 0;
        if (wifi_boot() == 1) {
            result = wifi_report_temp();
        }

        if (result == 1) {
            esp_cmd((char*) "AT+GSLP=60000");
            USART_read_to_buf();

            while (report_interval_counter < report_interval) {
                SLEEP();
            }

        } else {
            // sleep for 3s and try again.
            report_interval = report_interval_counter + 3;

            while (report_interval_counter < report_interval) {
                SLEEP();
            }
            report_interval = S_DEVICE_REPORT_INTERVAL;
        }

        report_interval_counter = 0;
        WIFI_OFF;
        delay(10);
    }

}

