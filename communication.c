/* communication.c
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 *
 * Description: Handles the communication between
 * modules.
 */

#include "led.h"
#include "ledInterface.h"

#define COMMAND_LENGTH 7 // Number of command parameters + 1 

volatile uint8_t TX_command_array[COMMAND_LENGTH] = {};
volatile uint8_t RX_command_array[COMMAND_LENGTH] = {};

void sendBitStream(uint8_t bitstream[], uint8_t bitstreamSize, uint8_t ledID) {
    
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