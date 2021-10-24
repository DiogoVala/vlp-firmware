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

#define TX_TIMEOUT 50000 /* Some generic timeout value if there is no transmission for a while */
#define	MAX_PLD_SIZE (NRF24_MAX_PAYLOAD - 1) /* Number of data bytes in the payload */
#define PKT_ID_IDX 0 /* First byte of the packet is reserved for the packet identifier */
#define PKT_DATA_START 1 /* Index of actual data in the transmission packet  */
#define TX_RETRIES 10000
#define UART_BUFFER_SIZE 254 /* Number of bytes in the data buffer that stores received bytes from UART */

#define my_delay(msec) delay8((int) (F_CPU / 8000L * (msec)))

/* STK500 Messages */
#define CRC_EOP 0x20  /* Every avrdude message ends with this byte */

static bool first_tx = true;

/* When CRC_EOP is received, new_tx turns true, which means a message is ready to be sent */
static bool new_tx = false;

/* UART Buffer */
static struct circular_fifo_s{
	uint8_t data[UART_BUFFER_SIZE];
	uint8_t data_len;
	uint8_t head;
	uint8_t tail;
} uart_buffer;

/* This is the command that the luminary firmware is expecting before performing a reset */
static uint8_t reset_cmd[]={0xFF};

/* RX address is this module's address and TX address is the target luminary's address */
uint8_t RX_addr[NRF24_ADDR_WIDTH]={'M', 'T', 'R'};
uint8_t TX_addr[NRF24_ADDR_WIDTH]={'L', 'M', '1'};

/* Function prototypes */
static void uart_to_rf(void);
static void rf_to_uart(void);
static void send_first_tx();
uint8_t min(uint8_t x, uint8_t y);
static void delay8(uint16_t count);

void circular_fifo_input(uint8_t input){
	uart_buffer.data[uart_buffer.head]=input;
	uart_buffer.head++;
	if (uart_buffer.head==UART_BUFFER_SIZE)
	{
		uart_buffer.head=0;
	}
	uart_buffer.data_len++;
}
uint8_t circular_fifo_output(){
	uint8_t output;
	output=uart_buffer.data[uart_buffer.tail];
	uart_buffer.tail++;
	if (uart_buffer.tail==UART_BUFFER_SIZE)
	{
		uart_buffer.tail=0;
	}
	uart_buffer.data_len--;
	return output;
}

void uart_handler(uint8_t byte){
	circular_fifo_input(byte);
	if(byte==CRC_EOP)
		new_tx=true;
}

int main() {
	uint8_t status;
	
	uart_init();
	uart_set_RX_handler(uart_handler);
	uart_RX_IE(true);
	
	spi_init();
	
	status=nrf24_config(TX_addr, RX_addr);
	if(status!=NRF24_CHIP_NOMINAL)
	{
		//uart_puts("\r\nChip Disconnected");
		return EXIT_FAILURE;
	}
	
	sei(); /* Enable interrupts */
	
	while(1){
		uart_to_rf();
		my_delay(20);
		rf_to_uart();
	}
}

static void rf_to_uart(void){
	
	static uint8_t pkt_id = UINT8_MAX; /* Number of the packet we are currently receiving */
	static uint8_t pkt_len=0; /* Length of packet received from RF24 */
	static uint8_t pkt_buf[NRF24_MAX_PAYLOAD]; /* Stores packets from RF24 */

	if(nrf24_rxFifoEmpty() == NRF24_DATA_AVAILABLE){
		
		nrf24_getData(pkt_buf, &pkt_len); /* Get data from RF24 */
		
		if(pkt_len){ /* Sanity check: getData should never return 0 as pkt_len */
			
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

	uint32_t tx_retries = TX_RETRIES;
	
	static uint8_t pkt_id = 0; /* Number of the packet we are currently sending */
	uint8_t pkt_len=0; /* Length of the packet we are sending (does not include the ID byte) */
	uint8_t pkt_buf[NRF24_MAX_PAYLOAD]; /* Buffer where we store the packet before sending */
	
	uint8_t buf_len;
	cli();
	buf_len = uart_buffer.data_len; /* Number of bytes to send [1-31] */
	sei();
	
	if( new_tx || (buf_len >= MAX_PLD_SIZE)){ /* A new AVRDUDE packet has been assembled in the buffer and is ready to be sent */
		new_tx=false;
		/* Sends reset command */
		send_first_tx();
		
		cli();
		pkt_len = min(uart_buffer.data_len, MAX_PLD_SIZE); /* Number of bytes to send [1-31] */
		sei();

		/* First byte of the buffer has the packet identifier  */
		pkt_buf[PKT_ID_IDX] = pkt_id++; /* May overflow, but that's ok */
		
		/* Note: The way the AVRDUDE messages are saved in the UART buffer and
		* then sent should ensure there is always mutual exclusion with the
		* UART ISR but it is safer to disable interrupts while accessing the buffer */
		cli(); /* Mutual exclusion with UART interrupt */
		
		for(uint8_t i=PKT_DATA_START; i<pkt_len+1; i++){
			pkt_buf[i]=circular_fifo_output();
			//uart_putc(pkt_buf[i]);
		}
		
		sei(); /* End of exclusion */
		
		tx_retries = TX_RETRIES;
		while (tx_retries--) { /* Send until received or retry amount reached */
			my_delay(5); /* Give the receiver some time to process data */
			
			nrf24_sendData(pkt_buf, pkt_len+1); /* ID + buffer */
			if (nrf24_wait_tx_result() == NRF24_MESSAGE_SENT)
			break;
		}
		
	}
}

static void send_first_tx(){
	
	static bool first_tx_complete = false;
	
	/* First transmission sends a reset command to the Luminary program
	* and waits for it to enter the Bootloader before sending data */
	if (first_tx) {
		/*
		* Our protocol requires any program running on the board
		* to reset if it receives a single 0xff byte.
		*/
		while(!first_tx_complete){
			nrf24_sendData(reset_cmd, 1);
			if(nrf24_wait_tx_result() == NRF24_MESSAGE_SENT)
			first_tx_complete=true;
			
			/* Give the board time to reboot and enter the Bootloader */
			my_delay(200);
		}
		first_tx = false;
	}
}

uint8_t min(uint8_t x, uint8_t y){
	return (x < y) ? x : y;
}


static void delay8(uint16_t count) {
	while (count --)
	__asm__ __volatile__ (
	"\tnop\n"
	"\tnop\n"
	"\tnop\n"
	"\tnop\n"
	"\tnop\n"
	"\tnop\n"
	"\tnop\n"
	"\twdr\n"
	);
}