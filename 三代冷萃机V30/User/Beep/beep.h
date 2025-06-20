#ifndef __BEEP_H
#define __BEEP_H

#include "gd32f30x.h"

#define BEEP_CLOCK      RCU_GPIOA
#define BEEP_PROT       GPIOA
#define BEEP_PIN        GPIO_PIN_8

void Beep_init(void);
void Beep_set(uint8_t state);

#endif

