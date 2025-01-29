/*
 * macros.h
 *
 *  Created on: Sep 14, 2024
 *      Author: Malak Waleed
 */

#ifndef MACROS_H_
#define MACROS_H_

/*sets a certain bit in a register*/
#define SET_BIT(REG, BIT) (REG |= (1<<BIT))

/*clears a certain bit in a register*/
#define CLEAR_BIT(REG, BIT) (REG &= ~(1<<BIT))

/*toggles a certain bit in a register*/
#define TOGGLE_BIT(REG, BIT) (REG ^= (1<<BIT))

/*checks if a bit is set in a register*/
#define BIT_IS_SET(REG, BIT) (REG & (1<<BIT))

/*checks if a bit is clear in a register*/
#define BIT_IS_CLEAR(REG, BIT) (!(REG & (1<<BIT)))

/*gets the value of a certain bit*/
#define GET_BIT(REG, BIT) ((REG & (1<<BIT)) >> BIT)

#endif /* MACROS_H_ */
