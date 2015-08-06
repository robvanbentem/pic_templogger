/* 
 * File:   main.c
 * Author: rob
 *
 * Created on 23 December 2014, 12:45
 */

#include "config.h"

// IO Config
#define OWPIN PORTCbits.RC2
#define LED PORTCbits.RC5
#define ESP PORTBbits.RB4

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <xc.h>
#include <pic18f13k50.h>

#include "onewire.h"
#include "usart.h"


#define WIFI_ON ESP = 1
#define WIFI_OFF ESP = 0

// AT Commands
#define AT_ECHO_OFF (char*)"ATE0"
#define AT_MUX_ON (char*)"AT+CIPMUX=1"
#define AT_SERVER_ON (char*)"AT+CIPSERVER=1,4444"
#define AT_RESET (char*)"AT+RST"

char sign;
char degr;

volatile unsigned char cmd = 0;


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
    TRISCbits.RC7 = 1;
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

void format_temp(char *p) {
    int16_t temp = (sign << 8) + degr;
    sprintf(p, "%f", (temp * 0.0625));
}

void report_temp(){
    get_temp();
    
    char *temp[16];
    format_temp((char *)temp);

    USART_puts((char *)temp);
    USART_put_eol();
}

void store_buf() {
    for (unsigned char i = 0; i < BUFLEN; i++) {
        eeprom_write(i, *rxbuf[i]);
    }
}

void clear_buf() {
    memset(&rxbuf, 0, BUFLEN);
}



// INTERRUPT

void interrupt rx() {
    if (PIR1bits.RCIF == 1) {
        PIE1bits.RCIE = 0; // disable rx interrupt while recieving
        cmd = 0xA1;
        return;
    }

    if (PIR1bits.TMR1IF) {
        if (--rxtoc == 0) {
            rxto = 1;
            T1CONbits.TMR1ON = 0;
        }
        tmr1_reset();
        PIR1bits.TMR1IF = 0;
        T1CONbits.TMR1ON = 1;
    }
}

void esp_cmd(unsigned char *data){
    USART_puts(data);
    USART_put_eol();
}

void start_wifi_server(){
    PIE1bits.RCIE = 0;

    esp_cmd(AT_ECHO_OFF);
    delay(100);

    esp_cmd(AT_MUX_ON);
    delay(100);

    esp_cmd(AT_SERVER_ON);
    delay(100);

    PIE1bits.RCIE = 1;
}

void reset_wifi(){
    if(ESP == 0){
        WIFI_ON;
    } else {
        esp_cmd(AT_RESET);
    }

    delay(2500);
    start_wifi_server();
}

int main() {
    setup();
    setup_usart();

    while(1){
        //reset_wifi();
        report_temp();
        delay(2000);
    }

    PIR1bits.RCIF = 0; // clear interrupt flag
    INTCONbits.PEIE = 1; // enable peripheral interrupts
    INTCONbits.GIE = 1; // enable global interrupt

    while (1) {
        while (cmd == 0);
        
        if (cmd == 0xA1) {
            cmd = 0;
            read_usart();
        }

        if(cmd == 0xB1){
            report_temp();
        }

        clear_buf();
        reset_wifi();
    }
}

