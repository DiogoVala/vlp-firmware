/* digPot.c
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 * 
 */
/* Library Includes */
#include <stdint.h>

#include "digPot.h"
#include "config.h"
#include "spi.h"

void digitalPotWrite(uint8_t LedIntensity) {
    float V_WB = LedIntensity * 0.01 * POT_MAX_CURRENT*V_A;

    float R_WB = ((V_WB - V_B) / (V_A - V_B)) * R_AB; /*Resistance WB for desired Voltage*/
    float D = ((R_WB) / R_AB)*(MAX_D + 1); /* Equivalent D for desired voltage V_WB*/

    if (D < 0) D = 0;
    else if (D > MAX_D) D = MAX_D;

    /* Clear the SS bit to select the slave line */
    clr_bit(SPI_PORT, SPI_SS_DIGPOTSLAVE);

    for (int i = 0; i < 5; i++); //NOTE: WHY THE DELAY?
    //  send in the address and value via SPI:
    spi_exchange((uint8_t) D);
    for (int i = 0; i < 5; i++);
    /* Set the SS bit to deselect the slave line */
    set_bit(SPI_PORT, SPI_SS_DIGPOTSLAVE);
}

