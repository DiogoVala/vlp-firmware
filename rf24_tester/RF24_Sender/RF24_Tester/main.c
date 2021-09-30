/*
 * main.c
 *
 * Created: 9/29/2021 3:23:35 PM
 *  Author: diogo
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
#include "nrf24l01.h"
#include "spi.h"
#include "uart.h"
#include "config.h"

int main(void)
{
	wdt_disable();
	uart_init();
	spi_init();
	nrf24_init();
	
	uint8_t command[]={'T', 'E', 'S', 'T', 'I', 'N', 'G'};
	
	uint8_t tx_result;
	
	uint32_t try=0;

	uint8_t buf[20]={};
		
	uart_puts("\r\n");
	
    while(1)
    {
		sprintf(buf, "\rTry %d ... ", (int)try++);
		//uart_puts(buf);
		nrf24_send(command, sizeof(command));
		nrf24_wait_tx_result();
		
    }
}