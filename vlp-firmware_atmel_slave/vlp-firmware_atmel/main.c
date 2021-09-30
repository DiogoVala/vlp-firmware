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
	
	uint8_t TX_addr[nrf24_ADDR_WIDTH];
	uint8_t RX_addr[nrf24_ADDR_WIDTH];
	
	uart_init();
	uart_puts("\n\x1b[2J\r"); /*Clear screen */
	uart_puts("\r\nInitializing SLAVE.");
	
	spi_init();
	uart_puts("\r\nSPI init.");
	
	for(uint8_t i=0; i<nrf24_ADDR_WIDTH; i++){
		RX_addr[i]=eeprom_read(EEPROM_IDX_RX_ADDR+i);
		TX_addr[i]=eeprom_read(EEPROM_IDX_TX_ADDR+i);
	}
	
	nrf24_config(RX_addr, TX_addr);
	uart_puts("\r\nRF24 init.");
	
	/* Initialize LED object with default parameters */
    initLEDObject(&led); 
	startupLED(&led);
			
	while (1) {
		/* Checks for incoming messages and updates LED */
		checkRF(&led); 
	}

	return(EXIT_SUCCESS);
}

