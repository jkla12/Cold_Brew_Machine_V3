#include "waterLevel.h"
#include "app_filter.h"
#include "adc.h"
#include "relay.h"
#include "eeprom.h"
#include "timer.h"
#include "uart.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "waterLevel"

EwmaFilter pressure_filter;
waterLevel_TypeDef waterLevel;
void waterLevel_init(void)
{
    ewma_filter_init(&pressure_filter, DEFAULT_ALPHA);
    waterLevel.state = 0;
    waterLevel.err = 0;
    waterLevel.vol = 0;
}

/**
 * ************************************************************************
 * @brief 水位更新
 *
 *
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-04-29
 *  10ms计算一次
 * ************************************************************************
 */
void waterLevel_update(void)
{
    static uint8_t errCnt;
    float height, val = 0.0f;
    uint16_t adc_filter;
    adc_filter = ewma_filter_apply(&pressure_filter, adc_val);
    val = (float)adc_filter / 4096.0f * 3300;
    if (val < 100)
    {
        errCnt++;
        if (errCnt > 10) // 连续10次数据异常，判定传感器异常
        {
            waterLevel.sensorErr = 1; // 传感器故障
        }
    }
    else
    {
        errCnt = 0;
        waterLevel.sensorErr = 0;
    }
    if (val < 480)
    {
        val = 480;
    }
    height = (float)(val - 480) * 100 / 3840;
    waterLevel.vol = height * WATER_COEFFICIENT;
}

/**
 * ************************************************************************
 * @brief 进水控制
 *
 *
 * @return 进水状态，0进水中，1进水完成，2上水超时
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-04-29
 *
 * ************************************************************************
 */
uint8_t waterInletControl(uint16_t vol)
{
    static uint16_t lastWaterLevel = 0;
    static uint8_t overtimeCnt = 0;    // 上水超时计数，5秒比对一次上水量，10秒上水少于100ML，判断上水超时
    static uint8_t inletFinishCnt = 0; // 上水完成计数，连续五次大于设定值，表示上水完成
    switch (waterLevel.inlteStep)
    {
    case 0:                                      // 开始进水
        if (waterLevel.vol < (vol + ZERO_LEVEL)) // 水量小于设置水量
        {
            waterLevel.inlteStep = 1; // 进入下一步
            log_d("inlet start vol %d",vol);
        }
        else
        {
            waterLevel.inlteStep = 4; // 上水完成
        }
        return 0;
    case 1:
        overtimeCnt = 0;
        inletFinishCnt = 0;
        waterLevel.state = 1; // 开始进水
        waterLevel.inlteStep = 2; // 进入下一步
        openInletValve();
        waterLevel.inletstartTime = Timer.system1Sec;
        lastWaterLevel = waterLevel.vol;
        log_v("inlet start");
        return 0;
    case 2:
        if (waterLevel.vol > (vol + ZERO_LEVEL))
        {
            inletFinishCnt++;
            if (inletFinishCnt > 5) // 上水完成
            {
                log_d("inlet end");
                closeInletValve(); // 关阀
                waterLevel.inlteStep = 4;
            }
        }
        else
        {
            inletFinishCnt = 0;
            overtimeCnt++;
            if (overtimeCnt > 150) // 10秒计算一下和上次的水位差值，差值小于100ML，判断进水故障
            {
                overtimeCnt = 0;
                if (waterLevel.vol - lastWaterLevel < 1) // 10秒水量差小于1
                {
                    log_d("inlet over time");
                    waterLevel.err = 1; // 进水故障
                    waterLevel.inlteStep = 3;
                    closeInletValve();
                }
                lastWaterLevel = waterLevel.vol; // 更新上一次的水位
            }
            if (Timer.system1Sec - waterLevel.inletstartTime > INLET_OVER_TIME) // 进水时间大于设置的超时时间
            {
                waterLevel.err = 1; // 进水故障
                waterLevel.inlteStep = 3;
                closeInletValve();
            }
        }
        return 0;
    case 3: // 上水超时
        waterInletStop();
        return 2;

    case 4: // 上水完成
        waterLevel.inlteStep = 0;
        return 1;
    default:
        return 0;
    }
}

/**
 * ************************************************************************
 * @brief 排水控制
 *
 *
 * @return 0 未完成，1 完成
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-08
 *
 * ************************************************************************
 */
uint8_t waterdrainControl(void)
{
    //static uint16_t lastWaterLevel = 0;
    switch (waterLevel.drainStep)
    {
    case 0: // 判断排水选择
        waterLevel.state = 2;
        if (waterLevel.drainOption == 0) // 排废水
        {
            log_d("drain sewage");
            drainValveSewage();
            waterLevel.drainstartTime = Timer.system1Sec;
            waterLevel.drainStep = 1;
        }
        else // 排萃取液
        {
            log_d("drain extract");
            drainValveExtract();
            waterLevel.drainstartTime = Timer.system1Sec;
            waterLevel.drainStep = 1;
        }
        return 0;
    case 1: // 等待排水三通球阀运动到位
        if (Timer.system1Sec - waterLevel.drainstartTime > DRAIN_VALVE_SWICTH_TIME)
        {
            waterLevel.drainStep = 2;
        }
        return 0;
    case 2: // 打开循环三通球阀
    {
        log_d("open Circulation Valve");
        openCirculationValve();
        waterLevel.drainstartTime = Timer.system1Sec;
        waterLevel.drainStep = 3;
    }
        return 0;
    case 3:
        if (Timer.system1Sec - waterLevel.drainstartTime > CIRCULATION_VALVE_SWICTH_TIME)
        {
            waterLevel.drainStep = 4;
        }
        return 0;
    case 4: // 开启水泵加速排水
        log_d("open Circulation pump");
        openCirculationPump();
        waterLevel.drainStep = 5;
        return 0;
    case 5:
        if (waterLevel.vol < ZERO_LEVEL) // 水位小于0位水位，延迟排水
        {
            waterLevel.draindelayTime = Timer.system1Sec; // 记录时间
            waterLevel.drainStep = 6;
        }
        return 0;
    case 6:
        if (Timer.system1Sec - waterLevel.draindelayTime > DRAIN_DELAY_TIME) // 等待排水延迟
        {
            waterLevel.drainStep = 7;
        }
        return 0;
    case 7:
        log_d("close Circulation valve");
        closeCirculationPump();                       // 关闭循环泵
        closeCirculationValve();                      // 关闭循环阀
        waterLevel.drainstartTime = Timer.system1Sec; // 记录当前时间
        waterLevel.drainStep = 8;
        return 0;
    case 8:
        if (Timer.system1Sec - waterLevel.drainstartTime > CIRCULATION_VALVE_SWICTH_TIME) // 等待循环三通球阀运动到位
        {
            waterLevel.drainStep = 9;
        }
        return 0;
    case 9: // 结束排水
        log_d("drain end");

        drainValveSewage(); // 三通球阀切换到污水
        waterLevel.drainStep = 0;
        waterLevel.state = 0;
        return 1;
	default:
        return 0;
    }
}

/**
 * ************************************************************************
 * @brief 进水停止
 *
 *
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-08
 *
 * ************************************************************************
 */
void waterInletStop(void)
{
    if(waterLevel.state == 1)
    {
        log_d("inlet stop");
    }
    
    closeInletValve();
    waterLevel.state = 0;
    waterLevel.inlteStep = 0;
}

/**
 * ************************************************************************
 * @brief 排水停止
 *
 *
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-08
 *
 * ************************************************************************
 */
void waterDrainStop(void)
{
    if(waterLevel.state == 2)
    {
        log_d("drain stop");
    }
    
    drainValveSewage();
    closeCirculationPump();
    closeCirculationValve();
    waterLevel.state = 0;
    waterLevel.drainStep = 0;
}
