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

uint8_t payload_len;

#define ADDRESS_MASTER 0
#define ADDRESS_SLAVE 1

/* TX and RX Address bytes */
uint8_t nrf24_addr[][5] = {
	{'M','A','S','T','R'},
	{'S','L','A','V','E'}
};

/* init the hardware pins */
void nrf24_init()
{
	NRF24_DDR |= (_BV(NRF24_CE) | _BV(NRF24_CS)); /* Set Chip Enable pin as output */
	nrf24_tx_address(nrf24_addr[ADDRESS_SLAVE]);
	nrf24_rx_address(nrf24_addr[ADDRESS_MASTER]);
	nrf24_ce_digitalWrite(LOW);
	nrf24_csn_digitalWrite(HIGH);
	
	nrf24_config(NRF24_CHANNEL,NRF24_PAYLENGTH);
}

/* configure the module */
void nrf24_config(uint8_t channel, uint8_t pay_length)
{
	/* Use static payload length ... */
	payload_len = pay_length;

	// Set RF channel
	nrf24_configRegister(RF_CH,channel);

	// Set length of incoming payload
	nrf24_configRegister(RX_PW_P0, 0x00); // Auto-ACK pipe ...
	nrf24_configRegister(RX_PW_P1, payload_len); // Data payload pipe
	nrf24_configRegister(RX_PW_P2, 0x00); // Pipe not used
	nrf24_configRegister(RX_PW_P3, 0x00); // Pipe not used
	nrf24_configRegister(RX_PW_P4, 0x00); // Pipe not used
	nrf24_configRegister(RX_PW_P5, 0x00); // Pipe not used
	
	// 2 Mbps, TX gain: 0dbm
	nrf24_configRegister(RF_SETUP, (1 << RF_PWR_LOW) | (1 << RF_PWR_HIGH) | (0 << RF_DR_LOW) | (1 << RF_DR_HIGH));

	// CRC enable, 1 byte CRC length
	nrf24_configRegister(CONFIG,nrf24_CONFIG);

	// Auto Acknowledgment
	nrf24_configRegister(EN_AA,(0<<ENAA_P0)|(0<<ENAA_P1)|(0<<ENAA_P2)|(0<<ENAA_P3)|(0<<ENAA_P4)|(0<<ENAA_P5));

	// Enable RX addresses
	nrf24_configRegister(EN_RXADDR,(1<<ERX_P0)|(1<<ERX_P1)|(0<<ERX_P2)|(0<<ERX_P3)|(0<<ERX_P4)|(0<<ERX_P5));

	// Auto retransmit delay: 1000 us and Up to 15 retransmit trials
	//nrf24_configRegister(SETUP_RETR,(0x04<<ARD)|(0x0F<<ARC));

	// Dynamic length configurations: No dynamic length
	nrf24_configRegister(DYNPD,(0<<DPL_P0)|(0<<DPL_P1)|(0<<DPL_P2)|(0<<DPL_P3)|(0<<DPL_P4)|(0<<DPL_P5));
	
	// Start listening
	nrf24_powerUpRx();
}

/* Set the RX address */
void nrf24_rx_address(uint8_t * adr)
{
	nrf24_writeRegister(RX_ADDR_P1,adr,nrf24_ADDR_LEN);
	nrf24_ce_digitalWrite(HIGH);
}

/* Set the TX address */
void nrf24_tx_address(uint8_t* adr)
{
	/* RX_ADDR_P0 must be set to the sending addr for auto ack to work. */
	nrf24_writeRegister(RX_ADDR_P0,adr,nrf24_ADDR_LEN);
	nrf24_writeRegister(TX_ADDR,adr,nrf24_ADDR_LEN);
}

/* Checks if there is data in the RX FIFO  */
uint8_t nrf24_dataReady()
{
	// See note in getData() function - just checking RX_DR isn't good enough
	uint8_t status = nrf24_getStatus();

	/* Check if Data Ready flag is set.*/
	/* RX_DR just means data has arrived in the FIFO,
	 * but we still need to verify if there is already
	 * data in the FIFO in the occasions where RX_DR
	 * isn't set. */
	if ( status & (1 << RX_DR) )
	{
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
	for(uint8_t i = 0; i < *pkt_len; i++) {
		*data++ = spi_exchange(0);
	}
	
	/* Pull up chip select */
	nrf24_csn_digitalWrite(HIGH);

	/* Reset status register */
	nrf24_configRegister(STATUS,(1<<RX_DR));
}

/* Returns the number of retransmissions occurred for the last message */
uint8_t nrf24_retransmissionCount()
{
	uint8_t rv;
	nrf24_readRegister(OBSERVE_TX,&rv,1);
	rv = rv & 0x0F;
	return rv;
}

/* Sends a packet of data with dynamic length */
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
	for(uint8_t i = 0; i < pkt_len; i++) {
		spi_exchange(*data++);
	}
	
	/* Pull up chip select */
	nrf24_csn_digitalWrite(HIGH);

	/* Start the transmission */
	nrf24_ce_digitalWrite(HIGH);
	
}
#if 0
uint8_t nrf24_isSending()
{
	uint8_t status;

	/* read the current status */
	status = nrf24_getStatus();
	
	/* if sending successful (TX_DS) or max retries exceeded (MAX_RT). */
	if(status & ((1 << TX_DS)))
	{
		return 1; 
	}
	return 0;
}
#endif

/* Get status register data */
uint8_t nrf24_getStatus()
{
	uint8_t rv;
	nrf24_csn_digitalWrite(LOW);
	rv = spi_exchange(NOP);
	nrf24_csn_digitalWrite(HIGH);
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

/* send and receive multiple bytes over SPI */
void nrf24_transferSync(uint8_t* dataout,uint8_t* datain,uint8_t len)
{
	for(uint8_t i=0; i<len; i++)
	{
		datain[i] = spi_exchange(dataout[i]);
	}

}

/* send multiple bytes over SPI */
void nrf24_transmitSync(uint8_t* dataout,uint8_t len)
{
	for(uint8_t i=0; i<len; i++)
	{
		spi_exchange(dataout[i]);
	}

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
void nrf24_readRegister(uint8_t reg, uint8_t* data, uint8_t len)
{
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(R_REGISTER | (REGISTER_MASK & reg));
	nrf24_transferSync(data,data,len);
	nrf24_csn_digitalWrite(HIGH);
}

/* Write to a single register of nrf24 */
void nrf24_writeRegister(uint8_t reg, uint8_t* data, uint8_t len)
{
	nrf24_csn_digitalWrite(LOW);
	spi_exchange(W_REGISTER | (REGISTER_MASK & reg));
	nrf24_transmitSync(data,len);
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
