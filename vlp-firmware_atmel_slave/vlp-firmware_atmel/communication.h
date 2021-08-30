/* 
 * File:   communication.h
 * Author: Diogo Vala <your.name at your.org>
 *
 * Created on August 3, 2021, 10:39 PM
 */

#ifndef COMMUNICATION_H
#define	COMMUNICATION_H

#include <stdint.h>

#include "led.h"

#define COMMAND_LENGTH 7
#define BITSTREAM_MAX_BYTES 7 
#define BITSTREAM_MAX_BITS 40 /* 5 bytes * 8 bit */

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
void checkRF();

/* Gets the next bit in the bitstream, according to the mode of operation*/
uint8_t getBit();

#endif	/* COMMUNICATION_H */

