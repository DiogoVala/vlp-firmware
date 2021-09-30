/* uart.h
 * 
 * Author: Diogo Vala
 *
 * Description: Handles UART 
 * 
 */

#ifndef UART_H
#define UART_H

#include "config.h"

#define BAUD_PRESCALER (((F_CPU / (UART_BAUD_RATE * 16UL))) - 1)

#define ASYNCHRONOUS (0<<UMSEL00) // USART Mode Selection

#define DISABLED    (0<<UPM00)
#define EVEN_PARITY (2<<UPM00)
#define ODD_PARITY  (3<<UPM00)
#define PARITY_MODE  DISABLED // USART Parity Bit Selection

#define ONE_BIT (0<<USBS0)
#define TWO_BIT (1<<USBS0)
#define STOP_BIT ONE_BIT      // USART Stop Bit Selection

#define FIVE_BIT  (0<<UCSZ00)
#define SIX_BIT   (1<<UCSZ00)
#define SEVEN_BIT (2<<UCSZ00)
#define EIGHT_BIT (3<<UCSZ00)
#define DATA_BIT   EIGHT_BIT  // USART Data Bit Selection

void uart_init();
void uart_putc(uint8_t DataByte);
void uart_puts(const char *s );
uint8_t uart_getc();

#endif
