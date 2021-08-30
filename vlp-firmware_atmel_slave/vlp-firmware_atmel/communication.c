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
#include "ledInterface.h"
#include "communication.h"
#include "timer.h"
#include "nrf24l01.h"
#include "uart.h"

uint8_t RX_command_array[COMMAND_LENGTH] = {};
uint8_t ACK_Array[COMMAND_LENGTH] = {};
uint8_t bitstream_byte_array[BITSTREAM_MAX_BYTES] = {0};
uint8_t bitstream[BITSTREAM_MAX_BITS] = {0};
uint8_t byte_count = 0;
uint8_t bit_count = 0;

/* Prototypes of private functions */
void buildLEDCommand(led_t* ledp);
void updateBitstream();
void updateLED(led_t* ledp);
void byteArrayToBits(uint8_t byte_array[], uint8_t bitstreamSize);
void bitsToByteArray(uint8_t bitstream[], uint8_t bitstreamSize);

/* Checks the RF module for new data and processes it */
void checkRF(led_t* ledp) {

    while (nrf24_dataReady() == 0); // Wait for message
    cli();

    nrf24_getData(RX_command_array); /* Store received bytes into temp array */
	
	uint8_t buf[4]={0};
	uart_puts("\r\n");
	uart_puts("\r\nData Received.\r\n");
	for (uint8_t i=0; i<COMMAND_LENGTH; i++)
	{
		sprintf(buf, "%d, ", RX_command_array[i]);
		uart_puts(buf);
	}
	uart_puts("\r\n");
	
	uart_puts("\r\nSending Ack");
	memset(ACK_Array, '\0', COMMAND_LENGTH);
	ACK_Array[0]=RX_command_array[ID];
	ACK_Array[1]='A';
	
	for(uint16_t i=0; i<65534; i++)
	{
		nrf24_send(ACK_Array);
		uart_puts("\r\nSending Ack... ");
		while(nrf24_isSending());
		uart_puts("Sent.");
	}
	
    /* Evaluate data*/
    if (RX_command_array[ID] == ledp->ledID) {
        if (RX_command_array[IDENTIFIER] != 0xFF) /* Command received */ {
            updateLED(ledp); /* Update LED*/
			uart_puts("\r\nCommand received.");
        }
        else /* Bitstream received */ {
            updateBitstream();
			uart_puts("\r\nBitstream received.\r\n");
			for(uint8_t i=0; i<bit_count; i++)
			{
				sprintf(buf, "%d, ", bitstream[i]);
				uart_puts(buf);
			}
        }
    }
    sei();
}

/* Updates bitstream array with new data from RF */
void updateBitstream() {
    bit_count = RX_command_array[BIT_COUNT];
    byte_count = bit_count / 8;
    if (bit_count % 8 != 0) byte_count += 1; /* Ceil */

    for (uint8_t i = 0; i < byte_count; i++) {
        bitstream_byte_array[i] = RX_command_array[BITSTREAM + i];
    }
    byteArrayToBits(bitstream_byte_array, bit_count);
}

/* Updates the LED with the new params and does HW changes required */
void updateLED(led_t* ledp) {
    /* Update the LED parameters from received command */
    setLedState(ledp, RX_command_array[STATE]);
    setLedMode(ledp, RX_command_array[MODE]);
    setLedIntensity(ledp, RX_command_array[INTENSITY]);
    setLedFrequency(ledp, ((RX_command_array[FREQUENCY_HB] << 8) | RX_command_array[FREQUENCY_LB]));
    setLedDutyCycle(ledp, RX_command_array[DUTYCYCLE]);
    updateLEDHW(ledp); /* Changes state pin, digpot position and timer behavior */
}

/* Gets the next bit in the bitstream, according to the mode of operation*/
uint8_t getBit(led_t *ledp) {
    static uint8_t pos = 0;

    if (pos >= bit_count)
        pos = 0;
    else
        pos++;

    if (ledp->ledMode != LED_MODE_VPPM)
        return bitstream[pos];
    else
        return !bitstream[pos]; /* In VPPM we want the bit to start at the oposite value and invert at OCR1B */
}


/* Transforms an array of bytes to an array of bits */
void byteArrayToBits(uint8_t byte_array[], uint8_t bitstreamSize) {

    for (uint8_t index = 0; index < bitstreamSize; index++) {
        bitstream[index] = ((byte_array[index / 8] & (1 << (7 - (index % (7 + index / 8))))) >> (7 - (index % (7 + index / 8))));
    }
}
