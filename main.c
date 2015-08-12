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

#define S_DEVICE_ID "poc1"
#define S_DEVICE_ATTR_TEMP "a"

#define WIFI_ON WIFI = 1
#define WIFI_OFF WIFI = 0

// AT Commands
#define AT_ECHO_OFF (char*)"ATE0"
#define AT_MUX_ON (char*)"AT+CIPMUX=1"
#define AT_SERVER_ON (char*)"AT+CIPSERVER=1,4444"
#define AT_RESET (char*)"AT+RST"
#define AT_SEND_OPEN (char*)"AT+CIPSTART=4,\"TCP\",\"192.168.1.1\",4555"
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

void led_on() {
    PORTC = 1;
}

char wifi_report_temp() {
    get_temp();

    esp_cmd(AT_SEND_OPEN);
    if (USART_search(AT_RESP_OK)) {
        PORTC |= (1 << LED2);
        uint16_t itemp = (sign << 8) | degr;
        char data[64];
        unsigned char n = sprintf(data, "GET /log?d=%s&a=%s&v=%d HTTP/1.0\r\n\r\n", S_DEVICE_ID, S_DEVICE_ATTR_TEMP, itemp);

        char cmd[18];
        sprintf(cmd, "AT+CIPSEND=4,%d", n);

        USART_clear_buf();
        esp_cmd((char*) cmd);
        if (USART_search_chr('>')) {
            PORTC |= (1 << LED1);

            // Write temperture
            USART_puts((char*) data);
            if (USART_search(AT_RESP_OK)) {
                PORTC |= (1 << LED0);
                return 1;
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

char wifi_setup() {
    RCSTAbits.CREN = 1;
    esp_cmd(AT_ECHO_OFF);
    if (USART_search(AT_RESP_OK)) {
        USART_clear_buf();
        esp_cmd(AT_MUX_ON);
        if (USART_search(AT_RESP_OK)) {
            USART_clear_buf();
            PORTC |= (1 << LED3);
            return 1;
        }
    }

    return 0;
}

char wifi_reset() {
    RCSTAbits.CREN = 0;
    WIFI_OFF;
    delay(1000);
    WIFI_ON;

    delay(5000); // allow 5s for booting of esp
    return wifi_setup();
}

int main() {
    setup();
    WIFI_ON;

    INTCONbits.PEIE = 1; // enable peripheral interrupts
    INTCONbits.GIE = 1; // enable global interrupt
    RCSTAbits.SPEN = 1; // enable uart

    if (wifi_reset()) {
        PORTC = (1 << LED3);
        while (1) {

            if (wifi_report_temp() == 0) {
                PORTC = (1 << LED4);
                wifi_reset();
            }

            delay(25);
            PORTC &= (1 << LED3);
        }
    }
}

