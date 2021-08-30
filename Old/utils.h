/* utils.h
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 *
 * Description: Useful macro functions 
 * 
 */

#ifndef UTILS_H
#define	UTILS_H

#define F_CPU 16000000UL
#define UART_BAUD_RATE      9600 

#define set_bit(REGISTER, BIT)    REGISTER |=  _BV(BIT)
#define clr_bit(REGISTER, BIT)    REGISTER &= ~(_BV(BIT))
#define check_bit(REGISTER, BIT) (REGISTER & (_BV(BIT))) >> BIT
#define xor_bit(REGISTER, BIT)    REGISTER ^= _BV(BIT)

#endif	/* UTILS_H */

