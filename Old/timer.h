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

#include "led.h"

void setupTimer(led_t *ledp);

void stopTimer();

#endif	/* TIMER_H */

