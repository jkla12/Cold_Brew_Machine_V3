#include "dial.h"

uint8_t device_addr;

void dial_init(void)
{
    rcu_periph_clock_enable(DIAL_CLOCK);
    rcu_periph_clock_enable(RCU_AF);
    gpio_init(ADDR0_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, ADDR0_PIN);
    gpio_init(ADDR1_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, ADDR1_PIN);
}



