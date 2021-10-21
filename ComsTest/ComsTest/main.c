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
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

/* File includes */
#include "../../Common/uart.h"
#include "../../Common/spi.h"
#include "../../Common/nrf24l01.h"
#include "stk500.h"

#define PKT_DATA_START 1 /* Start of data in the transfer buffer */
#define MAX_PLD_SIZE 31
#define RADIO_ON 1
#define RADIO_OFF 0
static uint8_t radio_mode = RADIO_OFF;

uint8_t TX_addr[NRF24_ADDR_WIDTH]={'M', 'T', 'R'};
uint8_t RX_addr[NRF24_ADDR_WIDTH]={'L', 'M', '1'};

#define my_delay(msec) delay8((int) (F_CPU / 8000L * (msec)))

static void delay8(uint16_t count);
void putch(char);
uint8_t getch(void);

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

	uint8_t chars[100]={};
	uint8_t buf[10]={};
	uint8_t data_len;
		
	uint8_t i=0;
	while(1){
		#if 0
		if (nrf24_dataReady() == NRF24_DATA_AVAILABLE) {
			uart_puts("\r\nData ready\r\n");
			nrf24_getData(chars, &data_len);
			for (uint8_t i=0; i<data_len; i++)
			{
				sprintf(buf, "0x%x ", chars[i]);
				uart_puts(buf);
			}
		}
		#endif
		
	
		
		chars[i]=getch();
		putch(chars[i]);
		i++;
		my_delay(100);

	}
}

void putch(char ch) {

	uint8_t pkt_len = 0;
	uint8_t pkt_buf[NRF24_MAX_PAYLOAD]={};
	uint32_t tx_retries = 500;

	if (radio_mode == RADIO_OFF || radio_mode == RADIO_ON) {
		while (1) {
			while (( UCSR0A & _BV(UDRE0)) == 0);
			UDR0 = ch;
			break;
		}
	}
	else { /* Radio ON */
		pkt_buf[0]++;
		pkt_buf[PKT_DATA_START+pkt_len++] = ch; /* Fills the local buffer */

		if ((ch == STK_OK) || (pkt_len == MAX_PLD_SIZE)) { /* When last byte or buffer full */
			while (tx_retries--) { /* Send buffer until received */

				nrf24_sendData(pkt_buf, pkt_len);
				if (nrf24_wait_tx_result() == NRF24_MESSAGE_SENT)
				break; /* Payload sent and acknowledged*/

				/* Wait 4ms to allow the remote end to switch to Rx mode */
				my_delay(4);
			}

			/* Reset the local buffer */
			pkt_len = 0;
		}
	}
}

uint8_t getch(void) {

	uint8_t ch = '\0';
	static uint8_t pkt_id = UINT8_MAX;  /* Number (ID) of the packet */
	static uint8_t pkt_len = 0;  /* Number of bytes in the local buffer */
	static uint8_t pkt_ptr = 1;  /* Start of data in the buffer */
	static uint8_t pkt_buf[32];  /* Local buffer to store bytes */

	//watchdogReset();

	while (1) {
		if (( UCSR0A & (1 << RXC0)) != 0) /* If we have data in the UART */
		{
			ch = UDR0;
			return ch;
		}
		
		PORTB &= ~ (1<<PORTB1);

		/* If there is data in the local buffer or new data in RF24 fifo */
		if (pkt_len || (nrf24_dataReady() == NRF24_DATA_AVAILABLE)) {

			radio_mode = RADIO_ON; /* From now on, we're in radio mode */

			/* If our local buffer is empty, get more data */
			while(pkt_len == 0) {
				nrf24_getData(pkt_buf, &pkt_len);

				PORTB |= (1<<PORTB1);

				if(pkt_len>1){
					if (pkt_buf[0] == pkt_id) { /* We have already received this packet */
						pkt_len = 0; /* Ignore it */
					}
					else if (pkt_buf[0] != pkt_id){
						pkt_id = pkt_buf[0]; /* It's a new packet, update the current ID */
						pkt_ptr=1;
						break;
					}
				}
				else{
					pkt_len=0;
				}
			}

			/* If there is data in the local buffer */
			if (pkt_ptr < pkt_len ) {
				ch = pkt_buf[pkt_ptr]; /* Grab next byte in the buffer */
				pkt_ptr++;

				if (pkt_ptr == pkt_len) { /* We have read all the bytes in the buffer */
					/* Reset the buffer */
					pkt_ptr = 1;
					pkt_len = 0;
				}
				return ch;
			}
		}
	}
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