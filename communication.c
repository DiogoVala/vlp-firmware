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
#include "communication.h"

uint8_t TX_command_array[COMMAND_LENGTH] = {};
uint8_t RX_command_array[COMMAND_LENGTH] = {};
uint8_t bitstream_byte_array[BITSTREAM_MAX_BYTES] = {0};
uint8_t bitstream[BITSTREAM_MAX_SIZE] = {0};

void sendBitStream(uint8_t bitstream[], uint8_t bitstreamSize, uint8_t ledID) {

}

void sendCommand(uint8_t TX_command_array, uint8_t command_length) {

}

void bitsToByteArray(uint8_t bitstream[], uint8_t bitstreamSize) {

    uint8_t index = 0;
    uint8_t newByte = 0;

    for (uint8_t index = 0; index < bitstreamSize; index++) {
        newByte = newByte << 1 | bitstream[index];
        if (index % 7 == 0) {
            bitstream_byte_array[index / 8] = newByte;
        }
    }
    if (index % 7 != 0) {
        /* Shift remaining bits to the left */
        bitstream_byte_array[index / 8] = newByte << (8 - (index % 8));
    }
}

void byteArrayToBits(uint8_t byte_array[], uint8_t bitstreamSize) {
    
    for (uint8_t index = 0; index < bitstreamSize; index++) {
        
    }
    

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