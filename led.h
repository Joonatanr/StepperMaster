/*
 * led.h
 *
 *  Created on: 5. okt 2018
 *      Author: JRE
 */

#ifndef HW_LED_H_
#define HW_LED_H_

#include "typedefs.h"

typedef enum
{
    LED_ONE,
    LED_TWO_RED,
    LED_TWO_GREEN,
    LED_TWO_BLUE,
    NUMBER_OF_LEDS
} led_id;

extern void led_init(void);
extern void led_cyclic50ms(void);

extern void led_set_state(led_id led, Boolean state);
extern void led_show_period(led_id led, U16 ms);

extern void set_led_one(Boolean state);
extern void set_led_two_red(Boolean state);
extern void set_led_two_green(Boolean state);
extern void set_led_two_blue(Boolean state);


#endif /* HW_LED_H_ */
