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
	
	uart_puts("\x1b[2J\r\n");
	
	spi_init();

	uint8_t tx_addr[]={'M', 'A', 'S', 'T', 'R'};
	uint8_t rx_addr[]={'S', 'L', 'A', 'V', 'E'};
	
	nrf24_config(rx_addr,tx_addr);
	
	uint8_t data[32];
	uint8_t data_len=7;
	
	uint8_t uart_buffer[20]={};
	
	uart_puts("\r\n");
	
    while(1)
    {
		while(!nrf24_dataReady());
		nrf24_getData(data, &data_len);
		
		uart_puts("\r\nData ready: ");
		for(uint8_t i=0; i<data_len; i++)
		{
			sprintf(uart_buffer, "%c ", data[i]);
			uart_puts(uart_buffer);
		}
		
    }
}