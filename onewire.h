#ifndef ONEWIRE_H
#define	ONEWIRE_H

#ifndef OWPIN
#define OWPIN PORTCbits.RC2
#endif

#define OWIN OWPIN = 1
#define OWOUT OWPIN = 0

char ow_reset();
void ow_write_bit(char Bit);
void ow_write_byte(char byte);
char ow_read_bit();
char ow_read_byte();

#endif	/* ONEWIRE_H */

