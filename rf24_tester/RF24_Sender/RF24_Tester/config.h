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

/* Macro functions d*/
#define set_bit(REGISTER, BIT)    REGISTER |=  _BV(BIT)
#define clr_bit(REGISTER, BIT)    REGISTER &= ~(_BV(BIT))
#define check_bit(REGISTER, BIT) (REGISTER & (_BV(BIT))) >> BIT
#define xor_bit(REGISTER, BIT)    REGISTER ^= _BV(BIT)

#endif	/* __CONFIG_H__ */

