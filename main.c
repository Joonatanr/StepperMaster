#include "msp.h"
#include "typedefs.h"
#include "driverlib.h"
#include "register.h"

/**
 * main.c
 */
void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	register_init();
}
