/* ledInterface.c
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 * 
 * Description: Handles the hardware changes and some
 * of the logic needed to control the LED. 
 * 
 */

/* Library Includes */
#include <avr/io.h>

/* File includes */
#include "../../Common/led.h"
#include "../../Common/utils.h"
#include "ledInterface.h"
#include "digPot.h"
#include <util/delay.h>

/* Prototypes of private functions */
void setLEDIOpins();
void setLEDPowerSwitchPin(uint8_t ledPower);
void setLEDStatePin(uint8_t ledState);
void setHWLEDIntensity(uint8_t ledIntensity);

/* LED setup with default parameters */
void startupLED(led_t* ledp) {
	
    setLEDIOpins();
	setHWLEDIntensity(0); /* Set Intensity to zero before switching on LED to avoid full brightness */
    setLEDPowerSwitchPin(LED_ON);
    setLEDStatePin(ledp->ledState);

    /* Soft start  */
    for (uint8_t intensity = 0; intensity < ledp->ledIntensity; intensity++) {
        _delay_ms(10);
        setHWLEDIntensity(intensity);
    }
}

/* Hardware update of LED */
void updateLEDHW(led_t* ledp) {
    setLEDStatePin(ledp->ledState);
    setHWLEDIntensity(ledp->ledIntensity);
}

void setLEDPowerSwitchPin(uint8_t ledPower) {
    if (ledPower) {
        set_bit(LED_PORT, LED_POWERSWITCH);
    } else {
        clr_bit(LED_PORT, LED_POWERSWITCH);
    }
}

void setLEDStatePin(uint8_t ledState) {
    if (ledState) {
        set_bit(LED_PORT, LED_CTL);
    } else {
        clr_bit(LED_PORT, LED_CTL);
    }
}

void setHWLEDIntensity(uint8_t ledIntensity) {
    digitalPotWrite(ledIntensity);
}

void setLEDIOpins() {
    LED_DDR = LED_DDR | _BV(LED_POWERSWITCH) | _BV(LED_CTL);
}

