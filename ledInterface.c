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
#include "digPot.h"

#define COMMAND_LENGTH 7 // Number of command parameters + 1 
const uint32_t DELAY_COUNT = F_CPU / 100; /* 10ms delay */

volatile uint8_t TX_command_array[COMMAND_LENGTH] = {};
extern volatile uint8_t RX_command_array[COMMAND_LENGTH];

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

void updateLED(led_t* ledp) {
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
    /* Define a posição do Wiper do Pot de acordo com a intensidade que se quer */
    digitalPotWrite(ledIntensity);
}

void buildLEDCommand(led_t* ledp) {
    /* Array of bytes to send to RF module */
    TX_command_array[ID] = getLedID(ledp);
    TX_command_array[STATE] = getLedState(ledp);
    TX_command_array[MODE] = getLedMode(ledp);
    TX_command_array[INTENSITY] = getLedIntensity(ledp);
    TX_command_array[FREQUENCY_LB] = (uint8_t) (getLedFrequency(ledp) & 0x00FF);
    TX_command_array[FREQUENCY_HB] = (uint8_t) ((getLedFrequency(ledp) & 0xFF00) >> 8);
    TX_command_array[DUTYCYCLE] = getLedDutyCycle(ledp);
}

void updateLEDParams(led_t* ledp) {
    /* Update the LED parameters from received command */
    setLedState(ledp, RX_command_array[ID]);
    setLedMode(ledp, RX_command_array[STATE]);
    setLedIntensity(ledp, RX_command_array[MODE]);
    setLedFrequency(ledp, ((RX_command_array[FREQUENCY_HB] << 8) | RX_command_array[FREQUENCY_LB]));
    setLedDutyCycle(ledp, RX_command_array[DUTYCYCLE]);
}

void setLEDIOpins() {
    LED_DDR |= _BV(LED_POWERSWITCH) | _BV(LED_CTL);
}

