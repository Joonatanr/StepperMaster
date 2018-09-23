/*
 * register.h
 *
 *  Created on: Sep 23, 2018
 *      Author: Joonatan
 */

#ifndef REGISTER_H_
#define REGISTER_H_

#include "typedefs.h"

extern void register_init(void);
extern void delay_msec(U16 msec);

extern void set_led_one(U8 state);
extern void set_led_two_red(U8 state);
extern void set_led_two_green(U8 state);
extern void set_led_two_blue(U8 state);

extern void timer_10msec_callback(void);


#endif /* REGISTER_H_ */
