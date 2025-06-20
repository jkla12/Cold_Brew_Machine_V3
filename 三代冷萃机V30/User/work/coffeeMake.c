/**
 * 
 * @file coffeeMake.c
 * @author jiaokai 
 * @brief 咖啡萃取程序
 * 
 * @copyright Copyright (c) 2025
 */

#include "coffeeMake.h"
#include "eeprom.h"
#include "relay.h"
#include "waterlevel.h"
#include "config.h"
#include "work.h"
#include "lcd_data_process.h"
#include "timer.h"
#include "uart.h"
#include "string.h"
#include "coffeeExtractionPage.h"
#include "coffeeSetPage.h"
#include "app_mqtt.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "coffeeMake"

coffee_TypeDef coffee;


/**
 * ************************************************************************
 * @brief 咖啡制作初始化
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-12
 * 
 * ************************************************************************
 */
void coffeeMakeInit(void)
{
    log_v("coffeeMakeInit");
    coffee.num = config.data.coffeeMake.currentNumber;  //读取保存的上次制作编号
    coffee.state = 0;         
    coffee.isFinish = 0;       //是否完成
    coffee.step = 0;           //步骤
    coffee.startTime = 0;     //开始时间 秒
    coffee.pauseTime = 0;     //暂停时间 秒
    coffee.pauseStartTime = 0;//暂停开始时间 秒
    coffee.pauseEndTime = 0;  //暂停结束时间 秒
}

/**
 * ************************************************************************
 * @brief 咖啡制作流程
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-12
 * 
 * ************************************************************************
 */
void coffeeProcessControl(void)
{
    uint16_t timeLeft = 0;
    static uint16_t lastTimeLeft;  //上次剩余时间
    static uint16_t makeTime;
    uint8_t waterFlag = 0;
    static uint8_t waterVol;
    switch (coffee.step)
    {
        case 0:
            coffee.state = 1; //开始制作
            coffee.pauseTime = 0;
            coffee.pauseStartTime = 0;
            coffee.pauseEndTime = 0;
            coffee.isFinish = 0;
            coffee.step = 1;
            makeTime = config.data.coffeeMake.time[coffee.num]*60; //获取制作时间
            waterVol = config.data.coffeeMake.vol[coffee.num]+config.data.coffeeCompensateLevel; //获取制作水量
            makeCoffeeShowTime(makeTime,makeTime);
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
            log_v("coffee step 0");
            break;
        case 1: //进水
            waterFlag = waterInletControl(waterVol);
            if(waterFlag == 1) //进水完成
            {
                coffee.step = 2;
                coffee.startTime = Timer.system1Sec; //获取开始时间
                coffee.pauseStartTime = 0;
                coffee.pauseEndTime = 0;
                coffee.pauseTime = 0;
                log_v("coffee step 1 end");
            }
            else if(waterFlag == 2) //进水超时
            {
                log_e("coffee step 1 water inlet over time");
                coffee.step = 0xFF;
                waterInletStop();
            }
            break;
        case 2: //萃取
            if((Timer.system1Sec-coffee.startTime-coffee.pauseTime) >= makeTime) //萃取时间到
            {
                log_v("coffee step 2 end");
                if(config.data.coffeeMake.autoDrainangeFlag[coffee.num] == 1) //自动排水
                {
                    coffee.step = 3;//排水
                    waterLevel.drainOption = 1; //排萃取液
                    log_v("coffee auto drain");
                }
                else    //不排水
                {
                   coffee.step = 4; //萃取完成
                   waterLevel.drainOption = 1; //排萃取液
                    
                }
                closeCirculationPump();   //关闭循环泵
                stopCooling();  //关闭半导体制冷
                //XXX 下面代码不知道为啥加上，测试看看再说
                uart2_struct.tx_buf[0] = 0xEE;
                uart2_struct.tx_buf[1] = 0xB1;
                uart2_struct.tx_buf[2] = 0x12;
                uart2_struct.tx_buf[3] = 0x00;
                uart2_struct.tx_buf[4] = make_coffe_page;
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
                if(outputState.circulationPump == 0)   //启动循环泵
                {
                    openCirculationPump();
                }
                if(outputState.cool == 0)   //启动制冷
                {
                    startCooling();
                }
                timeLeft = makeTime - (Timer.system1Sec-coffee.startTime-coffee.pauseTime);
                if(timeLeft != lastTimeLeft)
                {
                    lastTimeLeft = timeLeft;
                    makeCoffeeShowTime(makeTime,timeLeft);
                }
            }
            break;
        case 3: //排水
            if(waterdrainControl() == 1)
            {
                log_v("coffee step 3 end");
                coffee.step = 4; //萃取完成
            }
            break;
        case 4: //萃取完成
            AnimationPlayFrame(make_coffe_page,2,0,UART2_ID);   
            SetControlEnable(make_coffe_page,4,1,UART2_ID);   //使能返回键
            if(config.data.coffeeMake.autoDrainangeFlag[coffee.num] == 0) //手动排水
            {
                SetControlVisiable(make_coffe_page,14,1,UART2_ID);//显示排水图标
                SetControlEnable(make_coffe_page,15,1,UART2_ID);//使能排水按钮
            }
            coffeeMakeReset();
            deviceRunState = 0;   //XXX 运行状态
            record.data.coffeeMakeCnt++;
            if(WIFI.mqttConnected == 1)
            {
                if(publish_coffeeMakeCnt())
                {
                    log_i("publish coffee make count");
                }
                else
                {
                    log_e("publish coffee make count failed");
                }
            }
            write_record_data(RECORD_ALL);   //保存所有数据
            log_v("coffee step 4 end");
            delete_task(coffeeProcessControl);        //[ ]删除任务
            log_d("delete task coffeeProcessControl");
            break;
        case 0xFF: //进水超时
            coffeeMakeReset();   //复位
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
            AnimationPlayFrame(make_coffe_page,2,0,UART2_ID);   
            SetControlEnable(make_coffe_page,4,1,UART2_ID);   //使能返回键
            SetScreen(water_ingress_page,UART2_ID);     //进水故障页面
            delete_task(coffeeProcessControl);        //[ ]删除任务
            log_d("delete task coffeeProcessControl");
            break;
        default:
            break;
    }
}




/**
 * ************************************************************************
 * @brief 咖啡制作暂停
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-12
 * 
 * ************************************************************************
 */
void coffeeMakePause(void)
{
    //关闭水泵
    closeCirculationPump();
    //关闭进水，关闭排水
    if(waterLevel.state == 1)   //当前是进水
    {
        waterInletStop();    //关闭进水
        
    }
    else if(waterLevel.state == 2)   //当前是排水
    {
         waterDrainStop();   //关闭排水
    }
    stopCooling();  //关闭半导体制冷
}

/**
 * ************************************************************************
 * @brief 咖啡制作复位
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void coffeeMakeReset(void)
{
    coffee.state = 0;
    coffee.pauseEndTime = 0;
    coffee.pauseStartTime = 0;
    coffee.pauseTime = 0;
    coffee.isFinish = 0;
    coffee.step = 0;
    coffee.startTime = 0;
    if(waterLevel.state == 1)
    {
        waterInletStop();
    }
    else if(waterLevel.state == 2)
    {
        waterDrainStop();
    }
    stopCooling();  //关闭半导体制冷
}
