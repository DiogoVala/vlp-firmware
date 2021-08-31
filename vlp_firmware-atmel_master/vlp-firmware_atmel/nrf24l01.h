#ifndef NRF24
#define NRF24

#include "nrf24l01_config.h"
#include <stdint.h>

/* Number of bytes to receive over RF - Must match the Master */
#define NRF24_PAYLENGTH 7

/* RF24 Module operating at (2400 + NRF24_CHANNEL) MHz*/
#define NRF24_CHANNEL 2

#define nrf24_ADDR_LEN 5
#define nrf24_CONFIG ((1<<EN_CRC)|(0<<CRCO))

/* Error codes */
#define NRF24_TRANSMISSON_OK 0
#define NRF24_MESSAGE_LOST   1

/* NRF24 Pinout */
#define NRF24_DDR	DDRB
#define NRF24_PORT	PORTB
#define NRF24_CE	DDB0
#define NRF24_CS	DDB2

/* General Defines */
#define LOW 0
#define HIGH 1

/* adjustment functions */
void    nrf24_init();
void    nrf24_rx_address(uint8_t* adr);
void    nrf24_tx_address(uint8_t* adr);
void    nrf24_config(uint8_t channel, uint8_t pay_length);

/* state check functions */
uint8_t nrf24_dataReady();
uint8_t nrf24_isSending();
uint8_t nrf24_getStatus();
uint8_t nrf24_rxFifoEmpty();

/* core TX / RX functions */
void nrf24_send(uint8_t* value);
void nrf24_getData(uint8_t* data);

/* use in dynamic length mode */
uint8_t nrf24_payloadLength();

/* post transmission analysis */
uint8_t nrf24_lastMessageStatus();
uint8_t nrf24_retransmissionCount();

/* power management */
void    nrf24_powerUpRx();
void    nrf24_powerUpTx();
void    nrf24_powerDown();

/* low level interface ... */
void nrf24_transmitSync(uint8_t* dataout,uint8_t len);
void nrf24_transferSync(uint8_t* dataout,uint8_t* datain,uint8_t len);
void nrf24_configRegister(uint8_t reg, uint8_t value);
void nrf24_readRegister(uint8_t reg, uint8_t* value, uint8_t len);
void nrf24_writeRegister(uint8_t reg, uint8_t* value, uint8_t len);
void nrf24_ce_digitalWrite(uint8_t state);
void nrf24_csn_digitalWrite(uint8_t state);

#endif