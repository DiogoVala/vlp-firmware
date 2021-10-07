/* spi.h
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 * 
 */

/* File includes */
#include "spi.h"

uint8_t spi_exchange(uint8_t data) {
    SPDR = data;
    while (!(SPSR & _BV(SPIF))); /* Waits until SPIF is set */
    return SPDR; /* Reading the data register after reading SPIF clears SPIF */
}

void spi_init(void) {
	SPI_DDR |= _BV(SPI_SS_NRF24) |_BV(SPI_SS_POT) | _BV(SPI_MOSI) |  _BV(SPI_SCK); /* Set pins as output */
	SPI_DDR &= ~_BV(SPI_MISO); /* Set MISO as input */
    SPCR = _BV(SPE) | _BV(MSTR) | (SPI_MODE0 & SPI_MODE_MASK) | (SPI_CLOCK_DIV4 & SPI_CLOCK_MASK);
	SPSR = 1 << SPI2X; /* Double speed */
}
