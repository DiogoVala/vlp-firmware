/* led.c
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 * 
 * Description: Handles the interactivity with the LED
 * object.
 * 
 */

#include <stdint.h>
#include "led.h"

void initLEDObject(led_t *ledp, uint8_t LED_HW_ID) {

    setLedID(ledp, LED_HW_ID);
    setLedState(ledp, LED_ON);
    setLedMode(ledp, LED_MODE_DC);
    setLedIntensity(ledp, LED_DFLT_INTENSITY);
    setLedFrequency(ledp, LED_DFLT_FREQUENCY);
    setLedDutyCycle(ledp, LED_DFLT_DUTYCYCLE);
}

uint8_t getLedID(led_t *ledp) {
    return ledp->ledID;
}

void setLedID(led_t *ledp, uint8_t ledID) {
    ledp->ledID = ledID;
}

uint8_t getLedState(led_t *ledp) {
    return ledp->ledState;
}

void setLedState(led_t *ledp, uint8_t ledState) {
    ledp->ledState = ledState;
}

uint8_t getLedMode(led_t *ledp) {
    return ledp->ledMode;
}

const char * getLedModeVerbose(led_t *ledp)
{
	switch(ledp->ledMode){
		case LED_MODE_DC:
			return "DC";
			break;
		case LED_MODE_ARBITRARY:
			return "ARBITRARY";
			break;
		case LED_MODE_VPPM:
			return "VPPM";
			break;
		default:
			return "INVALID";
			break;
	}
	return "INVALID";
}

void setLedMode(led_t *ledp, led_mode_t ledMode) {
    ledp->ledMode = ledMode;
}

uint8_t getLedIntensity(led_t *ledp) {
    return ledp->ledIntensity;
}

void setLedIntensity(led_t *ledp, uint8_t ledIntensity) {
    ledp->ledIntensity = ledIntensity;
}

uint16_t getLedFrequency(led_t *ledp) {
    return ledp->ledFrequency;
}

void setLedFrequency(led_t *ledp, uint16_t ledFrequency) {
    ledp->ledFrequency = ledFrequency;
}

uint8_t getLedDutyCycle(led_t *ledp) {
    return ledp->ledDutyCycle;
}

void setLedDutyCycle(led_t *ledp, uint8_t ledDutyCycle) {
    ledp->ledDutyCycle = ledDutyCycle;
}

