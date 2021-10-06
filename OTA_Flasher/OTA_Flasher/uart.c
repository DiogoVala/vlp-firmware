/* uart.c
 * 
 * Author: Diogo Vala
 *
 * Description: Handles UART 
 * 
 */

#include <avr/io.h>      // Contains all the I/O Register Macros

#include "uart.h"

void uart_init()
{
	// Set Baud Rate
	UBRR0H = BAUD_PRESCALER >> 8;
	UBRR0L = BAUD_PRESCALER;
	
	// Set Frame Format
	UCSR0C = ASYNCHRONOUS | PARITY_MODE | STOP_BIT | DATA_BIT;
	
	// Enable Receiver and Transmitter
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
}

void uart_putc(uint8_t DataByte)
{
	while (( UCSR0A & (1<<UDRE0)) == 0) {}; // Do nothing until UDR is ready
	UDR0 = DataByte;
}

void uart_puts(const char *s )
{
	while (*s)
	uart_putc(*s++);

	}/* uart_puts */

uint8_t uart_getc()
{
	uint8_t DataByte;
	while (( UCSR0A & (1<<RXC0)) == 0) {}; // Do nothing until data have been received
	DataByte = UDR0 ;
	return DataByte;
}
