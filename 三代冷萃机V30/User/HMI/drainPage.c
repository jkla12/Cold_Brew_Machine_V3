/**
 * 
 * @file drainPage.c
 * @author jiaokai 
 * @brief 排水页面响应函数
 * 
 * @copyright Copyright (c) 2025
 */
#include "drainPage.h"
#include "lcd_data_process.h"
#include "waterlevel.h"
#include "uart.h"
#include "drain.h"
#include "timer.h"
#include "work.h"
#include "drain.h"
#include "string.h"
#include "config.h"
#include "app_mqtt.h"
#include "elog.h"
#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "drainPage"

/**
 * ************************************************************************
 * @brief 排水页面按钮响应函数
 * 
 * @param[in] num  Comment
 * @param[in] state  Comment
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void drainPageButton(uint8_t num,uint8_t state)
{
	switch(num)
	{
		case 3:
			if(state == 1)
			{
				if(drain.state == 0) //当前没有排水
				{
					add_task(drainProcessControl,NULL,100, true); //FIXME 添加排水任务,100ms执行一次
					drain.startTime = Timer.system1Sec; //记录开始时间
					//drain.state = 1; //设置为排水状态
					AnimationPlayFrame(drain_page,2,1,UART2_ID);
                    SetControlEnable(drain_page,4,0,UART2_ID);//禁用返回键
                    SetControlEnable(drain_page,11,0,UART2_ID);//禁用排水选择按钮
                    log_v("开始排水");
				}
				else if(drain.state == 1) //当前是排水状态
				{
					drain.state = 2; //设置为暂停状态
					AnimationPlayFrame(drain_page,2,0,UART2_ID);
                    SetControlEnable(drain_page,4,1,UART2_ID);//使能返回键
					drain.pauseStartTime = Timer.system1Sec; //记录暂停开始时间
					waterDrainStop(); //停止排水
					disable_task(drainProcessControl); //FIXME 暂停排水任务
					log_d("disable task drainProcessControl");
					log_v("暂停排水");
				}
				else if(drain.state == 2) //当前是暂停状态
				{
					drain.state = 1; //设置为排水状态
					enable_task(drainProcessControl); //FIXME 继续排水任务
					log_d("enable task drainProcessControl");
					AnimationPlayFrame(drain_page,2,1,UART2_ID);
					SetControlEnable(drain_page,4,0,UART2_ID);//禁用返回键
					drain.pauseEndTime = Timer.system1Sec; //记录暂停结束时间
					drain.pauseTime += (drain.pauseEndTime - drain.pauseStartTime); //计算暂停时间
					log_v("继续排水");
				}
			}
			break;
		case 4:
			deviceRunState = 0;   //XXX 运行状态
			if(WIFI.mqttConnected == 1)
			{
				if(publish_drainRelatData())
				{
					log_i("publish drain related data");
				}
				else
				{
					log_e("publish drain related data failed");
				}
			}
			SetScreen(setting_page,UART2_ID);   //切换到设置页面
			if(drain.state != 0)
			{
				delete_task(drainProcessControl);   //删除排水任务
				log_d("delete task drainProcessControl");
			}
			drainReset();   //重置排水状态
			break;
		default:
			break;
	}
}


/**
 * ************************************************************************
 * @brief 排水选择页面按钮响应
 * 
 * @param[in] num  Comment
 * @param[in] state  Comment
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void drainChoosePageButton(uint8_t num,uint8_t state)
{
	switch(num)
	{
		case 2:
			if(state == 1)
			{
				AnimationPlayFrame(drain_page,10,1,UART2_ID);
				waterLevel.drainOption = 0;   //选择排水
				drainShowTime(DRAIN_TIME,DRAIN_TIME);   //显示排水时间
				SetScreen(drain_page,UART2_ID);   //切换到排水页面
			}
			break;
		case 3:
			if(state == 1)
			{
				AnimationPlayFrame(drain_page,10,0,UART2_ID);
				waterLevel.drainOption = 1;   //选择取水
				drainShowTime(DRAIN_TIME,DRAIN_TIME);   //显示排水时间
				SetScreen(drain_page,UART2_ID);   //切换到排水页面
			}
			break;
		default:
			break;
	}
}



/**
 * ************************************************************************
 * @brief 排水进度条显示
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
void drainShowTime(uint32_t setTime,uint32_t remainTime)
{
    uint8_t lastMin,lastSec;
    uint8_t strLen;
    uint8_t len;
    char str[10];
    uint32_t progressBarVal;    //进度条值
    lastMin = remainTime/60;
    lastSec = remainTime%60;
    uart2_struct.tx_buf[0] = 0xEE;
	uart2_struct.tx_buf[1] = 0xB1;
	uart2_struct.tx_buf[2] = 0x12;
	uart2_struct.tx_buf[3] = 0x00;
	uart2_struct.tx_buf[4] = drain_page;
	uart2_struct.tx_buf[5] = 0x00;
	uart2_struct.tx_buf[6] = 0x08;
	uart2_struct.tx_buf[7] = 0x00;
	sprintf(str,"%d",lastMin);
	strLen = strlen(str);
	uart2_struct.tx_buf[8] = strLen;
    for(uint8_t i = 0;i < strLen;i++)
    {
        uart2_struct.tx_buf[9+i] = str[i];
    }
    len  = strLen + 9;
    uart2_struct.tx_buf[len] = 0x00;
	uart2_struct.tx_buf[len+1] = 0x09;
	uart2_struct.tx_buf[len+2] = 0x00;
	sprintf(str,"%d",lastSec);
	strLen = strlen(str);
	uart2_struct.tx_buf[len+3] = strLen;
    len = len+4;
    for(uint8_t i = 0;i < strLen;i++)
    {
        uart2_struct.tx_buf[len+i] = str[i];
    }
    len = len+strLen;
    uart2_struct.tx_buf[len] = 0x00;
	uart2_struct.tx_buf[len+1] = 0x05;
	uart2_struct.tx_buf[len+2] = 0x00;
	uart2_struct.tx_buf[len+3] = 0x04;
	uart2_struct.tx_buf[len+4] = 0x00;
	uart2_struct.tx_buf[len+5] = 0x00;
	uart2_struct.tx_buf[len+6] = 0x00;
	progressBarVal = (uint32_t)(setTime-remainTime)*100;
    progressBarVal = progressBarVal/setTime;
	uart2_struct.tx_buf[len+7] = progressBarVal;
	len = len+8;
    uart2_struct.tx_buf[len] = 0xFF;
	uart2_struct.tx_buf[len+1] = 0xFC;
	uart2_struct.tx_buf[len+2] = 0xFF;
	uart2_struct.tx_buf[len+3] = 0xFF;
	uart2_struct.tx_count = len+4;
	uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);
}



