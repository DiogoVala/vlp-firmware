/* 
 * File:   communication.h
 * Author: Diogo Vala <your.name at your.org>
 *
 * Created on August 3, 2021, 10:39 PM
 */

#ifndef COMMUNICATION_H
#define	COMMUNICATION_H

#include <stdint.h>

#include "../../Common/led.h"

/* Maximum number of bits in the bitstream */
#define BITSTREAM_MAX_BITS 40 /* COMMAND_LENGTH*8 */

/* Send message to all slaves if ID is: */
#define BROADCAST 0xFF

/* Identifies that the bytes contain a bitstream */
#define BITSTREAM_IDENTIFIER 0xFF

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
    DUTYCYCLE
};

enum bitstream_params {
    IDENTIFIER = 1,
    BIT_COUNT,
    BITSTREAM
};

/* Checks the RF module for new data and processes it */
void checkRF(led_t* ledp);

/* Gets the next bit in the bitstream, according to the mode of operation*/
uint8_t getBit();

#endif	/* COMMUNICATION_H */

