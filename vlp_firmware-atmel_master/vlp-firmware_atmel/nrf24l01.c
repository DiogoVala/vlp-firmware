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
#include <stdio.h>

/* Setup the module */
void nrf24_config(uint8_t *TX_addr, uint8_t *RX_addr)
{
	/* Set Chip enable as output  */
	NRF24_DDR |= _BV(NRF24_CE);
	/*Note: Chip Select is handled in the SPI module */
	
	/* Address width */
	nrf24_configRegister(SETUP_AW, (nrf24_ADDR_WIDTH-2) << AW);

	/* Config addresses */
	nrf24_set_TX_address(TX_addr);
	nrf24_set_RX_address(RX_addr);
	
	//nrf24_configRegister(ACTIVATE, ACTIVATION);
	
	/* Initialize pins */
	nrf24_ce_digitalWrite(LOW);
	nrf24_csn_digitalWrite(HIGH);
	
	_delay_ms(5);
	
	// Set RF channel
	nrf24_configRegister(RF_CH, NRF24_CHANNEL);

	/* Dynamic payload length for TX & RX (pipes 0 and 1) */
	nrf24_configRegister(DYNPD,(1<<DPL_P0)|(1<<DPL_P1));
	nrf24_configRegister(FEATURE, 1 << EN_DPL);
	
	// 2 Mbps, TX gain: 0dbm
	nrf24_configRegister(RF_SETUP, (1 << RF_PWR_LOW) | (1 << RF_PWR_HIGH) | (0 << RF_DR_LOW) | (1 << RF_DR_HIGH));

	// CRC enable, 1 byte CRC length
	nrf24_configRegister(CONFIG,nrf24_CONFIG);

	// Enable ACKing on pipes 0 & 1 for TX & RX ACK support
	nrf24_configRegister(EN_AA,(1<<ENAA_P0)|(1<<ENAA_P1)|(0<<ENAA_P2)|(0<<ENAA_P3)|(0<<ENAA_P4)|(0<<ENAA_P5));

	// Auto retransmit delay: 2000 us and Up to 15 retransmit trials
	nrf24_configRegister(SETUP_RETR,(0x07<<ARD)|(0x0F<<ARC));
	
	/* Reset status bits */
	nrf24_configRegister(STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));

	// Start listening
	nrf24_powerUpRx();
}

/* Set the RX address */
void nrf24_set_RX_address(uint8_t* adr)
{
	nrf24_ce_digitalWrite(LOW);

	nrf24_writeRegister(RX_ADDR_P1,adr,nrf24_ADDR_WIDTH);

	nrf24_ce_digitalWrite(HIGH);
}

/* Get the RX address */
void nrf24_get_RX_address(uint8_t* adr)
{
	nrf24_readRegister(RX_ADDR_P1,adr,nrf24_ADDR_WIDTH);
}

/* Set the TX address */
void nrf24_set_TX_address(uint8_t* adr)
{
	nrf24_ce_digitalWrite(LOW);

	/* The pipe 0 address is the address we listen on for ACKs */
	nrf24_writeRegister(RX_ADDR_P0,adr,nrf24_ADDR_WIDTH);
	nrf24_writeRegister(TX_ADDR,adr,nrf24_ADDR_WIDTH);

	nrf24_ce_digitalWrite(HIGH);
}

/* Get the TX address */
void nrf24_get_TX_address(uint8_t* adr)
{
	nrf24_readRegister(TX_ADDR,adr,nrf24_ADDR_WIDTH);
}

/* Get the ACK address */
void nrf24_get_ACK_address(uint8_t* adr)
{
	nrf24_readRegister(RX_ADDR_P0,adr,nrf24_ADDR_WIDTH);
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
	{
		//uart_puts("\r\nFIFO not empty.");
		return NRF24_DATA_AVAILABLE; // THIS IS PRODUCING FALSE FLAGS - STILL TO BE DEBUGGED
	}
}

/* Returns the length of data waiting in the RX FIFO */
uint8_t nrf24_payloadLength()
{
	uint8_t payload_len=0;
	
	nrf24_readRegister(R_RX_PL_WID,&payload_len,1);
	
	uint8_t buf[30];
	sprintf(buf, "\r\nSize received: %d\r\n", payload_len);
	uart_puts(buf);
	
	return payload_len;
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
	spi_exchange_n(data, data, *pkt_len);
	
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
	spi_exchange_n(data, data, pkt_len);
	
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
	status = nrf24_getStatus();
	uint8_t msg_sent = status & (1 << TX_DS);
	
	while ((!msg_sent || (status & (1 << TX_FULL))) && !(status & (1 << MAX_RT)) && --timeout) {
		status = nrf24_getStatus();
		msg_sent = status & (1 << TX_DS);
		_delay_us(10);
	} /* Transmission will end here */
	
	/* Switch back to RX mode */
	nrf24_powerUpRx();
	
	/* Reset status register */
	nrf24_configRegister(STATUS,(1<<RX_DR) | (1<<TX_DS));
	
	/* Finally check if data was sent or if conditions weren't met */
	if(msg_sent)
		return NRF24_MESSAGE_SENT;
	else
		return NRF24_MESSAGE_LOST;
}

/* Get status register data */
uint8_t nrf24_getStatus()
{
	uint8_t status;
	nrf24_csn_digitalWrite(LOW);
	status = spi_exchange(NOP);
	nrf24_csn_digitalWrite(HIGH);
	return status;
}

/* Returns the number of retransmissions occurred for the last message */
uint8_t nrf24_retransmissionCount()
{
	uint8_t rv;
	nrf24_readRegister(OBSERVE_TX,&rv,1);
	rv &= 0x0F;
	return rv;
}

/* Set chip as receiver */
void nrf24_powerUpRx()
{
	
	// Use Pipe 1 to receive data. Pipe 0 is for transmitting ACKs
	nrf24_configRegister(EN_RXADDR,(1<<ERX_P1)|(0<<ERX_P0));
	
	/* Flush RX FIFO */
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(FLUSH_RX);
	nrf24_csn_digitalWrite(HIGH);

	/* Reset Status register */
	nrf24_configRegister(STATUS,(1<<RX_DR)|(1<<TX_DS)|(1<<MAX_RT));

	/* Config RF24 as Receiver */ 
	nrf24_configRegister(CONFIG,nrf24_CONFIG|((1<<PWR_UP)|(1<<PRIM_RX)));
	
	_delay_us(100); /* Settling time */
}

/* Set chip as emitter */
void nrf24_powerUpTx()
{

	// Use Pipe 0 to receive ACKs
	nrf24_configRegister(EN_RXADDR,(0<<ERX_P1)|(1<<ERX_P0));
	
	/* Flush TX FIFO */
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(FLUSH_TX);
	nrf24_csn_digitalWrite(HIGH);
	
	/* Reset Status register */
	nrf24_configRegister(STATUS,(1<<RX_DR)|(1<<TX_DS)|(1<<MAX_RT));

	/* Config RF24 as Transmitter */
	nrf24_configRegister(CONFIG,nrf24_CONFIG|((1<<PWR_UP)|(0<<PRIM_RX)));
	
	_delay_us(100); /* Settling time */
}

/* Set chip as idle */
void nrf24_powerDown()
{
	/* Disable chip */
	nrf24_ce_digitalWrite(LOW);
	
	/* Still set base config even in idle */
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

void nrf24_print_info(){
	uint8_t uart_buffer[50];
	uint8_t temp_array[50];
	
	uart_puts("\r\nTX Address: ");
	nrf24_get_TX_address(temp_array);
	for (uint8_t i=0; i<nrf24_ADDR_WIDTH; i++)
	{
		sprintf(uart_buffer, "0x%x ", temp_array[i]);
		uart_puts(uart_buffer);
	}

	uart_puts("\r\nRX Address: ");
	nrf24_get_RX_address(temp_array);
	for (uint8_t i=0; i<nrf24_ADDR_WIDTH; i++)
	{
		sprintf(uart_buffer, "0x%x ", temp_array[i]);
		uart_puts(uart_buffer);
	}
	uart_puts("\r\nACK Address: ");
	nrf24_get_ACK_address(temp_array);
	for (uint8_t i=0; i<nrf24_ADDR_WIDTH; i++)
	{
		sprintf(uart_buffer, "0x%x ", temp_array[i]);
		uart_puts(uart_buffer);
	}
}