
#define	MAX_PLD_SIZE (NRF24_MAX_PAYLOAD - 1)
#define START (NRF24_MAX_PAYLOAD - MAX_PLD_SIZE)
#define FIFO_MASK 255

static void flasher_setup(void);
static void flasher_rx_handle(void);
static void flasher_tx_handle(void);

/*
 * Passthrough between UART and an nRF24.
 *
 * Licensed under AGPLv3.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <avr/wdt.h>
#include <math.h>
#include <util/delay.h>
#include <stdlib.h>

//#include "timer1.h"
#include "../../Common/uart.h"
#include "../../Common/spi.h"
#include "../../Common/nrf24l01.h"

static struct ring_buffer_s {
	uint8_t data[FIFO_MASK + 1];
	uint8_t start, len;
} tx_fifo;

static void handle_input(char ch) {
	tx_fifo.data[(tx_fifo.start + tx_fifo.len ++) & FIFO_MASK] = ch;
}

void main() {
	uint8_t RX_addr[NRF24_ADDR_WIDTH]={'M', 'T', 'R'};
	uint8_t TX_addr[NRF24_ADDR_WIDTH]={'L', 'M', '1'};
	uint8_t status;
	
	uart_init();
	//timer_init();
	spi_init();
	status=nrf24_config(TX_addr, RX_addr);
	if(status!=NRF24_CHIP_NOMINAL)
	{
		uart_puts("\r\nChip Disconnected");
		return EXIT_FAILURE;
	}
	
	flasher_tx_handle();
	sei();

	//serial_set_handler(handle_input);
	#if 0
	while(1){
		static uint8_t tx_cnt; /* Consecutive TX packets counter */
		
		uint8_t pkt_len; /* Length of packet received from RF24 */
		uint8_t pkt_buf[NRF24_MAX_PAYLOAD] /* Stores packets from RF24 */
		
		
		if(nrf24_dataReady() == NRF24_DATA_AVAILABLE){
			
			static uint8_t seqn = 0xff;

			flasher_rx_handle();

			nrf24_getData(pkt_buf, &pkt_len); /* Get reply from RF24 */

			if (pkt_buf[0] != seqn) {
				seqn = pkt_buf[0];
				for (uint8_t i = 1; i < pkt_len; i ++)
					serial_write1(pkt_buf[i]);
			}
			tx_cnt = 0;
		}

		if (tx_fifo.len) { 
			uint8_t pkt_len, pkt_buf[NRF24_MAX_PAYLOAD], split;

			static uint8_t seqn = 0x00;
			uint8_t count = 128;
	

			pkt_buf[0] = seqn ++;

			flasher_tx_handle();
			
			tx_cnt ++;

			cli();
			pkt_len = min(tx_fifo.len, MAX_PLD_SIZE);
			sei();

			/* HACK */
			if (tx_cnt == 2 && MAX_PLD_SIZE > 2 && pkt_len == MAX_PLD_SIZE)
				pkt_len = MAX_PLD_SIZE - 2;
			else if (MAX_PLD_SIZE > 2 && tx_cnt == 3)
				pkt_len = 1;

			split = min(pkt_len,
					(uint16_t) (~tx_fifo.start & FIFO_MASK) + 1);

	
			memcpy(pkt_buf + START, tx_fifo.data +
					(tx_fifo.start & FIFO_MASK), split);
			memcpy(pkt_buf + START + split, tx_fifo.data, pkt_len - split);


			cli();
			tx_fifo.len -= pkt_len;
			tx_fifo.start += pkt_len;
			sei();

			while (-- count) {
				/* Don't flood the remote end with the comms */
				_delay_ms(4);

				nrf24_tx(pkt_buf, pkt_len + START);
				if (!nrf24_tx_result_wait())
					break;
			}
		}
	}
	#endif
}


static uint32_t prev_txrx_ts = 0;

static void flasher_rx_handle(void) {
	//prev_txrx_ts = timer_read();
}

static void flasher_tx_handle(void) {
	static uint8_t first_tx = 1;

	/*
	 * If more than a second has passed since previous communication
	 * the bootloader will have left the flash mode by now so this is
	 * probably a new boot and a new flashing attempt.
	 */
	//if ((uint32_t) (timer_read() - prev_txrx_ts) > F_CPU)
	//	first_tx = 1;

	/*
	 * Before any actual STK500v2 communication begins we need to
	 * attempt to reset the board to start the bootloader, and send it
	 * our radio address to return the ACK packets to.
	 */
	if (first_tx) {
		/*
		 * Our protocol requires any program running on the board
		 * to reset it if it receives a 0xff byte.
		 */
		uart_puts("\r\nSending Reset");
		nrf24_sendData(reset_cmd, 5);
		nrf24_wait_tx_result();

		/* Give the board time to reboot and enter the bootloader */
		_delay_ms(100);

		first_tx = 0;
	}

	//prev_txrx_ts = timer_read();
}