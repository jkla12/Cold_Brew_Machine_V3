#ifndef __DS18B20_H
#define __DS18B20_H

#include "gd32f30x.h"

#define DS18B20_PORT1       GPIOB
#define DS18B20_PIN1        GPIO_PIN_0
#define DS18B20_PORT2       GPIOA
#define DS18B20_PIN2        GPIO_PIN_7

void ds18b20_init(void);
void ds18b20_write(uint8_t data);
uint8_t ds18b20_read(void);
void ds18b20_readBytes(uint8_t num, uint8_t *data);
unsigned char ds18b20_crc8(unsigned char *data, unsigned char len);
float  ds18b20_readTemp(void);


#endif


