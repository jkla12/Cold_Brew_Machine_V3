#include "led.h"

void led_init(void)
{
	rcu_periph_clock_enable(LED_CLOCK);
	gpio_init(LED_USER1_PORT,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,LED_USER1_PIN);
  GPIO_BOP(LED_USER1_PORT) = LED_USER1_PIN;	//电平置高，关闭LED
	gpio_init(LED_USER2_PORT,GPIO_MODE_OUT_OD,GPIO_OSPEED_50MHZ,LED_USER2_PIN);
  GPIO_BOP(LED_USER2_PORT) = LED_USER2_PIN;	//电平置低，开启LED
}

void LED_ON(led_typedef_enum number)
{
    GPIO_BC(GPIO_PORT[number]) = GPIO_PIN[number];
}

void LED_OFF(led_typedef_enum number)
{
    GPIO_BOP(GPIO_PORT[number]) = GPIO_PIN[number];
}

void gd_eval_led_toggle(led_typedef_enum number)
{
    gpio_bit_write(GPIO_PORT[number], GPIO_PIN[number], 
        (bit_status)(1-gpio_input_bit_get(GPIO_PORT[number], GPIO_PIN[number])));
}
