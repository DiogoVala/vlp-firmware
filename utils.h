/* 
 * File:   utils.h
 * Author: Diogo Vala <your.name at your.org>
 *
 * Created on August 2, 2021, 8:34 PM
 */

#ifndef UTILS_H
#define	UTILS_H

#define F_CPU 16000000UL
#define UART_BAUD_RATE      9600 

#define set_bit(REGISTER, BIT)    REGISTER |=  _BV(BIT)
#define clr_bit(REGISTER, BIT)    REGISTER &= ~(_BV(BIT))
#define check_bit(REGISTER, BIT) (REGISTER & (_BV(BIT)))
#define xor_bit(REGISTER, BIT)    REGISTER ^= _BV(BIT)

#endif	/* UTILS_H */

