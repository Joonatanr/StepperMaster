/*
 * commandHandler.c
 *
 *  Created on: 22. juuni 2018
 *      Author: JRE
 */


#include "uartCommandHandler.h"
#include "uartmgr.h"
#include <stdlib.h>
#include "misc.h"
#include "stepper.h"


/********************************************* Private definitions ************************************/
typedef Boolean (*CommandHandlerFunc)(char * data, U8 len);
typedef Boolean (*SubFunctionHandler)(Stepper_Id id, int arg);

typedef enum
{
    UART_CMD_IDLE,
    UART_CMD_RESPONSE_PENDING,
    NUMBER_OF_UART_CMD_STATES
} uartCmdState_T;

typedef struct
{
    const char * prefix;
    CommandHandlerFunc handler_fptr;
} CommandHandler_T;


typedef struct
{
    char prefix;
    SubFunctionHandler handler_fptr;
} SubFunc_T;


/************************************* Private function prototypes ************************************/

Private Boolean handleCommand(char * cmd, U16 msg_len);

/* Command handlers */
Private Boolean HandleCommandToStepper(char * data, U8 len);
Private Boolean HandleQueryState(char * data, U8 len);
Private Boolean HandleExtendedQueryState(char * data, U8 len);

/* Subfunction handlers */
Private Boolean HandleCommandRPMSet(Stepper_Id id, int arg);
Private Boolean HandleCommandMicroStepSet(Stepper_Id id, int arg);


Private Boolean appendResponse(const char * resp);


/************************************ Private variable definitions ***********************************/

Public UartCommandHandler CmdHandlerFunc = handleCommand;


Private const CommandHandler_T priv_command_handlers[] =
{
     { .prefix = "S", .handler_fptr = HandleCommandToStepper    },
     { .prefix = "Q", .handler_fptr = HandleQueryState          },
     { .prefix = "EQ",.handler_fptr = HandleExtendedQueryState  }
};


Private const SubFunc_T priv_subfunctions[] =
{
     { .prefix = 'R', .handler_fptr = HandleCommandRPMSet       },
     { .prefix = 'M', .handler_fptr = HandleCommandMicroStepSet },
};


#define MAX_RESPONSE_LENGTH 128u
#define GENERAL_PURPOSE_STR_LENGTH 64u

Private char priv_response_buf[MAX_RESPONSE_LENGTH + 1];
Private char priv_gp_str[GENERAL_PURPOSE_STR_LENGTH + 1u];

Private char * priv_response_ptr;   //Points to the end of the current response.
Private U16    priv_remaining_buf_len;  //Keeps track of the number of bytes remaining in the response buffer.

Private uartCmdState_T priv_state;

/********************************** Public function definitions  **************************************/


Public void uartCommandHandler_init(void)
{
    priv_response_ptr = NULL;
    priv_state = UART_CMD_IDLE;
}


Public void uartCommandHandler_cyclic50ms(void)
{
    /* TODO : Handle command timeouts here. */
}


/*********************************  Private function definitions  *******************/

Private Boolean handleCommand(char * cmd, U16 msg_len)
{
    Boolean res = FALSE;
    U8 ix;

    if (priv_state != UART_CMD_IDLE)
    {
        /* We are busy processing another request... */
        return FALSE;
    }

    //TODO : This is quick fix, must remove later.
    msg_len = msg_len - 2;

    /* Set up response buffer. Also set up a pointer to the beginning of the response buffer. This keeps track of the end of the response string. */
    priv_response_buf[0] = 0;
    priv_response_ptr = &priv_response_buf[0];
    priv_remaining_buf_len = MAX_RESPONSE_LENGTH;

    /* Create the response header */
    strncpy(priv_gp_str, cmd, msg_len);
    priv_gp_str[msg_len] = '-';
    priv_gp_str[msg_len + 1] = '>';
    priv_gp_str[msg_len + 2] = 0;

    /* appendResponse function should be used to add to the response string. */
    appendResponse(priv_gp_str);

    for (ix = 0u; ix < NUMBER_OF_ITEMS(priv_command_handlers); ix++)
    {
        if(strncmp(priv_command_handlers[ix].prefix, cmd, strlen(priv_command_handlers[ix].prefix)) == 0)
        {
           res = priv_command_handlers[ix].handler_fptr(cmd + 1, msg_len - 1u);
           break;
        }
    }

    if (priv_state == UART_CMD_RESPONSE_PENDING)
    {
        /* We do not get an immediate response. */
        if (res == FALSE)
        {
            /* Something went wrong.. */
            priv_state = UART_CMD_IDLE;
        }
    }
    else if (strlen(priv_response_buf) > 0)
    {
        /* Send an immediate response. */
        addchar(priv_response_buf, '\n');
        uartmgr_send_str(priv_response_buf);
    }

    return res;
}


/********************************* Command handlers ********************************/

/* Format is S1R200 */
Private Boolean HandleCommandToStepper(char * data, U8 len)
{
    Boolean res = FALSE;
    int stepper_id;
    char subcmd;
    char * ps = data;
    U8 ix;

    if (len > 0u)
    {
        stepper_id = atoi(ps);
        if ((stepper_id >= 0) && (stepper_id < NUMBER_OF_STEPPERS))
        {
            ps++;

            if(!(*ps))
            {
                return FALSE;
            }

            /* Extract sub command */
            subcmd = *ps;
            ps++;

            if(!(*ps))
            {
                return FALSE;
            }

            for (ix = 0u; ix < NUMBER_OF_ITEMS(priv_subfunctions); ix++)
            {
                if (priv_subfunctions[ix].prefix == subcmd)
                {
                    int arg = atoi(ps);
                    res = priv_subfunctions[ix].handler_fptr((Stepper_Id)stepper_id, arg);
                    break;
                }
            }
        }
    }

    return res;
}


Private Boolean HandleQueryState(char * data, U8 len)
{
    U8 id;
    Stepper_Query_t query;

    for (id = 0u; id < NUMBER_OF_STEPPERS; id++)
    {
        stepper_getState((Stepper_Id)id, &query);
        sprintf(priv_gp_str, "S%d:%dRPM(%d step) ", id, query.rpm, query.microstepping_mode); /* TODO : Might not want to use sprintf here, as it takes a lot of processing power. */
        appendResponse(priv_gp_str);
    }

    return TRUE;
}


/* Format should be easily parseable */
/* S1-> RPM:240, Step:32, Int:20000 */
/* Returns extended data for a given stepper. */
Private Boolean HandleExtendedQueryState(char * data, U8 len)
{
    U8 id;
    Stepper_Query_t query;
    Boolean res;

    if (len == 0u)
    {
        return FALSE;
    }

    id = data[0];
    res = stepper_getState((Stepper_Id)id, &query);

    if (res)
    {
        sprintf(priv_gp_str, "S%d-> RPM:%d, Step:%d, Int:%d", id, query.rpm, query.microstepping_mode, query.interval);
        appendResponse(priv_gp_str);
    }

    return res;
}


/********************************* Subfunction handlers ********************************/


Private Boolean HandleCommandRPMSet(Stepper_Id id, int arg)
{
    Boolean res = FALSE;

    if ((id < NUMBER_OF_STEPPERS) && (arg >= 0))
    {
        stepper_setSpeed(arg, id);
        priv_state = UART_CMD_RESPONSE_PENDING;
        res = TRUE;
    }
    return res;
}


Private Boolean HandleCommandMicroStepSet(Stepper_Id id, int arg)
{
    Boolean res = FALSE;

    if ((id < NUMBER_OF_STEPPERS) && (arg >= 0))
    {
        res = stepper_setMicrosteppingMode(id, (U8)arg);
        priv_state = UART_CMD_RESPONSE_PENDING;
    }
    return res;
}


/********************************* Private function definitions ********************************/

/* Appends to the end of an existing response string. */
/* If there is no more room, then we simply cut off the string.     */
/* This function also keeps track of the remaining buffer length.   */
/* TODO : Test this. */
Private Boolean appendResponse(const char * resp)
{
    U16 resp_len;
    Boolean res = TRUE;

    if (priv_response_ptr == NULL)
    {
        //Something has gone really wrong...
        return FALSE;
    }

    resp_len = strlen(resp);

    if (resp_len > priv_remaining_buf_len)
    {
        resp_len = priv_remaining_buf_len;
        res = FALSE; //Indicate that the response did not fit.
    }

    strncpy(priv_response_ptr, resp, resp_len);
    priv_response_ptr[resp_len] = 0; //Add 0 terminator just in case.
    priv_response_ptr  += resp_len;
    priv_remaining_buf_len -= resp_len;

    return res;
}

#if 0
//TODO : Finish this.
//Returns length of string.
Private U8 removeRn(char * str)
{
    U8 res = 0u;
    char * ps = str;

    while(*ps)
    {
        res++;
        ps++;

        if ((*ps == '\n') || (*ps == '\r'))
        {
            *ps = 0;
            return res;
        }
    }
}
#endif
