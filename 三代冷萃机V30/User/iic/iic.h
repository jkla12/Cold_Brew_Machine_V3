#ifndef __IIC_H
#define __IIC_H

#include "gd32f30x.h"


#define I2CX_SLAVE_ADDRESS7     0xA0			//AT24C02µÿ÷∑
#define I2C_SPEED               100000		//100K
#define I2C_PAGE_SIZE           8
#define I2CX                    I2C0
#define RCU_GPIO_I2C            RCU_GPIOB
#define RCU_I2C                 RCU_I2C0
#define I2C_SCL_PORT            GPIOB
#define I2C_SDA_PORT            GPIOB
#define I2C_SCL_PIN             GPIO_PIN_8
#define I2C_SDA_PIN             GPIO_PIN_9
#define I2C_DMA                 DMA0
#define DMA_TX_CH               DMA_CH5
#define DMA_RX_CH               DMA_CH6


/* configure the I2C interfaces */
void i2c0_config(void);
/* reset i2c bus */
void i2c0_bus_reset(void);



#endif

