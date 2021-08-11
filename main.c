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

/* File includes */
#include "led.h"
#include "ledInterface.h"
//#include "RF24.h"
#include "spi.h"
#include "uart.h"
#include "utils.h"
#include "communication.h"

#define INPUT_BUFFER_SIZE 8

#define BITSTREAM false
#define COMMAND true

#ifndef MODE
#define MODE MASTER /* Luminaries = SLAVE ; Controller = MASTER */
#endif

extern uint8_t bitstream[BITSTREAM_MAX_SIZE];

#if (MODE == SLAVE)

/* Em OOK+Manchester, só a ISR COMPA é utilizada para definir os periodos
 * de cada bit. Em VPPM, esta também altera o valor do OCR1B conforme o bit
 * seja 0 ou 1.
 * A ISR COMPB muda o bit dentro do periodo OCR1A, de acordo com a codificação
 * VPPM.
 */
ISR(TIMER1_COMPA_vect) // Timer1 ISR COMPA
{
#if 0
    uint8_t bit = Ler bit da bitstream
    OCR1B = compB[bit];
    if(bit) { 
        set_bit(LED_PORT, LED_CTL); /* Turn on */
    }
    else {
        clr_bit(LED_PORT, LED_CTL); /* Turn off */
    }
#endif
}

/* Controla o disparo de VPPM, conforme o duty cycle*/
ISR(TIMER1_COMPB_vect) // Timer1 ISR COMPB
{
#if 0
    uint8_t bit = Ler bit da bitstream;
    if(bit) { 
        set_bit(LED_PORT, LED_CTL); /* Turn on */
    }
    else {
        clr_bit(LED_PORT, LED_CTL); /* Turn off */
    }
#endif
}
#endif

int main() {

#if (MODE == MASTER)

    led_t led; /* LED object to send to luminaries */

    uart_init(UART_BAUD_SELECT(UART_BAUD_RATE, F_CPU));

    uint8_t msgBuffer[INPUT_BUFFER_SIZE]; /* Command buffer */
    uint8_t msgbufferIdx = 0; /* Current buffer index */
    uint8_t command_pos = 0; /* What command is being read */

    uint8_t bitstreamIdx = 0; 
    uint8_t bitstreamSize = 0; /* Dynamic size of bitstream */

    uint16_t uart_retval; /* Uart returns 16 bit (error code + data) */
    uint16_t uart_error_code;
    uint8_t uart_char;
    
    bool input_type = false; /* false = normal command ; true = bitstream */
    
    sei();

    while (1) {
        uart_retval = uart_getc();
        uart_error_code = (uint16_t) (uart_retval & 0xFF00);
        uart_char = (uint8_t) (uart_retval & 0x00FF);

        if (!(uart_error_code & UART_NO_DATA)) { /* If there is data */

            if (uart_error_code & UART_FRAME_ERROR) {
                /* Framing Error detected, i.e no stop bit detected */
                uart_puts_P("UART Frame Error: ");
            }
            if (uart_error_code & UART_OVERRUN_ERROR) {
                /* 
                 * Overrun, a character already present in the UART UDR register was 
                 * not read by the interrupt handler before the next character arrived,
                 * one or more received characters have been dropped
                 */
                uart_puts_P("UART Overrun Error: ");
            }
            if (uart_error_code & UART_BUFFER_OVERFLOW) {
                /* 
                 * We are not reading the receive msgBuffer fast enough,
                 * one or more received character have been dropped 
                 */
                uart_puts_P("Buffer overflow error: ");
            }
            uart_putc(uart_char); /* Send character back to terminal */

            switch (uart_char) {

                case 'b': /* Start of bitstream input */
                case 'B':
                    memset(bitstream, '\0', BITSTREAM_MAX_SIZE);
                    bitstreamIdx = 0;
                    input_type=BITSTREAM;
                    break;
                    
                case '$': /* Start of command message */
                    msgbufferIdx = 0;
                    command_pos = 0;
                    memset(msgBuffer, 0, INPUT_BUFFER_SIZE);
                    input_type=COMMAND;
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
                    break;
                case '*': /* End of message */
                    if(input_type==BITSTREAM)
                    {
                        bitstreamSize=bitstreamIdx;
                        sendBitStream(bitstream, bitstreamSize, led.ledID);
                    }
                    else
                    {
                        buildLEDCommand(&led); /* Generate array of bytes to send to RF module*/
                        uart_puts("Message sent!\r\n");
                    }
                    break;
                    
                default: /* Add character to buffer */
                    if(input_type==BITSTREAM)
                    {
                        bitstream[bitstreamIdx] = uart_char - '0'; /* char to int */
                        bitstreamIdx++;
                    }
                    else
                    {
                        msgBuffer[msgbufferIdx] = uart_char;
                        msgbufferIdx++;
                    }
                    break;
            }
        }
    }
#elif (MODE == SLAVE)
    led_t led; /* Luminary LED object*/
    initLEDObject(&led); /* Initialize LED object with default parameters */

    // Led setup


    while (1) {
        //checkRF();
    }
#else
    while (1);
#endif



    return (EXIT_SUCCESS);
}

