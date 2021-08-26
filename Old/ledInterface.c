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
#include "led.h"
#include "ledInterface.h"
#include "utils.h"
#include "timer.h"
#include "digPot.h"

const uint32_t DELAY_COUNT = F_CPU / 100; /* 10ms delay */

/* Prototypes of private functions */
void setLEDIOpins();
void setLEDPowerSwitchPin(uint8_t ledPower);
void setLEDStatePin(uint8_t ledState);
void setHWLEDIntensity(uint8_t ledIntensity);

/* LED setup with default parameters */
void startupLED(led_t* ledp) {
    setLEDIOpins();
    setLEDPowerSwitchPin(LED_ON);
    setLEDStatePin(ledp->ledState);

    /* Incremental increase in intensity to avoid burning people's eyes */
    for (uint8_t intensity = 0; intensity < ledp->ledIntensity; intensity++) {
        for (uint8_t i = 0; i < DELAY_COUNT; i++); /* Apply delay */

        setHWLEDIntensity(intensity);
    }
}

/* Hardware update of LED */
void updateLEDHW(led_t* ledp) {
    setLEDStatePin(ledp->ledState);
    setHWLEDIntensity(ledp->ledIntensity);
    setupTimer(ledp); /* Changes how the timer behaves, depends on mode*/
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
    LED_DDR |= _BV(LED_POWERSWITCH) | _BV(LED_CTL);
}

