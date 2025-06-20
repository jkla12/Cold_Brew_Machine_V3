#ifndef __BSP_WIFI_H
#define __BSP_WIFI_H

#include "gd32f30x.h"

#define WIFI_USART USART4




#define WIFI_EN_CLOCK      RCU_GPIOC
#define WIFI_EN_PORT    GPIOC
#define WIFI_EN_PIN     GPIO_PIN_9


void wifiInit(uint32_t baud);
void WIFI_ENABLE(void);
void WIFI_DISABLE(void);


#endif


