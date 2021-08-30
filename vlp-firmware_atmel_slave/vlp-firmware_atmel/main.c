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

#define INPUT_BUFFER_SIZE 8

extern uint8_t bitstream[BITSTREAM_MAX_BITS];
extern uint16_t * compB[2];

volatile led_t led; /* LED object */

/* Em OOK+Manchester, só a ISR COMPA é utilizada para definir os periodos
 * de cada bit. Em VPPM, esta também altera o valor do OCR1B conforme o duty cycle
 * A ISR COMPB muda o bit dentro do periodo OCR1A, de acordo com a codificação
 * VPPM.
 */
ISR(TIMER1_COMPA_vect) // Timer1 ISR COMPA
{
    uint8_t bit = getBit(&led); /* Get next bit in bitstream */
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
	
    /* Initialize common modules */
	uart_init();
	uart_puts("\n\x1b[2J\r"); //Clear screen
	uart_puts("\r\nInitializing SLAVE.");
	
	spi_master_init();
	nrf24_init();
	nrf24_config(NRF24_CHANNEL,NRF24_PAYLENGTH);
	
    initLEDObject(&led); /* Initialize LED object with default parameters */
	startupLED(&led);
			
	while (1) {
		checkRF(&led); /* Checks for incoming messages and updates LED */
	}

	return(EXIT_SUCCESS);
}

