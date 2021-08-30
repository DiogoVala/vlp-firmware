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
#define BITSTREAM_MAX_BYTES 30 
#define BITSTREAM_MAX_BITS 240 /* 30 bytes * 8 bit */

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

/* Builds and sends the bitstream via RF*/
void sendBitStream(uint8_t bitstream[], uint8_t bitstreamSize, led_t* ledp);

/* Builds and sends command with led params */
void sendCommand(led_t* ledp);

#endif	/* COMMUNICATION_H */

