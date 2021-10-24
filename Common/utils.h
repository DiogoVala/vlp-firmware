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
#define	__UTILS_H__

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define my_delay(msec) delay8((int) (F_CPU / 8000L * (msec)))

/* Macro functions d*/
#define set_bit(REGISTER, BIT)    REGISTER |=  _BV(BIT)
#define clr_bit(REGISTER, BIT)    REGISTER &= ~(_BV(BIT))
#define check_bit(REGISTER, BIT) (REGISTER & (_BV(BIT))) >> BIT
#define xor_bit(REGISTER, BIT)    REGISTER ^= _BV(BIT)

#endif

static void delay8(uint16_t count) {
	while (count --)
	__asm__ __volatile__ (
	"\tnop\n"
	"\tnop\n"
	"\tnop\n"
	"\tnop\n"
	"\tnop\n"
	"\tnop\n"
	"\tnop\n"
	"\twdr\n"
	);
}