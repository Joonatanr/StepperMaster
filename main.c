#include "msp.h"
#include "typedefs.h"
#include "driverlib.h"
#include "register.h"
#include "spidrv.h"

/**
 * main.c
 */
void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	register_init();

	spidrv_init();

    //Go to low power mode with interrupts.
    while(1)
    {
        PCM_gotoLPM0();
    }
}


/* 10msec timer callback from interrupt. */
void timer_10msec_callback(void)
{
    static U8 led_state;
    static U8 counter = 0u;
    static U8 spi_timer = 0u;

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
