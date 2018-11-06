/*
 * stepper.c
 *
 *  Created on: 6. nov 2018
 *      Author: JRE
 */


/* This module keeps track of the stepper state that is reported and controlled by the Slave MSP432. */

/* Current implementation is a placeholder... */

#include "stepper.h"
#include "SpiCommandHandler.h"
#include "uartCommandHandler.h"


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
    static U8 data[8];
    U8 sub;

    if(id < NUMBER_OF_STEPPERS)
    {
        sub = 0x01u << id;

        memset(data, 0xffu, sizeof(data));
        data[id * 2] = (U8)((rpm >> 8u) & 0xffu);
        data[(id * 2) + 1] = (U8)(rpm & 0xffu);

        spiCommandHandler_setNextCommand((U8)CMD_SET_MOTOR_SPEED, sub, data, 8u);
    }

    return TRUE;
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


Public Boolean stepper_handleSpeedSetResponse(U8 * data, U8 data_len)
{
    U8 ix;
    U16 speed;

    if (data_len != 8u)
    {
        return FALSE;
    }

    for (ix = 0u; ix < NUMBER_OF_STEPPERS; ix++)
    {
        speed = (data[ix * 2] << 8);
        speed |= data[(ix * 2) + 1] & 0xffu;

        priv_stepper_states[ix].rpm = speed;
    }

    /* Notify the UART layer of a response being ready. */
    uartCommandHandler_setSpeedResponse();

    return TRUE;
}


Public Boolean stepper_handleStatusResponse(U8 * data, U8 data_len)
{
    U8 ix;
    U8 * data_ptr = data;

    if (data_len != (NUMBER_OF_STEPPERS * 5))
    {
        return FALSE;
    }


    for (ix = 0u; ix < NUMBER_OF_STEPPERS; ix++)
    {
        priv_stepper_states[ix].microstepping_mode = data_ptr[0];
        priv_stepper_states[ix].interval = (U16)(data_ptr[1] << 8u);
        priv_stepper_states[ix].interval |= (U16)(data_ptr[2] & 0xffu);

        priv_stepper_states[ix].rpm =  (U16)(data_ptr[3] << 8u);
        priv_stepper_states[ix].rpm |= (U16)(data_ptr[4] & 0xffu);

        data_ptr += 5;
    }

    return TRUE;
}

