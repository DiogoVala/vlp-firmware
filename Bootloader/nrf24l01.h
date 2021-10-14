/* nrf24l01.h
 *
 * Author: Diogo Vala
 * 
 * Based on the optiboot rf24 adaptation
 * Heavily modified for improved clarity and stability
 * 
 * Description: Controls the RF24 chip and its operations 
 */

#ifndef __NRF24_H__
#define __NRF24_H__

/* Library includes */
#include <stdint.h>
#include <avr/io.h>

/* File includes */
#include "nrf24l01_config.h"

/* RF24 Module operating at (2400 + NRF24_CHANNEL) MHz*/
#define NRF24_CHANNEL 2

/* Address width - 3, 4 or 5 bytes */
#define NRF24_ADDR_WIDTH 3

/* Misc. Messages */
#define NRF24_DATA_AVAILABLE 1
#define NRF24_DATA_UNAVAILABLE 0
#define NRF24_MESSAGE_SENT 0
#define NRF24_MESSAGE_LOST 1
#define NRF24_CHIP_DISCONNECTED 1
#define NRF24_CHIP_NOMINAL 0

/* NRF24 Pinout - Must match SPI file, if used */
#define NRF24_DDR	DDRB
#define NRF24_PORT	PORTB
#define NRF24_CE	DDB0
#define NRF24_CS	DDB2

/* General Defines */
#define LOW 0
#define HIGH 1

/* Modes of operation */
enum nrf24_modes{
	MODE_RX,
	MODE_TX,
	MODE_IDLE
};

/* Setup functions */
uint8_t nrf24_config(uint8_t TX_addr[NRF24_ADDR_WIDTH],uint8_t RX_addr[NRF24_ADDR_WIDTH]);
void	nrf24_rx_address(uint8_t* adr);
void	nrf24_tx_address(uint8_t* adr);

/* Core TX / RX functions */
void	nrf24_sendData(uint8_t* data, uint8_t pkt_len);
void	nrf24_getData(uint8_t * data, uint8_t * pkt_len);

/* State check functions */
uint8_t nrf24_dataReady();
uint8_t nrf24_rxFifoEmpty();
uint8_t nrf24_getStatus();
void	nrf24_resetStatus();

/* Used in dynamic length mode */
uint8_t nrf24_payloadLength();

/* Post transmission analysis */
uint8_t nrf24_wait_tx_result();

/* Mode of operation management */
void	nrf24_setMode_RX();
void	nrf24_setMode_TX();

/* Low level interface */
void	nrf24_configRegister(uint8_t reg, uint8_t value);
uint8_t nrf24_readRegister(uint8_t reg);
void	nrf24_ce_digitalWrite(uint8_t state);
void	nrf24_csn_digitalWrite(uint8_t state);

#endif