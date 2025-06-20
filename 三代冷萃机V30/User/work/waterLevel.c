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
 * @brief ˮλ����
 *
 *
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-04-29
 *  10ms����һ��
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
        if (errCnt > 10) // ����10�������쳣���ж��������쳣
        {
            waterLevel.sensorErr = 1; // ����������
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
 * @brief ��ˮ����
 *
 *
 * @return ��ˮ״̬��0��ˮ�У�1��ˮ��ɣ�2��ˮ��ʱ
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
    static uint8_t overtimeCnt = 0;    // ��ˮ��ʱ������5��ȶ�һ����ˮ����10����ˮ����100ML���ж���ˮ��ʱ
    static uint8_t inletFinishCnt = 0; // ��ˮ��ɼ�����������δ����趨ֵ����ʾ��ˮ���
    switch (waterLevel.inlteStep)
    {
    case 0:                                      // ��ʼ��ˮ
        if (waterLevel.vol < (vol + ZERO_LEVEL)) // ˮ��С������ˮ��
        {
            waterLevel.inlteStep = 1; // ������һ��
            log_d("inlet start vol %d",vol);
        }
        else
        {
            waterLevel.inlteStep = 4; // ��ˮ���
        }
        return 0;
    case 1:
        overtimeCnt = 0;
        inletFinishCnt = 0;
        waterLevel.state = 1; // ��ʼ��ˮ
        waterLevel.inlteStep = 2; // ������һ��
        openInletValve();
        waterLevel.inletstartTime = Timer.system1Sec;
        lastWaterLevel = waterLevel.vol;
        log_v("inlet start");
        return 0;
    case 2:
        if (waterLevel.vol > (vol + ZERO_LEVEL))
        {
            inletFinishCnt++;
            if (inletFinishCnt > 5) // ��ˮ���
            {
                log_d("inlet end");
                closeInletValve(); // �ط�
                waterLevel.inlteStep = 4;
            }
        }
        else
        {
            inletFinishCnt = 0;
            overtimeCnt++;
            if (overtimeCnt > 150) // 10�����һ�º��ϴε�ˮλ��ֵ����ֵС��100ML���жϽ�ˮ����
            {
                overtimeCnt = 0;
                if (waterLevel.vol - lastWaterLevel < 1) // 10��ˮ����С��1
                {
                    log_d("inlet over time");
                    waterLevel.err = 1; // ��ˮ����
                    waterLevel.inlteStep = 3;
                    closeInletValve();
                }
                lastWaterLevel = waterLevel.vol; // ������һ�ε�ˮλ
            }
            if (Timer.system1Sec - waterLevel.inletstartTime > INLET_OVER_TIME) // ��ˮʱ��������õĳ�ʱʱ��
            {
                waterLevel.err = 1; // ��ˮ����
                waterLevel.inlteStep = 3;
                closeInletValve();
            }
        }
        return 0;
    case 3: // ��ˮ��ʱ
        waterInletStop();
        return 2;

    case 4: // ��ˮ���
        waterLevel.inlteStep = 0;
        return 1;
    default:
        return 0;
    }
}

/**
 * ************************************************************************
 * @brief ��ˮ����
 *
 *
 * @return 0 δ��ɣ�1 ���
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
    case 0: // �ж���ˮѡ��
        waterLevel.state = 2;
        if (waterLevel.drainOption == 0) // �ŷ�ˮ
        {
            log_d("drain sewage");
            drainValveSewage();
            waterLevel.drainstartTime = Timer.system1Sec;
            waterLevel.drainStep = 1;
        }
        else // ����ȡҺ
        {
            log_d("drain extract");
            drainValveExtract();
            waterLevel.drainstartTime = Timer.system1Sec;
            waterLevel.drainStep = 1;
        }
        return 0;
    case 1: // �ȴ���ˮ��ͨ���˶���λ
        if (Timer.system1Sec - waterLevel.drainstartTime > DRAIN_VALVE_SWICTH_TIME)
        {
            waterLevel.drainStep = 2;
        }
        return 0;
    case 2: // ��ѭ����ͨ��
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
    case 4: // ����ˮ�ü�����ˮ
        log_d("open Circulation pump");
        openCirculationPump();
        waterLevel.drainStep = 5;
        return 0;
    case 5:
        if (waterLevel.vol < ZERO_LEVEL) // ˮλС��0λˮλ���ӳ���ˮ
        {
            waterLevel.draindelayTime = Timer.system1Sec; // ��¼ʱ��
            waterLevel.drainStep = 6;
        }
        return 0;
    case 6:
        if (Timer.system1Sec - waterLevel.draindelayTime > DRAIN_DELAY_TIME) // �ȴ���ˮ�ӳ�
        {
            waterLevel.drainStep = 7;
        }
        return 0;
    case 7:
        log_d("close Circulation valve");
        closeCirculationPump();                       // �ر�ѭ����
        closeCirculationValve();                      // �ر�ѭ����
        waterLevel.drainstartTime = Timer.system1Sec; // ��¼��ǰʱ��
        waterLevel.drainStep = 8;
        return 0;
    case 8:
        if (Timer.system1Sec - waterLevel.drainstartTime > CIRCULATION_VALVE_SWICTH_TIME) // �ȴ�ѭ����ͨ���˶���λ
        {
            waterLevel.drainStep = 9;
        }
        return 0;
    case 9: // ������ˮ
        log_d("drain end");

        drainValveSewage(); // ��ͨ���л�����ˮ
        waterLevel.drainStep = 0;
        waterLevel.state = 0;
        return 1;
	default:
        return 0;
    }
}

/**
 * ************************************************************************
 * @brief ��ˮֹͣ
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
 * @brief ��ˮֹͣ
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
