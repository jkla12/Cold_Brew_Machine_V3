#include "ds18b20.h"
#include "systick.h"

const unsigned char CrcTable[256] = {
    0,94,188,226,97,63,221,131,194,156,126,32,163,253,31,65,
    157,195,33,127,252,162,64,30,95,1,227,189,62,96,130,220,
    35,125,159,193,66,28,254,160,225,191,93,3,128,222,60,98,
    190,224,2,92,223,129,99,61,124,34,192,158,29,67,161,255,
    70,24,250,164,39,121,155,197,132,218,56,102,229,187,89,7,
    219,133,103,57,186,228,6,88,25,71,165,251,120,38,196,154,
    101,59,217,135,4,90,184,230,167,249,27,69,198,152,122,36,
    248,166,68,26,153,199,37,123,58,100,134,216,91,5,231,185,
    140,210,48,110,237,179,81,15,78,16,242,172,47,113,147,205,
    17,79,173,243,112,46,204,146,211,141,111,49,178,236,14,80,
    175,241,19,77,206,144,114,44,109,51,209,143,12,82,176,238,
    50,108,142,208,83,13,239,177,240,174,76,18,145,207,45,115,
    202,148,118,40,171,245,23,73,8,86,180,234,105,55,213,139,
    87,9,235,181,54,104,138,212,149,203,41,119,244,170,72,22,
    233,183,85,11,136,214,52,106,43,117,151,201,74,20,246,168,
    116,42,200,150,21,75,169,247,182,232,10,84,215,137,107,53
};


void ds18b20_init(void)
{

    rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
    gpio_init(DS18B20_PORT1, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, DS18B20_PIN1);    //配置为输出
    gpio_bit_write(DS18B20_PORT1, DS18B20_PIN1, 0);//输出低
    delay_1us(490);
    gpio_bit_write(DS18B20_PORT1, DS18B20_PIN1, 1);//输出高
    delay_1us(60);
    gpio_init(DS18B20_PORT1, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, DS18B20_PIN1);    //配置为输出
    delay_1us(40);
    if(gpio_input_bit_get(DS18B20_PORT1, DS18B20_PIN1) == SET)
    {
        // DS18B20检查成功
        
    }
    else
    {
        // DS18B20检测失败
    }
    delay_1us(200);
    //gpio_bit_write(DS18B20_PORT1, DS18B20_PIN1, 1);//输出高
}

// void ds18b20_reset(void)
// {
//     gpio_bit_write(DS18B20_PORT1, DS18B20_PIN1, 0);//输出低
//     delay_1us(500);
//     gpio_bit_write(DS18B20_PORT1, DS18B20_PIN1, 1);//输出高
//     delay_1us(500);
// }

void ds18b20_write(uint8_t data)
{
   uint8_t i;
   gpio_init(DS18B20_PORT1, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, DS18B20_PIN1);    //配置为输出
   for (i = 0; i < 8; i++)
   {
       // Write the least significant bit first
        gpio_bit_reset(DS18B20_PORT1, DS18B20_PIN1); // Pull low
        delay_1us(1);  // Wait for a short time
        if(data&0x01)
        {
            gpio_bit_set(DS18B20_PORT1, DS18B20_PIN1); // Release the bus
        }
        else
        {
            gpio_bit_reset(DS18B20_PORT1, DS18B20_PIN1);
        }
        delay_1us(59);
        gpio_bit_set(DS18B20_PORT1, DS18B20_PIN1); 
        delay_1us(3);
        data >>= 1; 
   }
}

uint8_t ds18b20_read(void)
{
   uint8_t i, value = 0;
   for (i = 0; i < 8; i++)
   {
        value>>=1;
        gpio_init(DS18B20_PORT1, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, DS18B20_PIN1);    //配置为输出
       gpio_bit_reset(DS18B20_PORT1, DS18B20_PIN1);  // Pull low
       delay_1us(2);
       gpio_bit_set(DS18B20_PORT1, DS18B20_PIN1);    // Release the bus
       delay_1us(8); // Wait for the bit to be read
       gpio_init(DS18B20_PORT1, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, DS18B20_PIN1);    //配置为输入
       if (gpio_input_bit_get(DS18B20_PORT1, DS18B20_PIN1) == SET)
       {
           value |= 0x80; // Set the bit in the result
       }
       delay_1us(50); // Wait for the rest of the cycle
    //     gpio_init(DS18B20_PORT1, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, DS18B20_PIN1);    //配置为输出
    //     gpio_bit_set(DS18B20_PORT1, DS18B20_PIN1);    // Release the bus
    //    delay_1us(2); // Wait for the bit to be read
   }
   return value;
}

void ds18b20_readBytes(uint8_t num, uint8_t *data)
{
    for (int i = 0; i < num; i++)
    {
        data[i] = ds18b20_read();
    }
}

unsigned char ds18b20_crc8(unsigned char *data, unsigned char len)
{
    unsigned char crc = 0;
    for (int i = 0; i < len; i++)
    {
        crc = CrcTable[crc ^ data[i]];
    }
    return crc;
}

//读取温度，返回温度值，执行一次5.65ms
float  ds18b20_readTemp(void)
{

    uint16_t tempI;
    float tempF;
    uint8_t data[9];

    ds18b20_init();
    ds18b20_write(0xcc);
    ds18b20_write(0x44);
    delay_1us(750);
    ds18b20_init();
    ds18b20_write(0xcc);
    ds18b20_write(0xbe);
    ds18b20_readBytes(2, data);

    //计算DS18B20温度
    tempI = (data[1] << 8) | data[0];
    //判断前四位是否为1
    if(tempI >0x8000)
    {
        //计算温度为负时候的温度
        tempI = (tempI ^ 0xffff) + 1;  // 对补码取反
        tempF = (float)tempI * -0.0625f;
    }
    else
    {
        tempF = (float)tempI * 0.0625f;
    }
    if(tempF < -55.0f)
    {
        tempF = 1000.0f;
    }
    else if (tempF > 125.0f)
    {
        tempF = 1000.0f;
    }
    //温度输出1000时，表示温度读取异常
    return tempF;
}

