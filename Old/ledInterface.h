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

/* LED setup with default parameters*/
void startupLED(led_t* ledp);

/* Hardware update of LED */
void updateLEDHW(led_t* ledp);

#endif	/* LEDINTERFACE_H */

