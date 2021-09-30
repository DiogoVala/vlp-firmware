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
	
	uint8_t command[]={'T', 'E', 'S', 'T', 'I', 'N', 'G'};
		
	uint8_t tx_addr[]={'M', 'A', 'S', 'T', 'R'};
	uint8_t rx_addr[]={'S', 'L', 'A', 'V', 'E'};
		
	nrf24_config(tx_addr,rx_addr);
	
	uint8_t tx_result;
	
	uint32_t try=0;

	uart_puts("\r\n");
	
	uint8_t flag=0;
	
    while(try++ < 100)
    {
		_delay_ms(1000);
		nrf24_sendData(command, sizeof(command));
		
		tx_result=nrf24_wait_tx_result();
		if(tx_result != NRF24_MESSAGE_SENT)
			flag=1;
    }
	if(flag==1)
		uart_puts("\r\nfailed");
	else
		uart_puts("\r\nno fails");
	
}