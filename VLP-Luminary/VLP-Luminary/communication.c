/* communication.c
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 *
 * Description: Handles the communication between
 * modules.
 */

/* Library Includes */
#include <avr/interrupt.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/wdt.h>

/* File includes */
#include "../../Common/led.h"
#include "../../Common/nrf24l01.h"
#include "../../Common/uart.h"

#include "ledInterface.h"
#include "communication.h"
#include "timer.h"

/* Debug mode */
/* Presents useful information in the terminal */
#define DEBUG_COMM 1

uint8_t RX_command_array[NRF24_MAX_PAYLOAD] = {}; /* Received bytes*/
uint8_t bitstream_byte_array[NRF24_MAX_PAYLOAD] = {0}; /* Received bitstream bytes*/
uint8_t bitstream[BITSTREAM_MAX_BITS] = {1,0}; /* Bitstream bits*/
uint8_t byte_count = 0; /* Number of bytes in received bitstream */
uint8_t bitstreamSize = 2; /* Number of bits in received bitstream */
uint8_t data_len=0; /* Length of data received */

/* Checks the RF module for new data and processes it */
void checkRF(led_t* ledp) {
	
	/* Reset watchdog timer */
	wdt_reset();
	
	if(nrf24_dataReady() == NRF24_DATA_AVAILABLE){
	
		cli(); /* Disable interrupts */
	
		memset(RX_command_array, '\0', sizeof(RX_command_array));
		
		/* Get data from RF24 chip */
		nrf24_getData(RX_command_array, &data_len);
	
		/* If the received command is a reset command */
		if(command_IsReset())
		{
			#if DEBUG_COMM 
			uart_puts("\r\nLuminary should reset now.");
			#endif
			
			/* Re-enable watchdog to force a reset */
			wdt_disable();
			wdt_enable(WDTO_120MS);
			while(1);
		}
	
		#if DEBUG_COMM /* Sends all received bytes to the terminal */
			uint8_t buf[10]={0};
			uart_puts("\r\nReceived data:\r\n");
			sprintf(buf, "Size: %d\r\n", data_len);
			uart_puts(buf);
			for (uint8_t i=0; i<data_len; i++)
			{
				sprintf(buf, "0x%x, ", RX_command_array[i]);
				uart_puts(buf);
			}
			uart_puts("\r\n");
		#endif
	
		/* Evaluate data*/
		if (RX_command_array[ID] == ledp->ledID) { /* If command concerns this luminary */
			
			if (RX_command_array[IDENTIFIER] == BITSTREAM_IDENTIFIER) /* Bitstream received */ {
           
				updateBitstream();
            
				#if DEBUG_COMM /* Sends the bitstream to the terminal */
				uart_puts("\r\nBitstream received.\r\n");
				for(uint8_t i=0; i<bitstreamSize; i++)
				{
					sprintf(buf, "%d", bitstream[i]);
					uart_puts(buf);
				}
				#endif
			}
			else /* Command received */ 
			{
				if(command_IsValid()){
					updateLED(ledp); /* Update LED with new parameters */
				
					#if DEBUG_COMM
					uart_puts("\r\nCommand received.");
					#endif
				}
			}
		}
		sei(); /* Enable interrupts */
	}
}

/* Updates bitstream array with new data from RF */
void updateBitstream() {
    bitstreamSize = RX_command_array[BIT_COUNT]; /* Number of bitstream bits received */
    byte_count = bitstreamSize / 8; /* Number of bytes to check */
	
    if (bitstreamSize % 8 != 0) byte_count += 1; /* Ceil of byte_count */

    for (uint8_t i = 0; i < byte_count; i++) {
		/* Store received array into bitstream byte array */
        bitstream_byte_array[i] = RX_command_array[BITSTREAM + i]; 
    }
	
    byteArrayToBits(bitstream_byte_array, bitstreamSize);
}

/* Updates the LED with the new params and does HW changes required */
void updateLED(led_t* ledp) {
    /* Update the LED parameters from received command */
    setLedState(ledp, RX_command_array[STATE]);
    setLedMode(ledp, RX_command_array[MODE]);
    setLedIntensity(ledp, RX_command_array[INTENSITY]);
    setLedFrequency(ledp, ((RX_command_array[FREQUENCY_HB] << 8) | RX_command_array[FREQUENCY_LB]));
    setLedDutyCycle(ledp, RX_command_array[DUTYCYCLE]);
    updateLEDHW(ledp); /* Changes state pin and digpot position */
	setupTimer(ledp); /* Changes how the timer behaves - depends on mode */
	
	/* Shows the new LED settings */
	#if DEBUG_COMM
		uint8_t buf[100]={};
		sprintf(buf, "\r\nState: %d\r\nMode: %s\r\nIntensity: %d\r\nFrequency: %d\r\nDuty: %d", \
		getLedState(ledp), getLedModeVerbose(ledp), getLedIntensity(ledp), getLedFrequency(ledp), getLedDutyCycle(ledp));
		uart_puts(buf);
	#endif
}

/* Gets the next bit in the bitstream, according to the mode of operation*/
uint8_t getBit(led_t *ledp) {
	/* Para implementar PAM, pode-se adaptar esta fun??o para percorrer a 
	 * bitstream em termos de simbolos, fazer um switch ao simbolo e 
	 * alterar o valor do Pot. Pode retornar sempre 1 nesse caso. */
	
    static uint8_t array_pos = 0;

	/* Cycle through array */
    if (array_pos >= (bitstreamSize-1)){
        array_pos = 0;
	}
    else{
        array_pos++;
	}

    if (ledp->ledMode != LED_MODE_VPPM)
        return bitstream[array_pos];
    else
        return !bitstream[array_pos]; /* In VPPM we want the bit to start at the opposite value and invert at OCR1B */
}

/* Transforms an array of bytes into an array of bits */
void byteArrayToBits(uint8_t byte_array[], uint8_t bitstreamSize) {
    uint8_t bit_count=1;
    
    for (uint8_t i = 0; i < bitstreamSize; i++) {
	    bitstream[i] = ( byte_array[i/8] & (1<<(8-bit_count)) ) >> (8-bit_count) ;
	    bit_count++;
	    if(bit_count>8)
			bit_count=1;
    }
}

/* True if the received command is a reset command */
bool command_IsReset() {
	if(data_len==1 && RX_command_array[0]==RESET_COMMAND)
		return true;
	return false;
}

/* True if the received command has valid parameters */
bool command_IsValid(){
	bool validCommand=true;

	if(RX_command_array[STATE] > LED_MAX_FREQUENCY){
		validCommand=false;
	}
	if(RX_command_array[INTENSITY]> LED_MAX_INTENSITY){
		validCommand=false;
	}
	if( ((RX_command_array[FREQUENCY_HB] << 8) | RX_command_array[FREQUENCY_LB]) > LED_MAX_FREQUENCY ){
		validCommand=false;
	} 
	if(RX_command_array[DUTYCYCLE]> LED_MAX_DUTYCYCLE){
		validCommand=false;
	}
	return validCommand;
}