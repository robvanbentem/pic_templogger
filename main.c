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


#define WIFI_ON WIFI = 1
#define WIFI_OFF WIFI = 0

// AT Commands
#define AT_ECHO_OFF (char*)"ATE0"
#define AT_MUX_ON (char*)"AT+CIPMUX=1"
#define AT_SERVER_ON (char*)"AT+CIPSERVER=1,4444"
#define AT_RESET (char*)"AT+RST"
#define AT_SEND_OPEN (char*)"AT+CIPSTART=4,\"TCP\",\"192.168.1.2\",4444"
#define AT_SEND_CLOSE (char*)"AT+CIPCLOSE=4"

#define AT_RESP_OK (char*)"OK"

char sign;
char degr;

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
    TRISB = 0;
    TRISC = 0;
    PORTB = 0;
    PORTC = 0;

    OSCCON = 0b11100000; // Set 8Mhz and select internal osc.
    OSCTUNEbits.SPLLEN = 1; // Enable PPL

    setup_usart();
}

// APP FUNC.

void get_temp() {
    if (ow_reset()) {

        ow_write_byte(0xCC);
        ow_write_byte(0x44);

        delay(1000);

        if (ow_reset()) {

            ow_write_byte(0xCC);
            ow_write_byte(0xBE);

            degr = ow_read_byte();
            sign = ow_read_byte();
        }
    }
}


// INTERRUPT

void interrupt rx() {
    USART_interrupt();
}

void esp_cmd(unsigned char *data) {
    USART_puts(data);
    USART_put_eol();
}

void led_off() {
    PORTC = 0;
}

char wifi_report_temp() {
    get_temp();

    esp_cmd(AT_SEND_OPEN);
    if (USART_search(AT_RESP_OK)) {
        LED3 = 1;
        USART_clear_buf();
        esp_cmd((char*) "AT+CIPSEND=4,2");
        if (USART_search_chr('>')) {
            LED2 = 1;
            // Write temperture
            USART_putc(sign);
            USART_putc(degr);
            if (USART_search(AT_RESP_OK)) {
                LED1 = 1;
                //USART_clear_buf();
                esp_cmd(AT_SEND_CLOSE);
                if (USART_search((char*)"CLOSED")) {
                    LED0 = 1;
                    USART_clear_buf();
                    return 1;
                }
            }
        }
    }

    return 0;
}

char wifi_start_server() {
    PIE1bits.RCIE = 0;

    esp_cmd(AT_SERVER_ON);
    if (USART_search(AT_RESP_OK)) {
        USART_clear_buf();
        PIE1bits.RCIE = 1;
        return 1;
    }

    return 0;
}

char wifi_reset() {
    if (WIFI == 0) {
        WIFI_ON;
    } else {
        esp_cmd(AT_RESET);
        USART_search(AT_RESP_OK);
        USART_clear_buf();
    }

    delay(2500);


    esp_cmd(AT_ECHO_OFF);
    if (USART_search(AT_RESP_OK)) {
        USART_clear_buf();
        esp_cmd(AT_MUX_ON);
        if (USART_search(AT_RESP_OK)) {
            USART_clear_buf();
            LED4 = 1;
            return 1;
        }
    }

    return 0;
}

int main() {
    setup();

    INTCONbits.PEIE = 1; // enable peripheral interrupts
    INTCONbits.GIE = 1; // enable global interrupt

    while (1) {
        if (WIFI == 0) {
            wifi_reset();
        } else {
            LED4 = 1;
        }

        if (wifi_report_temp() == 0) {
            WIFI_OFF;
        }


        delay(30000);
        led_off();
        delay(25000);
    }
}

