/* timer.c
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 *
 * Description: Handles timer behavior according to
 * mode of operation of the luminary
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "timer.h"
#include "led.h"
#include "config.h"

/* COMPB register values for VPPM transitions */
volatile uint16_t compB0_1;
volatile uint16_t compB1_0;
volatile uint16_t * compB[2] = {&compB1_0, &compB0_1};

void stopTimer();

void setupTimer(led_t *ledp) {
    cli(); /* Disable global interrupts */

    TCCR1A = 0x00; /* COM1A1 | COM1A0 | COM1B1 | COM1B0 | - | - | WGM 11 | WGM10 */
    TCCR1B = 0x00; /* ICNC1 | ICES1 | - | WGM13 | WGM12 | CS12 | CS 11 | CS 10   */
    TCNT1 = 0; /* Timer1 Counter */

    uint16_t compA; /* Timer compare  */

    switch (ledp->ledMode) {

        case LED_MODE_DC:
			TIMSK1 = (0 << OCIE1A) | (0 << OCIE1B); /* Enable interrupts on output compare for OCR1A and OCR1B */
            stopTimer();
            break;

        case LED_MODE_ARBITRARY:
            compA = ((F_CPU / getLedFrequency(ledp)) - 1); /* Define o periodo de um bit */
            OCR1A = compA; /* Timer1A compare register */
            TIMSK1 = (1 << OCIE1A) | (0 << OCIE1B); /* Enable timer1A Interrupts*/
			sei(); /* Enable global interrupts */
            break;

        case LED_MODE_VPPM:
            compA = (F_CPU / getLedFrequency(ledp)) - 1; /* Define o periodo de um bit */
            OCR1A = compA; /* Timer1A compare register */

            compB1_0 = (uint16_t) compA * (getLedDutyCycle(ledp) / LED_MAX_DUTYCYCLE);
            compB0_1 = (uint16_t) compA * ((LED_MAX_DUTYCYCLE - getLedDutyCycle(ledp)) / LED_MAX_DUTYCYCLE);
            OCR1B = compB0_1; /* First interrupt will be useless, so any value is ok */

            TIMSK1 = (1 << OCIE1A) | (1 << OCIE1B); /* Enable interrupts on output compare for OCR1A and OCR1B */
			sei(); /* Enable global interrupts */
            break;

        default:
			TIMSK1 = (0 << OCIE1A) | (0 << OCIE1B); /* Enable interrupts on output compare for OCR1A and OCR1B */
			stopTimer();
            break;
    }
    TCCR1B = (1 << WGM12) | (1 << CS10); /* "Clear Timer on Compare Match with OCR1A" and "Clock with no prescaling" */
}

void stopTimer() {
    cli();
    TCCR1A = 0x00; /* Clear control registers */
    TCCR1B = 0x00;
    TCNT1 = 0; /* Clear timer counter */
    sei();
}

