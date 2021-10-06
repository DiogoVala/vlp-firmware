/*
 * eeprom.h
 *
 * Created: 9/27/2021 5:55:59 PM
 *  Author: diogo
 */ 


#ifndef EEPROM_H_
#define EEPROM_H_

/* Memory index of data */
#define EEPROM_IDX_RX_ADDR 0
#define EEPROM_LED_ID	   2
#define EEPROM_IDX_TX_ADDR 3

void eeprom_write(uint16_t address, uint8_t byte);

uint8_t eeprom_read(uint16_t address);

void eeprom_write_page (uint16_t saddress, uint16_t n, uint8_t* bytes);

void eeprom_read_page (uint16_t saddress, uint16_t n, uint8_t* bytes);

#endif /* EEPROM_H_ */