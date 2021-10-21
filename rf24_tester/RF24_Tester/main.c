/*
 * main.c
 *
 * Created: 10/21/2021 1:51:20 PM
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
#include "../../Common/nrf24l01.h"
#include "../../Common/spi.h"
#include "../../Common/uart.h"
#include "../../Common/utils.h"

#define SLAVE

static uint8_t data[32];
static uint8_t data_len;
static uint8_t test[]={1,2,3,4,5,6,7,8,9};

int main(void)
{
	#ifdef SLAVE
	uint8_t TX_addr[NRF24_ADDR_WIDTH]={'M', 'T', 'R'};
	uint8_t RX_addr[NRF24_ADDR_WIDTH]={'L', 'M', '1'};
	#endif
	#ifdef MASTER
	uint8_t TX_addr[NRF24_ADDR_WIDTH]={'L', 'M', '1'};
	uint8_t RX_addr[NRF24_ADDR_WIDTH]={'M', 'T', 'R'};
	#endif
	uart_init();
	spi_init();
	
	uint8_t status = nrf24_config(TX_addr, RX_addr);
	if( status != NRF24_CHIP_NOMINAL){
		uart_puts("\r\nRF24 Chip not detected.");
		return(EXIT_FAILURE);
	}
	bool flag=true;
	static uint8_t it=0;
	while(1)
	{
		#ifdef SLAVE
		_delay_ms(20);
		if(nrf24_rxFifoEmpty() == NRF24_DATA_AVAILABLE){
			nrf24_getData(data, &data_len);
			
			uint8_t buf[10]={0};
			uart_puts("\r\nI:\r\n");
			sprintf(buf, "%d, \r\n", it++);
			uart_puts(buf);
			for (uint8_t i=0; i<data_len; i++)
			{
				sprintf(buf, "0x%x, ", data[i]);
				uart_puts(buf);
			}
			uart_puts("\r\n");
		}
		#endif
	
		#ifdef MASTER
		
		if(uart_getc()=='a' && flag == true){
			nrf24_sendData(test, 8);
			nrf24_wait_tx_result();
			flag = false;
		}
		#endif
		
		
		
	}
}