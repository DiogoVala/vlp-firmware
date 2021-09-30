/* spi.h
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 * 
 */

#ifndef __SPI_H__
#define __SPI_H__

#include <avr/io.h>

/* Pinout concerning SPI */
#define SPI_PORT	 PORTB
#define SPI_DDR		 DDRB
#define SPI_SS_POT	 DDB1
#define SPI_SS_NRF24 DDB2
#define SPI_MOSI	 DDB3
#define SPI_MISO	 DDB4
#define SPI_SCK		 DDB5

/* Clock Division */
#define SPI_CLOCK_DIV4 0x00
#define SPI_CLOCK_DIV16 0x01
#define SPI_CLOCK_DIV64 0x02
#define SPI_CLOCK_DIV128 0x03
#define SPI_CLOCK_DIV2 0x04
#define SPI_CLOCK_DIV8 0x05
#define SPI_CLOCK_DIV32 0x06

/* Check Atmega datasheet for info on modes */
#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

#define SPI_MODE_MASK 0x0C  // CPOL = bit 3, CPHA = bit 2 on SPCR
#define SPI_CLOCK_MASK 0x03  // SPR1 = bit 1, SPR0 = bit 0 on SPCR

inline static uint8_t spi_exchange(uint8_t data) {
	
    SPDR = data;
    while (!(SPSR & _BV(SPIF))); /* Waits until SPIF is set */
    return SPDR; /* Reading the data register after reading SPIF clears SPIF */
}

inline static void spi_init(void) {
	SPI_DDR = SPI_DDR | _BV(SPI_SS_NRF24) |_BV(SPI_SS_POT) | _BV(SPI_MOSI) |  _BV(SPI_SCK); /* Set pins as output */
    SPCR = _BV(SPE) | 0 | _BV(MSTR) | (SPI_MODE0 & SPI_MODE_MASK) | (SPI_CLOCK_DIV4 & SPI_CLOCK_MASK);
}

#endif
