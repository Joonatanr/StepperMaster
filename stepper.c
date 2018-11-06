/*
 * stepper.c
 *
 *  Created on: 6. nov 2018
 *      Author: JRE
 */


/* This module keeps track of the stepper state that is reported and controlled by the Slave MSP432. */

/* Current implementation is a placeholder... */

#include "stepper.h"


Private Stepper_Query_t priv_stepper_states[NUMBER_OF_STEPPERS];

Public void stepper_init(void)
{
    U8 ix;

    /* Initialize for default values... */
    for (ix = 0u; ix < NUMBER_OF_STEPPERS; ix++)
    {
        priv_stepper_states[ix].interval = 0xffffu;
        priv_stepper_states[ix].microstepping_mode = 0xffu;
        priv_stepper_states[ix].rpm = 0xffffu;
    }
}

/* Returns with delay... */
Public Boolean stepper_setSpeed(U32 rpm, Stepper_Id id)
{
    /* TODO */
    return FALSE;
}

/* Returns immediately... */
Public U16 stepper_getSpeed(Stepper_Id id)
{
    return priv_stepper_states[id].rpm;
}

/* Returns with delay... */
Public Boolean stepper_setMicrosteppingMode(Stepper_Id id, U8 mode)
{
    /* TODO */
    return FALSE;
}

/* Returns immediately... */
Public Boolean stepper_getState(Stepper_Id id, Stepper_Query_t * res)
{
    memcpy(res, &priv_stepper_states[id], sizeof(Stepper_Query_t));
    return TRUE;
}

