#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "gd32f30x.h"


#define Button1_GPIO        GPIOC
#define Button1_PIN         GPIO_PIN_2
#define Button2_GPIO        GPIOC
#define Button2_PIN         GPIO_PIN_3


typedef enum 
{
    Button1 = 0,
    Button2 = 1,
} key_typedef_enum;

static uint32_t KEY_PORT[4] = {Button1_GPIO, Button2_GPIO};
static uint32_t KEY_PIN[4] = {Button1_PIN,Button2_PIN};


void Button_gpio_init(void);
uint8_t gd_eval_key_state_get(key_typedef_enum number);
void timer6_config(void);
#endif


