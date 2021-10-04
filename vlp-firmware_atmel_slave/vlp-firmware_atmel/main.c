/* main.c
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 * 
 */

/* Library Includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>  

/* File includes */
#include "led.h"
#include "ledInterface.h"
#include "nrf24l01.h"
#include "spi.h"
#include "uart.h"
#include "config.h"
#include "communication.h"
#include "eeprom.h"

/* Debug the bits the ISR is putting out. 
 * Using the uart delays the ISR, so use a low frequency when testing */
#define DEBUG_ISR false

/* COMPB values are taken from timer module */
extern uint16_t * compB[2];

/* LED object */
static led_t led; 

/* Em OOK+Manchester, só a ISR COMPA é utilizada para definir os periodos
 * de cada bit. Em VPPM, esta também altera o valor do OCR1B conforme o duty cycle
 * A ISR COMPB muda o bit dentro do periodo OCR1A, de acordo com a codificação
 * VPPM.
 */
ISR(TIMER1_COMPA_vect) // Timer1 ISR COMPA
{
    uint8_t bit = getBit(&led); /* Get next bit in bitstream */
	
	#if DEBUG_ISR
		uint8_t buf[5]={};
		sprintf(buf, "%d", bit);
		uart_puts(buf);
	#endif
	
	OCR1B = *compB[bit];
	if (bit) {
        set_bit(LED_PORT, LED_CTL); /* Turn on */
	} else {
        clr_bit(LED_PORT, LED_CTL); /* Turn off */
	}
}

/* If using VPPM, the bit inverts at OCR1B */
ISR(TIMER1_COMPB_vect) // Timer1 ISR COMPB
{
    /* Invert state */
	if (!check_bit(LED_PORT, LED_CTL)) {
        set_bit(LED_PORT, LED_CTL); /* Turn on */
	} else {
        clr_bit(LED_PORT, LED_CTL); /* Turn off */
	}
}

int main() {
	
	uint8_t LED_HW_ID;
	
	uint8_t TX_addr[nrf24_ADDR_WIDTH];
	uint8_t RX_addr[nrf24_ADDR_WIDTH];
	
	uart_init();
	uart_puts("\n\x1b[2J\r"); /*Clear screen */
	uart_puts("\r\nInitializing Luminary.");
	
	spi_init();
	uart_puts("\r\nSPI init.");
	
	/* Get Addresses and Luminary ID from EEPROM */
	eeprom_read_page(EEPROM_IDX_RX_ADDR, nrf24_ADDR_WIDTH, RX_addr);
	eeprom_read_page(EEPROM_IDX_TX_ADDR, nrf24_ADDR_WIDTH, TX_addr);
	LED_HW_ID = eeprom_read(EEPROM_LED_ID);
	
	#if EEPROM_VERIFY
	bool valid_eeprom = true;
	for(uint8_t i=0; i<nrf24_ADDR_WIDTH; i++){
		if(RX_addr[i] == 0xFF){ /* Default eeprom byte value */
			uart_puts("\r\nValid luminary address not found in EEPROM.");
			memset(RX_addr, '0x0', nrf24_ADDR_WIDTH);
			valid_eeprom=false;
			break;
		}
		if(TX_addr[i] == 0xFF){
			uart_puts("\r\nValid master address not found in EEPROM.");
			memset(TX_addr, '0x0', nrf24_ADDR_WIDTH);
			valid_eeprom=false;
			break;
		}
		if(LED_HW_ID == 0xFF){
			uart_puts("\r\nValid luminary ID not found in EEPROM.");
			memset(TX_addr, '0x0', nrf24_ADDR_WIDTH);
			valid_eeprom=false;
			break;
		}
	}
	if(valid_eeprom){
		uart_puts("\r\nAddresses successfully loaded from EEPROM.");
		
		uint8_t buf[10]={0};
		uart_puts("\r\nRX: \r\n");
		
		for (uint8_t i=0; i<nrf24_ADDR_WIDTH; i++)
		{
			sprintf(buf, "0x%x, ", RX_command_array[i]);
			uart_puts(buf);
		}
		uart_puts("\r\n");
	}
	#endif
	
	uint8_t buf[10]={0};
		
	sprintf(buf, "0x%x", SPI_DDR);
	uart_puts(buf);

	nrf24_config(TX_addr, RX_addr);
	uart_puts("\r\nRF24 init.");
	
	/* Initialize LED object with default parameters and LED ID */
    initLEDObject(&led, LED_HW_ID); 
	startupLED(&led);
	
	nrf24_print_info();
			
	while (1) {
		/* Checks for incoming messages and updates LED */
		checkRF(&led); 
	}

	return(EXIT_SUCCESS);
}

