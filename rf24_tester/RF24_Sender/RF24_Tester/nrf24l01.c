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

static volatile uint8_t rf24_mode=MODE_IDLE;

/* Setup the module */
uint8_t nrf24_config(uint8_t *TX_addr, uint8_t *RX_addr)
{
	/* Set pins as output  */
	NRF24_DDR |= (_BV(NRF24_CE) | _BV(NRF24_CS));
	
	/* Initialize pins */
	nrf24_ce_digitalWrite(LOW);
	nrf24_csn_digitalWrite(HIGH);
	
	_delay_ms(5); /* Some startup settling time */
	
	/* Address width */
	nrf24_configRegister(SETUP_AW, (nrf24_ADDR_WIDTH-2) << AW); 

	/* Config addresses */
	nrf24_tx_address(TX_addr);
	nrf24_rx_address(RX_addr);
	
	// Set RF channel
	nrf24_configRegister(RF_CH, NRF24_CHANNEL);//

	/* Dynamic payload length for TX & RX (pipes 0 and 1) */
	nrf24_configRegister(DYNPD,(1<<DPL_P0)|(1<<DPL_P1)|(0<<DPL_P2)|(0<<DPL_P3)|(0<<DPL_P4)|(0<<DPL_P5));
	nrf24_configRegister(FEATURE, 1 << EN_DPL);//
	
	// 250kbps, TX gain: 0dbm
	nrf24_configRegister(RF_SETUP, (1 << RF_PWR_LOW) | (1 << RF_PWR_HIGH) | (0 << RF_DR_LOW) | (1 << RF_DR_HIGH));

	// Enable ACKing on pipes 0 & 1 for TX & RX ACK support
	nrf24_configRegister(EN_AA,(1<<ENAA_P0)|(1<<ENAA_P1)|(0<<ENAA_P2)|(0<<ENAA_P3)|(0<<ENAA_P4)|(0<<ENAA_P5));

	// Auto retransmit delay: 2000 us and Up to 15 retransmit trials
	nrf24_configRegister(SETUP_RETR,(0x07<<ARD)|(0x0F<<ARC));
	
	uint8_t regval = nrf24_readRegister(SETUP_RETR);
	if (regval != ((0x07<<ARD)|(0x0F<<ARC)))
		return 1; /* There may be no nRF24 connected */
	
	/* Reset status bits */
	nrf24_configRegister(STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));

	// Start listening
	nrf24_powerUpRx();
	
	return 0;
}

/* Set the RX address */
void nrf24_rx_address(uint8_t* adr)
{
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(RX_ADDR_P1 | W_REGISTER);
	for(uint8_t i=0; i<nrf24_ADDR_WIDTH; i++)
	{
		spi_exchange(adr[i]);
	}
	nrf24_csn_digitalWrite(HIGH);
}

/* Set the TX address */
void nrf24_tx_address(uint8_t* adr)
{
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(TX_ADDR | W_REGISTER);
	for(uint8_t i=0; i<nrf24_ADDR_WIDTH; i++)
	{
		spi_exchange(adr[i]);
	}
	nrf24_csn_digitalWrite(HIGH);
	
	_delay_us(100);
	
	/* RX_ADDR_P0 must be set to the sending addr for auto ack to work. */
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(RX_ADDR_P0 | W_REGISTER);
	for(uint8_t i=0; i<nrf24_ADDR_WIDTH; i++)
	{
		spi_exchange(adr[i]);
	}
	nrf24_csn_digitalWrite(HIGH);

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
		//uart_puts("\r\nData received");
		return NRF24_DATA_AVAILABLE;
	}
	return nrf24_rxFifoEmpty();
}

/* Returns the length of data waiting in the RX FIFO */
uint8_t nrf24_payloadLength()
{
	uint8_t payload_len;
	
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(R_RX_PL_WID);
	payload_len=spi_exchange(0);
	nrf24_csn_digitalWrite(HIGH);
	
	/* If payload is larger than 32 or 0, an error has occurred in transmission
	 * and payload must be discarded */
	if( (payload_len == 0) | (payload_len > NRF24_MAX_PAYLOAD)){
		/* Flush RX FIFO */
		nrf24_csn_digitalWrite(LOW);
		spi_exchange(FLUSH_RX);
		nrf24_csn_digitalWrite(HIGH);
		return 0;
	}
	return payload_len;
}

/* Reads packet bytes into data array */
void nrf24_getData(uint8_t * data, uint8_t * pkt_len)
{
	/* Reset Received Data flag bit */
	nrf24_configRegister(STATUS,(1<<RX_DR));

	/* Number of bytes in the RX FIFO */
	*pkt_len = nrf24_payloadLength();
	
	/* Pull down chip select */
	nrf24_csn_digitalWrite(LOW);

	/* Send cmd to read RX payload */
	spi_exchange(R_RX_PAYLOAD);
	
	/* Read payload */
	for (uint8_t i=0; i<*pkt_len; i++)
	{
		data[i]=spi_exchange(0);
	}
	
	/* Pull up chip select */
	nrf24_csn_digitalWrite(HIGH);
}

/* Sends a packet of data with dynamic length */
void nrf24_sendData(uint8_t* data, uint8_t pkt_len)
{
	/* Go to Standby first */
	nrf24_ce_digitalWrite(LOW);
	
	/* Set to transmitter mode */
	nrf24_powerUpTx();

	/* Pull down chip select */
	nrf24_csn_digitalWrite(LOW);

	/* Write cmd to write payload */
	spi_exchange(W_TX_PAYLOAD);
	
	/* Write payload */
	for (uint8_t i=0; i<pkt_len; i++)
	{
		spi_exchange(data[i]);
	}
	
	/* Pull up chip select */
	nrf24_csn_digitalWrite(HIGH);

	/* Start the transmission */
	nrf24_ce_digitalWrite(HIGH);
}

uint8_t nrf24_wait_tx_result()
{
	uint16_t timeout = 10000; /* ~100ms timeout with 10us delay each assertion */
	uint8_t status = nrf24_getStatus();
	
	/* Waits while packet is being sent 
	 * TS_DS: Data sent. If auto_ack is on, this is only set when ACK is received.
	 * TX_FULL: Transmission FIFO is full.
	 * MAX_RT: Maximum retries reached. */
	status = nrf24_getStatus();
	uint8_t msg_sent = status & (1 << TX_DS);
	
	while ((!msg_sent || (status & (1 << TX_FULL))) && !(status & (1 << MAX_RT)) && --timeout) {
		status = nrf24_getStatus();
		msg_sent = status & (1 << TX_DS);
		_delay_us(10);
	} /* Transmission will end here */
	
	/* Reset status register */
	nrf24_configRegister(STATUS,(1<<RX_DR) | (1<<TX_DS));
	
	/* Switch back to RX mode */
	nrf24_powerUpRx();
	
	/* Finally check if data was sent or if conditions weren't met */
	if(msg_sent)
		return NRF24_MESSAGE_SENT;
	else
		return NRF24_MESSAGE_LOST;
}

/* Set chip as receiver */
void nrf24_powerUpRx()
{
	if(rf24_mode != MODE_RX){
		/* Config RF24 as receiver */
		nrf24_configRegister(CONFIG,nrf24_CONFIG|((1<<PWR_UP)|(1<<PRIM_RX)));
	
		/* Enable receiver address on Pipe 1. Pipe 0 is for transmitting ACKs */
		nrf24_configRegister(EN_RXADDR,(0<<ERX_P0)|(1<<ERX_P1));
	
		/* Settling time */
		_delay_us(100);
	
		/* Turn on chip */
		nrf24_ce_digitalWrite(HIGH);
	
		rf24_mode=MODE_RX;
	}
}

/* Set chip as transmitter */
void nrf24_powerUpTx()
{
	if(rf24_mode != MODE_TX){
		/* Turn on chip in TX mode*/
		nrf24_configRegister(CONFIG, nrf24_CONFIG | (1<<PWR_UP) | (0<<PRIM_RX) );
	
		/* Enable receiver address on pipe 0 for ACKs*/
		nrf24_configRegister(EN_RXADDR,(1<<ERX_P0)|(0<<ERX_P1));
	
		/* Flush TX FIFO */
		nrf24_csn_digitalWrite(LOW);
		spi_exchange(FLUSH_TX);
		nrf24_csn_digitalWrite(HIGH);
	
		/* Settling time */
		_delay_us(100); 
		
		rf24_mode=MODE_TX;
	}
}

/* Checks if RX FIFO is empty or not */
uint8_t nrf24_rxFifoEmpty()
{
	uint8_t fifoStatus=nrf24_readRegister(FIFO_STATUS);

	if(fifoStatus & (1 << RX_EMPTY)){
		//uart_puts("\r\nFifo empty");
		return NRF24_DATA_UNAVAILABLE;
	}
	else{
		//uart_puts("\r\nFifo has data");
		return NRF24_DATA_AVAILABLE;
	}
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

/* Write a single byte to register */
void nrf24_configRegister(uint8_t reg, uint8_t data)
{
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(W_REGISTER | (REGISTER_MASK & reg));
	spi_exchange(data);
	nrf24_csn_digitalWrite(HIGH);
}

/* Read n bytes from nrf24 register */
uint8_t nrf24_readRegister(uint8_t reg)
{
	uint8_t data;
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(R_REGISTER | (REGISTER_MASK & reg));
	data=spi_exchange(0);
	nrf24_csn_digitalWrite(HIGH);
	return data;
}

void nrf24_ce_digitalWrite(uint8_t state)
{
	if(state){
		set_bit(NRF24_PORT,NRF24_CE);
		_delay_us(10); /* Minimum CE High period for stuff to work */
	}
	else{
		clr_bit(NRF24_PORT,NRF24_CE);
	}
}

void nrf24_csn_digitalWrite(uint8_t state)
{
	if(state)
		set_bit(NRF24_PORT, NRF24_CS);
	else
		clr_bit(NRF24_PORT, NRF24_CS);
}
