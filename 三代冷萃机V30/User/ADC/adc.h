#ifndef __ADC_H__
#define __ADC_H__

#include "gd32f30x.h"

#define ADC0_CLOCK  RCU_GPIOA

#define ADC0_PORT   GPIOA
#define ADC0_PIN    GPIO_PIN_0
#define ADC1_PORT   GPIOA
#define ADC1_PIN    GPIO_PIN_1

extern uint16_t adc_val;

void adc0_config(void);
void moving_average_filter(uint16_t new_value1,uint16_t new_value2,uint16_t *filterValue);
void filter_init(void);
void readADCValue(uint16_t *value);
#endif

