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

volatile uint8_t rx_addr[]={0x20, 0x10, 0x05};
volatile uint8_t tx_addr[]={0x05, 0x10, 0x20};

volatile uint8_t data[32];
volatile uint8_t data_len=0;

int main(void)
{
	uint8_t status;
	uint8_t uart_buffer[20]={};
	
	wdt_disable();
	uart_init();
	
	uart_puts("\x1b[2J\r\n");
	
	spi_init();
	
	status=nrf24_config(tx_addr,rx_addr);
	if(status!=0){
		uart_puts("\r\nNRF24 Not connected!");
		return -1;
	}
		
	uart_puts("\r\nReady to receive.");
	
    while(1)
    {
		while(nrf24_dataReady()==NRF24_DATA_UNAVAILABLE);
		nrf24_getData(data, &data_len);
		
		uart_puts("\r\nData ready: ");
		for(uint8_t i=0; i<data_len; i++)
		{
			sprintf(uart_buffer, "%c ", data[i]);
			uart_puts(uart_buffer);
		}
		sprintf(uart_buffer, " - %d",data_len);
		uart_puts(uart_buffer);
		
    }
}