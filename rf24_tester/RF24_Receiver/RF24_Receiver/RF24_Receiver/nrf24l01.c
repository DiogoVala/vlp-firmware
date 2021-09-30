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

/* Setup the module */
void nrf24_config(uint8_t *TX_addr, uint8_t *RX_addr)
{
	/* Set pins as output  */
	NRF24_DDR |= (_BV(NRF24_CE) | _BV(NRF24_CS));
	
	/* Address width */
	nrf24_configRegister(SETUP_AW, (nrf24_ADDR_WIDTH-2) << AW);

	/* Config addresses */
	nrf24_tx_address(TX_addr);
	nrf24_rx_address(RX_addr);
	
	/* Initialize pins */
	nrf24_ce_digitalWrite(LOW);
	nrf24_csn_digitalWrite(HIGH);
	
	// Set RF channel
	nrf24_configRegister(RF_CH, NRF24_CHANNEL);

	/* Dynamic payload length for TX & RX (pipes 0 and 1) */
	nrf24_configRegister(DYNPD,(1<<DPL_P0)|(1<<DPL_P1)|(0<<DPL_P2)|(0<<DPL_P3)|(0<<DPL_P4)|(0<<DPL_P5));
	nrf24_configRegister(FEATURE, 1 << EN_DPL);
	
	// 2 Mbps, TX gain: 0dbm
	nrf24_configRegister(RF_SETUP, (1 << RF_PWR_LOW) | (1 << RF_PWR_HIGH) | (0 << RF_DR_LOW) | (1 << RF_DR_HIGH));

	// CRC enable, 1 byte CRC length
	nrf24_configRegister(CONFIG,nrf24_CONFIG);

	// Enable RX addresses
	nrf24_configRegister(EN_RXADDR,(1<<ERX_P0)|(1<<ERX_P1)|(0<<ERX_P2)|(0<<ERX_P3)|(0<<ERX_P4)|(0<<ERX_P5));

	// Enable ACKing on pipes 0 & 1 for TX & RX ACK support
	nrf24_configRegister(EN_AA,(1<<ENAA_P0)|(1<<ENAA_P1)|(0<<ENAA_P2)|(0<<ENAA_P3)|(0<<ENAA_P4)|(0<<ENAA_P5));

	// Auto retransmit delay: 2000 us and Up to 15 retransmit trials
	nrf24_configRegister(SETUP_RETR,(0x07<<ARD)|(0x0F<<ARC));

	// Start listening
	nrf24_powerUpRx();
}

/* Set the RX address */
void nrf24_rx_address(uint8_t* adr)
{
	nrf24_writeRegister(RX_ADDR_P1,adr,nrf24_ADDR_WIDTH);
	nrf24_ce_digitalWrite(HIGH);
}

/* Set the TX address */
void nrf24_tx_address(uint8_t* adr)
{
	/* RX_ADDR_P0 must be set to the sending addr for auto ack to work. */
	nrf24_writeRegister(RX_ADDR_P0,adr,nrf24_ADDR_WIDTH);
	nrf24_writeRegister(TX_ADDR,adr,nrf24_ADDR_WIDTH);
}

/* Checks if there is data in the RX FIFO  */
uint8_t nrf24_dataReady()
{
	uint8_t status = nrf24_getStatus();

	/* Check if Data Ready flag is set.*/
	/* RX_DR just means data has arrived in the FIFO,
	 * but we still need to verify if there is already
	 * data in the FIFO in the occasions where RX_DR
	 * isn't set. */
	if ( status & (1 << RX_DR) ) {
		return NRF24_DATA_AVAILABLE;
	}
	return nrf24_rxFifoEmpty();
}

/* Checks if RX FIFO is empty or not */
uint8_t nrf24_rxFifoEmpty()
{
	uint8_t fifoStatus;

	nrf24_readRegister(FIFO_STATUS,&fifoStatus,1);
	
	if(fifoStatus & (1 << RX_EMPTY))
		return NRF24_DATA_UNAVAILABLE;
	else
		return NRF24_DATA_AVAILABLE;
}

/* Returns the length of data waiting in the RX FIFO */
uint8_t nrf24_payloadLength()
{
	uint8_t status;
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(R_RX_PL_WID);
	status = spi_exchange(0);
	nrf24_csn_digitalWrite(HIGH);
	return status;
}

/* Reads packet bytes into data array */
void nrf24_getData(uint8_t * data, uint8_t * pkt_len)
{
	/* Number of bytes in the RX FIFO */
	*pkt_len = nrf24_payloadLength(); 
	
	/* Pull down chip select */
	nrf24_csn_digitalWrite(LOW);

	/* Send cmd to read RX payload */
	spi_exchange(R_RX_PAYLOAD);
	
	/* Read payload */
	spi_exchange_n(0, data, *pkt_len);
	/*
	for(uint8_t i = 0; i < *pkt_len; i++) {
		*data++ = spi_exchange(0);
	}*/
	
	/* Pull up chip select */
	nrf24_csn_digitalWrite(HIGH);

	/* Reset status register */
	nrf24_configRegister(STATUS,(1<<RX_DR));
}

/* Sends a packet of data with dynamic length */
void nrf24_sendData(uint8_t* data, uint8_t pkt_len)
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
	spi_exchange_n(data, 0, pkt_len);
	
	/* Pull up chip select */
	nrf24_csn_digitalWrite(HIGH);

	/* Start the transmission */
	nrf24_ce_digitalWrite(HIGH);
}

uint8_t nrf24_wait_tx_result()
{
	uint16_t timeout = 10000; /* ~100ms timeout with 10us delay each assertion */
	uint8_t status = nrf24_getStatus();
	
	/* Waits while packet is being sent */
	/* TS_DS: Data sent. If auto_ack is on, this is only set when ACK is received.
	 * TX_FULL: Transmission FIFO is full.
	 * MAX_RT: Maximum retries reached. */
	while ((!(status & (1 << TX_DS)) || (status & (1 << TX_FULL))) && !(status & (1 << MAX_RT)) && --timeout) {
		status = nrf24_getStatus();
		_delay_us(10);
	}
	
	/* Switch back to RX mode */
	nrf24_powerUpRx();
	
	/* Finally check if data was sent or if conditions weren't met */
	if(status & (1 << TX_DS))
		return NRF24_MESSAGE_SENT;
	else
		return NRF24_MESSAGE_LOST;
}

/* Get status register data */
uint8_t nrf24_getStatus()
{
	uint8_t rv;
	nrf24_csn_digitalWrite(LOW);
	rv = spi_exchange(NOP);
	nrf24_csn_digitalWrite(HIGH);
	return rv;
}

/* Returns the number of retransmissions occurred for the last message */
uint8_t nrf24_retransmissionCount()
{
	uint8_t rv;
	nrf24_readRegister(OBSERVE_TX,&rv,1);
	rv &= 0x0F;
	return rv;
}

/* Set chip as emitter */
void nrf24_powerUpRx()
{
	/* Flush FIFO */
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(FLUSH_RX);
	nrf24_csn_digitalWrite(HIGH);

	/* Reset Status register */
	nrf24_configRegister(STATUS,(1<<RX_DR)|(1<<TX_DS)|(1<<MAX_RT));

	/* Config RF24 as Emitter */
	nrf24_ce_digitalWrite(LOW); 
	nrf24_configRegister(CONFIG,nrf24_CONFIG|((1<<PWR_UP)|(1<<PRIM_RX)));
	nrf24_ce_digitalWrite(HIGH);
}

/* Set chip as receiver */
void nrf24_powerUpTx()
{
	/* Reset Status register */
	nrf24_configRegister(STATUS,(1<<RX_DR)|(1<<TX_DS)|(1<<MAX_RT));

	/* Config RF24 as Transmitter */
	nrf24_configRegister(CONFIG,nrf24_CONFIG|((1<<PWR_UP)|(0<<PRIM_RX)));
}

/* Set chip as idle */
void nrf24_powerDown()
{
	nrf24_ce_digitalWrite(LOW);
	nrf24_configRegister(CONFIG,nrf24_CONFIG);
}

/* Write a single byte to register */
void nrf24_configRegister(uint8_t reg, uint8_t data)
{
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(W_REGISTER | (REGISTER_MASK & reg));
	spi_exchange(data);
	nrf24_csn_digitalWrite(HIGH);
}

/* Read n bytes from nrf24 register */
void nrf24_readRegister(uint8_t reg, uint8_t* data, uint8_t n)
{
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(R_REGISTER | (REGISTER_MASK & reg));
	spi_exchange_n(data,data,n);
	nrf24_csn_digitalWrite(HIGH);
}

/* Write n bytes to nrf24 register */
void nrf24_writeRegister(uint8_t reg, uint8_t* data, uint8_t n)
{
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(W_REGISTER | (REGISTER_MASK & reg));
	spi_exchange_n(data,0,n);
	nrf24_csn_digitalWrite(HIGH);
}

void nrf24_ce_digitalWrite(uint8_t state)
{
	if(state)
		set_bit(NRF24_PORT,NRF24_CE);
	else
		clr_bit(NRF24_PORT,NRF24_CE);
}

void nrf24_csn_digitalWrite(uint8_t state)
{
	if(state)
		set_bit(NRF24_PORT, NRF24_CS);
	else
		clr_bit(NRF24_PORT, NRF24_CS);
}
