/*
* ----------------------------------------------------------------------------
* “THE COFFEEWARE LICENSE” (Revision 1):
* <ihsan@kehribar.me> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a coffee in return.
* -----------------------------------------------------------------------------
* This library is based on this library:
*   https://github.com/aaronds/arduino-nrf24l01
* Which is based on this library:
*   http://www.tinkerer.eu/AVRLib/nRF24L01
* -----------------------------------------------------------------------------
*/
#include "nrf24l01.h"
#include "nrf24l01_config.h"
#include "spi.h"
#include "config.h"
#include "uart.h"
#include <util/delay.h>
#include "eeprom.h"

#define DEBUG_ADDRESSES 1

#define ADDRESS_MASTER 0
#define ADDRESS_SLAVE 1

/* TX and RX Address bytes */
uint8_t nrf24_addr[][5] = {
	{'M','A','S','T','R'},
	{'S','L','A','V','E'}
};

/* Enable 16-bit CRC */
#define nrf24_CONFIG ((1 << MASK_RX_DR) | (1 << MASK_TX_DS) | \
(1 << MASK_MAX_RT) | (1 << CRCO) | (1 << EN_CRC))

/* init the hardware pins */
void nrf24_init()
{
	uint8_t tx_addr[nrf24_ADDR_LEN];
	uint8_t rx_addr[nrf24_ADDR_LEN];
	
	for(uint8_t i=0; i< nrf24_ADDR_LEN; i++){
		rx_addr[i] = eeprom_read(EEPROM_IDX_RX_ADDR+i);
		tx_addr[i] = eeprom_read(EEPROM_IDX_TX_ADDR+i);
	}
	
	NRF24_DDR |= (_BV(NRF24_CE) | _BV(NRF24_CS)); /* Set Chip Enable pin as output */
	
	nrf24_tx_address(tx_addr);
	
	nrf24_rx_address(rx_addr);
	
	nrf24_ce_digitalWrite(LOW);
	nrf24_csn_digitalWrite(HIGH);
	
	nrf24_config(NRF24_CHANNEL);
}

/* configure the module */
void nrf24_config(uint8_t channel)
{
	// Set RF channel
	nrf24_configRegister(RF_CH,channel);
	
	/* Dynamic payload length for TX & RX (pipes 0 and 1) */
	nrf24_configRegister(DYNPD, 0x03);
	nrf24_configRegister(FEATURE, 1 << EN_DPL);

	/* Reset status bits */
	nrf24_configRegister(STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));
	
	// 2 Mbps, TX gain: 0dbm
	nrf24_configRegister(RF_SETUP, (1 << RF_PWR_LOW) | (1 << RF_PWR_HIGH) | (1 << RF_DR_LOW) | (0 << RF_DR_HIGH));

	// CRC enable, 2 byte CRC length
	nrf24_configRegister(CONFIG,nrf24_CONFIG);

	// Enable ACKing on pipes 0 & 1 for TX & RX ACK support 
	nrf24_configRegister(EN_AA,(1<<ENAA_P0)|(1<<ENAA_P1)|(0<<ENAA_P2)|(0<<ENAA_P3)|(0<<ENAA_P4)|(0<<ENAA_P5));

	// Auto retransmit delay: 2000 us and Up to 15 retransmit trials
	nrf24_configRegister(SETUP_RETR,(0x07<<ARD)|(0x0F<<ARC));

	// Start listening
	nrf24_powerUpRx();
}

/* Set the RX address */
void nrf24_rx_address(uint8_t * rx_adr)
{
	uint8_t adr_len=nrf24_ADDR_LEN;
	nrf24_ce_digitalWrite(LOW);
	while(adr_len--) {
		nrf24_writeRegister(RX_ADDR_P1,*rx_adr++);
	}
	nrf24_ce_digitalWrite(HIGH);
}

/* Set the TX address */
void nrf24_tx_address(uint8_t tx_adr[nrf24_ADDR_LEN])
{
	uint8_t buf[10]={0};
	
	nrf24_ce_digitalWrite(LOW);
	
	for(uint8_t i=0; i<nrf24_ADDR_LEN; i++)
	{
		nrf24_writeRegister(TX_ADDR,tx_adr[i]);
		/* RX_ADDR_P0 must be set to the sending addr for auto ack to work. */
		nrf24_writeRegister(RX_ADDR_P0,tx_adr[i]);
		
	}
	nrf24_ce_digitalWrite(HIGH);
}

/* Checks if data is available for reading */
/* Returns 1 if data is ready ... */
uint8_t nrf24_dataReady()
{
    // See note in getData() function - just checking RX_DR isn't good enough
    uint8_t status = nrf24_getStatus();

    // We can short circuit on RX_DR, but if it's not set, we still need
    // to check the FIFO for any pending packets
    if ( status & (1 << RX_DR) )
    {
		uart_puts("\r\nData ready");
	    return 1;
    }
    return !nrf24_rxFifoEmpty();
}

/* Checks if receive FIFO is empty or not */
uint8_t nrf24_rxFifoEmpty()
{
	uint8_t fifoStatus;
	fifoStatus = nrf24_readRegister(FIFO_STATUS);
	return (fifoStatus & (1 << RX_EMPTY));
}

/* Returns the length of data waiting in the RX fifo */
uint8_t nrf24_rx_data_len()
{
	uint8_t data_length;
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(R_RX_PL_WID);
	data_length = spi_exchange(0x00);
	nrf24_csn_digitalWrite(HIGH);
	return data_length;
}

/* Reads payload bytes into data array */
void nrf24_getData(uint8_t * data, uint8_t * pkt_len)
{
	uint8_t len;
	len = nrf24_rx_data_len();
	*pkt_len = len;
	
	/* Pull down chip select */
	nrf24_csn_digitalWrite(LOW);

	/* Send cmd to read rx payload */
	spi_exchange( R_RX_PAYLOAD );
	
	/* Read payload */
	while(len--)
	{
		*data++ = spi_exchange(0);
	}
	
	/* Pull up chip select */
	nrf24_csn_digitalWrite(HIGH);

	/* Reset status register */
	nrf24_configRegister(STATUS,(1<<RX_DR));
}

/* Returns the number of retransmissions occured for the last message */
uint8_t nrf24_retransmissionCount()
{
	uint8_t retries;
	retries = nrf24_readRegister(OBSERVE_TX);
	retries = retries & 0x0F;
	return retries;
}

// Sends a data package to the default address. Be sure to send the correct
// amount of bytes as configured as payload on the receiver.
void nrf24_send(uint8_t* data, uint8_t pkt_len)
{
	/* Go to Standby-I first */
	nrf24_ce_digitalWrite(LOW);
	
	/* Set to transmitter mode , Power up if needed */
	nrf24_powerUpTx();

	/* Pull down chip select */
	nrf24_csn_digitalWrite(LOW);

	/* Write cmd to write payload */
	spi_exchange(W_TX_PAYLOAD);

	/* Write payload */
	while(pkt_len--){
		spi_exchange(*data++);
	}
	
	/* Pull up chip select */
	nrf24_csn_digitalWrite(HIGH);

	/* Start the transmission */
	nrf24_ce_digitalWrite(HIGH);
}

uint8_t nrf24_wait_tx_result()
{
	uint8_t status;
	uint16_t timeout = 10000; /* ~100ms timeout */

	/* read the current status */
	status = nrf24_getStatus();
	
	/* if sending successful (TX_DS) or max retries exceeded (MAX_RT). */
	while ((!(status & (1 << TX_DS)) || (status & (1 << TX_FULL))) && !(status & (1 << MAX_RT)) && --timeout) {
		status = nrf24_getStatus();
		_delay_us(10);
	}
	
	/* Switch back to RX mode */
	nrf24_powerUpRx();
	
	if(status & (1 << TX_DS))
		return NRF24_TRANSMISSON_OK;
	else
		return NRF24_MESSAGE_LOST;
}

uint8_t nrf24_getStatus()
{
	uint8_t rv;
	nrf24_csn_digitalWrite(LOW);
	rv = spi_exchange(NOP);
	nrf24_csn_digitalWrite(HIGH);
	return rv;
}

void nrf24_powerUpRx()
{
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(FLUSH_RX);
	nrf24_csn_digitalWrite(HIGH);

	nrf24_ce_digitalWrite(LOW);
	
	/* Rx mode */
	nrf24_configRegister(CONFIG, nrf24_CONFIG | (1 << PWR_UP) | (1 << PRIM_RX));
	/* Only use data pipe 1 for receiving, pipe 0 is for TX ACKs */
	nrf24_configRegister(EN_RXADDR, 0x02);
	
	nrf24_ce_digitalWrite(HIGH);
}

void nrf24_powerUpTx()
{
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(FLUSH_TX);
	nrf24_csn_digitalWrite(HIGH);
	
	nrf24_configRegister(CONFIG, nrf24_CONFIG | (1 << PWR_UP));
	nrf24_configRegister(EN_RXADDR, 0x01);
}

void nrf24_powerDown()
{
	nrf24_ce_digitalWrite(LOW);
	nrf24_configRegister(CONFIG,nrf24_CONFIG);
}

/* Clocks only one byte into the given nrf24 register */
void nrf24_configRegister(uint8_t reg, uint8_t value)
{
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(W_REGISTER | (REGISTER_MASK & reg));
	spi_exchange(value);
	nrf24_csn_digitalWrite(HIGH);
}

/* Read single register from nrf24 */
uint8_t nrf24_readRegister(uint8_t reg)
{
	uint8_t reg_val;
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(R_REGISTER | (REGISTER_MASK & reg));
	reg_val = spi_exchange(NOP);
	nrf24_csn_digitalWrite(HIGH);
	
	return reg_val;
}

/* Write to a single register of nrf24 */
void nrf24_writeRegister(uint8_t reg, uint8_t value)
{
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(W_REGISTER | (REGISTER_MASK & reg));
	spi_exchange(value);
	nrf24_csn_digitalWrite(HIGH);
}

void nrf24_ce_digitalWrite(uint8_t state)
{
	if(state)
	{
		set_bit(NRF24_PORT,NRF24_CE);
	}
	else
	{
		clr_bit(NRF24_PORT,NRF24_CE);
	}
}

void nrf24_csn_digitalWrite(uint8_t state)
{
	if(state)
	{
		set_bit(NRF24_PORT, NRF24_CS);
	}
	else
	{
		clr_bit(NRF24_PORT, NRF24_CS);
	}
}
