#ifndef __LED_H__
#define __LED_H__

#include "gd32f30x.h"


#define LED_CLOCK		RCU_GPIOA

#define LED_USER1_PORT  GPIOA
#define LED_USER1_PIN   GPIO_PIN_4
#define LED_USER2_PORT  GPIOA
#define LED_USER2_PIN   GPIO_PIN_5


void led_init(void);


static uint32_t GPIO_PORT[2] = {
    LED_USER1_PORT,LED_USER2_PORT
};
static uint32_t GPIO_PIN[2] = {
    LED_USER1_PIN,LED_USER2_PIN
};

typedef enum
{
    LED1 = 0,
    LED2 = 1,

}led_typedef_enum;


void led_init(void);
void LED_ON(led_typedef_enum number);
void LED_OFF(led_typedef_enum number);
void gd_eval_led_toggle(led_typedef_enum number);

#endif 

