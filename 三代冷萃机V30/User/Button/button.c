#include "button.h"
#include "dr_button_api.h"
#include "led.h"
#include "uart.h"
#include <stdio.h>
#include "work.h"
#include "app_wifi_set.h"

int key_val[4] = {0};
uint8_t timer8_flag;
void Button_gpio_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_init(Button1_GPIO, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, Button1_PIN);
    gpio_init(Button2_GPIO, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, Button2_PIN);
    timer6_config();
}

uint8_t gd_eval_key_state_get(key_typedef_enum number)
{
    return gpio_input_bit_get(KEY_PORT[number], KEY_PIN[number]);
}

void timer6_config(void)
{
    timer_parameter_struct timer_initpara;
    rcu_periph_clock_enable(RCU_TIMER6);
    timer_deinit(TIMER6);
    timer_struct_para_init(&timer_initpara);

    timer_initpara.prescaler = 119; // 119分频
    timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection = TIMER_COUNTER_UP;
    timer_initpara.period = 4999; // 定时5ms
    timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER6, &timer_initpara);

    timer_interrupt_flag_clear(TIMER6, TIMER_INT_FLAG_UP);
    timer_interrupt_enable(TIMER6, TIMER_INT_UP);
    timer_enable(TIMER6);
    nvic_irq_enable(TIMER6_IRQn, 1, 2);
}


void TIMER6_IRQHandler(void)
{

    if (timer_interrupt_flag_get(TIMER6, TIMER_INT_FLAG_UP) != RESET)
    {
        // 嘎，清除 TIMER8 中断标志
        timer_interrupt_flag_clear(TIMER6, TIMER_INT_FLAG_UP);

        Key_Tick();
        // 读取按键
        key_val[0] = Get_Key1();
        key_val[1] = Get_Key2();
        if (key_val[0] != 0)
        {
            
            printf("key1:%d\r\n", key_val[0]);
        }
        if (key_val[1] != 0)
        {
            printf("key2:%d\r\n", key_val[1]);
        }
    }
}
