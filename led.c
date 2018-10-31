/*
 * led.c
 *
 *  Created on: 5. okt 2018
 *      Author: JRE
 */

/* Small module for handling all the debug LEDs on the board. */


#include "led.h"
#include "driverlib.h"
#include "register.h"

typedef struct
{
    U32 port;
    U32 pin;
    int counter; /* Used for displaying LED for a given amount of time. */
} led_state;


Private led_state priv_led_conf[NUMBER_OF_LEDS] =
{
     { .port = GPIO_PORT_P1, .pin = GPIO_PIN0 }, /* LED_ONE       */
     { .port = GPIO_PORT_P2, .pin = GPIO_PIN0 }, /* LED_TWO_RED   */
     { .port = GPIO_PORT_P2, .pin = GPIO_PIN1 }, /* LED_TWO_GREEN */
     { .port = GPIO_PORT_P2, .pin = GPIO_PIN2 }, /* LED_TWO_BLUE  */
};


/**********  Private function forward declarations *************/
Private void led_cyclic1000ms(void);



Public void led_init(void)
{
    /* Port initialisation is currently done elsewhere. */
    U8 ix;
    for (ix = 0u; ix < NUMBER_OF_LEDS; ix++)
    {
        priv_led_conf[ix].counter = 0;
    }
}


Public void led_cyclic50ms(void)
{
    static U8 counter = 0u;
    U8 ix;

    if (++counter > 20)
    {
        counter = 0u;
        led_cyclic1000ms();
    }

    /* Handle LEDs that are shown for a set period of time. */
    for (ix = 0u; ix < NUMBER_OF_LEDS; ix++)
    {
        if (priv_led_conf[ix].counter > 0)
        {
            priv_led_conf[ix].counter -= 50;

            if (priv_led_conf[ix].counter <= 0)
            {
                priv_led_conf[ix].counter = 0;
                led_set_state((led_id)ix, FALSE);
            }
        }
    }
}


Public void led_set_state(led_id led, Boolean state)
{
    if (led < NUMBER_OF_LEDS)
    {
        ports_setPort(priv_led_conf[led].port, priv_led_conf[led].pin, state);
    }
}


/* Function displays a LED for a given period. */
Public void led_show_period(led_id led, U16 ms)
{
    if (led < NUMBER_OF_LEDS)
    {
        led_set_state(led, TRUE);
    }

    priv_led_conf[led].counter = ms;
}


/****************************************************************************************
 *
 * Direct LED control functions.
 *
 ****************************************************************************************/

Public void set_led_one(Boolean state)
{
    ports_setPort(GPIO_PORT_P1, GPIO_PIN0, state);
}

Public void set_led_two_red(Boolean state)
{
    ports_setPort(GPIO_PORT_P2, GPIO_PIN0, state);
}

Public void set_led_two_green(Boolean state)
{
    ports_setPort(GPIO_PORT_P2, GPIO_PIN1, state);
}

Public void set_led_two_blue(Boolean state)
{
    ports_setPort(GPIO_PORT_P2, GPIO_PIN2, state);
}


/**********  Private function definitions *************/

/* This function simply blinks the RED LED to show that scheduler (and CPU) is working properly. */
Private void led_cyclic1000ms(void)
{
    static U8 led_state;

    led_state = !led_state;
    set_led_one((Boolean)led_state);
}
