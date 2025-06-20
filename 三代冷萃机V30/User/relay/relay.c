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
 * @brief �򿪽�ˮ��ŷ�
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
        //TODO �����ϱ���ŷ�����
    }
    
    outputState.inletValve = 1;
}

/**
 * ************************************************************************
 * @brief �رս�ˮ��ŷ�
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
 * @brief ��ͨ���л�������ȡҺ
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
        record.data.drainValveCnt++;    //��ˮ��ͨ�򷧼�����һ
        //TODO �����ϱ���ˮ��ͨ�򷧶�������
    }
    outputState.drainValve = 1;

}

/**
 * ************************************************************************
 * @brief ��ͨ���л����ŷ�ˮ
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
    //�ŷ�ˮ
    relay_OFF(Relay6);
    outputState.drainValve = 0;
}

/**
 * ************************************************************************
 * @brief ѭ����ͨ���л�����ˮλ��
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
        //TODO �����ϱ�ѭ����ͨ�򷧶�������
        
    }
    outputState.circulationValve = 1;
}

/**
 * ************************************************************************
 * @brief  ѭ����ͨ���л���ѭ��λ��
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
 * @brief ����ѭ����
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
    if(outputState.circulationPump == 0)    //��ǰ�ǹر�״̬
    {   
        outputState.pumpOpeningTime = Timer.system1Sec; //  ��¼��ʼ����ʱ��
    }
    outputState.circulationPump = 1;
}

/**
 * ************************************************************************
 * @brief �ر�ѭ����
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
    
    if(outputState.circulationPump == 1)    //��ǰ�ǿ���״̬����¼����ʱ��
    {   
        outputState.pumpCurrentRuntime = outputState.pumpCurrentRuntime + (Timer.system1Sec - outputState.pumpOpeningTime);    //��¼���ο���ʱ��
        if(outputState.pumpCurrentRuntime>=60)
        {
            record.data.circulationPumpRunTime += outputState.pumpCurrentRuntime/60;    //��¼����ʱ��
            outputState.pumpCurrentRuntime = outputState.pumpCurrentRuntime%60;             //������
        }
        write_record_data(RECORD_CIRCULATION_PUMP_RUN_TIME);    //��¼ѭ��������ʱ��
    }
    outputState.circulationPump = 0;
}

/**
 * ************************************************************************
 * @brief ��������
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
        outputState.coolOpeningTime = Timer.system1Sec; //��¼����Ƭ����ʱ��
    }
    outputState.cool = 1;
}

/**
 * ************************************************************************
 * @brief ��������
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
        outputState.coolCurrentRuntime = outputState.coolCurrentRuntime + (Timer.system1Sec - outputState.coolOpeningTime);    //��¼���ο���ʱ��
        if(outputState.coolCurrentRuntime>=60)
        {
            record.data.coolingRunTime += outputState.coolCurrentRuntime/60;    //��¼����ʱ��
            outputState.coolCurrentRuntime = outputState.coolCurrentRuntime%60;             //������
        }
        write_record_data(RECORD_COOLING_RUN_TIME);    //��¼��������ʱ��
    }
    outputState.cool = 0;
}

/**
 * ************************************************************************
 * @brief ��ֱ����1
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
        outputState.pump1OpeningTime = Timer.system1Sec; //  ��¼��ʼ����ʱ��
    }
    outputState.pump1 = 1;
}
/**
 * ************************************************************************
 * @brief �ر�ֱ����1
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
    if(outputState.pump1 == 1)    //��ǰ�ǿ���״̬����¼����ʱ��
    {   
        outputState.pump1CurrentRuntime = outputState.pump1CurrentRuntime + (Timer.system1Sec - outputState.pump1OpeningTime);    //��¼���ο���ʱ��
        if(outputState.pump1CurrentRuntime>=60)
        {
            record.data.DCPumpRunTime += outputState.pump1CurrentRuntime/60;    //��¼����ʱ��
            outputState.pump1CurrentRuntime = outputState.pump1CurrentRuntime%60;             //������
        }
        write_record_data(RECORD_DC_PUMP_RUN_TIME);    //��¼ֱ��������ʱ��
    }
    outputState.pump1 = 0;
}
/**
 * ************************************************************************
 * @brief ��ֱ����2
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
 * @brief �ر�ֱ����2
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
 * @brief ����ϴ��ŷ�
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
    write_record_data(RECORD_WATER_TAP_WASH_CNT);    //��¼��ϴ����
}

/**
 * ************************************************************************
 * @brief �ر���ϴ��ŷ�
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
 * @brief ��¼���״̬
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


//���ݼ�¼״̬�ָ����
/**
 * ************************************************************************
 * @brief ���ݼ�¼״̬�ָ����
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
    //��ϴ��ŷ�������
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


