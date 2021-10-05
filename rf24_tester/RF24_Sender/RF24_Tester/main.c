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

volatile uint8_t tx_addr[]={0x20, 0x10, 0x05};
volatile uint8_t rx_addr[]={0x05, 0x10, 0x20};
	
volatile uint8_t command[32]={};

int main(void)
{
	uint8_t status;
	uint8_t tx_result;
	uint8_t try=1;
	uint8_t flag=0;
	uint8_t uart_buffer[20];
	
	wdt_disable();
	uart_init();
	spi_init();
	
	memset(command, 'a', 32);
		
	status=nrf24_config(tx_addr,rx_addr);
	if(status!=0){
		uart_puts("\r\nNRF24 Not connected!");
		return -1;
	}

	uart_puts("\r\nReady to transmit");
	uint8_t dir=0;
		while(1)
		{
			flag=0;
			sprintf(uart_buffer, "\r\n%d", try);
			uart_puts(uart_buffer);
			
			for(uint8_t i=0; i<100; i++)
				_delay_ms(10);
			
			nrf24_sendData(command, try);
			
			tx_result=nrf24_wait_tx_result();
			
			if(tx_result != NRF24_MESSAGE_SENT)
				flag=1;
			
			if (dir==0)
				try++;
			else
				try--;
			if (try==32)
				dir=1;
			else if(try==0)
				dir=0;
				
			if(flag==1)
				uart_puts("\r\nfailed");
			else
				uart_puts("\r\nno fails");
		}

}