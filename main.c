#include "msp.h"
#include "typedefs.h"
#include "driverlib.h"
#include "register.h"
#include "spidrv.h"
#include "uartmgr.h"
#include "spiCommandHandler.h"
#include "uartCommandHandler.h"
#include "led.h"
#include "stepper.h"

/**
 * main.c
 */


Boolean priv_isInitComplete = FALSE;

void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	register_init();

	uartmgr_init();

	spidrv_init();

	led_init();

	stepper_init();

	spiCommandHandler_init();

	uartCommandHandler_init();

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
    static U8 counter = 0u;
    static U8 spi_timer = 0u;

    if (!priv_isInitComplete)
    {
        return;
    }

    if (++counter >= 100u)
    {
        counter = 0u;
    }

    if (++spi_timer >= 5u)
    {
        spi_timer = 0u;
        spidrv_cyclic50ms();
        led_cyclic50ms();
        uartCommandHandler_cyclic50ms();
    }

    spiCommandHandler_cyclic10ms();
}
