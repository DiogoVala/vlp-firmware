/* main.c
*
* Author: Diogo Vala
*
* Description:
*
*/

/* Library includes */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <avr/wdt.h>
#include <math.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

/* File includes */
#include "../../Common/uart.h"
#include "../../Common/spi.h"
#include "../../Common/nrf24l01.h"

#define	MAX_PLD_SIZE (NRF24_MAX_PAYLOAD - 1) /* Number of data bytes in the payload */
#define PKT_ID_IDX 0 /* First byte of the payload is reserved for the packet identifier */
#define PKT_DATA_START 1 /* Size of the packet identifier (in bytes) */
#define CIRC_BUFFER_SIZE 254 /* Number of bytes in the data buffer that stores received bytes from UART */
#define TX_TIMEOUT 50000 /* Some generic timeout value if there is no transmission for a while */

/* STK500 Messages */
#define CRC_EOP 0x20  /* Every avrdude message ends with this byte */

static bool new_tx=false;

static uint8_t reset_cmd[]={0xFF};
uint8_t RX_addr[NRF24_ADDR_WIDTH]={'M', 'T', 'R'};
uint8_t TX_addr[NRF24_ADDR_WIDTH]={'L', 'M', '1'};

/* Function prototypes */
static void uart_to_rf(void);
static void rf_to_uart(void);
uint8_t min(uint8_t x, uint8_t y);

/* FIFO to receive data from UART ISR */
static struct ring_buffer_s {
	uint8_t data[CIRC_BUFFER_SIZE];
	uint8_t len; /* Number of bytes currently stored */
	uint8_t r_idx;
	uint8_t w_idx;
} ring_buf;

static bool circ_buf_empty() {
	return(ring_buf.len == 0);
}

static bool circ_buf_full() {
	return(ring_buf.len == CIRC_BUFFER_SIZE);
}

/* function to fill the circular buffer */
static void circ_buf_input(volatile uint8_t ch) {
	if(circ_buf_full()){
		//uart_puts("\r\nBuffer full");
		return;
	}
	
		
	ring_buf.data[ring_buf.w_idx] = ch;
	ring_buf.w_idx++;
	if(ring_buf.w_idx == CIRC_BUFFER_SIZE)
	ring_buf.w_idx = 0;
	ring_buf.len++;
	
	if(ch == CRC_EOP){
		new_tx=true;
	}
}

/* function to read from the circular buffer */
static uint8_t circ_buf_output() {
	uint8_t retval;
	retval=ring_buf.data[ring_buf.r_idx];
	ring_buf.r_idx++;
	if(ring_buf.r_idx == CIRC_BUFFER_SIZE)
	ring_buf.r_idx = 0;
	ring_buf.len--;
	return retval;
}

int main() {
	uint8_t status;
	
	uart_init();
	uart_set_RX_handler(circ_buf_input);
	uart_RX_IE(true);
	
	spi_init();
	
	status=nrf24_config(TX_addr, RX_addr);
	if(status!=NRF24_CHIP_NOMINAL)
	{
		uart_puts("\r\nChip Disconnected");
		return EXIT_FAILURE;
	}
		
	sei(); /* Enable interrupts */

	while(1){
		uart_to_rf();
		rf_to_uart();
	}
}

static void rf_to_uart(void){
	
	static uint8_t pkt_id = UINT8_MAX; /* Number of the packet we are currently receiving */
	uint8_t pkt_len; /* Length of packet received from RF24 */
	uint8_t pkt_buf[NRF24_MAX_PAYLOAD]; /* Stores packets from RF24 */

	if(nrf24_dataReady() == NRF24_DATA_AVAILABLE){
		
		nrf24_getData(pkt_buf, &pkt_len); /* Get data from RF24 */
		
		if(pkt_len){ /* Sanity check */
			/* First byte identifies this packet. If it's a new one, we process it */
			if (pkt_buf[0] != pkt_id) {

				pkt_id = pkt_buf[0];
				for (uint8_t i = PKT_DATA_START; i < pkt_len; i++)
				{
					uart_putc(pkt_buf[i]); /* Redirect to UART */
				}
			}
		}
	}
}

static void uart_to_rf(void) {

	static bool first_tx = true;
	static uint32_t first_tx_timeout = TX_TIMEOUT;
	static bool first_tx_complete = false;
	uint32_t tx_retries = 500;
	
	static uint8_t pkt_id = 0; /* Number of the packet we are currently sending */
	uint8_t pkt_len=0;
	uint8_t pkt_buf[NRF24_MAX_PAYLOAD];

	/* If there is no transmission for a while, start over */
	if (!first_tx_timeout--)
	{
		first_tx_complete=false;
		first_tx_timeout=TX_TIMEOUT;
		first_tx=true;
	}
	
	/* First transmission sends a reset command to the Luminary program
	* and waits for it to enter the bootloader before sending data */
	if (first_tx) {
		/*
		* Our protocol requires any program running on the board
		* to reset if it receives a single 0xff byte.
		*/
		while(!first_tx_complete){
			nrf24_sendData(reset_cmd, 1);
			if(nrf24_wait_tx_result() == NRF24_MESSAGE_SENT)
			first_tx_complete=true;
		}

		/* Give the board time to reboot and enter the bootloader */
		_delay_ms(100);
		
		first_tx = false;
	}
	
	else if(new_tx){ /* A new avrdude packet has been assembled in the buffer */

		pkt_len = min(ring_buf.len, MAX_PLD_SIZE); /* Number of bytes to send [1-31] */
		
		while(pkt_len != 0){ /* Send the buffer contents until all is sent */
			
			/* First byte of the buffer has the packet identifier  */
			pkt_buf[PKT_ID_IDX] = pkt_id ++; /* May overflow, but that's ok */

			cli(); /* Mutual exclusion with UART interrupt */
			for(uint8_t i=0; i<pkt_len; i++){
				pkt_buf[PKT_DATA_START+i]=circ_buf_output();
			}
			sei(); /* End of exclusion */
			
			#if 0
			uint8_t buf[10];
			sprintf(buf, "%d : ", pkt_buf[0]);
			uart_puts(buf);
			for (uint8_t i=1; i<=pkt_len; i++)
			{
				if(pkt_buf[i] == '\r')
				uart_puts("\r\n");
				else{
					sprintf(buf, "%c", pkt_buf[i]);
					uart_puts(buf);
				}
			}
			uart_puts("\r\n");
			#endif
			
			while (tx_retries--) { /* Send until received or timeout */

				nrf24_sendData(pkt_buf, pkt_len);
				if (nrf24_wait_tx_result() == NRF24_MESSAGE_SENT)
					break;
				
				_delay_ms(4); /* Give the receiver some time to process data */
			}

			/* Accessing ring_buf.len should be atomic */
			/* Check the remaining number of bytes we still need to send */
			pkt_len = min(ring_buf.len, MAX_PLD_SIZE); /* Number of bytes to send [1-31] */
		}
		new_tx=false; /* avrdude packet was sent */
	}
	first_tx_timeout = TX_TIMEOUT; /* Reset timeout */
}

uint8_t min(uint8_t x, uint8_t y){
	return (x < y) ? x : y;
}