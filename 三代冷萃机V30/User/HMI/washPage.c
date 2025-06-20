/**
 * 
 * @file washPage.c
 * @author jiaokai 
 * @brief 清洗页面响应函数
 * 
 * @copyright Copyright (c) 2025
 */

#include "washPage.h"
#include "lcd_data_process.h"
#include "uart.h"
#include "string.h"
#include "eeprom.h"
#include "work.h"
#include "timer.h"
#include "config.h"
#include "wash.h"
#include "waterLevel.h"
#include "relay.h"
#include "app_mqtt.h"
#include "app_wifi.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "washPage"

/**
 * ************************************************************************
 * @brief  清洗页面按钮响应函数
 * 
 * @param[in] num  控件编号
 * @param[in] state  控件状态
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void washPageButton(uint8_t num,uint8_t state)
{
    switch(num)
    {
        case 3:
                if(wash.state == 0) //当前空闲
                {
                    //[ ] 注册清洗任务
                    add_task(washProcessControl, NULL,100, true);  //HACK 使用的是模式2，需要测试
                    log_d("add task washProcessControl");
                    waterLevel.drainOption = 0; //排废水
                    AnimationPlayFrame(wash_page,2,1,UART2_ID);
                    SetControlEnable(wash_page,4,0,UART2_ID);//禁用返回键
                    log_v("wash start");
                }
                else if(wash.state  == 1) //当前正在清洗
                {
                    wash.state = 2; //设置为暂停状态
                    recordOutputState();    //XXX 记录当前状态 
                    disable_task(washProcessControl);  //HACK 禁用清洗任务
                    log_d("disable task washProcessControl");
                    washPause();   //暂停清洗
                    wash.pauseStartTime = Timer.system1Sec;   //记录暂停开始时间
                    AnimationPlayFrame(wash_page,2,0,UART2_ID);
                    SetControlEnable(wash_page,4,1,UART2_ID);//使能返回键
                    log_v("wash pause");
                }
                else if(wash.state == 2)//当前在暂停
                {
                    wash.state = 1; //设置为清洗状态
                    AnimationPlayFrame(wash_page,2,1,UART2_ID);
                    SetControlEnable(wash_page,4,0,UART2_ID);//禁用返回键
                    enable_task(washProcessControl);  //HACK 使能清洗任务
                    log_d("enable task washProcessControl");
                    wash.pauseEndTime = Timer.system1Sec;   //记录暂停结束时间
                    wash.pauseTime += (wash.pauseEndTime - wash.pauseStartTime);   //计算暂停时间
                    restoreOutputState();   //XXX 恢复输出状态  
                    log_v("wash continue");
                }
            break;
        case 4:
            if(state == 1)
            {
                SetScreen(main_page,UART2_ID);          //返回主页面
                AnimationPlayFrame(19,1,1,UART2_ID);    //图标改成萃取液
                
                deviceRunState = 0;         //XXX 运行状态
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
                
                if(wash.state != 0) //当前是暂停状态
                {
                    delete_task(washProcessControl);        //[ ]删除任务
                    log_d("delete task washProcessControl");

                }
                washReset();   //清洗复位
            }
            break;
        default:
            break;
    }
}

/**
 * ************************************************************************
 * @brief 显示清洗倒计时
 * 
 * @param[in] Set_time  设置时间
 * @param[in] Remain_time  剩余时间
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-09
 * 
 * ************************************************************************
 */
void washShowTime(uint32_t setTime,uint32_t remainTime)//清洗倒计时
{
    uint8_t lastMin,lastSec;   //剩余分钟秒
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
    uart2_struct.tx_buf[4] = wash_page;   //页面地址
    uart2_struct.tx_buf[5] = 0x00;
    uart2_struct.tx_buf[6] = 0x08;              //控件地址
    uart2_struct.tx_buf[7] = 0x00;
    sprintf(str,"%d",lastMin);
    strLen = strlen(str);
    uart2_struct.tx_buf[8] = strLen;
    for(uint8_t i=0;i<strLen;i++)
    {
        uart2_struct.tx_buf[9+i] = str[i];
    }
    len = 9+strLen;
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len+1] = 0x09;
    uart2_struct.tx_buf[len+2] = 0x00;
    sprintf(str,"%d",lastSec);
    strLen = strlen(str);
    uart2_struct.tx_buf[len+3] = strLen;
    for(uint8_t i=0;i<strLen;i++)
    {
        uart2_struct.tx_buf[len+4+i] = str[i];
    }
    len = len+4+strLen;
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len+1] = 0x05;
    uart2_struct.tx_buf[len+2] = 0x00;
    uart2_struct.tx_buf[len+3] = 0x04;
    uart2_struct.tx_buf[len+4] = 0x00;
    uart2_struct.tx_buf[len+5] = 0x00;
    uart2_struct.tx_buf[len+6] = 0x00;
    if(setTime == 0)
    {
        progressBarValue = 100; //设置时间为0，进度条为100%
    }
    else
    {
        progressBarValue = ((uint32_t)(setTime-remainTime)*100)/setTime;
    }
    if(progressBarValue > 100)
    {
        progressBarValue = 100;
    }
    uart2_struct.tx_buf[len+7] = (uint8_t)progressBarValue;
    len = len+8;
    uart2_struct.tx_buf[len] = 0xFF;
    uart2_struct.tx_buf[len+1] = 0xFC;
    uart2_struct.tx_buf[len+2] = 0xFF;
    uart2_struct.tx_buf[len+3] = 0xFF;
    uart2_struct.tx_count = len+4;
    uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);   //发送数据
}


