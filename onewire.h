#ifndef ONEWIRE_H
#define	ONEWIRE_H

#ifndef ONEWIRE_PIN
#define ONEWIRE_PIN PORTCbits.RC1
#endif

#define ONEWIRE_IN ONEWIRE_PIN = 1
#define ONEWIRE_OUT ONEWIRE_PIN = 0

char ow_reset();
void ow_write_bit(char Bit);
void ow_write_byte(char byte);
char ow_read_bit();
char ow_read_byte();

#endif	/* ONEWIRE_H */
