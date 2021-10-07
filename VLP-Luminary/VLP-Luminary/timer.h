/* timer.h
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 *
 * Description: Handles timer behavior according to
 * mode of operation of the luminary
 */

#ifndef TIMER_H
#define	TIMER_H

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* File includes */
#include "../../Common/led.h"

/* Configures the timer and interrupts according to the desired LED mode of operation */
void setupTimer(led_t *ledp);

/* Stops timer and disables timer interrupts */
void stopTimer();

#endif	/* TIMER_H */

