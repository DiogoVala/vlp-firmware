/* ledInterface.c
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 * 
 * Description: Performs the hardware operations that
 * control the LED.
 * 
 */

#ifndef LEDINTERFACE_H
#define	LEDINTERFACE_H

/* Pin configuration */
#define LED_DDR DDRD
#define LED_PORT PORTD
#define LED_CTL DDD6
#define LED_POWERSWITCH DDD7

enum command_params{
    ID,
    STATE,
    MODE,
    INTENSITY,
    FREQUENCY_LB,
    FREQUENCY_HB,
    DUTYCYCLE
};

void startupLED(led_t* ledp);

void updateLED(led_t* ledp);

void setLEDIOpins();

void setLEDPowerSwitchPin(uint8_t ledPower);

void setLEDStatePin(uint8_t ledState);

void setHWLEDIntensity(uint8_t ledIntensity);

void buildLEDCommand(led_t* ledp);

void updateLEDParams(led_t* ledp);

#endif	/* LEDINTERFACE_H */

