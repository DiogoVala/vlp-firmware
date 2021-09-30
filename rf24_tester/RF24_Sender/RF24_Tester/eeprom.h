/*
 * eeprom.h
 *
 * Created: 9/27/2021 5:55:59 PM
 *  Author: diogo
 */ 


#ifndef EEPROM_H_
#define EEPROM_H_

#define EEPROM_IDX_RX_ADDR 0
#define EEPROM_IDX_TX_ADDR 3

uint8_t eeprom_read(uint16_t addr);
/* To do: add write */

#endif /* EEPROM_H_ */