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

/* File includes */
#include "../../Common/uart.h"
#include "../../Common/spi.h"
#include "../../Common/nrf24l01.h"

#define	MAX_PLD_SIZE (NRF24_MAX_PAYLOAD - 1) /* Number of data bytes in the payload */
#define PKT_ID_IDX 0 /* First byte of the payload is reserved for the packet identifier */
#define PKT_ID_SIZE 1 /* Size of the packet identifier (in bytes) */
#define FIFO_SIZE 256 /* Number of bytes in the data buffer that stores received bytes from UART */
#define TX_TIMEOUT 1000000 /* Some generic timeout value if there is no transmission for a while */

static uint8_t reset_cmd[]={0xFF};

/* Function prototypes */
static void uart_to_rf(void);
static void rf_to_uart(void);
uint8_t min(uint8_t x, uint8_t y);

/* FIFO to receive data from UART ISR */
static struct fifo_buffer_s {
	uint8_t data[FIFO_SIZE];
	uint8_t len; /* Number of bytes currently stored */
} tx_fifo;

/* Function to fill the FIFO */
void handle_input(uint8_t ch) {
	tx_fifo.data[tx_fifo.len++] = ch;
}

int main() {
	uint8_t RX_addr[NRF24_ADDR_WIDTH]={'M', 'T', 'R'};
	uint8_t TX_addr[NRF24_ADDR_WIDTH]={'L', 'M', '1'};
	uint8_t status;
	
	uart_init();
	uart_set_RX_handler(handle_input);
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
		rf_to_uart();
		uart_to_rf();
	}
}

static void rf_to_uart(void){

	static uint8_t pkt_id = UINT8_MAX; /* Number of the packet we are currently receiving */
	uint8_t rx_pkt_len; /* Length of packet received from RF24 */
	uint8_t rx_pkt_buf[NRF24_MAX_PAYLOAD]; /* Stores packets from RF24 */

	if(nrf24_dataReady() == NRF24_DATA_AVAILABLE){

		nrf24_getData(rx_pkt_buf, &rx_pkt_len); /* Get data from RF24 */

		/* First byte identifies this packet. If it's a new one, we process it */
		if (rx_pkt_buf[0] != pkt_id) {

			pkt_id = rx_pkt_buf[0];
			for (uint8_t i = PKT_ID_SIZE; i < rx_pkt_len; i++)
			{
				uart_putc(rx_pkt_buf[i]); /* Redirect to UART */
			}
		}
	}
}

static void uart_to_rf(void) {

	static bool first_tx = true;
	static uint8_t pkt_id = 0; /* Number of the packet we are currently sending */
	uint8_t tx_pkt_len;
	uint8_t tx_pkt_buf[NRF24_MAX_PAYLOAD];
	static uint32_t first_tx_timeout = TX_TIMEOUT;

	/* If there is no transmission for a while, start over */
	if (!first_tx_timeout--)
	{
		first_tx=true;
	}

	/* First transmission sends a reset command to the Luminary program 
	 * and waits for it to enter the bootloader before sending data */
	if (first_tx) {
		/*
		 * Our protocol requires any program running on the board
		 * to reset if it receives a single 0xff byte.
		 */

		nrf24_sendData(reset_cmd, 1);
		nrf24_wait_tx_result();

		/* Give the board time to reboot and enter the bootloader */
		_delay_ms(100);

		first_tx = false;
	}
	else if (tx_fifo.len){ /* If there is data in the FIFO */

		/* First byte of the buffer has the packet identifier  */
		tx_pkt_buf[PKT_ID_IDX] = pkt_id ++;

		/* UART may be placing byte into the buffer, so disabling interrupt is needed */
		cli();
		tx_pkt_len = min(tx_fifo.len, MAX_PLD_SIZE); /* Number of bytes to send*/
		
		memcpy(tx_pkt_buf+PKT_ID_SIZE, tx_fifo.data, tx_pkt_len); /* Copy bytes from FIFO into the payload array */
		
		memmove(tx_fifo.data, tx_fifo.data+PKT_ID_SIZE, tx_pkt_len); /* Move FIFO data to the start position */
		
		tx_fifo.len -= tx_pkt_len;
		sei();

		while (true) { /* Send until received */

			nrf24_sendData(tx_pkt_buf, tx_pkt_len);
			if (nrf24_wait_tx_result() == NRF24_MESSAGE_SENT)
				break;

			_delay_ms(4); /* Give the receiver some time to process data */
		}

		/* Reset timeout */
		first_tx_timeout = TX_TIMEOUT;
	}
}

uint8_t min(uint8_t x, uint8_t y){
	return (x < y) ? x : y;
}