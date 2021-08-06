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

void sendBitStream(uint8_t bitstream[], uint8_t bitstreamSize);

void buildLEDCommand(led_t* ledp);

void updateLEDParams(led_t* ledp);

#endif	/* COMMUNICATION_H */

