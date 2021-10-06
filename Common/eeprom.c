/*
 * eeprom.c
 *
 * Created: 9/27/2021 5:55:46 PM
 *  Author: diogo
 */ 

#include <avr/io.h>
#include "eeprom.h"

/* Write a byte to EEPROM at address */
void eeprom_write(uint16_t address, uint8_t byte) {
	while (EECR & _BV(EEPE)) ; /* Wait until any write operation is complete */
	EEAR = address;
	EEDR = byte;
	EECR |= ( 1 << EEMPE ); /* Trigger write operation */
	EECR |= ( 1 << EEPE );  /* Start write operation*/
}

/* Read single byte from EEPROM address */
uint8_t eeprom_read(uint16_t address) {
	while (EECR & _BV(EEPE));
	EEAR = address;
	EECR |= ( 1 << EERE ); /* Trigger read operation */
	return EEDR;
}

/* Write n bytes to EEPROM from starting address */
void eeprom_write_page (uint16_t saddress, uint16_t n, uint8_t* bytes) {              
	for(uint8_t i=0; i<n; i++){
		eeprom_write(saddress+i, bytes[i]);
	}
}

/* Read n bytes to EEPROM from starting address */
void eeprom_read_page (uint16_t saddress, uint16_t n, uint8_t* bytes) {
	for(uint8_t i=0; i<n; i++){
		bytes[i] = eeprom_read(saddress+i);
	}
}