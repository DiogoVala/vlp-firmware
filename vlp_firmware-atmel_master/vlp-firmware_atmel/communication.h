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
#define DEBUG_COMM true

/* Number of bytes to receive over RF - Must match the Master */
#define COMMAND_LENGTH 7

/* Maximum number of bits in the bitstream */
#define BITSTREAM_MAX_BITS 56 /* COMMAND_LENGTH*8 */

/* Number of TX retries if ACK not received */
#define MAX_TX_RETRIES 10

/* Number of retries to receive ACK */
#define MAX_ACK_RX_RETRIES 1000

/* Send message to all slaves if ID is: */
#define BROADCAST 0xFF

/* Identifies that the command contain a bitstream */
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

/* Builds and sends the bitstream */
void sendBitStream(uint8_t bitstream[], uint8_t bitstreamSize, led_t* ledp);

/* Builds and sends command with led params */
void sendCommand(led_t* ledp);

#endif	/* COMMUNICATION_H */

