/*
 * stepper.h
 *
 *  Created on: 8. juuni 2018
 *      Author: JRE
 */

#ifndef HW_STEPPER_H_
#define HW_STEPPER_H_

#include "typedefs.h"

typedef enum
{
    STEPPER0,
    STEPPER1,
    STEPPER2,
    STEPPER3,
    NUMBER_OF_STEPPERS
} Stepper_Id;

typedef struct
{
    U16 rpm;
    U16 interval;
    U8  microstepping_mode;
} Stepper_Query_t;

extern void stepper_init(void);
extern Boolean stepper_setSpeed(U32 rpm, Stepper_Id id);
extern U16 stepper_getSpeed(Stepper_Id id);
extern Boolean stepper_setMicrosteppingMode(Stepper_Id id, U8 mode);
extern Boolean stepper_getState(Stepper_Id id, Stepper_Query_t * res);


extern Boolean stepper_handleStatusResponse(U8 * data, U8 data_len);

#endif /* HW_STEPPER_H_ */
