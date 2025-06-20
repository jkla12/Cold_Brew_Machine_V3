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
 * @brief  ��ϴ��ʼ��
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
    wash.state = 0;          //״̬  0���У�1��ʼ��2��ͣ
    wash.step = 0;           //����
    wash.startTime = 0;     //��ʼʱ�� ��
    wash.time = 0;
    wash.pauseTime = 0;     //��ͣʱ�� ��
    wash.pauseStartTime = 0;//��ͣ��ʼʱ�� ��
    wash.pauseEndTime = 0;  //��ͣ����ʱ�� ��
    wash.repeatCnt = 0;     //��ϴ�ظ�����  
    wash.endTime = 0;      //����ʱ��
}



/**
 * ************************************************************************
 * @brief ��ϴ��������
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
    uint16_t timeLeft;  //ʣ��ʱ��
    static uint16_t lastTimeLeft;  //�ϴ�ʣ��ʱ��
    static uint8_t washCnt = 0;         //����ѭ����ϴ����
    uint8_t waterLevelFlag = 0;
    switch(wash.step)
    {
        case 0: //��ʼ��
            deviceRunState = 1;    //XXX����״̬
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
            waterLevel.drainOption = 0; //�ŷ�ˮ
            wash.repeatCnt = 0;
            wash.startTime = 0;
            wash.pauseTime = 0;
            log_d("Wash Start");
            wash.state = 1;
            washCnt = 0;        
            wash.step = 1; //����Ϊ��һ����ˮ
            break;
        case 1: //��һ����ϴ��ˮ
            waterLevelFlag = waterInletControl(config.data.washFirstVolume);
            if(waterLevelFlag == 1) //��ˮ���
            {
                log_d("Wash First Water Inlet Complete");
                wash.step = 2;
                wash.startTime = Timer.system1Sec-wash.pauseTime;   //��¼��ʼʱ��
                wash.time = wash.startTime;
                washCnt = 1;   
            }
            else if(waterLevelFlag == 2)    //��ˮ��ʱ
            {
                log_d("Wash First Water Inlet Timeout");
                wash.step = 0xFF;
                
            }
            else   //������ˮ
            {   
                if(waterLevel.vol>20 && outputState.circulationPump == 0)   //�����趨ˮ������ˮ��
                {
                    openCirculationPump();  //����ˮ��
                    log_d("Wash First Water Inlet Start Pump");
                }
            }
            washShowTime(config.data.washTime,config.data.washTime);
            break;
        case 2://��ˮ���
            if(washCnt<=config.data.washLoopTimes)
            {
                if(Timer.system1Sec-wash.time-wash.pauseTime<(config.data.washSingleTime*washCnt)+(config.data.washPauseTime*(washCnt-1)))
                {
                    //����ˮ�ó�ϴ
                    if(outputState.circulationPump == 0)
                    {
                        openCirculationPump();
                        log_d("wash first open pump %d" ,washCnt);
                    }
                    //XXX ��μ�������
                    if(outputState.cool == 0)
                    {
                        startCooling();
                    }
                    timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //����ʣ��ʱ��
                    if(lastTimeLeft != timeLeft)
                    {
                        lastTimeLeft = timeLeft;
                        washShowTime(config.data.washTime,timeLeft);
                    }
                    
                }
                else if(Timer.system1Sec-wash.time-wash.pauseTime<((config.data.washSingleTime+config.data.washPauseTime)*washCnt))
                {
                    //�ر�ˮ��
                    if(outputState.circulationPump == 1)
                    {
                        log_d("wash first close pump %d",washCnt);
                        closeCirculationPump();
                    }
                    if(outputState.cool == 1)
                    {
                        stopCooling();
                    }
                    timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //����ʣ��ʱ��
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
                //����ˮ�ó�ϴ
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
        case 4: //��ˮ
            timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //����ʣ��ʱ��
            if(lastTimeLeft != timeLeft)
            {
                lastTimeLeft = timeLeft;
                washShowTime(config.data.washTime,timeLeft);
            }
            
            waterLevel.drainOption = 0; //�ŷ�ˮ
            waterLevelFlag = waterdrainControl();
            if(waterLevelFlag == 1)
            {
                log_d("wash first drain end");
                log_v("wash second start vol:%d",config.data.washSecondVolume);
                wash.repeatCnt++;
                wash.step = 5;
            }
            break;
        case 5: //������ˮ
            waterLevelFlag = waterInletControl(config.data.washSecondVolume);
            if(waterLevelFlag == 1)
            {
                log_d("wash sencond inlet end");
                wash.time = Timer.system1Sec-wash.pauseTime;   //��¼��ʼʱ��
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
                  if(waterLevel.vol>20 && outputState.circulationPump == 0)   //�����趨ˮ������ˮ��
                {
                    openCirculationPump();  //����ˮ��
                    log_d("Wash Second Water Inlet Start Pump");
                }
            }
            timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //����ʣ��ʱ��
            if(lastTimeLeft != timeLeft)
            {
                lastTimeLeft = timeLeft;
                washShowTime(config.data.washTime,timeLeft);
            }
            break;
        case 6: //�ڶ���ѭ����ϴ
            if(washCnt<=config.data.washLoopTimes)
            {
                if(Timer.system1Sec-wash.time-wash.pauseTime<(config.data.washSingleTime*washCnt)+(config.data.washPauseTime*(washCnt-1)))
                {
                    //����ˮ�ó�ϴ
                    if(outputState.circulationPump == 0)
                    {
                        openCirculationPump();
                        log_d("wash second open pump %d",washCnt);
                    }
                    //XXX ��μ�������
                    if(outputState.cool == 0)
                    {
                        startCooling();
                    }
                    timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //����ʣ��ʱ��
                    if(lastTimeLeft != timeLeft)
                    {
                        lastTimeLeft = timeLeft;
                        washShowTime(config.data.washTime,timeLeft);
                    }
                    
                }
                else if(Timer.system1Sec-wash.time-wash.pauseTime<((config.data.washSingleTime+config.data.washPauseTime)*washCnt))
                {
                    //�ر�ˮ��
                    if(outputState.circulationPump == 1)
                    {
                        log_d("wash second close pump %d",washCnt);
                        closeCirculationPump();
                    }
                    if(outputState.cool == 1)
                    {
                        stopCooling();
                    }
                    timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //����ʣ��ʱ��
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
            timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //����ʣ��ʱ��
            if(lastTimeLeft != timeLeft)
            {
                lastTimeLeft = timeLeft;
                washShowTime(config.data.washTime,timeLeft);
            }
            
            waterLevel.drainOption = 0; //�ŷ�ˮ
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
            deviceRunState = 0;    //XXX ������ϴ����״̬
            record.data.washCnt++;
            
            write_record_data(RECORD_ALL); //XXX ������������
            AnimationPlayFrame(wash_page,2,0,UART2_ID);
            SetControlEnable(wash_page,4,1,UART2_ID);   //ʹ�ܷ��ؼ�
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
            //[ ] ɾ������
            log_d("delete task washProcessControl");
            delete_task(washProcessControl);
            break;
        case 0xFF: //��ˮ��ʱ
            AnimationPlayFrame(wash_page,2,0,UART2_ID);
            SetControlEnable(wash_page,4,1,UART2_ID);   //ʹ�ܷ��ؼ�
            SetScreen(water_ingress_page,UART2_ID);     //��ˮ����ҳ��
            deviceRunState = 2;    //�쳣
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
            delete_task(washProcessControl);        //[ ]ɾ������
            break;
        default:
            break;
    }   
}

/**
 * ************************************************************************
 * @brief ��ϴ��λ
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
    closeCirculationPump(); //�ر�ѭ����
    waterInletStop();   //��ˮֹͣ
    waterDrainStop();   //��ˮֹͣ
    stopCooling();   //ֹͣ����
}

/**
 * ************************************************************************
 * @brief ��ϴ��ͣ
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
    waterInletStop();   //��ˮֹͣ
    waterDrainStop();
}




/**
 * ************************************************************************
 * @brief ��ϴ�������� ģʽ2 ���԰汾
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
    uint16_t timeLeft;  //ʣ��ʱ��
    static uint16_t lastTimeLeft;  //�ϴ�ʣ��ʱ��
    static uint8_t washCnt = 0;         //����ѭ����ϴ����
    uint8_t waterLevelFlag = 0;
    switch(wash.step)
    {
        case 0: //��ʼ��
            waterLevel.drainOption = 0; //�ŷ�ˮ
            wash.repeatCnt = 0;
            wash.startTime = 0;
            wash.pauseTime = 0;
            log_d("Wash Start");
            wash.state = 1;
            washCnt = 0;        
            wash.startTime = Timer.system1Sec;   //��¼��ʼʱ��
            wash.step = 1; //����Ϊ��һ����ˮ
            break;
        case 1: //��һ����ϴ��ˮ
            log_v("wash step 1");
            drainValveSewage();//�ŷ�ˮ
            openCirculationValve(); //��ˮ
            openInletValve();   //�򿪽�ˮ��ŷ�
            wash.step = 2;
            break;
        case 2:
            
            timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //����ʣ��ʱ��
            if(lastTimeLeft != timeLeft)
            {
                lastTimeLeft = timeLeft;
                washShowTime(config.data.washTime,timeLeft);
            }

            if(Timer.system1Sec - wash.startTime-wash.pauseTime < 120) //TODO ��ϴ120��
            {
                if(waterLevel.vol > 30)
                {
                    if(outputState.circulationPump == 0)
                    {
                        openCirculationPump();  //����ѭ����
                    }
                }
                if(waterLevel.vol < 20)
                {
                    if(outputState.circulationPump == 1)
                    {
                        closeCirculationPump(); //�ر�ѭ����
                    }
                }
            }
            else
            {
                log_d("wash first water inlet end");
                closeInletValve();  //�رս�ˮ��ŷ�
                wash.step = 4;
            }
            break;
        case 4: //��ˮ
            timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //����ʣ��ʱ��
            if(lastTimeLeft != timeLeft)
            {
                lastTimeLeft = timeLeft;
                washShowTime(config.data.washTime,timeLeft);
            }
            
            waterLevel.drainOption = 0; //�ŷ�ˮ
            waterLevelFlag = waterdrainControl();
            if(waterLevelFlag == 1)
            {
                log_d("wash drain end");
                wash.repeatCnt++;
                wash.step = 5;
            }
            break;
        case 5: //������ˮ
            waterLevelFlag = waterInletControl(config.data.washSecondVolume);
            if(waterLevelFlag == 1)
            {
                log_d("wash inlet end");
                wash.time = Timer.system1Sec-wash.pauseTime;   //��¼��ʼʱ��
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
                  if(waterLevel.vol>20 && outputState.circulationPump == 0)   //�����趨ˮ������ˮ��
                {
                    openCirculationPump();  //����ˮ��
                    log_d("Wash Second Water Inlet Start Pump");
                }
            }
            break;
        case 6: //�ڶ���ѭ����ϴ
            if(washCnt<=config.data.washLoopTimes)
            {
                if(Timer.system1Sec-wash.time-wash.pauseTime<(config.data.washSingleTime*washCnt)+(config.data.washPauseTime*(washCnt-1)))
                {
                    //����ˮ�ó�ϴ
                    if(outputState.circulationPump == 0)
                    {
                        openCirculationPump();
                        log_d("wash open pump");
                    }

                    timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //����ʣ��ʱ��
                    if(lastTimeLeft != timeLeft)
                    {
                        lastTimeLeft = timeLeft;
                        washShowTime(config.data.washTime,timeLeft);
                    }
                    
                }
                else if(Timer.system1Sec-wash.time-wash.pauseTime<((config.data.washSingleTime+config.data.washPauseTime)*washCnt))
                {
                    //�ر�ˮ��
                    if(outputState.circulationPump == 1)
                    {
                        log_d("wash close pump");
                        closeCirculationPump();

                        timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //����ʣ��ʱ��
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
            timeLeft = config.data.washTime-(Timer.system1Sec-wash.startTime-wash.pauseTime);   //����ʣ��ʱ��
            if(lastTimeLeft != timeLeft)
            {
                lastTimeLeft = timeLeft;
                washShowTime(config.data.washTime,timeLeft);
            }
            
            waterLevel.drainOption = 0; //�ŷ�ˮ
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
            //[ ] ɾ������
            delete_task(washProcess2Control);
            log_d("delete task washProcess2Control");
            break;
        case 0xFF: //��ˮ��ʱ
            washReset();
            AnimationPlayFrame(wash_page,2,0,UART2_ID);
            SetControlEnable(wash_page,4,1,UART2_ID);   //ʹ�ܷ��ؼ�
            SetScreen(water_ingress_page,UART2_ID);     //��ˮ����ҳ��
            delete_task(washProcess2Control);        //[ ]ɾ������
            log_d("delete task washProcess2Control");
            break;
        default:
            break;
    }   
}
