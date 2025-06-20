#include "relay.h"
#include "eeprom.h"
#include "timer.h"
#include "elog.h"
#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "relay"
OutputState_TypeDef outputState = {0};

void relay_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
		rcu_periph_clock_enable(RCU_GPIOC);
    for(uint8_t i = 0; i < 9; i++)
    {
        gpio_init(RELAY_GPIO_PORT[i], GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, RELAY_GPIO_PIN[i]);
        GPIO_BC(RELAY_GPIO_PORT[i]) = RELAY_GPIO_PIN[i];
    }
    gpio_init(MOTOR_PWM_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, MOTOR_PWM_PIN);
    GPIO_BOP(MOTOR_PWM_PORT) = MOTOR_PWM_PIN;
    outputState.pumpOpeningTime = 0;
    outputState.coolOpeningTime = 0;
    outputState.pumpCurrentRuntime = 0;
    outputState.coolCurrentRuntime = 0;
}

void relay_ON(Relay_typedef_enum number)
{
    GPIO_BOP(RELAY_GPIO_PORT[number]) = RELAY_GPIO_PIN[number];
}

void relay_OFF(Relay_typedef_enum number)
{
    GPIO_BC(RELAY_GPIO_PORT[number]) = RELAY_GPIO_PIN[number];
}

/**
 * ************************************************************************
 * @brief 打开进水电磁阀
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-30
 * 
 * ************************************************************************
 */
void openInletValve(void)
{
    
    relay_ON(Relay3);
    if(outputState.inletValve == 0)
    {
        record.data.waterInletValveCnt++;
        //TODO 更新上报电磁阀数据
    }
    
    outputState.inletValve = 1;
}

/**
 * ************************************************************************
 * @brief 关闭进水电磁阀
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-30
 * 
 * ************************************************************************
 */
void closeInletValve(void)
{
    relay_OFF(Relay3);
    outputState.inletValve = 0;
}

/**
 * ************************************************************************
 * @brief 三通球阀切换到排萃取液
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-30
 * 
 * ************************************************************************
 */
void drainValveExtract(void)
{
    relay_ON(Relay6);
    if(outputState.drainValve == 0)
    {
        record.data.drainValveCnt++;    //排水三通球阀计数加一
        //TODO 更新上报排水三通球阀动作次数
    }
    outputState.drainValve = 1;

}

/**
 * ************************************************************************
 * @brief 三通球阀切换到排废水
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-30
 * 
 * ************************************************************************
 */
void drainValveSewage(void)
{
    //排废水
    relay_OFF(Relay6);
    outputState.drainValve = 0;
}

/**
 * ************************************************************************
 * @brief 循环三通球阀切换到排水位置
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-30
 * 
 * ************************************************************************
 */
void openCirculationValve(void)
{
    relay_ON(Relay5);
    if(outputState.circulationValve == 0)
    {
        record.data.circulationValveCnt++;
        //TODO 更新上报循环三通球阀动作次数
        
    }
    outputState.circulationValve = 1;
}

/**
 * ************************************************************************
 * @brief  循环三通球阀切换到循环位置
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-30
 * 
 * ************************************************************************
 */
void closeCirculationValve(void)
{
    relay_OFF(Relay5);
    outputState.circulationValve = 0;
}

/**
 * ************************************************************************
 * @brief 开启循环阀
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-08
 * 
 * ************************************************************************
 */
void openCirculationPump(void)
{
    relay_ON(Relay1);
    if(outputState.circulationPump == 0)    //当前是关闭状态
    {   
        outputState.pumpOpeningTime = Timer.system1Sec; //  记录开始运行时间
    }
    outputState.circulationPump = 1;
}

/**
 * ************************************************************************
 * @brief 关闭循环阀
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-08
 * 
 * ************************************************************************
 */
void closeCirculationPump(void)
{
    relay_OFF(Relay1);
    
    if(outputState.circulationPump == 1)    //当前是开启状态，记录运行时间
    {   
        outputState.pumpCurrentRuntime = outputState.pumpCurrentRuntime + (Timer.system1Sec - outputState.pumpOpeningTime);    //记录当次开启时间
        if(outputState.pumpCurrentRuntime>=60)
        {
            record.data.circulationPumpRunTime += outputState.pumpCurrentRuntime/60;    //记录运行时间
            outputState.pumpCurrentRuntime = outputState.pumpCurrentRuntime%60;             //保留秒
        }
        write_record_data(RECORD_CIRCULATION_PUMP_RUN_TIME);    //记录循环泵运行时间
    }
    outputState.circulationPump = 0;
}

/**
 * ************************************************************************
 * @brief 开启制冷
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void startCooling(void)
{
    relay_ON(Relay7);
    if(outputState.cool == 0)
    {
        outputState.coolOpeningTime = Timer.system1Sec; //记录制冷片开启时间
    }
    outputState.cool = 1;
}

/**
 * ************************************************************************
 * @brief 结束制冷
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void stopCooling(void)
{
    relay_OFF(Relay7);
    if(outputState.cool == 1)
    {
        outputState.coolCurrentRuntime = outputState.coolCurrentRuntime + (Timer.system1Sec - outputState.coolOpeningTime);    //记录当次开启时间
        if(outputState.coolCurrentRuntime>=60)
        {
            record.data.coolingRunTime += outputState.coolCurrentRuntime/60;    //记录运行时间
            outputState.coolCurrentRuntime = outputState.coolCurrentRuntime%60;             //保留秒
        }
        write_record_data(RECORD_COOLING_RUN_TIME);    //记录制冷运行时间
    }
    outputState.cool = 0;
}

/**
 * ************************************************************************
 * @brief 打开直流泵1
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void openPump1(void)
{
    relay_ON(Relay8);
    if(outputState.pump1 == 0)
    {
        outputState.pump1OpeningTime = Timer.system1Sec; //  记录开始运行时间
    }
    outputState.pump1 = 1;
}
/**
 * ************************************************************************
 * @brief 关闭直流泵1
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void closePump1(void)
{
    relay_OFF(Relay8);
    if(outputState.pump1 == 1)    //当前是开启状态，记录运行时间
    {   
        outputState.pump1CurrentRuntime = outputState.pump1CurrentRuntime + (Timer.system1Sec - outputState.pump1OpeningTime);    //记录当次开启时间
        if(outputState.pump1CurrentRuntime>=60)
        {
            record.data.DCPumpRunTime += outputState.pump1CurrentRuntime/60;    //记录运行时间
            outputState.pump1CurrentRuntime = outputState.pump1CurrentRuntime%60;             //保留秒
        }
        write_record_data(RECORD_DC_PUMP_RUN_TIME);    //记录直流泵运行时间
    }
    outputState.pump1 = 0;
}
/**
 * ************************************************************************
 * @brief 打开直流泵2
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void openPump2(void)
{
    relay_ON(Relay9);
    outputState.pump2 = 1;
}
/**
 * ************************************************************************
 * @brief 关闭直流泵2
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void closePump2(void)
{
    relay_OFF(Relay9);
    outputState.pump2 = 0;
}
/**
 * ************************************************************************
 * @brief 打开清洗电磁阀
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-19
 * 
 * ************************************************************************
 */
void openWashValve(void)
{
    relay_ON(Relay4);
    outputState.washValve = 1;
    record.data.waterTapwashCnt++;
    write_record_data(RECORD_WATER_TAP_WASH_CNT);    //记录清洗次数
}

/**
 * ************************************************************************
 * @brief 关闭清洗电磁阀
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-19
 * 
 * ************************************************************************
 */
void closeWashValve(void)
{
    relay_OFF(Relay4);
    outputState.washValve = 0;
}
/**
 * ************************************************************************
 * @brief 记录输出状态
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void recordOutputState(void)
{
    outputState.recordState[0] = outputState.circulationPump;
    outputState.recordState[1] = outputState.relay2;
    outputState.recordState[2] = outputState.inletValve;
    outputState.recordState[3] = outputState.washValve;
    outputState.recordState[4] = outputState.circulationValve;
    outputState.recordState[5] = outputState.drainValve;
    outputState.recordState[6] = outputState.cool;
}


//根据记录状态恢复输出
/**
 * ************************************************************************
 * @brief 根据记录状态恢复输出
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void restoreOutputState(void)
{
    if(outputState.recordState[0] == 1)
    {
        openCirculationPump();
    }
    else
    {
        closeCirculationPump();
    }
    
    if(outputState.recordState[1] == 1)
    {
        relay_ON(Relay2);
    }
    else
    {
        relay_OFF(Relay2);
    }
    
    if(outputState.recordState[2] == 1)
    {
        openInletValve();
    }
    else
    {
        closeInletValve();
    }
    //清洗电磁阀不处理
    // if(outputState.recordState[3] == 1)
    // {
    //     relay_ON(Relay4);
    // }
    // else
    // {
    //     relay_OFF(Relay4);
    // }
    if(outputState.recordState[4] == 1)
    {
        openCirculationValve();
    }
    else
    {
        closeCirculationValve();
    }
    if(outputState.recordState[5] == 1)
    {
        drainValveExtract();
    }
    else
    {
        drainValveSewage();
    }
    if(outputState.recordState[6] == 1)
    {
        startCooling();
    }
    else
    {
        stopCooling();
    }
}


