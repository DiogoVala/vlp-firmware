/* digPot.h
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 * 
 */

#ifndef __DIGPOT_H_
#define __DIGPOT_H_

/* Pot nomenclature
 *    A
 *    |
 * W--|
 *    |
 *    B
 */

#define MAX_D 255 /* Max register value (in decimal) for the wiper position */
#define V_A 2.5f /* Regulated voltage at A */
#define V_B 0.0f /* Voltage at B */
#define R_AB 5000.0f /* Resistance from A to B */
#define R_WIPER 50.0f /* Wiper resistance */
#define POT_MAX_CURRENT 0.530F

/* Sets the voltage between W and B to V_WB */
void digitalPotWrite(uint8_t LedIntensity);

#endif
