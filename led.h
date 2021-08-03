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

#ifndef __LED_H_
#define __LED_H_

#define LED_HW_ID 13 /* Physical ID of this LED*/

/* General defs */
#define LED_ON 1
#define LED_OFF 0

/* Default values */
#define LED_DFLT_INTENSITY 100 /* Intensity in percentage (100% = 530mA ) */
#define LED_DFLT_FREQUENCY 10000 /* Frequency in Hz */
#define LED_DFLT_DUTYCYCLE 50 /* VPPM dutyCycle in percentage (t_on/T) */

/* Max values */
#define LED_MAX_INTENSITY 100
#define LED_MAX_DUTYCYCLE 100
#define LED_MAX_FREQUENCY 37000 /* 37KHz */

/* Modes of operation */
typedef enum{
    LED_MODE_DC,
    LED_MODE_ARBITRARY,
    LED_MODE_VPPM,
} led_mode_t;

/* Led object */
typedef struct {
    uint8_t ledID; /* Hardware ID of LED*/
    uint8_t ledState; /* On/Off state */
    led_mode_t ledMode; /* Mode of operation  */
    uint8_t ledIntensity; /* Intensity in percentage (100% = 530mA ) */
    uint16_t ledFrequency; /* Frequency in Hz */
    uint8_t ledDutyCycle; /* VPPM dutyCycle in percentage (t_on/T) */
} led_t;


void initLEDObject(led_t *ledp);

uint8_t getLedID(led_t *ledp);

void setLedID(led_t *ledp, uint8_t ledID);

uint8_t getLedState(led_t *ledp);

void setLedState(led_t *ledp, uint8_t ledState);

uint8_t getLedMode(led_t *ledp);

void setLedMode(led_t *ledp, led_mode_t ledMode);

uint8_t getLedIntensity(led_t *ledp);

void setLedIntensity(led_t *ledp, uint8_t ledIntensity);

uint16_t getLedFrequency(led_t *ledp);

void setLedFrequency(led_t *ledp, uint16_t ledFrequency);

uint8_t getLedDutyCycle(led_t *ledp);

void setLedDutyCycle(led_t *ledp, uint8_t ledDutyCycle);

#endif