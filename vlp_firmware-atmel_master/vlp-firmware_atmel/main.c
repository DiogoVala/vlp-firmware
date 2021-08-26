/* main.c
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 * 
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

/* File includes */
#include "led.h"
#include "nrf24l01.h"
#include "spi.h"
#include "uart.h"
#include "config.h"
#include "communication.h"

#define INPUT_BUFFER_SIZE 8

#define BITSTREAM false
#define COMMAND true

extern uint8_t bitstream[BITSTREAM_MAX_BITS];

led_t led; /* LED object */

int main() {

    /* Initialize common modules */
    uart_init();
    uart_puts("\n\x1b[2J\r"); //Clear screen
    uart_puts("\r\nInitializing MASTER.");

    spi_master_init();
    nrf24_init();
    nrf24_config(NRF24_CHANNEL, NRF24_PAYLENGTH);
	

    uint8_t msgBuffer[INPUT_BUFFER_SIZE]; /* Command buffer */
    uint8_t msgbufferIdx = 0; /* Current buffer index */
    uint8_t command_pos = 0; /* What command is being read */

    uint8_t bitstreamIdx = 0;
    uint8_t bitstreamSize = 0; /* Dynamic size of bitstream */

    uint8_t uart_char;

    bool input_type = false; /* false = normal command ; true = bitstream */
    uart_puts("\r\n\nWaiting command input.");
	uart_puts("\r\n($ID,State,Mode,Intensity,Freq,Duty,*)\r\n\n");

    sei();

    while (1) {
        uart_char = uart_getc();
        uart_putc(uart_char);
        switch (uart_char) {

            case 'b': /* Start of bitstream input */
            case 'B':
                memset(bitstream, '\0', BITSTREAM_MAX_BITS);
                bitstreamIdx = 0;
                input_type = BITSTREAM;
                break;

            case '$': /* Start of command message */
                msgbufferIdx = 0;
                command_pos = 0;
                memset(msgBuffer, 0, INPUT_BUFFER_SIZE);
                input_type = COMMAND;
                break;

            case ',': /* Command delimiter*/
                switch (command_pos) {
                        /*NOTE: THERE IS NO VERIFICATION OF INPUTS */
                    case 0:
                        setLedID(&led, (uint8_t) atoi((char*) msgBuffer));
                        break;
                    case 1:
                        setLedState(&led, (uint8_t) atoi((char*) msgBuffer));
                        break;
                    case 2:
                        setLedMode(&led, (uint8_t) atoi((char*) msgBuffer));
                        break;
                    case 3:
                        setLedIntensity(&led, (uint8_t) atoi((char*) msgBuffer));
                        break;
                    case 4:
                        setLedFrequency(&led, (uint16_t) atoi((char*) msgBuffer));
                        break;
                    case 5:
                        setLedDutyCycle(&led, (uint8_t) atoi((char*) msgBuffer));
                        break;
                    default:
                        break;
                }
                memset(msgBuffer, '\0', INPUT_BUFFER_SIZE);
                command_pos++;
                msgbufferIdx = 0;
                break;
            case '*': /* End of message */
                if (input_type == BITSTREAM) {
                    bitstreamSize = bitstreamIdx;
                    sendBitStream(bitstream, bitstreamSize, &led);
                }
                else {
                    sendCommand(&led); /* Generate array of bytes to send to RF module*/
                }
                break;

            case '\r':
                uart_puts("\r\n"); //Clear screen/
                break;

            default: /* Add character to buffer */
                if (input_type == BITSTREAM) {
                    bitstream[bitstreamIdx] = uart_char - '0'; /* char to int */
                    bitstreamIdx++;
                } else {
                    msgBuffer[msgbufferIdx] = uart_char;
                    msgbufferIdx++;
                }
                break;
        }
    }
    return (EXIT_SUCCESS);
}

