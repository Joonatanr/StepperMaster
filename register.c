/*
 * register.c
 *
 *  Created on: Sep 23, 2018
 *      Author: Joonatan
 */


#include "register.h"
#include "driverlib.h"





/*****************************************************************************************************
 *
 * Private Functions forward declarations
 *
 *****************************************************************************************************/

Private void clocks_init(void);
Private void ports_init(void);
Private void timerA_init(void);

Private void TA0_0_IRQHandler(void);


/*****************************************************************************************************
 *
 * Private variables.
 *
 *****************************************************************************************************/

//Hi priority timer runs at 10msec interval (might need to be faster)
Private const Timer_A_UpModeConfig hi_prio_timer_config =
{
     .captureCompareInterruptEnable_CCR0_CCIE = TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE, /* We enable capture compare, since this is a periodic timer. */
     .clockSource = TIMER_A_CLOCKSOURCE_SMCLK,
     .clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_16, //Currently divided by 16.
     .timerClear = TIMER_A_DO_CLEAR,
     .timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE, //Disable general interrupt.
     .timerPeriod = 7500u
};

Public volatile U16 priv_delay_counter = 0u;


/*****************************************************************************************************
 *
 * Public Functions
 *
 *****************************************************************************************************/

Public void register_init(void)
{
    clocks_init();

    ports_init();

    timerA_init();

    //Not quite sure what this does yet.
    //MAP_Interrupt_enableSleepOnIsrExit();

    //Enable interrupts in general.
    //Interrupt_enableMaster();
}


#pragma FUNCTION_OPTIONS(delay_msec, "--opt_level=off")
Public void delay_msec(U16 msec)
{
    priv_delay_counter = msec / 10;
    while(priv_delay_counter > 0u);
}


/****************************************************************************************
 * INPUT OUTPUT PORTS
 ****************************************************************************************/

Public void set_led_one(U8 state)
{
    if (state == 1u)
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }
    else
    {
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }
}

Public void set_led_two_red(U8 state)
{
    if (state == 1u)
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
    }
    else
    {
        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
    }
}

Public void set_led_two_green(U8 state)
{
    if (state == 1u)
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
    }
    else
    {
        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
    }
}

Public void set_led_two_blue(U8 state)
{
    if (state == 1u)
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
    }
    else
    {
        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
    }
}


/*****************************************************************************************************
 *
 * Private Functions
 *
 *****************************************************************************************************/

Private void clocks_init(void)
{
    WDT_A_holdTimer();

    /* This should be set in case of higher frequency. */
    //MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);

    /* Set 2 flash wait states for Flash bank 0 and 1, also required for 48MHz */
    //MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    //MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

    //Lets configure the DCO to 12MHz
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

    /* Initializing the clock source as follows:
     *      MCLK = MODOSC/2 = 12MHz
     *      ACLK = REFO/2 = 16kHz   --- TODO Not used, should remove.
     *      HSMCLK = DCO/2 = 6Mhz
     *      SMCLK = DCO =  12MHz
     *      BCLK  = REFO = 32kHz    --- TODO Not used, should remove.
     */
    MAP_CS_initClockSignal(CS_MCLK,     CS_MODOSC_SELECT,   CS_CLOCK_DIVIDER_2);
    MAP_CS_initClockSignal(CS_ACLK,     CS_REFOCLK_SELECT,  CS_CLOCK_DIVIDER_2);
    MAP_CS_initClockSignal(CS_HSMCLK,   CS_DCOCLK_SELECT,   CS_CLOCK_DIVIDER_2);
    MAP_CS_initClockSignal(CS_SMCLK,    CS_DCOCLK_SELECT,   CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_BCLK,     CS_REFOCLK_SELECT,  CS_CLOCK_DIVIDER_1);
}


Private void ports_init(void)
{
    //First lets set up LED ports as outputs.
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2);

    //Clear other LEDs.
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
}

Private void timerA_init(void)
{
    //Set up timer for high priority interrupts.
    Timer_A_configureUpMode(TIMER_A0_BASE, &hi_prio_timer_config);
    Timer_A_registerInterrupt(TIMER_A0_BASE, TIMER_A_CCR0_INTERRUPT, TA0_0_IRQHandler);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);

    //Enable this interrupt in NVIC.
    Interrupt_setPriority(INT_TA0_0, 4u); /* TODO : 4U has been chosen quite randomly... */
    Interrupt_enableInterrupt(INT_TA0_0);
}


/* This should be fired every 10 msec */
//Hi priority interrupt handler.
Private void TA0_0_IRQHandler(void)
{
    Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
    if (priv_delay_counter > 0u)
    {
        priv_delay_counter--;
    }
    timer_10msec_callback();
}


