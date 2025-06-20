/**
 * 
 * @file teaMake.c
 * @author jiaokai 
 * @brief 茶萃取程序
 * 
 * @copyright Copyright (c) 2025
 */

#include "teaMake.h"
#include "eeprom.h"
#include "relay.h"
#include "waterlevel.h"
#include "work.h"
#include "config.h"
#include "lcd_data_process.h"
#include "timer.h"
#include "uart.h"
#include "string.h"
#include "teaExtractionPage.h"
#include "teaSetPage.h"
#include "app_mqtt.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "teaMake"

tea_TypeDef tea;

/**
 * ************************************************************************
 * @brief 茶萃取程序初始化
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void teaMakeInit(void)
{
    log_v("teaMakeInit");
    tea.num = config.data.teaMake.currentNumber;  //读取保存的上次制作编号
    tea.state = 0;         
    tea.isFinish = 0;       //是否完成
    tea.step = 0;           //步骤
    tea.startTime = 0;     //开始时间 秒
    tea.pauseTime = 0;     //暂停时间 秒
    tea.pauseStartTime = 0;//暂停开始时间 秒
    tea.pauseEndTime = 0;  //暂停结束时间 秒
}





/**
 * ************************************************************************
 * @brief 茶制作流程
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void teaProcessControl(void)
{
    uint16_t timeLeft;   //剩余时间
    static uint16_t lastTime = 0; //上次时间
    static uint16_t makeTime;
    uint8_t waterFlag;
    static uint8_t waterVol;
    switch (tea.step)
    {
        case 0:
            tea.state = 1; //开始制作
            tea.pauseTime = 0;
            tea.pauseStartTime = 0;
            tea.pauseEndTime = 0;
            makeTime= config.data.teaMake.time[tea.num]*60; //制作时间
            waterVol = config.data.teaMake.vol[tea.num]+config.data.teaCompensateLevel; //制作水量
            makeTeaShowTime(makeTime,makeTime); //显示制作时间
            log_v("tea step 0");
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
            tea.step = 1; //上水
            break;
        case 1: //上水
            waterFlag = waterInletControl(waterVol);
            if(waterFlag == 1) //进水完成
            {
                tea.step = 2;
                tea.startTime = Timer.system1Sec; //获取开始时间
                tea.pauseStartTime = 0;
                tea.pauseEndTime = 0;
                tea.pauseTime = 0;
                log_v("tea step 1 end");
            }
            else if(waterFlag == 2) //进水超时
            {
                log_e("tea step 1 water inlet over time");
                tea.step = 0xFF;
                waterInletStop();
            }
            break;
        case 2: //萃取
            if((Timer.system1Sec-tea.startTime-tea.pauseTime) >= makeTime) //萃取时间到
            {
                log_v("coffee step 2 end");
                if(config.data.teaMake.autoDrainangeFlag[tea.num] == 1) //自动排水
                {
                    tea.step = 3; //排水
                    waterLevel.drainOption = 1; //排废水
                    log_v("tea auto drain");
                }
                else
                {
                    tea.step = 4; //萃取完成
                    waterLevel.drainOption = 1; //排冷萃液
                }
                closeCirculationPump(); //关闭循环泵
                uart2_struct.tx_buf[0] = 0xEE;
                uart2_struct.tx_buf[1] = 0xB1;
                uart2_struct.tx_buf[2] = 0x12;
                uart2_struct.tx_buf[3] = 0x00;
                uart2_struct.tx_buf[4] = make_tea_page;
                uart2_struct.tx_buf[5] = 0x00;
                uart2_struct.tx_buf[6] = 0x08;
                uart2_struct.tx_buf[7] = 0x00;
                uart2_struct.tx_buf[8] = 0x01;
                uart2_struct.tx_buf[9] = 0x30;
                uart2_struct.tx_buf[10] = 0x00;
                uart2_struct.tx_buf[11] = 0x09;
                uart2_struct.tx_buf[12] = 0x00;
                uart2_struct.tx_buf[13] = 0x01;
                uart2_struct.tx_buf[14] = 0x30;
                uart2_struct.tx_buf[15] = 0xFF;
                uart2_struct.tx_buf[16] = 0xFC;
                uart2_struct.tx_buf[17] = 0xFF;
                uart2_struct.tx_buf[18] = 0xFF;
                uart2_struct.tx_count = 19;
                uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);
            }
            else
            {
                if(outputState.circulationPump == 0)
                {
                    openCirculationPump(); //打开循环泵
                }
                if(outputState.cool == 0)
                {
                    startCooling(); //打开半导体制冷
                }
                timeLeft = makeTime - (Timer.system1Sec-tea.startTime-tea.pauseTime); //剩余时间
                if(timeLeft != lastTime) //时间变化
                {
                    lastTime = timeLeft;
                    makeTeaShowTime(makeTime,timeLeft); //显示制作时间
                }
            }
            break;
        case 3: //排水
            if(waterdrainControl() == 1)
            {
                log_v("tea step 3 end");
                tea.step = 4; //排水完成
            }
            break;
        case 4: //萃取完成
            AnimationPlayFrame(make_tea_page,2,0,UART2_ID);   //动画
            SetControlEnable(make_tea_page,4,1,UART2_ID);   //使能返回键
            if(config.data.teaMake.autoDrainangeFlag[tea.num] == 0)//手动排水
            {
                SetControlVisiable(make_tea_page,14,1,UART2_ID);//显示排水图标
                SetControlEnable(make_tea_page,15,1,UART2_ID);//使能排水按钮
            }
            deviceRunState = 0;   //XXX 结束制作运行状态
            record.data.teaMakeCnt++;
            if(WIFI.mqttConnected == 1)
            {
                if(publish_teaMakeCnt())
                {
                    log_i("publish device run state");
                }
                else
                {
                    log_e("publish device run state failed");
                }
            }
            write_record_data(RECORD_ALL); //XXX 保存所有数据
            teaMakeReset();   //复位
            log_v("tea step 4 end");
            delete_task(teaProcessControl);        //[ ]删除任务
            log_d("delete task teaProcessControl");
            break;
        case 0xFF: //上水超时
            teaMakeReset();   //复位
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
            AnimationPlayFrame(make_tea_page,2,0,UART2_ID);   //动画
            SetControlEnable(make_tea_page,4,1,UART2_ID);   //使能返回键
            SetScreen(water_ingress_page,UART2_ID);     //进水故障页面
            delete_task(teaProcessControl);        //[ ]删除任务
            log_d("delete task teaProcessControl");
            break;
    }
}

/**
 * ************************************************************************
 * @brief 暂停茶制作
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void teaMakePause(void)
{
    closeCirculationPump(); //关闭循环泵
    if(waterLevel.state == 1)   //当前是进水
    {
        waterInletStop();   //进水停止
    }
    else if(waterLevel.state == 2)    //当前是排水
    {
        waterDrainStop();
    }
    stopCooling();  //关闭半导体制冷
}

/**
 * ************************************************************************
 * @brief 茶制作复位
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void teaMakeReset(void)
{
    tea.state = 0;         
    tea.isFinish = 0;       //是否完成
    tea.step = 0;           //步骤
    tea.startTime = 0;     //开始时间 秒
    tea.pauseTime = 0;     //暂停时间 秒
    tea.pauseStartTime = 0;//暂停开始时间 秒
    tea.pauseEndTime = 0;  //暂停结束时间 秒
    if(waterLevel.state == 1)   //当前是进水
    {
        waterInletStop();   //进水停止
    }
    else    //当前是排水
    {
        waterDrainStop();
    }
    stopCooling();  //关闭半导体制冷
}


