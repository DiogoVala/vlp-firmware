/* communication.c
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 *
 * Description: Handles the communication between
 * modules.
 */

/* NOTE: Broadcast needs testing */

/* Library Includes */
#include <avr/interrupt.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <util/delay.h>

/* File includes */
#include "../../Common/led.h"
#include "../../Common/nrf24l01.h"
#include "../../Common/uart.h"
#include "communication.h"

/* Debug mode */
/* Presents useful information in the terminal */
#define DEBUG_COMM false

/* Default RF24 addresses */
uint8_t TX_addr[NRF24_ADDR_WIDTH]={'L', 'M', '1'};
uint8_t RX_addr[NRF24_ADDR_WIDTH]={'M', 'T', 'R'};

uint8_t TX_command_array[NRF24_MAX_PAYLOAD] = {}; /* Bytes to transmit go here */
uint8_t ACK_Array[NRF24_MAX_PAYLOAD] = {}; /* Received ACK message goes here */
uint8_t bitstream_byte_array[NRF24_MAX_PAYLOAD] = {0}; /* Bitstream bits are stored here in the form of bytes before transmitting */
static volatile uint8_t byte_count = 0; /* Number of bytes needed to store the bitstream before transmitting */ 

/* Builds and sends command with led params */
void sendCommand(led_t* ledp) {
	
	uart_puts("\r\nSending Command... ");
	buildLEDCommand(ledp);
	
	#if DEBUG_COMM
	uart_puts("\r\nCommand: ");
	uint8_t buf[50];
	for (uint8_t i = 0; i <= SIZE_OF_COMMAND-1; i++) {
		sprintf(buf, "0x%x, ", TX_command_array[i]);
		uart_puts(buf);
	}
	uart_puts("\r\n");
	#endif

	/* If ID is 255, send to all luminaries */
	if(TX_command_array[ID]==BROADCAST)
	{
		for(uint8_t id=0; id<MAX_LUMINARIES; id++)
		{
			TX_command_array[ID]=id; /* Change the ID parameter of command */
			TX_addr[NRF24_ADDR_WIDTH-1]=id; /* Change the TX address */
			
			nrf24_tx_address(TX_addr); /* Update TX address */
			_delay_us(10);
			
			nrf24_sendData(TX_command_array, SIZE_OF_COMMAND); 
		}
	}
	else
	{
		TX_addr[NRF24_ADDR_WIDTH-1]=ledp->ledID;
		nrf24_tx_address(TX_addr);
		_delay_us(10);

		nrf24_sendData(TX_command_array, SIZE_OF_COMMAND);
		
		if(nrf24_wait_tx_result()==NRF24_MESSAGE_SENT)
			uart_puts("\r\nMessage sent and acknowledged");
		else
			uart_puts("\r\nMessage not acknowledged");
	}
}

/* Builds and sends the bitstream command */
void sendBitStream(uint8_t bitstream[], uint8_t bitstreamSize, led_t* ledp) {
	
	uart_puts("\r\nSending bitstream.");
	
	/*Transforms sequence of bits into bytes to save space during transmission*/
    bitsToByteArray(bitstream, bitstreamSize);
	
    memset(TX_command_array, '\0', sizeof (TX_command_array));
	
	/* Builds the array that will be sent */
    TX_command_array[ID] = ledp->ledID;  
    TX_command_array[IDENTIFIER] = BITSTREAM_IDENTIFIER; /* To indicate bitstream instead of normal command */
    TX_command_array[BIT_COUNT] = bitstreamSize; /* Number of bits in the bitstream, so the slave knows how many to read */
	/* Fill the rest with bytes containing the bitstream */
	for (uint8_t i = BITSTREAM; i < (BITSTREAM + byte_count); i++) {
        TX_command_array[i] = bitstream_byte_array[i - BITSTREAM];
    }
	
	#if DEBUG_COMM
		uart_puts("\r\nBitstream bytes: ");
		uint8_t buf[50];
		for (uint8_t i = BITSTREAM; i <= (BITSTREAM + byte_count); i++) {
			sprintf(buf, "0x%x, ", TX_command_array[i]);
			uart_puts(buf);
		}
	#endif
	
	/* If ID is 255, send to all luminaries */
	if(TX_command_array[ID]==BROADCAST)
	{
		for(uint8_t id=0; id<=MAX_LUMINARIES; id++)
		{
			TX_command_array[ID]=id; /* Change the ID parameter of command */
			TX_addr[NRF24_ADDR_WIDTH-1]=id; /* Change the TX address */
			
			nrf24_tx_address(TX_addr); /* Update TX address */
			_delay_us(10);
			
			nrf24_sendData(TX_command_array, BITSTREAM + byte_count); 
		}
	}
	else
	{
		TX_addr[NRF24_ADDR_WIDTH-1]=ledp->ledID;
		nrf24_tx_address(TX_addr);
		_delay_us(10);
		
		nrf24_sendData(TX_command_array, BITSTREAM + byte_count);
	}
}



/* Builds the command array with the led params to send via RF */
void buildLEDCommand(led_t* ledp) {
	
	memset(TX_command_array, '\0', SIZE_OF_COMMAND);
	
    /* Array of bytes to send to RF module */
    TX_command_array[ID] = getLedID(ledp);
    TX_command_array[STATE] = getLedState(ledp);
    TX_command_array[MODE] = getLedMode(ledp);
    TX_command_array[INTENSITY] = getLedIntensity(ledp);
    TX_command_array[FREQUENCY_LB] = (uint8_t) (getLedFrequency(ledp) & 0x00FF);
    TX_command_array[FREQUENCY_HB] = (uint8_t) ((getLedFrequency(ledp) >> 8 ) & 0x00FF);
    TX_command_array[DUTYCYCLE] = getLedDutyCycle(ledp);
	
}

/* Transforms an array of bits into an array of bytes */
void bitsToByteArray(uint8_t bitstream[], uint8_t bitstreamSize) {

	uint8_t newByte = 0;
	uint8_t bit_count=0;
	byte_count=0;

	for (uint8_t i = 0; i < bitstreamSize; i++) {
		newByte = (newByte << 1) | bitstream[i];
		bit_count++;
		if(bit_count%8==0)
		{
			bitstream_byte_array[byte_count] = newByte;
			newByte = 0;
			bit_count=0;
			byte_count++;
		}
	}
	if(bit_count!=0)
	{
		bitstream_byte_array[byte_count]=(newByte<<(8-bit_count));
		byte_count++;
	}
}