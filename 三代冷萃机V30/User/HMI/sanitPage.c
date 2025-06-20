/**
 * 
 * @file sanit.c
 * @author jiaokai 
 * @brief 消毒页面响应函数
 * 
 * @copyright Copyright (c) 2025
 */
#include "sanitPage.h"
#include "lcd_data_process.h"
#include "waterLevel.h"
#include "uart.h"
#include "string.h"
#include "sanit.h"
#include "timer.h"
#include "eeprom.h"
#include "config.h"
#include "work.h"
#include "relay.h"
#include "sanit.h"
#include "app_mqtt.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "SANIT"

/**
 * ************************************************************************
 * @brief  消毒页面按钮响应函数
 * 
 * @param[in] num  编号
 * @param[in] state  状态
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void sanitPageButton(uint8_t num,uint8_t state)
{
    switch(num)
    {
        case 3:
            if(state == 1)
            {
                if(sanit.state == 0)//空闲状态
                {
                    waterLevel.drainOption = 0;//排废水
                    AnimationPlayFrame(sanit_page,2,1,UART2_ID);
                    SetControlEnable(sanit_page,4,0,UART2_ID); //设置返回按钮不可用
                    add_task(sanitProcessControl,NULL,100,true); //[ ]添加任务
                    log_d("add sanitProcessControl task");
                    log_v("sanit start");
                }
                else if(sanit.state == 1) //当前是在消毒过程中
                {
                    sanit.state = 2; //设置为暂停状态
                    recordOutputState();    //记录当前输出状态
                    disable_task(sanitProcessControl); //[ ]暂停任务
                    log_d("disable sanitProcessControl task");
                    sanitPause();   //暂停消毒
                    sanit.pauseStartTime = Timer.system1Sec;   //记录暂停开始时间
                    AnimationPlayFrame(sanit_page,2,0,UART2_ID);
                    SetControlEnable(sanit_page,4,1,UART2_ID); //设置返回按钮可用
                    log_v("sanit pause");
                }
                else if(sanit.state == 2) //当前是在暂停状态
                {
                    sanit.state = 1; //设置为继续状态
                    AnimationPlayFrame(sanit_page,2,1,UART2_ID);
                    SetControlEnable(sanit_page,4,0,UART2_ID); //设置返回按钮不可用
                    enable_task(sanitProcessControl);  //[ ] 使能任务
                    log_d("enable sanitProcessControl task");
                    sanit.pauseEndTime = Timer.system1Sec;   //记录暂停结束时间
                    sanit.pauseTime += (sanit.pauseEndTime - sanit.pauseStartTime);   //计算暂停时间
                    restoreOutputState();   //恢复输出状态
                    log_v("sanit continue");
                }
            }
            break;
        case 4:
            if(state == 1)
            {

                SetScreen(main_page,UART2_ID);          //返回主页面
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
                if(sanit.state != 0) //当前是在消毒过程中
                {
                    delete_task(sanitProcessControl); //[ ] 暂停任务
                    log_d("disable sanitProcessControl task");
                }
                sanitReset();   //消毒复位
                
            }
            break;
        default:
            break; 
            
    }
}

/**
 * ************************************************************************
 * @brief  显示消毒倒计时
 * 
 * @param[in] setTime  设置时间
 * @param[in] remainTime  剩余时间
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void sanitShowTime(uint32_t setTime,uint32_t remainTime)
{
    uint8_t lastMin,lastSec;   //剩余分钟秒
    char str[10];               //发送字符串
    uint8_t strLen;             //字符串长度
    uint8_t len;
    uint32_t progressBarValue;  //进度条值
    if(remainTime>SANIT_TIME)  //剩余时间大于设置时间
    {
        remainTime = 1;
    }
    lastMin = remainTime/60;
    lastSec = remainTime%60;
    uart2_struct.tx_buf[0] = 0xEE;
    uart2_struct.tx_buf[1] = 0xB1;
    uart2_struct.tx_buf[2] = 0x12;
    uart2_struct.tx_buf[3] = 0x00;
    uart2_struct.tx_buf[4] = sanit_page;   //页面地址
    //显示分钟部分
    uart2_struct.tx_buf[5] = 0x00;
    uart2_struct.tx_buf[6] = 0x08;
    uart2_struct.tx_buf[7] = 0x00;
    sprintf(str,"%d",lastMin);
    strLen = strlen(str);
    uart2_struct.tx_buf[8] = strLen;
    for(uint8_t i=0;i<strLen;i++)
    {
        uart2_struct.tx_buf[9+i] = str[i];
    }
    len = 9+strLen;
    //显示秒部分
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
    //显示进度条
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
        progressBarValue = ((uint32_t)(setTime-remainTime)*100)/setTime; //计算进度条值
    }
    //确保进度条值不超过255
    if(progressBarValue > 255)
    {
        progressBarValue = 255;
    }
    uart2_struct.tx_buf[len+7] = (uint8_t)progressBarValue; //进度条值
    len = len+8;
    uart2_struct.tx_buf[len] = 0xFF; //结束符
    uart2_struct.tx_buf[len+1] = 0xFC;
    uart2_struct.tx_buf[len+2] = 0xFF;
    uart2_struct.tx_buf[len+3] = 0xFF;
    uart2_struct.tx_count = len+4; //发送数据长度
    uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);   //发送数据
}



