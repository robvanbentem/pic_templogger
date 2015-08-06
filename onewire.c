#include "config.h"
#include "onewire.h"
#include <xc.h>

/**
 * Drive bus low, delay 480 us.
 * Release bus, delay 70 us.
 * Sample bus: 0 = device(s) present, 1 = no device present
 * Delay 410 us.
 */
char ow_reset() {
    ONEWIRE_OUT;
    __delay_us(480); // wait for 480us+

    ONEWIRE_IN;
    __delay_us(70);

    short response = (ONEWIRE_PIN == 0);
    __delay_us(410);

    return response;
}

/*
 * Drive bus low.
 * Write 1: delay 6 us. Release bus, delay 64 us.
 * Write 0: delay 60 us. Release bus, delay 10 us.
 */
void ow_write_bit(char b) {
    ONEWIRE_OUT;

    if (b) {
        __delay_us(6);
        ONEWIRE_IN;
        __delay_us(64);
    } else {
        __delay_us(60);
        ONEWIRE_IN;
        __delay_us(10);
    }
}

/*
 * Drive bus low, delay 6 us.
 * Release bus, delay 9 us.
 * Sample bus to read bit from slave.
 * Delay 55 us.
 */
char ow_read_bit() {
    ONEWIRE_OUT;
    __delay_us(6);

    ONEWIRE_IN;
    __delay_us(9);

    char response = ONEWIRE_PIN;

    __delay_us(55);

    return response;
}

char ow_read_byte() {
    char c, r = 0;

    for (c = 0; c < 8; c++) {
        if (ow_read_bit()) {
            r |= 1 << c;
        }
    }

    return r;
}

void ow_write_byte(char B) {
    char c;

    for (c = 0; c < 8; c++) {
        ow_write_bit((B >> c) & 1);
    }
}