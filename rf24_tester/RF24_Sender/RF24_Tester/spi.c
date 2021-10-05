/* spi.h
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 * 
 */

#include <avr/io.h>
#include "uart.h"
#include "spi.h"

/* Send one byte over SPI */
uint8_t spi_exchange(uint8_t data) {

    SPDR = data;
    while (!(SPSR & _BV(SPIF))); /* Waits until SPIF is set */
    return SPDR; /* Reading the data register after reading SPIF clears SPIF */
}

/* Send and receive multiple bytes over SPI */
void spi_exchange_n(uint8_t* dataout,uint8_t* datain,uint8_t len) {
	for(uint8_t i=0; i<len; i++)
	{
		datain[i] = spi_exchange(dataout[i]);
	}
}

void spi_init(void) {
	SPI_DDR |= _BV(SPI_SS_NRF24) |_BV(SPI_SS_POT) | _BV(SPI_MOSI) |  _BV(SPI_SCK); /* Set pins as output */
	SPI_DDR &= ~_BV(SPI_MISO);
    SPCR = _BV(SPE) | _BV(MSTR) | (SPI_MODE0 & SPI_MODE_MASK) | (SPI_CLOCK_DIV4 & SPI_CLOCK_MASK);
	SPSR = 1 << SPI2X; /* double speed */
}
