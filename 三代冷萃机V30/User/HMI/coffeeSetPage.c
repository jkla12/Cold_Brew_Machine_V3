/**
 * 
 * @file coffeeSetPage.c
 * @author jiaokai 
 * @brief 咖啡萃取设置页面响应
 * 
 * @copyright Copyright (c) 2025
 */
#include "coffeeSetPage.h"
#include "lcd_data_process.h"
#include "waterLevel.h"
#include "uart.h"
#include "string.h"
#include "coffeeMake.h"
#include "coffeeExtractionPage.h"
#include "work.h"
#include "stdlib.h"
#include "config.h"
#include "relay.h"
#include <stddef.h> // For offsetof macro
#include "elog.h"
#include "eeprom.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "CoffeeSetPage"

uint8_t saveCoffeeDataChangeFlag = 0; //数据变更标志
/**
 * ************************************************************************
 * @brief 咖啡页面按钮响应函数
 * 
 * @param[in] num  控件编号
 * @param[in] state  控件状态
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-10
 * 
 * ************************************************************************
 */
void coffeeSetPageButton(uint8_t num,uint8_t state)
{
    char str[10];
    switch(num)
    {
        case 4://返回主页面
            //如果数据变更,保存数据到EEPROM
            if(saveCoffeeDataChangeFlag == 1)
            {
                saveCoffeeDataChangeFlag = 0; //清除数据变更标志
                write_config_data(CONFIG_COFFEE_MAKE);
            }
            else if(config.data.coffeeMake.currentNumber != coffee.num)
            {
                config.data.coffeeMake.currentNumber = coffee.num;
                saveCoffeeCurrentNum();
            }
            break;
        case 5: //进入设置页面
            //如果数据变更,保存数据到EEPROM
            if(saveCoffeeDataChangeFlag == 1)
            {
                saveCoffeeDataChangeFlag = 0; //清除数据变更标志
                write_config_data(CONFIG_COFFEE_MAKE);
            }
            else if(config.data.coffeeMake.currentNumber != coffee.num)
            {
                config.data.coffeeMake.currentNumber = coffee.num;
                saveCoffeeCurrentNum();
            }
            
            if(waterLevel.sensorErr == 1)   //传感器故障
            {
                SetScreen(water_level_page,UART2_ID);
            }
            else
            {
                enterCoffeeExtractionPage();
                SetScreen(make_coffe_page,UART2_ID);
            }
            break;
        case 9:     //液量减少
            if(config.data.coffeeMake.vol[coffee.num]> config.data.waterVolumeLow)
            {
                int16_t newVol = config.data.coffeeMake.vol[coffee.num] - config.data.waterChangeVal;
                config.data.coffeeMake.vol[coffee.num] = (newVol >= config.data.waterVolumeLow) ? newVol : config.data.waterVolumeLow;
            };
            log_v("vol:%d",config.data.coffeeMake.vol[coffee.num]);
            saveCoffeeDataChangeFlag = 1; //数据变更标志
            //屏幕更新数据
            sprintf((char *)str,"%d.%d",config.data.coffeeMake.vol[coffee.num]/10,config.data.coffeeMake.vol[coffee.num]%10);
            SetTextValue(make_coffe_set_page,vol_addr,(unsigned char *)str,UART2_ID);
            break;
        case 10:    //液量增加
            if(config.data.coffeeMake.vol[coffee.num] < config.data.waterVolumeHigh)
            {
                int16_t newVol = config.data.coffeeMake.vol[coffee.num] + config.data.waterChangeVal;
                config.data.coffeeMake.vol[coffee.num] = (newVol <= config.data.waterVolumeHigh) ? newVol :config.data.waterVolumeHigh;
            }
            log_v("vol:%d",config.data.coffeeMake.vol[coffee.num]);
            saveCoffeeDataChangeFlag = 1; //数据变更标志
            sprintf((char *)str,"%d.%d",config.data.coffeeMake.vol[coffee.num]/10,config.data.coffeeMake.vol[coffee.num]%10);
            SetTextValue(make_coffe_set_page,vol_addr,(unsigned char *)str,UART2_ID);
            break;
        case 11:    //时间减少
            if(config.data.coffeeMake.time[coffee.num] > EXTRACTION_MIN_TIME)
            {
                int16_t newTime = config.data.coffeeMake.time[coffee.num] - config.data.makeChangeTime;
                config.data.coffeeMake.time[coffee.num] = (newTime >= EXTRACTION_MIN_TIME) ? newTime : EXTRACTION_MIN_TIME;
            }
            log_v("time:%d",config.data.coffeeMake.time[coffee.num]);
            saveCoffeeDataChangeFlag = 1; //数据变更标志
            sprintf((char *)str,"%d",config.data.coffeeMake.time[coffee.num]);
            SetTextValue(make_coffe_set_page,time_addr,(unsigned char *)str,UART2_ID);
            break;
        case 12:    //时间增加
            if(config.data.coffeeMake.time[coffee.num] < EXTRACTION_MAX_TIME)
            {
                int16_t newTime = config.data.coffeeMake.time[coffee.num] + config.data.makeChangeTime;
                config.data.coffeeMake.time[coffee.num] = (newTime <= EXTRACTION_MAX_TIME) ? newTime : EXTRACTION_MAX_TIME;
            }
            log_v("time:%d",config.data.coffeeMake.time[coffee.num]);
            saveCoffeeDataChangeFlag = 1; //数据变更标志
            sprintf((char *)str,"%d",config.data.coffeeMake.time[coffee.num]);
            SetTextValue(make_coffe_set_page,time_addr,(unsigned char *)str,UART2_ID);
            break;
        case 18:    //收藏
            coffeeCollectChange(coffee.num,state);
            break;
        case 19:    //排水选择
            if(state == 0)  //开启自动排水
            {
                log_i("%d开启自动排水",coffee.num);
                config.data.coffeeMake.autoDrainangeFlag[coffee.num] = 1;
            }
            else //关闭自动排水
            {
                log_i("%d关闭自动排水",coffee.num);
                config.data.coffeeMake.autoDrainangeFlag[coffee.num] = 0;
            }
            saveCoffeeDataChangeFlag = 1; //数据变更标志
            break;
        case 20:    //排水选择
            if(state == 0)//关闭自动排水
            {
                log_i("%d关闭自动排水",coffee.num);
                config.data.coffeeMake.autoDrainangeFlag[coffee.num] = 0;
            }
            else    //开启自动排水
            {
                log_i("%d开启自动排水",coffee.num);
                config.data.coffeeMake.autoDrainangeFlag[coffee.num] = 1;
            }
            saveCoffeeDataChangeFlag = 1; //数据变更标志
            break;
        case collect1_button_addr:    //收藏按钮1
            coffeeCollectChoose(collect1_button_addr);
            break;
        case collect2_button_addr:    //收藏按钮2
            coffeeCollectChoose(collect2_button_addr);
            break;
        case collect3_button_addr:    //收藏按钮3
            coffeeCollectChoose(collect3_button_addr);
            break;
        case collect4_button_addr:    //收藏按钮4
            coffeeCollectChoose(collect4_button_addr);
            break;
        case 26:    //克重减少
            if(config.data.coffeeMake.weight[coffee.num] > config.data.weightMin)
            {
                int16_t newWeight = config.data.coffeeMake.weight[coffee.num] - config.data.weightChangeVal;
                config.data.coffeeMake.weight[coffee.num] = (newWeight >= config.data.weightMin) ? newWeight : config.data.weightMin;
            }
            log_v("weight:%d",config.data.coffeeMake.weight[coffee.num]);
            sprintf((char *)str,"%d",config.data.coffeeMake.weight[coffee.num]);
            SetTextValue(make_coffe_set_page,weight_addr,(unsigned char *)str,UART2_ID);
            saveCoffeeDataChangeFlag = 1; //数据变更标志
            break;
        case 27:    //克重增加
            if(config.data.coffeeMake.weight[coffee.num] < config.data.weightMax)
            {
                int16_t newWeight = config.data.coffeeMake.weight[coffee.num] + config.data.weightChangeVal;
                config.data.coffeeMake.weight[coffee.num] = (newWeight <= config.data.weightMax) ? newWeight : config.data.weightMax;
            }
            log_v("weight:%d",config.data.coffeeMake.weight[coffee.num]);
            sprintf((char *)str,"%d",config.data.coffeeMake.weight[coffee.num]);
            SetTextValue(make_coffe_set_page,weight_addr,(unsigned char *)str,UART2_ID);
            saveCoffeeDataChangeFlag = 1; //数据变更标志
            break;
        default:

            break;
    }
}

/**
 * ************************************************************************
 * @brief 咖啡设置页面文本响应函数
 * 
 * @param[in] num  控件编号
 * @param[in] str  传入的字符串
 * 
 * 
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void coffeeSetPageText(uint16_t num,uint8_t *str)
{
    uint8_t val[3];
    if(num == 13)   //编号选择，1-10
    {
        val[0] = atoi((char *)str); //将字符串转为整数
        //限幅
        val[0] = (val[0] >=1 )? val[0] : 1;
        val[0] = (val[0] <=10 )? val[0] : 10;
        // config.data.coffeeMake.currentNumber = val[0]-1; //编号从0开始
        // coffee.num = config.data.coffeeMake.currentNumber;
        coffee.num = val[0]-1; //编号从0开始
        loadingCoffeeMakeSet();
        log_v("coffee make num:%d",coffee.num);
    }
}
/**
 * ************************************************************************
 * @brief   保存当前咖啡编号到eeprom
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void saveCoffeeCurrentNum(void)
{
    eeprom_buffer_write_timeout((uint8_t*)&config.data.coffeeMake.currentNumber, offsetof(config_TypeDef, coffeeMake.currentNumber), sizeof(config.data.coffeeMake.currentNumber));
    log_v("save current num:%d",config.data.coffeeMake.currentNumber);
}

/**
 * ************************************************************************
 * @brief 更新咖啡收藏状态
 * 
 * @param[in] num  编号
 * @param[in] state  状态：0取消收藏，1收藏
 * 
 * 
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-12
 * 
 * ************************************************************************
 */
void coffeeCollectChange(uint8_t num,uint8_t state)
{
    if(state == 1) //收藏
    {
        if(config.data.coffeeMake.collectFlag[NUMBER_OF_COLLECTIONS-1] == 0)  //如果收藏列表的最后一个为空
        {
            config.data.coffeeMake.collectFlag[NUMBER_OF_COLLECTIONS-1] = num+1;
        }
        else if(find_num(config.data.coffeeMake.collectFlag,num+1) == 0)//如果收藏列表中不存在该收藏
        {
            for(uint8_t i=0;i<NUMBER_OF_COLLECTIONS-1;i++)
            {
                config.data.coffeeMake.collectFlag[i] = config.data.coffeeMake.collectFlag[i+1];
            }
            config.data.coffeeMake.collectFlag[NUMBER_OF_COLLECTIONS-1] = num+1;
        }
    }
    else    //取消收藏
    {
        if(find_num(config.data.coffeeMake.collectFlag,num+1) == 1) //如果收藏列表中存在该收藏
        {
            for(uint8_t i = 0;i<NUMBER_OF_COLLECTIONS;i++)
            {
                if(config.data.coffeeMake.collectFlag[i] == num+1)
                {
                    for(uint8_t j = i;j<NUMBER_OF_COLLECTIONS-1;j++)
                    {
                        config.data.coffeeMake.collectFlag[j] = config.data.coffeeMake.collectFlag[j+1];
                    }
                    config.data.coffeeMake.collectFlag[NUMBER_OF_COLLECTIONS-1] = 0;
                }
            }
        }
    }
    UpdateCoffeeCollect();
    saveCoffeeDataChangeFlag = 1; //数据变更标志
}

/**
 * ************************************************************************
 * @brief 屏幕更新收藏数据
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-12
 * 
 * ************************************************************************
 */
void UpdateCoffeeCollect(void)
{
    uint8_t len = 0;
    uint8_t str[10];
    uint8_t strLen = 0;
    uart2_struct.tx_buf[0] = 0xEE;
    uart2_struct.tx_buf[1] = 0xB1;
    uart2_struct.tx_buf[2] = 0x12;
    uart2_struct.tx_buf[3] = 0x00;
    uart2_struct.tx_buf[4] = make_coffe_set_page;
    len = 5;
    for (uint8_t i = 0;i<NUMBER_OF_COLLECTIONS;i++)
    {
        uart2_struct.tx_buf[len] = 0x00;
        uart2_struct.tx_buf[len+1] = collect1_addr+i;
        uart2_struct.tx_buf[len+2] = 0x00;
        len += 3;
        memset(str,0,10);
        sprintf((char *)str,"%d",config.data.coffeeMake.collectFlag[i]);
        strLen = strlen((char *)str);
        uart2_struct.tx_buf[len] = strLen;
        for (uint8_t j = 0;j<strLen;j++)
        {
            uart2_struct.tx_buf[len+j+1] = str[j];
        }
        len += strLen+1;
    }
    uart2_struct.tx_buf[len] = 0xFF;
    uart2_struct.tx_buf[len+1] = 0xFC;
    uart2_struct.tx_buf[len+2] = 0xFF;
    uart2_struct.tx_buf[len+3] = 0xFF;
    len += 4;
    uart2_dma_send(uart2_struct.tx_buf,len);

}

/**
 * ************************************************************************
 * @brief 保存新的收藏数据到eeprom中
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-12
 * 
 * ************************************************************************
 */
void saveCoffeeCollect(void)
{
    //write_config_data(CONFIG_COFFEE_MAKE);
    eeprom_buffer_write_timeout((uint8_t*)&config.data.coffeeMake.collectFlag[0], offsetof(config_TypeDef, coffeeMake.collectFlag), sizeof(config.data.coffeeMake.collectFlag));
}

/**
 * ************************************************************************
 * @brief 咖啡收藏选择
 * 
 * @param[in] num  编号
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-12
 * 
 * ************************************************************************
 */
void coffeeCollectChoose(uint8_t num)
{
    char str[10];
    coffee.num = config.data.coffeeMake.collectFlag[num-21]-1;
    //config.data.coffeeMake.currentNumber = coffee.num;
    sprintf((char *)str,"%d",coffee.num+1);
    SetTextValue(make_coffe_set_page,show_addr,(unsigned char *)str,UART2_ID);
    loadingCoffeeMakeSet();
}

/**
 * ************************************************************************
 * @brief 载入咖啡设置页面数据
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-12
 * 
 * ************************************************************************
 */
void loadingCoffeeMakeSet(void)
{
    char str[10];
    uint8_t len;
    uint8_t strLen;
    uart2_struct.tx_buf[0] = 0xEE;
    uart2_struct.tx_buf[1] = 0xB1;
    uart2_struct.tx_buf[2] = 0x12;
    uart2_struct.tx_buf[3] = 0x00;
    uart2_struct.tx_buf[4] = make_coffe_set_page;
    uart2_struct.tx_buf[5] = 0x00;
    uart2_struct.tx_buf[6] = vol_addr;
    uart2_struct.tx_buf[7] = 0x00;
    uart2_struct.tx_buf[8] = 0x03;  //数据长度
    len = 9;
    /************************************写入水量************************************ */
    //数据限幅
    config.data.coffeeMake.vol[coffee.num] = (config.data.coffeeMake.vol[coffee.num]>=config.data.waterVolumeLow)?config.data.coffeeMake.vol[coffee.num]:config.data.waterVolumeLow;
    config.data.coffeeMake.vol[coffee.num] = (config.data.coffeeMake.vol[coffee.num]<=config.data.waterVolumeHigh)?config.data.coffeeMake.vol[coffee.num]:config.data.waterVolumeHigh;
    sprintf((char *)str,"%d.%d",config.data.coffeeMake.vol[coffee.num]/10,config.data.coffeeMake.vol[coffee.num]%10);
    uart2_struct.tx_buf[9] = str[0];
    uart2_struct.tx_buf[10] = str[1];
    uart2_struct.tx_buf[11] = str[2];
    len = len+3;
    /************************************写入时间************************************ */
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len+1] = time_addr;
    uart2_struct.tx_buf[len+2] = 0x00;
    //数据限幅
    config.data.coffeeMake.time[coffee.num] = (config.data.coffeeMake.time[coffee.num]>=EXTRACTION_MIN_TIME)?config.data.coffeeMake.time[coffee.num]:EXTRACTION_MIN_TIME;
    config.data.coffeeMake.time[coffee.num] = (config.data.coffeeMake.time[coffee.num]<=EXTRACTION_MAX_TIME)?config.data.coffeeMake.time[coffee.num]:EXTRACTION_MAX_TIME;
    memset(str,0,sizeof(str));
    sprintf((char *)str,"%d",config.data.coffeeMake.time[coffee.num]);
    strLen = strlen((char *)str);
    uart2_struct.tx_buf[len+3] = strLen;
    for(uint8_t i=0;i<strLen;i++)
    {
        uart2_struct.tx_buf[len+4+i] = str[i];
    }
    len = len+4+strLen;
    /************************************写入重量************************************ */
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len+1] = weight_addr;
    uart2_struct.tx_buf[len+2] = 0x00;
    //数据限幅
    config.data.coffeeMake.weight[coffee.num] = (config.data.coffeeMake.weight[coffee.num]<=WEIGHT_MAX)?config.data.coffeeMake.weight[coffee.num]:WEIGHT_MAX;
    memset(str,0,sizeof(str));
    sprintf(str,"%d",config.data.coffeeMake.weight[coffee.num]);
    strLen = strlen(str);
    uart2_struct.tx_buf[len+3] = strLen;
    for(uint8_t i=0;i<strLen;i++)
    {
        uart2_struct.tx_buf[len+4+i] = str[i];
    }
    len = len+4+strLen;
    /************************************写入是否收藏************************************ */
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len+1] = 0x12;
    uart2_struct.tx_buf[len+2] = 0x00;
    uart2_struct.tx_buf[len+3] =0x01;
    if(find_num(config.data.coffeeMake.collectFlag,coffee.num+1) == 1)
    {
        uart2_struct.tx_buf[len+4] = 0x01;
    }
    else
    {
        uart2_struct.tx_buf[len+4] = 0x00;

    }
    len = len+5;
    /************************************写入排水模式************************************ */
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len+1] = 0x13;
    uart2_struct.tx_buf[len+2] = 0x00;
    uart2_struct.tx_buf[len+3] =0x01;
    if(config.data.coffeeMake.autoDrainangeFlag[coffee.num] == 1)
    {
        uart2_struct.tx_buf[len+4] = 0x00;
        uart2_struct.tx_buf[len+5] = 0x00;
        uart2_struct.tx_buf[len+6] = 0x14;
        uart2_struct.tx_buf[len+7] = 0x00;
        uart2_struct.tx_buf[len+8] = 0x01;
        uart2_struct.tx_buf[len+9] = 0x01;
    }
    else
    {
        uart2_struct.tx_buf[len+4] = 0x01;
        uart2_struct.tx_buf[len+5] = 0x00;
        uart2_struct.tx_buf[len+6] = 0x14;
        uart2_struct.tx_buf[len+7] = 0x00;
        uart2_struct.tx_buf[len+8] = 0x01;
        uart2_struct.tx_buf[len+9] = 0x00;
    }
    len = len+10;
    uart2_struct.tx_buf[len] = 0xFF;    //写入帧尾
    uart2_struct.tx_buf[len+1] = 0xFC;
    uart2_struct.tx_buf[len+2] = 0xFF;
    uart2_struct.tx_buf[len+3] = 0xFF;
    len = len+4;
    uart2_dma_send(uart2_struct.tx_buf,len);
}

