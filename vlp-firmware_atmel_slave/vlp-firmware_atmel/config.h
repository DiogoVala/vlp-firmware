/* utils.h
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 *
 * Description: Useful macro functions and configs
 * 
 */

#ifndef __CONFIG_H__
#define	__CONFIG_H__

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define UART_BAUD_RATE	9600 

#define NRF24_CHANNEL 2 /* RF Frequency = 2400 + NRF24_CHANNEL (MHZ) */
#define NRF24_PAYLENGTH 7 /* Bytes sent/received by RF module */


/* Macro functions d*/
#define set_bit(REGISTER, BIT)    REGISTER |=  _BV(BIT)
#define clr_bit(REGISTER, BIT)    REGISTER &= ~(_BV(BIT))
#define check_bit(REGISTER, BIT) (REGISTER & (_BV(BIT))) >> BIT
#define xor_bit(REGISTER, BIT)    REGISTER ^= _BV(BIT)

#endif	/* __CONFIG_H__ */

