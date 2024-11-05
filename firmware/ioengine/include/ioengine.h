#ifndef __EMULATED_IO_H__
#define __EMULATED_IO_H__

#include <avr/io.h>

#define EIO_VERSION "0.1.0"

#define Z80_RST PIN_PC0
#define Z80_WAIT PIN_PC1
#define Z80_BUSREQ PIN_PC2
#define Z80_BUSACK PIN_PC3
#define Z80_WR PIN_PC4
#define Z80_RD PIN_PC5

// PD0-7 = Z80D0-7
#define Z80_DATA PORTD
// PE0-3 = BANKSEL0-3
#define BANKSEL PORTE
// PF0-3 = Z80A0-3
#define Z80_ADDRESS PORTF
// PF4   = Z80A7
#define Z80_A7 PIN_PF4
#define BOOT_SEL PIN_PF5

#define UART_TX0 PIN_PA0
#define UART_RX0 PIN_PA1
#define I2C_SDA PIN_PA2
#define I2C_SCL PIN_PA3
#define SPI_MOSI PIN_PA4
#define SPI_MISO PIN_PA5
#define SPI_SCK PIN_PA6
#define SPI_SS PIN_PA7

#endif // __EMULATED_IO_H__