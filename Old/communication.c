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

/* File includes */
#include "led.h"
#include "ledInterface.h"
#include "communication.h"
#include "timer.h"
#include "nrf24l01.h"

#define NUM_LUMINARIES 16

uint8_t TX_command_array[COMMAND_LENGTH] = {};
uint8_t RX_command_array[COMMAND_LENGTH] = {};
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

/* Builds and sends the bitstream via RF*/
void sendBitStream(uint8_t bitstream[], uint8_t bitstreamSize, led_t* ledp) {
    bitsToByteArray(bitstream, bitstreamSize);
    memset(TX_command_array, '\0', sizeof (TX_command_array));

    TX_command_array[ID] = ledp->ledID;
    TX_command_array[IDENTIFIER] = 0xFF; /* To indicate bitstream */
    TX_command_array[BIT_COUNT] = bitstreamSize;
    for (uint8_t i = BITSTREAM; i < (byte_count + byte_count); i++) {
        TX_command_array[i] = bitstream_byte_array[i - BITSTREAM];
    }
    nrf24_send_message(TX_command_array);
}

/* Builds and sends command with led params */
void sendCommand(led_t* ledp) {
       
    memset(TX_command_array, '\0', sizeof (TX_command_array));
    buildLEDCommand(ledp);
    if(TX_command_array[ID]==0xFF)
    {
        for(uint8_t i = 0; i < NUM_LUMINARIES; i++) /* Depois mudo isto */
        {
            TX_command_array[ID]=i;
            nrf24_send_message(TX_command_array);
        }
    }
    nrf24_send_message(TX_command_array);
}


/* Checks the RF module for new data and processes it */
void checkRF(led_t* ledp) {

    char temp[COMMAND_LENGTH] = {};

    while (nrf24_available() == 0); // Wait for message

    cli();

    strcpy(temp, nrf24_read_message()); /* Store received bytes into temp array */

    for (uint8_t i = 0; i < 32; i++) /* Convert all bytes to uint8_t and store in RX array*/
        RX_command_array[i] = (uint8_t) temp[i];

    /* Evaluate data*/
    if (RX_command_array[ID] == ledp->ledID) {
        if (RX_command_array[IDENTIFIER] != 0xFF) /* Command received */ {
            updateLED(ledp); /* Update LED*/
        }
        else /* Bitstream received */ {
            updateBitstream();
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

/* Gets the next bit in the bitstrea, according to the mode of operation*/
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

/* Builds the command array with the led params to send via RF */
void buildLEDCommand(led_t* ledp) {
    /* Array of bytes to send to RF module */
    TX_command_array[ID] = getLedID(ledp);
    TX_command_array[STATE] = getLedState(ledp);
    TX_command_array[MODE] = getLedMode(ledp);
    TX_command_array[INTENSITY] = getLedIntensity(ledp);
    TX_command_array[FREQUENCY_LB] = (uint8_t) (getLedFrequency(ledp) & 0x00FF);
    TX_command_array[FREQUENCY_HB] = (uint8_t) ((getLedFrequency(ledp) & 0xFF00) >> 8);
    TX_command_array[DUTYCYCLE] = getLedDutyCycle(ledp);
}

/* Transforms an array of bytes to an array of bits */
void byteArrayToBits(uint8_t byte_array[], uint8_t bitstreamSize) {

    for (uint8_t index = 0; index < bitstreamSize; index++) {
        bitstream[index] = ((byte_array[index / 8] & (1 << (7 - (index % (7 + index / 8))))) >> (7 - (index % (7 + index / 8))));
    }
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