/**
 * 
 * @file coffeeExtractionPage.c
 * @author jiaokai 
 * @brief 咖啡萃取页面响应
 * 
 * @copyright Copyright (c) 2025
 */
#include "coffeeExtractionPage.h"
#include "lcd_data_process.h"
#include "waterLevel.h"
#include "uart.h"
#include "string.h"
#include "coffeeMake.h"
#include "drainPage.h"
#include "eeprom.h"
#include "config.h"
#include "timer.h"
#include "uart.h"
#include "work.h"
#include "drainPage.h"
#include "app_mqtt.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "coffeeExtractionPage"


/**
 * ************************************************************************
 * @brief 咖啡萃取页面按钮响应
 * 
 * @param[in] button 按钮编号
 * @param[in] state 按钮状态
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-10
 * 
 * ************************************************************************
 */
void coffeeExtractionPageButton(uint8_t num,uint8_t state)
{
    switch(num)
    {
        case 3: //开始暂停按钮
            if(coffee.state == 0)   //之前停止
            {
                add_task(coffeeProcessControl,NULL,100, true); //[ ] 添加咖啡制作任务,100ms执行一次
                log_d("add coffeeProcessControl task");
                AnimationPlayFrame(make_coffe_page,2,1,UART2_ID);   //切换开始暂停图标
                SetControlEnable(make_coffe_page,4,0,UART2_ID);     //禁用返回按钮
                SetControlVisiable(make_coffe_page,14,0,UART2_ID);  //隐藏排水图标
                SetControlEnable(make_coffe_page,15,0,UART2_ID);    //禁用排水按钮
                log_v("coffee start");
            }
            else if(coffee.state == 1)  //之前运行
            {
                coffee.state = 2;
                disable_task(coffeeProcessControl); //[ ] 暂停咖啡制作任务
                log_d("disable coffeeProcessControl task");
                AnimationPlayFrame(make_coffe_page,2,0,UART2_ID);   //切换开始暂停图标
                SetControlEnable(make_coffe_page,4,1,UART2_ID);     //使能返回按钮
                coffee.pauseStartTime = Timer.system1Sec;
                log_v("coffee pause");
                coffeeMakePause();
            }
            else if(coffee.state == 2)  //之前停止
            {
                coffee.state = 1;
                enable_task(coffeeProcessControl); //[ ] 继续咖啡制作任务
                log_d("enable coffeeProcessControl task");
                AnimationPlayFrame(make_coffe_page,2,1,UART2_ID);   //切换开始暂停图标
                SetControlEnable(make_coffe_page,4,0,UART2_ID);     //禁用返回按钮
                SetControlVisiable(make_coffe_page,14,0,UART2_ID);  //隐藏排水图标
                SetControlEnable(make_coffe_page,15,0,UART2_ID);    //禁用排水按钮
                coffee.pauseEndTime = Timer.system1Sec;
                coffee.pauseTime = coffee.pauseTime+(coffee.pauseEndTime-coffee.pauseStartTime);
                log_v("coffee continue");
            }
            break;
        case 4: //返回按钮
            
            SetScreen(make_coffe_set_page,UART2_ID);
            deviceRunState = 0;   //XXX 运行状态
            if(WIFI.mqttConnected == 1)
            {
                if(publish_deviceRunState())
                {
                    log_i("publish device run state");
                }
                else
                {
                    log_e("publish device run state failed");
                }
            }
           
            if(coffee.state != 0)
            {
                delete_task(coffeeProcessControl); //[ ] 删除咖啡制作任务
                log_d("delete coffeeProcessControl task");
            }
            coffeeMakeReset();
            break;
        case 15:  ///排水按钮
            AnimationPlayFrame(drain_page,10,0,UART2_ID);   //切换排水图标
            if(waterLevel.sensorErr == 1)   //如果传感器故障
            {
                SetScreen(water_level_page,UART2_ID);
            }
            else
            {
                SetScreen(drain_page,UART2_ID);                 //进入排水页面
                drainShowTime(DRAIN_TIME,DRAIN_TIME);   //显示排水时间
            }
            break;
        default:
            
            break;
    }
}



/**
 * ************************************************************************
 * @brief 进入咖啡萃取页面，刷新页面数据
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-10
 * 
 * ************************************************************************
 */
void enterCoffeeExtractionPage(void)
{
    char str[10] ;
    uint8_t strLen;
    uint8_t len;
    float volVal;
    //显示时间
    makeCoffeeShowTime(config.data.coffeeMake.time[coffee.num]*60,
            config.data.coffeeMake.time[coffee.num]*60);    
     // 显示时间，水量和排水模式
    uart2_struct.tx_buf[0] = 0xEE;
    uart2_struct.tx_buf[1] = 0xB1;
    uart2_struct.tx_buf[2] = 0x12;
    uart2_struct.tx_buf[3] = 0x00;
    uart2_struct.tx_buf[4] = make_coffe_page;
    uart2_struct.tx_buf[5] = 0x00;
    uart2_struct.tx_buf[6] = 0x0C;//时间
    uart2_struct.tx_buf[7] = 0x00;
    sprintf(str,"%d",config.data.coffeeMake.time[coffee.num]);  //XXX 之前是config.data.coffeeMake.currentNumber
    strLen = strlen(str);
    uart2_struct.tx_buf[8] = strLen;
    if(strLen == 1)
    {
        uart2_struct.tx_buf[9] = str[0];
        len = 10;
    }
    else if(strLen == 2)
    {
        uart2_struct.tx_buf[9] = str[0];
        uart2_struct.tx_buf[10] = str[1];
        len = 11;
    }
    else if(strLen == 3)
    {
        uart2_struct.tx_buf[9] = str[0];
        uart2_struct.tx_buf[10] = str[1];
        uart2_struct.tx_buf[12] = str[2];
        len = 13;
    }
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len+1] = 0x0D;//水量
    uart2_struct.tx_buf[len+2] = 0x00;
    len = len+3;
    volVal = (float)config.data.coffeeMake.vol[coffee.num]/10;
    sprintf(str,"%.1f",volVal);
    strLen = strlen(str);
    uart2_struct.tx_buf[len] = strLen;
    if(strLen == 3)
    {
        uart2_struct.tx_buf[len+1] = str[0];
        uart2_struct.tx_buf[len+2] = str[1];
        uart2_struct.tx_buf[len+3] = str[2];
        len = len+4; 
    }
    uart2_struct.tx_buf[len] = 0xFF;
    uart2_struct.tx_buf[len+1] = 0xFC;
    uart2_struct.tx_buf[len+2] = 0xFF;
    uart2_struct.tx_buf[len+3] = 0xFF;
    uart2_struct.tx_count = len+4;
    uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);
     
    //图标操作
    if(config.data.coffeeMake.autoDrainangeFlag[coffee.num] == 1)
    {
        SetControlVisiable(make_coffe_page,10,0,UART2_ID);
        SetControlVisiable(make_coffe_page,11,1,UART2_ID);
    }
    else
    {
        SetControlVisiable(make_coffe_page,10,1,UART2_ID);
        SetControlVisiable(make_coffe_page,11,0,UART2_ID);
    }
    SetControlVisiable(make_coffe_page,14,0,UART2_ID);//隐藏排水图标
    SetControlEnable(make_coffe_page,15,0,UART2_ID);//禁用排水按钮
}

/**
 * ************************************************************************
 * @brief 咖啡萃取页面进度条显示
 * 
 * @param[in] setTime  设置时间 秒
 * @param[in] remainTime  剩余时间 秒
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-10
 * 
 * ************************************************************************
 */
void makeCoffeeShowTime(uint32_t setTime,uint32_t remainTime)
{
    uint16_t lastMin,lastSec;   //剩余分钟秒
    char str[10];               //发送字符串
    uint8_t strLen;             //字符串长度
    uint8_t len;
    uint32_t progressBarValue;  //进度条值
    if(remainTime>setTime)  //剩余时间大于设置时间
    {
        remainTime = 1;
    }
    lastMin = remainTime/60;
    lastSec = remainTime%60;
    uart2_struct.tx_buf[0] = 0xEE;
    uart2_struct.tx_buf[1] = 0xB1;
    uart2_struct.tx_buf[2] = 0x12;
    uart2_struct.tx_buf[3] = 0x00;
    uart2_struct.tx_buf[4] = make_coffe_page;   //页面地址
    uart2_struct.tx_buf[5] = 0x00;
    uart2_struct.tx_buf[6] = 0x08;              //控件地址
    uart2_struct.tx_buf[7] = 0x00;
    sprintf(str,"%d",lastMin);
    strLen = strlen(str);
    uart2_struct.tx_buf[8] = strLen;
    if(strLen == 1)
    {
        uart2_struct.tx_buf[9] = str[0];
        len=9;
    }
    else if(strLen == 2)
    {
        uart2_struct.tx_buf[9] = str[0];
        uart2_struct.tx_buf[10] = str[1];
        len=10;
    }
    uart2_struct.tx_buf[len+1] = 0x00;
    uart2_struct.tx_buf[len+2] = 0x09;
    uart2_struct.tx_buf[len+3] = 0x00;
    len = len+3;

    // 处理秒部分
    sprintf(str,"%d",lastSec);
    strLen = strlen(str);
    uart2_struct.tx_buf[len+1] = strLen;
    if(strLen == 1)
    {
        uart2_struct.tx_buf[len+2] = str[0];
        len+=2;
    }
    else if(strLen == 2)
    {
        uart2_struct.tx_buf[len+2] = str[0];
        uart2_struct.tx_buf[len+3] = str[1];
        len+=3;
    }
    uart2_struct.tx_buf[len+1] = 0x00;
    uart2_struct.tx_buf[len+2] = 0x05;
    uart2_struct.tx_buf[len+3] = 0x00;
    uart2_struct.tx_buf[len+4] = 0x04;
    uart2_struct.tx_buf[len+5] = 0x00;
    uart2_struct.tx_buf[len+6] = 0x00;
    uart2_struct.tx_buf[len+7] = 0x00;
    if(setTime == 0)
    {
        progressBarValue = 100;  // 若 setTime 为 0，进度条设为 100%
    }
    else
    {
        progressBarValue = ((uint32_t)(setTime - remainTime) * 100) / setTime;
    }
     // 确保进度条值不超过 255
    if (progressBarValue > 255) 
    {
        progressBarValue = 255;
    }
    uart2_struct.tx_buf[len+8] = (uint8_t)progressBarValue;
    len += 8;

    uart2_struct.tx_buf[len+1] = 0xFF;  //帧尾
    uart2_struct.tx_buf[len+2] = 0xFC;
    uart2_struct.tx_buf[len+3] = 0xFF;
    uart2_struct.tx_buf[len+4] = 0xFF;
    uart2_struct.tx_count = len+5;
    uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);
}   


