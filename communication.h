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

#define COMMAND_LENGTH 7 // Number of command parameters + 1 
#define BITSTREAM_MAX_BYTES 30 
#define BITSTREAM_MAX_SIZE 240 /* 30 bytes * 8 bit */


void sendBitStream(uint8_t bitstream[], uint8_t bitstreamSize, uint8_t ledID);

void buildLEDCommand(led_t* ledp);

void updateLEDParams(led_t* ledp);

#endif	/* COMMUNICATION_H */

