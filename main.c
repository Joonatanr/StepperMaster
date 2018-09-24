#include "msp.h"
#include "typedefs.h"
#include "driverlib.h"
#include "register.h"
#include "spidrv.h"
#include "uartmgr.h"

/**
 * main.c
 */

Boolean UartCommandCallback(char *buf, U16 msg_len);
UartCommandHandler CmdHandlerFunc = UartCommandCallback; /* Connect callback to UART manager. */

Boolean priv_isInitComplete = FALSE;

void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	register_init();

	uartmgr_init();

	spidrv_init();

	Interrupt_enableMaster();

	priv_isInitComplete = TRUE;

	uartmgr_send_str("Stepper Master simulator ver 1.0 - Ready");

	while(1)
	{
	    /* Check the uart manager cyclically. */
	    uartmgr_cyclic();
	}
}


/* 10msec timer callback from interrupt. */
void timer_10msec_callback(void)
{
    static U8 led_state;
    static U8 counter = 0u;
    static U8 spi_timer = 0u;

    /* TODO : Should not call cylic functions directly from the interrupt.
     * This is probably OK for testing, but should consider switching to a flag based system... */

    if (!priv_isInitComplete)
    {
        return;
    }

    if (++counter >= 100u)
    {
        led_state = !led_state;
        set_led_two_blue(led_state);
        counter = 0u;
    }

    if (++spi_timer >= 5u)
    {
        spi_timer = 0u;
        spidrv_cyclic50ms();
    }
}


/* Placeholder. */
Boolean UartCommandCallback(char *buf, U16 msg_len)
{
    /* We end up here when receiving a line over the UART. */
    return TRUE;
}
