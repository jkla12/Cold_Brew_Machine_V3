#include "wash.h"
#include "uart.h"
#include "config.h"
#include "waterLevel.h"
#include "timer.h"
#include "relay.h"
#include "eeprom.h"
#include "work.h"
#include "washPage.h"
#include "lcd_data_process.h"
#include "app_mqtt.h"
#include "app_wifi.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "WASH"

wash_TypeDef wash;

/**
 * ************************************************************************
 * @brief  清洗初始化
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void washInit(void)
{
    wash.state = 0;          //状态  0空闲，1开始，2暂停
    wash.step = 0;           //步骤
    wash.startTime = 0;     //开始时间 秒
    wash.time = 0;
    wash.pauseTime = 0;     //暂停时间 秒
    wash.pauseStartTime = 0;//暂停开始时间 秒
    wash.pauseEndTime = 0;  //暂停结束时间 秒
    wash.repeatCnt = 0;     //清洗重复计数  
    wash.endTime = 0;      //结束时间
}



/**
 * ************************************************************************
 * @brief 清洗工作流程
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-09
 *  
 * ************************************************************************
 */
void washProcessControl(void)
{
    uint16_t timeLeft;  //剩余时间
    static uint16_t lastTimeLeft;  //上次剩余时间
    static uint8_t washCnt = 0;         //单次循环清洗次数
    uint8_t waterLevelFlag = 0;
    switch(wash.step)
    {
        case 0: //初始化
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
            waterLevel.drainOption = 0; //排废水
            wash.repeatCnt = 0;
            wash.startTime = 0;
            wash.pauseTime = 0;
            log_d("Wash Start");
            wash.state = 1;
            washCnt = 0;        
            wash.step = 1; //设置为第一次上水
            break;
        case 1: //第一次清洗上水
            waterLevelFlag = waterInletControl(config.data.washFirstVolume);
            if(waterLevelFlag == 1) //上水完成
            {
                log_d("Wash First Water Inlet Complete");
                wash.step = 2;
                wash.startTime = Timer.system1Sec-wash.pauseTime;   //记录开始时间
                wash.time = wash.startTime;
                washCnt = 1;   
            }
            else if(waterLevelFlag == 2)    //上水超时
            {
                log_d("Wash First Water Inlet Timeout");
                wash.step = 0xFF;
                
            }
            else   //正在上水
            {   
                if(waterLevel.vol>20 && outputState.circulationPump == 0)   //大于设定水量启动水泵
                {
                    openCirculationPump();  //启动水泵
                    log_d("Wash First Water Inlet Start Pump");
                }
            }
            washShowTime(config.data.washTime,config.data.washTime);
            break;
        case 2://上水完成
            if(washCnt<=config.data.washLoopTimes)
            {
                if(Timer.system1Sec-wash.time-wash.pauseTime<(config.data.washSingleTime*washCnt)+(config.data.washPauseTime*(washCnt-1)))
                {
                    //开启水泵冲洗
                    if(outputState.circulationPump == 0)
                    {
                        openCirculationPump();
                        log_d("wash first open pump %d" ,washCnt);
                    }
                    //XXX 这次加上制冷
                    if(outputState.cool == 0)
                    {
                        startCooling();
                    }
                    timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //计算剩余时间
                    if(lastTimeLeft != timeLeft)
                    {
                        lastTimeLeft = timeLeft;
                        washShowTime(config.data.washTime,timeLeft);
                    }
                    
                }
                else if(Timer.system1Sec-wash.time-wash.pauseTime<((config.data.washSingleTime+config.data.washPauseTime)*washCnt))
                {
                    //关闭水泵
                    if(outputState.circulationPump == 1)
                    {
                        log_d("wash first close pump %d",washCnt);
                        closeCirculationPump();
                    }
                    if(outputState.cool == 1)
                    {
                        stopCooling();
                    }
                    timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //计算剩余时间
                    if(lastTimeLeft != timeLeft)
                    {
                        lastTimeLeft = timeLeft;
                        washShowTime(config.data.washTime,timeLeft);
                    }
                }
                else
                {
                    washCnt++;
                    log_d("wash fisrt single end");
                }
            }
            else
            {
                //开启水泵冲洗
                if(outputState.circulationPump == 0)
                {
                    openCirculationPump();
                    log_d("wash first open pump %d" ,washCnt);
                }
                log_d("wash First end");
                wash.step = 4;
                washCnt = 1;
            }
            break;
        case 4: //排水
            timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //计算剩余时间
            if(lastTimeLeft != timeLeft)
            {
                lastTimeLeft = timeLeft;
                washShowTime(config.data.washTime,timeLeft);
            }
            
            waterLevel.drainOption = 0; //排废水
            waterLevelFlag = waterdrainControl();
            if(waterLevelFlag == 1)
            {
                log_d("wash first drain end");
                log_v("wash second start vol:%d",config.data.washSecondVolume);
                wash.repeatCnt++;
                wash.step = 5;
            }
            break;
        case 5: //二次上水
            waterLevelFlag = waterInletControl(config.data.washSecondVolume);
            if(waterLevelFlag == 1)
            {
                log_d("wash sencond inlet end");
                wash.time = Timer.system1Sec-wash.pauseTime;   //记录开始时间
                washCnt = 1;
                wash.step = 6;
            }
            else if(waterLevelFlag == 2) 
            {
                log_d("Wash second Water Inlet Timeout");
                wash.step = 0xFF;
            }
            else
            {
                  if(waterLevel.vol>20 && outputState.circulationPump == 0)   //大于设定水量启动水泵
                {
                    openCirculationPump();  //启动水泵
                    log_d("Wash Second Water Inlet Start Pump");
                }
            }
            timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //计算剩余时间
            if(lastTimeLeft != timeLeft)
            {
                lastTimeLeft = timeLeft;
                washShowTime(config.data.washTime,timeLeft);
            }
            break;
        case 6: //第二次循环清洗
            if(washCnt<=config.data.washLoopTimes)
            {
                if(Timer.system1Sec-wash.time-wash.pauseTime<(config.data.washSingleTime*washCnt)+(config.data.washPauseTime*(washCnt-1)))
                {
                    //开启水泵冲洗
                    if(outputState.circulationPump == 0)
                    {
                        openCirculationPump();
                        log_d("wash second open pump %d",washCnt);
                    }
                    //XXX 这次加上制冷
                    if(outputState.cool == 0)
                    {
                        startCooling();
                    }
                    timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //计算剩余时间
                    if(lastTimeLeft != timeLeft)
                    {
                        lastTimeLeft = timeLeft;
                        washShowTime(config.data.washTime,timeLeft);
                    }
                    
                }
                else if(Timer.system1Sec-wash.time-wash.pauseTime<((config.data.washSingleTime+config.data.washPauseTime)*washCnt))
                {
                    //关闭水泵
                    if(outputState.circulationPump == 1)
                    {
                        log_d("wash second close pump %d",washCnt);
                        closeCirculationPump();
                    }
                    if(outputState.cool == 1)
                    {
                        stopCooling();
                    }
                    timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //计算剩余时间
                    if(lastTimeLeft != timeLeft)
                    {
                        lastTimeLeft = timeLeft;
                        washShowTime(config.data.washTime,timeLeft);
                    }
                }
                else
                {
                    washCnt++;
                    log_d("wash second single end");
                }
            }
            else
            {
                log_d("wash second end");
                wash.step = 7;
                washCnt = 1;
            }
            break;
        case 7:
            timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //计算剩余时间
            if(lastTimeLeft != timeLeft)
            {
                lastTimeLeft = timeLeft;
                washShowTime(config.data.washTime,timeLeft);
            }
            
            waterLevel.drainOption = 0; //排废水
            waterLevelFlag = waterdrainControl();
            if(waterLevelFlag == 1)
            {
                log_d("wash second drain end");
                wash.repeatCnt++;
                washShowTime(config.data.washTime,0);
                wash.step = 8;
            }
            break;
        case 8:
            timeLeft = 0;
            wash.state = 0;
            wash.step = 0;
            wash.startTime = 0;
            wash.pauseStartTime = 0;
            wash.pauseEndTime = 0;
            wash.pauseTime = 0;
            wash.endTime = 0;
            wash.repeatCnt = 0;
            deviceRunState = 0;    //XXX 结束清洗运行状态
            record.data.washCnt++;
            
            write_record_data(RECORD_ALL); //XXX 保存所有数据
            AnimationPlayFrame(wash_page,2,0,UART2_ID);
            SetControlEnable(wash_page,4,1,UART2_ID);   //使能返回键
            if(WIFI.mqttConnected == 1)
            {
                if(publish_washCnt())
                {
                    log_i("publish device run state");
                }
                else
                {
                    log_e("publish device run state failed");
                }
            }
            //[ ] 删除任务
            log_d("delete task washProcessControl");
            delete_task(washProcessControl);
            break;
        case 0xFF: //上水超时
            AnimationPlayFrame(wash_page,2,0,UART2_ID);
            SetControlEnable(wash_page,4,1,UART2_ID);   //使能返回键
            SetScreen(water_ingress_page,UART2_ID);     //进水故障页面
            deviceRunState = 2;    //异常
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
            washReset();
            log_d("delete task washProcessControl");
            delete_task(washProcessControl);        //[ ]删除任务
            break;
        default:
            break;
    }   
}

/**
 * ************************************************************************
 * @brief 清洗复位
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-09
 * 
 * ************************************************************************
 */
void washReset(void)
{
    wash.state = 0;
    wash.step = 0;
    wash.startTime = 0;
    wash.endTime = 0;
    wash.pauseTime = 0;
    wash.pauseStartTime = 0;
    wash.pauseEndTime = 0;
    wash.repeatCnt = 0;
    closeCirculationPump(); //关闭循环泵
    waterInletStop();   //进水停止
    waterDrainStop();   //排水停止
    stopCooling();   //停止制冷
}

/**
 * ************************************************************************
 * @brief 清洗暂停
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-09
 * 
 * ************************************************************************
 */
void washPause(void)
{
    waterInletStop();   //进水停止
    waterDrainStop();
}




/**
 * ************************************************************************
 * @brief 清洗工作流程 模式2 测试版本
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void washProcess2Control(void)
{
    uint16_t timeLeft;  //剩余时间
    static uint16_t lastTimeLeft;  //上次剩余时间
    static uint8_t washCnt = 0;         //单次循环清洗次数
    uint8_t waterLevelFlag = 0;
    switch(wash.step)
    {
        case 0: //初始化
            waterLevel.drainOption = 0; //排废水
            wash.repeatCnt = 0;
            wash.startTime = 0;
            wash.pauseTime = 0;
            log_d("Wash Start");
            wash.state = 1;
            washCnt = 0;        
            wash.startTime = Timer.system1Sec;   //记录开始时间
            wash.step = 1; //设置为第一次上水
            break;
        case 1: //第一次清洗上水
            log_v("wash step 1");
            drainValveSewage();//排废水
            openCirculationValve(); //排水
            openInletValve();   //打开进水电磁阀
            wash.step = 2;
            break;
        case 2:
            
            timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //计算剩余时间
            if(lastTimeLeft != timeLeft)
            {
                lastTimeLeft = timeLeft;
                washShowTime(config.data.washTime,timeLeft);
            }

            if(Timer.system1Sec - wash.startTime-wash.pauseTime < 120) //TODO 冲洗120秒
            {
                if(waterLevel.vol > 30)
                {
                    if(outputState.circulationPump == 0)
                    {
                        openCirculationPump();  //开启循环泵
                    }
                }
                if(waterLevel.vol < 20)
                {
                    if(outputState.circulationPump == 1)
                    {
                        closeCirculationPump(); //关闭循环泵
                    }
                }
            }
            else
            {
                log_d("wash first water inlet end");
                closeInletValve();  //关闭进水电磁阀
                wash.step = 4;
            }
            break;
        case 4: //排水
            timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //计算剩余时间
            if(lastTimeLeft != timeLeft)
            {
                lastTimeLeft = timeLeft;
                washShowTime(config.data.washTime,timeLeft);
            }
            
            waterLevel.drainOption = 0; //排废水
            waterLevelFlag = waterdrainControl();
            if(waterLevelFlag == 1)
            {
                log_d("wash drain end");
                wash.repeatCnt++;
                wash.step = 5;
            }
            break;
        case 5: //二次上水
            waterLevelFlag = waterInletControl(config.data.washSecondVolume);
            if(waterLevelFlag == 1)
            {
                log_d("wash inlet end");
                wash.time = Timer.system1Sec-wash.pauseTime;   //记录开始时间
                washCnt = 1;
                wash.step = 6;
            }
            else if(waterLevelFlag == 2) 
            {
                log_d("Wash second Water Inlet Timeout");
                wash.step = 0xFF;
            }
            else
            {
                  if(waterLevel.vol>20 && outputState.circulationPump == 0)   //大于设定水量启动水泵
                {
                    openCirculationPump();  //启动水泵
                    log_d("Wash Second Water Inlet Start Pump");
                }
            }
            break;
        case 6: //第二次循环清洗
            if(washCnt<=config.data.washLoopTimes)
            {
                if(Timer.system1Sec-wash.time-wash.pauseTime<(config.data.washSingleTime*washCnt)+(config.data.washPauseTime*(washCnt-1)))
                {
                    //开启水泵冲洗
                    if(outputState.circulationPump == 0)
                    {
                        openCirculationPump();
                        log_d("wash open pump");
                    }

                    timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //计算剩余时间
                    if(lastTimeLeft != timeLeft)
                    {
                        lastTimeLeft = timeLeft;
                        washShowTime(config.data.washTime,timeLeft);
                    }
                    
                }
                else if(Timer.system1Sec-wash.time-wash.pauseTime<((config.data.washSingleTime+config.data.washPauseTime)*washCnt))
                {
                    //关闭水泵
                    if(outputState.circulationPump == 1)
                    {
                        log_d("wash close pump");
                        closeCirculationPump();

                        timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //计算剩余时间
                        if(lastTimeLeft != timeLeft)
                        {
                            lastTimeLeft = timeLeft;
                            washShowTime(config.data.washTime,timeLeft);
                        }
                    }
                }
                else
                {
                    washCnt++;
                    log_d("wash single end");
                }
            }
            else
            {
                log_d("wash First end");
                wash.step = 7;
                washCnt = 1;
            }
            break;
        case 7:
            timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //计算剩余时间
            if(lastTimeLeft != timeLeft)
            {
                lastTimeLeft = timeLeft;
                washShowTime(config.data.washTime,timeLeft);
            }
            
            waterLevel.drainOption = 0; //排废水
            waterLevelFlag = waterdrainControl();
            if(waterLevelFlag == 1)
            {
                log_d("wash drain end");
                wash.repeatCnt++;
                washShowTime(config.data.washTime,0);
                wash.step = 8;
            }
            break;
        case 8:
            timeLeft = 0;
            wash.state = 0;
            wash.step = 0;
            wash.startTime = 0;
            wash.pauseStartTime = 0;
            wash.pauseEndTime = 0;
            wash.pauseTime = 0;
            wash.endTime = 0;
            wash.repeatCnt = 0;
            SetControlEnable(wash_page,4,1,UART2_ID);
            //[ ] 删除任务
            delete_task(washProcess2Control);
            log_d("delete task washProcess2Control");
            break;
        case 0xFF: //上水超时
            washReset();
            AnimationPlayFrame(wash_page,2,0,UART2_ID);
            SetControlEnable(wash_page,4,1,UART2_ID);   //使能返回键
            SetScreen(water_ingress_page,UART2_ID);     //进水故障页面
            delete_task(washProcess2Control);        //[ ]删除任务
            log_d("delete task washProcess2Control");
            break;
        default:
            break;
    }   
}
