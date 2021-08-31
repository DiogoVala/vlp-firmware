/* communication.c
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 *
 * Description: Handles the communication between
 * modules.
 */

/* Library Includes */
#include <avr/interrupt.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <util/delay.h>

/* File includes */
#include "led.h"
#include "communication.h"
#include "nrf24l01.h"
#include "uart.h"

uint8_t TX_command_array[COMMAND_LENGTH] = {}; /* Bytes to transmit go here */
uint8_t ACK_Array[COMMAND_LENGTH] = {}; /* Received ACK message goes here */
uint8_t bitstream_byte_array[COMMAND_LENGTH] = {0}; /* Bitstream bits are stored here in the form of bytes before transmitting */
static volatile uint8_t byte_count = 0; /* Number of bytes needed to store the bitstream before transmitting */ 

/* Prototypes of private functions */
void sendAndWaitACK();
void buildLEDCommand(led_t* ledp);
void updateBitstream();
void updateLED(led_t* ledp);
void byteArrayToBits(uint8_t byte_array[], uint8_t bitstreamSize);
void bitsToByteArray(uint8_t bitstream[], uint8_t bitstreamSize);

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
			TX_command_array[ID]=id;
			sendAndWaitACK(); 
		}
	}
	else
	{
		sendAndWaitACK();
	}
}

/* Builds and sends command with led params */
void sendCommand(led_t* ledp) {
	
	uart_puts("\r\nSending Command... ");
	buildLEDCommand(ledp);

	/* If ID is 255, send to all luminaries */
	if(TX_command_array[ID]==BROADCAST)
	{
		for(uint8_t id=0; id<MAX_LUMINARIES; id++)
		{
			TX_command_array[ID]=id;
			sendAndWaitACK();
		}
	}
	else
	{
		sendAndWaitACK();
	}
}

/* Sends command array and waits for a ACK reply */
void sendAndWaitACK(){
	bool ack=false;
	uint16_t ACK_tries=0;
	uint16_t TX_tries=0;
	
	while(ack==false && TX_tries++ < MAX_TX_RETRIES) /* Send again if not ack not received */
	{
		nrf24_send(TX_command_array);
		_delay_ms(5);
		
		while(ack==false && ACK_tries++ < MAX_ACK_RX_RETRIES) /* Wait for Ack */
		{
			nrf24_getData(ACK_Array);
			if(ACK_Array[ID]==TX_command_array[ID] && ACK_Array[IDENTIFIER]=='A')
			{
				ack=true;
			}
		}
		ACK_tries=0;
	}

	#if DEBUG_COMM
		uint8_t message_string[100]={};
		if(ack)
		{
			sprintf(message_string, "\r\nCommand sent to luminary %03d. ACK received.\r\n", TX_command_array[ID]);
			uart_puts(message_string);
		}
		else
		{
			sprintf(message_string, "\r\nCommand sent to luminary %03d. ACK failed.\r\n", TX_command_array[ID]);
			uart_puts(message_string);
		}
	#endif
}

/* Builds the command array with the led params to send via RF */
void buildLEDCommand(led_t* ledp) {
	
	memset(TX_command_array, '\0', COMMAND_LENGTH);
	
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