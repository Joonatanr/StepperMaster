/*
 * comm.c
 *
 *  Created on: 28. mai 2017
 *      Author: Joonatan
 */

/* This file handles communication over the UART interface.
 * Unfortunately driverlib already has a source file named uart.c */

#include <uart.h>
#include <driverlib.h>
#include "typedefs.h"
#include "uartmgr.h"
#include "misc.h"
#include <stdlib.h>


/************************************* Private function prototypes ************************************/




/************************************* Private data declarations **************************************/

Private U8 priv_receive_buffer[UART_BUF_LEN];
Private char priv_uart_buffer[UART_BUF_LEN];
Private U8 priv_receive_cnt;
Private U8 priv_receive_flag = 0;
Private char priv_char_buf[16];

Private U8 priv_async_buf[UART_BUF_LEN];

volatile Boolean priv_is_async_transmit_in_progress = FALSE;
volatile U16     priv_async_transmit_len;

/* Set to 9600 baudrate */
Private const eUSCI_UART_Config uartConfig =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        78,                                      // BRDIV = 78
        2,                                       // UCxBRF = 2
        0,                                       // UCxBRS = 0
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_LSB_FIRST,                  // LSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
};


Public void uartmgr_init(void)
{
    /* Selecting P1.2 and P1.3 in UART mode */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
            GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configuring UART Module */
    MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);

    /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A0_BASE);

    /* Enabling interrupts */
    MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA0);
}


Public void uartmgr_send_char(char c)
{
    UART_transmitData(EUSCI_A0_BASE, c);

    //So should we also wait until data has been sent out???
    //We should not send any chars out if UART is busy.
}

Public void uartmgr_send_str(const char * str)
{
    const char * ps = str;
    while(*ps)
    {
        uartmgr_send_char(*ps);
        ps++;
    }
}


Public void uartmgr_send_data(const U8 * data, U16 len)
{
    const U8 * data_ptr = data;
    while (len--)
    {
        uartmgr_send_char(*data_ptr);
        data_ptr++;
    }
}


/* Transmits string on UART without blocking the CPU. */
Public void uartmgr_send_str_async(const char * str, U16 len)
{
    if (priv_is_async_transmit_in_progress)
    {
        return;
    }

    memcpy(priv_async_buf, str, len);
    priv_is_async_transmit_in_progress = TRUE;
    priv_async_transmit_len = len;
}


Public void uartmgr_send_number(long nr)
{
    U32 len = long2string(nr, priv_char_buf);
    priv_char_buf[len] = 0;
    uartmgr_send_str(priv_char_buf);
}

Public void uartmgr_send_rn(void)
{
    uartmgr_send_char('\r');
    uartmgr_send_char('\n');
}

/* EUSCI A0 UART ISR */
/* Handles incoming data over the UART.*/
Public void EUSCIA0_IRQHandler(void)
{
    uint32_t status = UART_getEnabledInterruptStatus(EUSCI_A0_BASE);
    U8 c;

    UART_clearInterruptFlag(EUSCI_A0_BASE, status);

    if (status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        //MAP_UART_transmitData(EUSCI_A0_BASE, MAP_UART_receiveData(EUSCI_A0_BASE));
        c = UART_receiveData(EUSCI_A0_BASE);

        if ((priv_receive_cnt < UART_BUF_LEN) && (priv_receive_flag == 0u))
        {
            priv_receive_buffer[priv_receive_cnt] = c;
            priv_receive_cnt++;

            if (c == '\n')
            {
                //We got new data that needs to be processed.
                priv_receive_flag = 1u;
            }
        }
        else
        {
            //Silently discard data to prevent buffer overflow.
        }
    }
}


Public U8 uartmgr_receiveData(char * dest)
{
    //Return length of received data. If none received then return 0.
    U8 res = 0u;

    if (priv_receive_flag)
    {
        res = priv_receive_cnt;
        (void)memcpy(dest, priv_receive_buffer, priv_receive_cnt);
        priv_receive_cnt = 0u;
        priv_receive_flag = 0u;
    }

    return res;
}


Public void uartmgr_cyclic(void)
{
    U8 msg_len;
    Boolean res = FALSE;

    msg_len = uartmgr_receiveData(priv_uart_buffer);

    if (msg_len > 0u)
    {
        if (CmdHandlerFunc != NULL)
        {
            res = CmdHandlerFunc(priv_uart_buffer, msg_len);
        }

        if (res)
        {
            /* Send positive response */
            uartmgr_send_str("OK");
            uartmgr_send_rn();
        }
        else
        {
           /* Send error response   */
            uartmgr_send_str("ERR");
            uartmgr_send_rn();
        }
    }

    /* This is called from the main (background thread) so interrupts can still happen during sending. */
    if (priv_is_async_transmit_in_progress)
    {
        //uartmgr_send_str(priv_async_buf);
        uartmgr_send_data(priv_async_buf, priv_async_transmit_len);
        priv_is_async_transmit_in_progress = FALSE;
    }
}





