#include "beep.h"

void Beep_init(void)
{
    rcu_periph_clock_enable(BEEP_CLOCK);
    gpio_init(BEEP_PROT,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,BEEP_PIN);
    GPIO_BC(BEEP_PROT) = BEEP_PIN;	//电平置低，关闭蜂鸣器
}

void Beep_set(uint8_t state)
{
    if(state)
    {
        GPIO_BOP(BEEP_PROT) = BEEP_PIN;	//电平置高，开启蜂鸣器
    }
    else
    {
        GPIO_BC(BEEP_PROT) = BEEP_PIN;	//电平置低，关闭蜂鸣器
    }
}

