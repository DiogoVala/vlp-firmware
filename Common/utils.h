/* utils.h
 * Forked from Pedro Rodrigues' vlp-setup-firmware
 * https://github.com/m-rego/vlp-setup-firmware
 * 
 * Updated by: Diogo Vala
 *
 * Description: Useful macro functions
 * 
 */

#ifndef __UTILS_H__
#define	__CUTILS_H__

/* Macro functions d*/
#define set_bit(REGISTER, BIT)    REGISTER |=  _BV(BIT)
#define clr_bit(REGISTER, BIT)    REGISTER &= ~(_BV(BIT))
#define check_bit(REGISTER, BIT) (REGISTER & (_BV(BIT))) >> BIT
#define xor_bit(REGISTER, BIT)    REGISTER ^= _BV(BIT)

#endif	/* __CONFIG_H__ */

