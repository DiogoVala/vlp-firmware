/*
 * main.c
 *
 * Created: 10/21/2021 12:07:19 PM
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

	static uint8_t data[32];
	static uint8_t data_len;

#define PKT_DATA_START 1 /* Index of actual data in the transmission packet  */
static void rf_to_uart(void){
	
	static uint8_t pkt_id = UINT8_MAX; /* Number of the packet we are currently receiving */
	static uint8_t pkt_len=0; /* Length of packet received from RF24 */
	static uint8_t pkt_buf[NRF24_MAX_PAYLOAD]; /* Stores packets from RF24 */

	uint8_t buf[10];

	if(nrf24_rxFifoEmpty() == NRF24_DATA_AVAILABLE){
		
		nrf24_getData(pkt_buf, &pkt_len); /* Get data from RF24 */
		
		if(pkt_len){ /* Sanity check: getData should never return 0 as pkt_len */
			
			/* First byte identifies this packet. If it's a new one, we process it */
			if (pkt_buf[0] != pkt_id) {
				pkt_id = pkt_buf[0];
				
				for (uint8_t i = 0; i < pkt_len; i++)
				{
					//uart_putc(pkt_buf[i]); /* Redirect to UART */
					
					sprintf(buf, "0x%x, ", pkt_buf[i]);
					uart_puts(buf);
				}
				uart_puts("\r\n");
			}
		}
	}
}

uint8_t getch(void) {

	uint8_t ch = '\0';
	static uint8_t pkt_id = UINT8_MAX;  /* Number (ID) of the packet */
	static uint8_t pkt_len = 0;  /* Number of bytes in the local buffer */
	static uint8_t pkt_buf[32];  /* Local buffer to store bytes */

	while (1) {
		/* If there is data in the local buffer or data in RF24 fifo */
		if (pkt_len || (nrf24_rxFifoEmpty() == NRF24_DATA_AVAILABLE)) {

			/* If our local buffer is empty, get more data from RF24 */
			if(pkt_len == 0) {
				while(nrf24_rxFifoEmpty() == NRF24_DATA_UNAVAILABLE);
				nrf24_getData(pkt_buf, &pkt_len);
				
				#if 0
				for (uint8_t i=0; i<pkt_len; i++)
				{
					sprintf(buf, "0x%x, ",pkt_buf[i]);
					uart_puts(buf);
				}
				#endif
				
				if(pkt_len>1){ /* Sanity check: pkt_len should always have a minimum of 2 bytes */
					if (pkt_buf[0] == pkt_id) { /* We have already received this packet */
						pkt_len = 0; /* Ignore it */
					}
					else{
						pkt_id = pkt_buf[0]; /* It's a new packet, update the current ID */
						pkt_len-=1; /* Ignore the first byte, which is the ID */
						uart_puts("\r\n");
					}
				}
				else{
					pkt_len=0;
				}
			}

			#if 1
			/* If there is data in the local buffer */
			if (pkt_len>0) {
				ch = pkt_buf[1]; /* Grab next byte in the buffer */
				for(uint8_t i=1; i<NRF24_MAX_PAYLOAD-1;i++)
				{
					pkt_buf[i]=pkt_buf[i+1];
				}
				pkt_len--;
				return ch;
			}
			#endif
		}
	}
	return ch; /* Should never be reached */
}

int main(void)
{
	uint8_t TX_addr[NRF24_ADDR_WIDTH]={'N', 'N', 'N'};
	uint8_t RX_addr[NRF24_ADDR_WIDTH]={'M', 'T', 'R'};
	//uint8_t RX_addr[NRF24_ADDR_WIDTH]={'L', 'M', '1'};	
	
	uart_init();
	spi_init();
	
	uint8_t status = nrf24_config(TX_addr, RX_addr);
	if( status != NRF24_CHIP_NOMINAL){
		uart_puts("\r\nRF24 Chip not detected.");
		return(EXIT_FAILURE);
	}
	uint8_t buf[10];
	uint8_t ch;
    while(1)
    {
		//ch=getch();
		//sprintf(buf, "0x%x, ", ch);
		//uart_puts(buf);
		rf_to_uart();
		_delay_ms(50);
		
		
		#if 0
		if(nrf24_rxFifoEmpty() == NRF24_DATA_AVAILABLE){
			nrf24_getData(data, &data_len);
			
			uint8_t buf[10]={0};
			uart_puts("\r\nLen:\r\n");
			sprintf(buf, "%d, \r\n", data_len);
			uart_puts(buf);
			for (uint8_t i=0; i<data_len; i++)
			{
				sprintf(buf, "0x%x, ", data[i]);
				uart_puts(buf);
			}
			uart_puts("\r\n");
		}
		#endif
    }
}


