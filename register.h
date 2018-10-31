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
extern void ports_setPort(U32 port, U32 pin, Boolean value);

extern void timer_10msec_callback(void);


#endif /* REGISTER_H_ */
