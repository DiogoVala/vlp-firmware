/* uart.c
 * 
 * Author: Diogo Vala
 *
 * Description: Handles UART 
 * 
 */

#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
/* File includes */
#include "uart.h"

void uart_init()
{
	/* Set Baud Rate */
	UBRR0H = (uint8_t)(BAUD_PRESCALER >> 8);
	UBRR0L = (uint8_t) BAUD_PRESCALER;
	
	/* Set Frame Format */
	UCSR0C = ASYNCHRONOUS | PARITY_MODE | STOP_BIT | DATA_BIT;
	
	/* Enable Receiver and Transmitter */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);
}


void uart_RX_IE(bool RX_IE)
{
	if(RX_IE){
		UCSR0B |= _BV(RXCIE0); /* Enable RX Interrupt */
	}
}

void uart_putc(uint8_t DataByte)
{
	uint16_t timeout = 10000;
	
	/* Do nothing until TX Data Register is empty */
	while (( UCSR0A & _BV(UDRE0)) == 0 && timeout--); 
	UDR0 = DataByte;
}

void uart_puts(const char *s )
{
	while (*s){
		uart_putc(*s++);
	}
}

uint8_t uart_getc()
{
	uint8_t DataByte;
	while (( UCSR0A & (1<<RXC0)) == 0) {}; // Do nothing until data have been received
	DataByte = UDR0 ;
	return DataByte;
}

/* Callback function pointer */
static void (*uart_rx_handler)(uint8_t ch) = NULL;

/* Set RX callback function */
void uart_set_RX_handler(void (*handler)(uint8_t ch)) {
	uart_rx_handler = handler;
}

/* UART RX ISR*/
void ISR(USART_RX_vect) {
	uint8_t status = UCSR0A;
	uint8_t ch;
	uint8_t dummy;

	if (uart_rx_handler != NULL ) /*Check if callback function exists */
	{
		/* If RX complete, no frame error and no parity error*/
		if((status & RXC0) && !(status & FE0) && !(status & UPE0))
		{
			ch = UDR0; /* Read data */
			uart_rx_handler(ch); /* Place the read character in the desired data structure */
		} 
		else
		{
			dummy = UDR0; /* Dummy read to clear register */
		}
	}
}