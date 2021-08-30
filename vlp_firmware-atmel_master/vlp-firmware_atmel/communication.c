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

#define NUM_LUMINARIES 16

uint8_t TX_command_array[COMMAND_LENGTH] = {};
uint8_t ACK_Array[COMMAND_LENGTH] = {};
uint8_t bitstream_byte_array[BITSTREAM_MAX_BYTES] = {0};
uint8_t bitstream[BITSTREAM_MAX_BITS] = {0};
uint8_t byte_count = 0;

/* Prototypes of private functions */
void buildLEDCommand(led_t* ledp);
void updateBitstream();
void updateLED(led_t* ledp);
void byteArrayToBits(uint8_t byte_array[], uint8_t bitstreamSize);
void bitsToByteArray(uint8_t bitstream[], uint8_t bitstreamSize);

/* Builds and sends the bitstream via RF*/
void sendBitStream(uint8_t bitstream[], uint8_t bitstreamSize, led_t* ledp) {
	uart_puts("\r\nSending bitstream.");
    bitsToByteArray(bitstream, bitstreamSize);
    memset(TX_command_array, '\0', sizeof (TX_command_array));
    TX_command_array[ID] = ledp->ledID;
    TX_command_array[IDENTIFIER] = 0xFF; /* To indicate bitstream */
    TX_command_array[BIT_COUNT] = bitstreamSize;
    for (uint8_t i = BITSTREAM; i < (byte_count + byte_count); i++) {
        TX_command_array[i] = bitstream_byte_array[i - BITSTREAM];
    }
    nrf24_send(TX_command_array);
	while(nrf24_isSending());
}

/* Builds and sends command with led params */
void sendCommand(led_t* ledp) {
	
	bool ack=false;
	uint8_t ACK_tries=0;
	uint8_t TX_tries=0;
	
	uart_puts("\r\nSending Command... ");
	buildLEDCommand(ledp);

	while(ack==false && TX_tries<10)
	{
		nrf24_send(TX_command_array);
		_delay_ms(5);
		while(ack==false && ACK_tries<1000)
		{
			nrf24_getData(ACK_Array); /* Store received bytes into temp array */
			if(ACK_Array[ID]==TX_command_array[ID] && ACK_Array[IDENTIFIER]=='A')
			{
				
				ack=true;
			}
			ACK_tries++;
		}
		TX_tries++;
	}
	
	uint8_t message_string[100]={};
	if(ack==false)
	{
		sprintf(message_string, "\r\nCommand to luminary %d sent but acknowledgment failed.", TX_command_array[ID]);
		uart_puts(message_string);
	}
	else
	{
		sprintf(message_string, "\r\nCommand to luminary %d  sent and successfully received.", TX_command_array[ID]);
		uart_puts(message_string);
	}
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

/* Transforms an array of bits to an array of bytes */
void bitsToByteArray(uint8_t bitstream[], uint8_t bitstreamSize) {

    uint8_t index = 0;
    uint8_t newByte = 0;

    for (index = 0; index < bitstreamSize; index++) {
        newByte = newByte << 1 | bitstream[index];
        if (index % 7 == 0 && index != 0) {
            bitstream_byte_array[index / 8] = newByte;
            byte_count++;
            newByte = 0;
        }
    }
    if (index % 7 != 0) {
        /* Shift remaining bits to the left */
        bitstream_byte_array[index / 8] = newByte << (8 - (index % 8));
        byte_count++;
    }
}