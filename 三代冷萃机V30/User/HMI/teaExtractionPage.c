/**
 * 
 * @file teaExtractionPage.c
 * @author jiaokai 
 * @brief 茶萃取页面按钮响应
 * 
 * @copyright Copyright (c) 2025
 */
#include "teaExtractionPage.h"
#include "lcd_data_process.h"
#include "waterLevel.h"
#include "uart.h"
#include "string.h"
#include "teaMake.h"
#include "timer.h"
#include "eeprom.h"
#include "config.h"
#include "work.h"
#include "drainPage.h"
#include "app_mqtt.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "teaExtractionPage"


/**
 * ************************************************************************
 * @brief 茶萃取页面按钮响应
 * 
 * @param[in] num  按钮编号
 * @param[in] state  按钮状态
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void teaExtractionPageButton(uint8_t num,uint8_t state)
{
    switch(num)
    {
        case 3: //开始暂停按钮
            if(tea.state == 0)   //之前停止
            {
                add_task(teaProcessControl,NULL,100, true); //[ ] 添加茶制作任务,100ms执行一次
                log_d("add teaProcessControl task");
                AnimationPlayFrame(make_tea_page,2,1,UART2_ID);   //切换开始暂停图标
                SetControlEnable(make_tea_page,4,0,UART2_ID);     //禁用返回按钮
                SetControlVisiable(make_tea_page,14,0,UART2_ID);  //隐藏排水图标
                SetControlEnable(make_tea_page,15,0,UART2_ID);    //禁用排水按钮
                log_v("tea start");
            }
            else if(tea.state == 1)  //之前运行
            {
                tea.state = 2;
                disable_task(teaProcessControl); //[ ] 暂停茶制作任务
                log_d("disable teaProcessControl task");
                AnimationPlayFrame(make_tea_page,2,0,UART2_ID);   //切换开始暂停图标
                SetControlEnable(make_tea_page,4,1,UART2_ID);     //使能返回按钮
                tea.pauseStartTime = Timer.system1Sec;
                log_v("tea pause");
                teaMakePause();
            }
            else if(tea.state == 2)  //之前停止
            {
                tea.state = 1;
                enable_task(teaProcessControl); //[ ] 继续茶制作任务
                log_d("enable teaProcessControl task");
                AnimationPlayFrame(make_tea_page,2,1,UART2_ID);   //切换开始暂停图标
                SetControlEnable(make_tea_page,4,0,UART2_ID);     //禁用返回按钮
                SetControlVisiable(make_tea_page,14,0,UART2_ID);  //隐藏排水图标
                SetControlEnable(make_tea_page,15,0,UART2_ID);    //禁用排水按钮
                tea.pauseEndTime = Timer.system1Sec;
                tea.pauseTime = tea.pauseTime+(tea.pauseEndTime-tea.pauseStartTime);
                log_v("tea continue");
            }
            break;
        case 4: //返回按钮
            
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
            SetScreen(make_tea_set_page,UART2_ID);   //返回设置页面
            
            if(tea.state != 0)   //当前在暂停状态
            {
                delete_task(teaProcessControl); //[ ] 删除茶制作任务
                log_d("delete teaProcessControl task");
            }
            teaMakeReset();   //重置制作状态
            break;
        case 15: //排水按钮
            AnimationPlayFrame(make_tea_page,10,0,UART2_ID);   //切换排水图标
            if(waterLevel.sensorErr == 1)   //传感器故障
            {
                SetScreen(water_level_page,UART2_ID);
            }
            else
            {
                SetScreen(drain_page,UART2_ID);   //进入排水页面
                drainShowTime(DRAIN_TIME,DRAIN_TIME);   //显示排水时间
            }
            break;
        default:
            
            break;
    }
}

/**
 * ************************************************************************
 * @brief 进入茶萃取页面
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void enterTeaExtractionPage(void)
{
    char str[10];
    uint8_t len;
    uint8_t strLen;
    float volVal;
    makeTeaShowTime(config.data.teaMake.time[tea.num]*60,
                    config.data.teaMake.time[tea.num]*60);   //显示萃取时间
    // 显示时间，水量和排水模式
    len = 0;
    uart2_struct.tx_buf[0] = 0xEE;
    uart2_struct.tx_buf[1] = 0xB1;
    uart2_struct.tx_buf[2] = 0x12;
    uart2_struct.tx_buf[3] = 0x00;
    uart2_struct.tx_buf[4] = make_tea_page;
    /************************************写入时间************************************ */
    uart2_struct.tx_buf[5] = 0x00;
    uart2_struct.tx_buf[6] = 0x0C;              //控件地址
    uart2_struct.tx_buf[7] = 0x00;              //数据长度
    sprintf(str,"%d",config.data.teaMake.time[tea.num]);
    strLen = strlen(str);
    uart2_struct.tx_buf[8] = strLen;
    for(uint8_t i=0;i<strLen;i++)
    {
        uart2_struct.tx_buf[9+i] = str[i];
    }
    len = len+9+strLen;
    /************************************写入水量************************************ */
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len+1] = 0x0D;              //控件地址
    uart2_struct.tx_buf[len+2] = 0x00;              //数据长度
    volVal = (float)config.data.teaMake.vol[tea.num]/10.0f;
    sprintf(str,"%.1f",volVal);
    strLen = strlen(str);
    uart2_struct.tx_buf[len+3] = strLen;
    for(uint8_t i=0;i<strLen;i++)
    {
        uart2_struct.tx_buf[len+4+i] = str[i];
    }
    len = len+4+strLen;
    uart2_struct.tx_buf[len] = 0xFF;
    uart2_struct.tx_buf[len+1] = 0xFC;
    uart2_struct.tx_buf[len+2] = 0xFF;
    uart2_struct.tx_buf[len+3] = 0xFF;
    uart2_struct.tx_count = len+4;
    uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);   //发送数据
    //图标操作
    if(config.data.teaMake.autoDrainangeFlag[tea.num] == 1)
    {
        SetControlVisiable(make_tea_page,10,0,UART2_ID);    
        SetControlVisiable(make_tea_page,11,1,UART2_ID);    
    }
    else
    {
        SetControlVisiable(make_tea_page,10,1,UART2_ID);  
        SetControlVisiable(make_tea_page,11,0,UART2_ID);   
    }
    SetControlVisiable(make_tea_page,14,0,UART2_ID);  //隐藏排水图标
    SetControlEnable(make_tea_page,15,0,UART2_ID);    //禁用排水按钮
}

/**
 * ************************************************************************
 * @brief 茶萃取页面进度条显示
 * 
 * @param[in] setTime  设置时间 秒
 * @param[in] remainTime  剩余时间 秒
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void makeTeaShowTime(uint32_t setTime,uint32_t remainTime)
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
    uart2_struct.tx_buf[4] = make_tea_page;   //页面地址
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
    // 处理秒部分
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
        progressBarValue = ((uint32_t)(setTime-remainTime)*100)/setTime; //计算进度条值
    }
    //确保进度条值不超过255
    if(progressBarValue > 255)
    {
        progressBarValue = 255;
    }
    uart2_struct.tx_buf[len+7] = (uint8_t)progressBarValue; //进度条值
    len = len+8;
    uart2_struct.tx_buf[len] = 0xFF;
    uart2_struct.tx_buf[len+1] = 0xFC;
    uart2_struct.tx_buf[len+2] = 0xFF;
    uart2_struct.tx_buf[len+3] = 0xFF;
    uart2_struct.tx_count = len+4;
    uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);   //发送数据
}

