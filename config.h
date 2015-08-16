#ifndef _config_h
#define _config_h

// reporting vars
#define S_DEVICE 2 // devive identifier
#define S_DEVICE_ATTR_TEMP "a" // attribute to report
#define S_DEVICE_REPORT_INTERVAL 60 // report every n seconds

// USART receive buffer size
#define USART_BUFLEN 255

// ds18b20 pin
#define ONEWIRE_PIN PORTBbits.RB4

// status led pin
#define LED0 5
#define LED1 4
#define LED2 3
#define LED3 6
#define LED4 7

// wifi
#define WIFI PORTCbits.RC1 // esp8266 RST pin
#define WIFI_ON WIFI = 1
#define WIFI_OFF WIFI = 0

// clock in hz

#if S_DEVICE == 1
#define _XTAL_FREQ 32000000
#define ___OSC IRC
#define S_DEVICE_ID "poc1"
#elif S_DEVICE == 2
#define _XTAL_FREQ 32000000
#define ___OSC IRC
#define S_DEVICE_ID "poc2"
#endif

// we use timer1 with a 1:8 prescaler so every overflow is 1000ms / (32Mhz / 4 FOSC / 8 prescaler / 2^16 register) = ~66ms
// USART_MAX_TIMEOUT_COUNT * 66ms = UART rx timeout (esp8266)
// @todo
#define USART_MAX_TIMEOUT_COUNT 15 // ~524ms

// CONFIG1L
#pragma config CPUDIV = NOCLKDIV// CPU System Clock Selection bits (No CPU System Clock divide)
#pragma config USBDIV = OFF     // USB Clock Selection bit (USB clock comes directly from the OSC1/OSC2 oscillator block; no divide)

// CONFIG1H
#pragma config FOSC = ___OSC
#pragma config PLLEN = ON       // 4 X PLL Enable bit (PLL is under software control)
#pragma config PCLKEN = OFF     // Primary Clock Enable bit (Primary clock is under software control)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode enabled)

// CONFIG2L
#pragma config PWRTEN = ON      // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 19        // Brown-out Reset Voltage bits (VBOR set to 1.9 V nominal)

// CONFIG2H
#pragma config WDTEN = OFF      // Watchdog Timer Enable bit (WDT is controlled by SWDTEN bit of the WDTCON register)
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config HFOFST = ON      // HFINTOSC Fast Start-up bit (HFINTOSC starts clocking the CPU without waiting for the oscillator to stablize.)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RA3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP enabled)
#pragma config BBSIZ = OFF      // Boot Block Size Select bit (512W boot block size)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Table Write Protection bit (Block 0 not write-protected)
#pragma config WRT1 = OFF       // Table Write Protection bit (Block 1 not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot block not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block not protected from table reads executed in other blocks)

#endif