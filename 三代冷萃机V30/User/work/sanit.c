/**
 * 
 * @file sanit.c
 * @author jiaokai 
 * @brief 消毒相关程序
 * 
 * @copyright Copyright (c) 2025
 */
#include "sanit.h"
#include "lcd_data_process.h"
#include "uart.h"
#include "relay.h"
#include "timer.h"
#include "sanit.h"
#include "work.h"
#include "eeprom.h"
#include "config.h"
#include "waterLevel.h"
#include "sanitPage.h"
#include "app_mqtt.h"
#include "elog.h"
#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "sanit"

sanit_TypeDef sanit;

/**
 * ************************************************************************
 * @brief 消毒初始化
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void sanitInit(void)
{
    sanit.state = 0;          //状态  0空闲，1开始，2暂停
    sanit.step = 0;           //步骤
    sanit.startTime = 0;     //开始时间 秒
    sanit.time = 0;
    sanit.pauseTime = 0;     //暂停时间 秒
    sanit.pauseStartTime = 0;//暂停开始时间 秒
    sanit.pauseEndTime = 0;  //暂停结束时间 秒
    sanit.repeatCnt = 0;     //清洗重复计数  
    sanit.endTime = 0;      //结束时间
}

/**
 * ************************************************************************
 * @brief 消毒流程
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void sanitProcessControl(void)
{
    uint16_t timeLeft;   //剩余时间
    static uint16_t lastTimeLeft = 0;   //上次剩余时间
    static uint8_t repeatCnt = 0;   //重复计数
    uint8_t waterFlag;
    switch(sanit.step)
    {
        case 0:
            sanit.repeatCnt = 0;
            sanit.startTime = 0;
            sanit.pauseTime = 0;
            sanit.time = 0;
            sanit.pauseStartTime = 0;
            sanit.pauseEndTime = 0;
            log_v("sanit step 0");
            sanit.state = 1;  
            sanit.step = 1;   //开始消毒
            repeatCnt = 0;
            deviceRunState = 1;   //XXX 运行状态
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
            else
            {
                log_e("mqtt not connected");
            }
            break;
        case 1:
            waterFlag = waterInletControl(SANIT_VOLUME);   //进水
            if(waterFlag == 2)//进水超时
            {
                sanit.step = 0xFF;   //进水超时
                log_d("sanit inlet over time");
            }
            else if(waterFlag == 1) //进水完成
            {
                log_d("sanit inlet end");
                sanit.step = 2;   //进水完成
                sanit.time = Timer.system1Sec-sanit.pauseTime;   //记录开始时间
                sanit.startTime = Timer.system1Sec-sanit.pauseTime;   //记录开始时间
                sanit.time = sanit.startTime;   
                repeatCnt = 1;
            }
            else //进水中
            {
                if(waterLevel.vol>20 && outputState.circulationPump == 0)   //大于设定水量启动水泵
                {
                    log_d("sanit open pump");
                    openCirculationPump();   //开启循环泵
                    startCooling();   //开启冷却 消毒
                }
            }
            break;
        case 2:
            if(repeatCnt <= SANIT_STEP1_NUM)
            {
                if(Timer.system1Sec-sanit.startTime-sanit.pauseTime < (SANIT_SINGLE_TIME*repeatCnt)+(SANIT_PAUSE_TIME*(repeatCnt-1)))
                {
                    //开启水泵冲洗
                    if(outputState.circulationPump == 0)
                    {
                        openCirculationPump();
                        log_d("sanit open pump");
                    }
                    timeLeft = SANIT_TIME-(Timer.system1Sec-sanit.startTime-sanit.pauseTime);   //计算剩余时间
                    if(lastTimeLeft != timeLeft)
                    {
                        lastTimeLeft = timeLeft;
                        sanitShowTime(SANIT_TIME,timeLeft);
                    }
                }
                else if(Timer.system1Sec-sanit.startTime-sanit.pauseTime < ((SANIT_SINGLE_TIME+SANIT_PAUSE_TIME)*repeatCnt))
                {
                    //关闭水泵
                    if(outputState.circulationPump == 1)
                    {
                        log_d("sanit close pump");
                        closeCirculationPump();
                        timeLeft = SANIT_TIME-(Timer.system1Sec-sanit.startTime-sanit.pauseTime);   //计算剩余时间
                        if(lastTimeLeft != timeLeft)
                        {
                            lastTimeLeft = timeLeft;
                            sanitShowTime(SANIT_TIME,timeLeft);
                        }
                    }
                }
                else
                {
                    repeatCnt++;
                    log_d("sanit single end");
                }
            }
            else
            {
                log_d("sanit first end");
                sanit.step = 3;   //消毒完成
                repeatCnt = 1;
                stopCooling();   //停止制冷
            }
            break;
        case 3://排水
            timeLeft = SANIT_TIME-(Timer.system1Sec-sanit.startTime-sanit.pauseTime);   //计算剩余时间
            if(lastTimeLeft != timeLeft)
            {
                lastTimeLeft = timeLeft;
                sanitShowTime(SANIT_TIME,timeLeft);
            }
            waterLevel.drainOption = 0; //排废水
            waterFlag = waterdrainControl();
            if(waterFlag == 1)
            {
                log_d("sanit drain end");
                sanit.repeatCnt++;
                sanit.step = 4;
            }
            break;
        case 4: //二次上水
            waterFlag = waterInletControl(SANIT_VOLUME);   //进水
            if(waterFlag == 1)
            {
                log_d("sanit inlet end");
                sanit.time = Timer.system1Sec-sanit.pauseTime;   //记录开始时间
                repeatCnt = 1;
                sanit.step = 5;   //进水完成
            }
            else if(waterFlag == 2)//进水超时
            {
                sanit.step = 0xFF;   //进水超时
                log_d("sanit inlet over time");
            }
            else //进水中
            {
                if(waterLevel.vol>20 && outputState.circulationPump == 0)   //大于设定水量启动水泵
                {
                    log_d("sanit open pump");
                    openCirculationPump();   //开启循环泵
                    startCooling();   //开启冷却 消毒
                }
            }
            timeLeft = SANIT_TIME-(Timer.system1Sec-sanit.startTime-sanit.pauseTime);   //计算剩余时间
            if(lastTimeLeft != timeLeft)
            {
                lastTimeLeft = timeLeft;
                sanitShowTime(SANIT_TIME,timeLeft);
            }
            break;
        case 5:
            if(repeatCnt <= SANIT_STEP2_NUM)
            {
                if(Timer.system1Sec-sanit.time-sanit.pauseTime < (SANIT_SINGLE_TIME*repeatCnt)+(SANIT_PAUSE_TIME*(repeatCnt-1)))
                {
                    //开启水泵冲洗
                    if(outputState.circulationPump == 0)
                    {
                        openCirculationPump();
                        log_d("sanit open pump");
                    }
                    timeLeft = SANIT_TIME-(Timer.system1Sec-sanit.startTime-sanit.pauseTime);   //计算剩余时间
                    if(lastTimeLeft != timeLeft)
                    {
                        lastTimeLeft = timeLeft;
                        sanitShowTime(SANIT_TIME,timeLeft);
                    }
                }
                else if(Timer.system1Sec-sanit.startTime-sanit.pauseTime < ((SANIT_SINGLE_TIME+SANIT_PAUSE_TIME)*repeatCnt))
                {
                    //关闭水泵
                    if(outputState.circulationPump == 1)
                    {
                        log_d("sanit close pump");
                        closeCirculationPump();
                        timeLeft = SANIT_TIME-(Timer.system1Sec-sanit.startTime-sanit.pauseTime);   //计算剩余时间
                        if(lastTimeLeft != timeLeft)
                        {
                            lastTimeLeft = timeLeft;
                            sanitShowTime(SANIT_TIME,timeLeft);
                        }
                    }
                }
                else
                {
                    repeatCnt++;
                    log_d("sanit single end");
                }
            }
            else
            {
                log_d("sanit second end");
                sanit.step = 6;   //消毒完成
                repeatCnt = 1;
                stopCooling();   //停止制冷
            }
            break;
        case 6:
            timeLeft = SANIT_TIME-(Timer.system1Sec-sanit.startTime-sanit.pauseTime);   //计算剩余时间
            if(lastTimeLeft != timeLeft)
            {
                lastTimeLeft = timeLeft;
                sanitShowTime(SANIT_TIME,timeLeft);
            }
            waterLevel.drainOption = 0; //排废水
            waterFlag = waterdrainControl();
            if(waterFlag == 1)
            {
                log_d("sanit drain end");
                sanit.repeatCnt++;
                sanitShowTime(config.data.washTime,0);
                sanit.step = 7;
            }
            break;
        case 7: //结束
            timeLeft = 0;
            sanit.state = 0;
            sanit.step = 0;  
            sanit.startTime = 0;
            sanit.time = 0;
            sanit.pauseTime = 0;
            sanit.pauseStartTime = 0;
            sanit.pauseEndTime = 0;
            sanit.repeatCnt = 0;
            deviceRunState = 0;   //XXX 运行状态
        
            record.data.sanitCnt++;
            write_record_data(RECORD_ALL);   
            SetControlEnable(sanit_page,4,1,UART2_ID);//
			AnimationPlayFrame(sanit_page,2,0,UART2_ID);   //动画
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
            else
            {
                log_e("mqtt not connected");
            }
            delete_task(sanitProcessControl);        //[ ]删除任务
            log_v("sanit end");
            break;
        case 0xFF:
            
            deviceRunState = 2;   //XXX 运行状态
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
            else
            {
                log_e("mqtt not connected");
            }
            AnimationPlayFrame(sanit_page,2,0,UART2_ID);   //动画
            SetControlEnable(sanit_page,4,1,UART2_ID);   //使能返回键
            SetScreen(water_ingress_page,UART2_ID);     //进水故障页面
            sanitReset();   //复位
            delete_task(sanitProcessControl);        //[ ]删除任务
            log_d("delete task sanitProcessControl");
            break;
        default:
            break;
    }
}


/**
 * ************************************************************************
 * @brief 暂停消毒
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void sanitPause(void)
{
    waterInletStop();   //进水停止
    waterDrainStop();   //排水停止
}

/**
 * ************************************************************************
 * @brief 消毒程序复位
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void sanitReset(void)   //消毒复位
{
    sanit.state = 0;          
    sanit.step = 0;           
    sanit.startTime = 0;     
    sanit.time = 0;
    sanit.pauseTime = 0;     
    sanit.pauseStartTime = 0;
    sanit.pauseEndTime = 0;  
    sanit.repeatCnt = 0;     
    sanit.endTime = 0;      
    closeCirculationPump(); //关闭循环泵
    waterInletStop();   //进水停止
    waterDrainStop();   //排水停止
}


