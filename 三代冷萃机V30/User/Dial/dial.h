#ifndef __DIAL_H
#define __DIAL_H
#include "gd32f30x.h"



#define DIAL_CLOCK	RCU_GPIOC
#define ADDR0_PORT  GPIOC
#define ADDR0_PIN   GPIO_PIN_0
#define ADDR1_PORT  GPIOC
#define ADDR1_PIN   GPIO_PIN_1



void dial_init(void);


#endif
