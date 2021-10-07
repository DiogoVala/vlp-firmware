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
#include <avr/wdt.h>

/* File includes */
#include "../../Common/led.h"
#include "../../Common/nrf24l01.h"
#include "../../Common/spi.h"
#include "../../Common/uart.h"
#include "../../Common/utils.h"
#include "../../Common/eeprom.h"
#include "communication.h"
#include "ledInterface.h"

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
	
	wdt_disable();
	
	uint8_t LED_HW_ID;
	uint8_t TX_addr[NRF24_ADDR_WIDTH]={'M', 'T', 'R'};
	uint8_t RX_addr[NRF24_ADDR_WIDTH]={'L', 'M', '1'};
		
	uint8_t status;
	
	uart_init();
	
	//uart_puts("\n\x1b[2J\r"); /*Clear screen */
	//uart_puts("\r\nInitializing Luminary.");
	
	spi_init();
	
	/* Get Addresses and Luminary ID from EEPROM */
	//eeprom_read_page(EEPROM_IDX_RX_ADDR, NRF24_ADDR_WIDTH, RX_addr);
	//eeprom_read_page(EEPROM_IDX_TX_ADDR, NRF24_ADDR_WIDTH, TX_addr);
	//LED_HW_ID = eeprom_read(EEPROM_LED_ID);
	LED_HW_ID=1;
	
	/* Initialize LED object with default params */
	initLEDObject(&led, LED_HW_ID);
	startupLED(&led);
	
	status = nrf24_config(TX_addr, RX_addr);
	if( status != NRF24_CHIP_NOMINAL){
		uart_puts("\r\nRF24 Chip not detected.");
		return(EXIT_FAILURE);
	}
	
	uart_puts("\r\nInitialization Complete.");
	
	while (1) {
		/* Checks for incoming messages and updates LED */
		checkRF(&led); 
	}

	return(EXIT_FAILURE);
}


/* Calling the reset function on hardware reset prevents potential 
 * infinite reset loops when watchdog is configured with a short 
 * period. */
void reset(void) __attribute__((naked)) __attribute__((section(".init3")));
/* Clear SREG_I on hardware reset. */
void reset(void)
{
	cli();
	// Note that for newer devices (any AVR that has the option to also
	// generate WDT interrupts), the watchdog timer remains active even
	// after a system reset (except a power-on condition), using the fastest
	// prescaler value (approximately 15 ms). It is therefore required
	// to turn off the watchdog early during program startup.
	MCUSR = 0; // clear reset flags
	wdt_disable();
}