/*
 * eeprom.c
 *
 * Created: 9/27/2021 5:55:46 PM
 *  Author: diogo
 */ 

#include <avr/io.h>
#include "eeprom.h"

uint8_t eeprom_read(uint16_t addr) {
	while (EECR & (1 << EEPE));

	EEAR = addr;
	EECR |= 1 << EERE;	/* Start eeprom read by writing EERE */

	return EEDR;
}