/* digPot.c
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 * 
 */
/* Library Includes */
#include <stdint.h>
#include <util/delay.h>

#include "digPot.h"
#include "spi.h"
#include "config.h"

void digitalPotWrite(uint8_t LedIntensity) {
    float V_WB = LedIntensity * 0.01 * POT_MAX_CURRENT*V_A;

    float R_WB = ((V_WB - V_B) / (V_A - V_B)) * R_AB; /*Resistance WB for desired Voltage*/
    float D = ((R_WB) / R_AB)*(MAX_D + 1); /* Equivalent D for desired voltage V_WB*/

    if (D < 0) D = 0;
    else if (D > MAX_D) D = MAX_D;

    clr_bit(POT_PORT, POT_CS);
	_delay_us(2);
    spi_exchange((uint8_t) D);
	_delay_us(2);
    set_bit(POT_PORT, POT_CS);
}

