/*
 * main.c
 *
 * Created: 10/1/2021 2:48:24 PM
 *  Author: diogo
 * 
 * Simple program to write the addresses
 * of each Luminary according to their ID.
 * Master address is the same for every luminary.
 */ 

#include <xc.h>
#include <avr/io.h>
#include "eeprom.h"

#define LED_ID 1 /* Only change this value */

#define ADDRESS_WIDTH 3

int main(void)
{
	uint8_t lum_address[ADDRESS_WIDTH]= {'L', 'M', LED_ID};
	uint8_t master_address[ADDRESS_WIDTH]= {'M', 'T', 'R'};
	
	eeprom_write_page(EEPROM_IDX_RX_ADDR, ADDRESS_WIDTH, lum_address);
	eeprom_write_page(EEPROM_IDX_TX_ADDR, ADDRESS_WIDTH, master_address);
}