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

/* Debug mode */
#define DEBUG_COMM false 

/* Number of bytes to receive over RF - Must match the Master */
#define COMMAND_LENGTH 7 

/* Maximum number of bits in the bitstream */
#define BITSTREAM_MAX_BITS 56 /* COMMAND_LENGTH*8 */

/* Number of ACK to send after receiving a message 
 * More replies ensures the master receives a response 
 * but will occupy the slave for a longer period of time */
#define ACK_REPLIES 20 

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

