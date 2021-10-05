#ifndef NRF24
#define NRF24

#include "nrf24l01_config.h"
#include <stdint.h>
#include <avr/io.h>

/* RF24 Module operating at (2400 + NRF24_CHANNEL) MHz*/
#define NRF24_CHANNEL 2

/* Address width - 3, 4 or 5 bytes */
#define nrf24_ADDR_WIDTH 3

/* Misc. Messages */
#define NRF24_DATA_AVAILABLE 1
#define NRF24_DATA_UNAVAILABLE 0

/* Error codes */
#define NRF24_MESSAGE_SENT 0
#define NRF24_MESSAGE_LOST 1

/* NRF24 Pinout */
#define NRF24_DDR	DDRB
#define NRF24_PORT	PORTB
#define NRF24_CE	DDB0
#define NRF24_CS	DDB2

/* General Defines */
#define LOW 0
#define HIGH 1

/* Setup functions */
uint8_t nrf24_config(uint8_t TX_addr[nrf24_ADDR_WIDTH], uint8_t RX_addr[nrf24_ADDR_WIDTH]);
void nrf24_rx_address(uint8_t* adr);
void nrf24_tx_address(uint8_t* adr);

/* Core TX / RX functions */
void nrf24_sendData(uint8_t* data, uint8_t pkt_len);
void nrf24_getData(uint8_t * data, uint8_t * pkt_len);

/* State check functions */
uint8_t nrf24_dataReady();
uint8_t nrf24_getStatus();
uint8_t nrf24_rxFifoEmpty();
void nrf24_resetStatus();

/* Used in dynamic length mode */
uint8_t nrf24_payloadLength();

/* Post transmission analysis */
uint8_t nrf24_wait_tx_result();
uint8_t nrf24_retransmissionCount();

/* Mode of operation management */
void nrf24_powerUpRx();
void nrf24_powerUpTx();
void nrf24_powerDown();

/* Low level interface */
void nrf24_configRegister(uint8_t reg, uint8_t value);
uint8_t nrf24_readRegister(uint8_t reg);
void nrf24_ce_digitalWrite(uint8_t state);
void nrf24_csn_digitalWrite(uint8_t state);

#endif