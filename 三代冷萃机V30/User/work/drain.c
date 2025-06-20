/**
 * 
 * @file drain.c
 * @author jiaokai 
 * @brief 手动排水相关控制代码 
 * 
 * @copyright Copyright (c) 2025
 */
#include "drain.h"
#include "eeprom.h"
#include "config.h"
#include "lcd_data_process.h"
#include "uart.h"
#include "work.h"
#include "waterLevel.h"
#include "timer.h"
#include "drainPage.h"
#include "app_mqtt.h"
#include "elog.h"
#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "drain"

drain_TypeDef drain;


/**
 * ************************************************************************
 * @brief  手动排水停止
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void drainReset(void)
{
    drain.state = 0;      //状态，0停止
    drain.step = 0;       //步骤
    drain.startTime = 0; //开始时间
    drain.pauseTime = 0; //暂停时间
    drain.pauseStartTime = 0; //暂停开始时间
    drain.pauseEndTime = 0;   //暂停结束时间
    waterDrainStop(); //停止排水
    SetControlEnable(drain_page,11,1,UART2_ID);//使能排水选择按钮
}

/**
 * ************************************************************************
 * @brief 手动排水流程控制
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void drainProcessControl(void)
{
    uint16_t lastTime;
    uint8_t waterFlag;
        switch(drain.step)
        {
            case 0: //初始化
            
                log_v("drain init");
                drain.state = 1; //设置为排水状态
                drain.startTime = Timer.system1Sec; //记录开始时间
                drain.pauseTime = 0; //清除暂停时间
                drain.pauseStartTime = 0; //清除暂停开始时间
                drain.pauseEndTime = 0; //清除暂停结束时间
                drain.step = 1; //设置为排水开始状态
                deviceRunState = 1;    //XXX运行状态
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
                break;
            case 1: //排水开始
                lastTime = config.data.drainTime-(Timer.system1Sec-drain.startTime-drain.pauseTime);   //计算剩余时间
                if(lastTime>config.data.drainTime)
                {
                    lastTime = 1;
                }
                
                drainShowTime(config.data.drainTime,lastTime);
                waterFlag = waterdrainControl(); //排水控制
                if(waterFlag == 1)//排水完成
                {
                    log_d("drain end");
                    drain.step = 2;
                    drainShowTime(config.data.drainTime,0);
                }
                else if(waterFlag == 2) //排水超时
                {
                    log_d("drain timeout");
                    drain.step = 2;
                }
                break;
            case 2: //排水结束
                drain.state = 0; //设置为停止状态
                drain.step = 0; //设置为初始化状态
                lastTime = 0;
                drain.pauseTime = 0;
                drain.pauseStartTime = 0;
                drain.pauseEndTime = 0;
                write_record_data(RECORD_ALL);   //记录所有数据
                AnimationPlayFrame(drain_page,2,0,UART2_ID);
                SetControlEnable(drain_page,4,1,UART2_ID);//使能返回键
                SetControlEnable(drain_page,11,1,UART2_ID);//使能排水选择按钮
                 
                log_d("delete task drainProcessControl");
                log_v("drain end");   
                deviceRunState = 0;    //XXX运行状态
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
                

                delete_task(drainProcessControl); //FIXME 删除任务    
                break;
            default:
                break;
        }
}

