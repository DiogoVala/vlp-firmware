/* 
 * File:   communication.h
 * Author: Diogo Vala <your.name at your.org>
 *
 * Created on August 3, 2021, 10:39 PM
 */

#ifndef COMMUNICATION_H
#define	COMMUNICATION_H

/* Library Includes */
#include <stdint.h>

/* File includes */
#include "../../Common/led.h"

/* Maximum number of bits in the bitstream */
#define BITSTREAM_MAX_BITS 40 /* COMMAND_LENGTH*8 */

/* Send message to all slaves if ID is: */
#define BROADCAST 0xFF

/* Identifies that the bytes contain a bitstream */
#define BITSTREAM_IDENTIFIER 0xFF

/* Payload with this single byte will reset the board */
#define RESET_COMMAND 0xFF

/* Maximum number of luminaries */
/* Maximum is 255, but we don't use that many, so it's faster to have only the necessary ones */
#define MAX_LUMINARIES 16 

/* Byte position of each parameter */
enum command_params {
    ID,
    STATE,
    MODE,
    INTENSITY,
    FREQUENCY_LB,
    FREQUENCY_HB,
    DUTYCYCLE,
	SIZE_OF_COMMAND
};

enum bitstream_params {
    IDENTIFIER = 1,
    BIT_COUNT,
    BITSTREAM
};

/* Checks the RF module for new data and processes it */
void checkRF(led_t* ledp);

/* Updates bitstream array with new data from RF */
void updateBitstream();

/* Updates the LED with the new params and does HW changes required */
void updateLED(led_t* ledp);

/* Gets the next bit in the bitstream, according to the mode of operation*/
uint8_t getBit();

/* Transforms an array of bytes into an array of bits */
void byteArrayToBits(uint8_t byte_array[], uint8_t bitstreamSize);

/* True if the received command is a reset command */
bool command_IsReset();

/* True if the received command has valid parameters */
bool command_IsValid();

#endif	/* COMMUNICATION_H */

