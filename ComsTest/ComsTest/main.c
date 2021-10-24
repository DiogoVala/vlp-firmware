/*
 * main.c
 *
 * Created: 10/20/2021 3:41:13 PM
 *  Author: diogo
 */ 

/* Library includes */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <avr/wdt.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

/* File includes */
#include "../../Common/uart.h"
#include "../../Common/spi.h"
#include "../../Common/nrf24l01.h"
#include "../../Common/utils.h"
#include "stk500.h"

//#define SLAVE

#ifdef SLAVE
uint8_t TX_addr[NRF24_ADDR_WIDTH]={'M', 'T', 'R'};
uint8_t RX_addr[NRF24_ADDR_WIDTH]={'L', 'M', '1'};
#else
uint8_t RX_addr[NRF24_ADDR_WIDTH]={'M', 'T', 'R'};
uint8_t TX_addr[NRF24_ADDR_WIDTH]={'L', 'M', '1'};
#endif


static void delay8(uint16_t count);
void send();
void receive();

static uint8_t data[32]={0,'a', 'b', 'c'};
static uint8_t data_len;

static uint8_t i=0;
static uint8_t buf[10]={};

static uint8_t result=NRF24_MESSAGE_LOST;


int main() {
	uint8_t status;
	
	uart_init();
	
	spi_init();
	
	status=nrf24_config(TX_addr, RX_addr);
	if(status!=NRF24_CHIP_NOMINAL)
	{
		//uart_puts("\r\nChip Disconnected");
		return EXIT_FAILURE;
	}
	
	sei(); /* Enable interrupts */
	
	uart_puts("\r\nOnline.");
	
	while(1){
		#ifdef SLAVE
		receive();
		my_delay(10);
		send();
		#else
		send();
		receive();
		#endif
	}
}

uint32_t j=0;

void send(){
	//nrf24_config(TX_addr, RX_addr);
	uint16_t timeout = 100;
	data[0]=i++;
	result=NRF24_MESSAGE_LOST;
	uart_puts("\r\nSending");
	while(result==NRF24_MESSAGE_LOST && timeout--){
		my_delay(4);
		nrf24_sendData(data, 4);
		result=nrf24_wait_tx_result();
	}
	if (result != NRF24_MESSAGE_SENT)
	{
		uart_puts("...Timed out.");
	}
	else
	{
		uart_puts("...Sent.");
	}
}

void receive(){
	uart_puts("\r\nReceiving.");
	while(nrf24_rxFifoEmpty() == NRF24_DATA_UNAVAILABLE){
		//uart_puts("\r\nNothing");
	}
	#ifdef SLAVE
	sprintf(buf, "\r\n%d ", (int)j++);
	uart_puts(buf);
	uart_puts("Slave Received: ");
	
	#else
	sprintf(buf, "\r\n%d ", (int)j++);
	uart_puts(buf);
	uart_puts("Master Received: ");
	#endif
	nrf24_getData(data, &data_len);
	for (uint8_t i=0; i<data_len; i++)
	{
		sprintf(buf, "0x%x ", data[i]);
		uart_puts(buf);
	}
}
