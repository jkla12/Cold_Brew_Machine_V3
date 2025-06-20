/**
 * 
 * @file sanit.c
 * @author jiaokai 
 * @brief ������س���
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
 * @brief ������ʼ��
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
    sanit.state = 0;          //״̬  0���У�1��ʼ��2��ͣ
    sanit.step = 0;           //����
    sanit.startTime = 0;     //��ʼʱ�� ��
    sanit.time = 0;
    sanit.pauseTime = 0;     //��ͣʱ�� ��
    sanit.pauseStartTime = 0;//��ͣ��ʼʱ�� ��
    sanit.pauseEndTime = 0;  //��ͣ����ʱ�� ��
    sanit.repeatCnt = 0;     //��ϴ�ظ�����  
    sanit.endTime = 0;      //����ʱ��
}

/**
 * ************************************************************************
 * @brief ��������
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
    uint16_t timeLeft;   //ʣ��ʱ��
    static uint16_t lastTimeLeft = 0;   //�ϴ�ʣ��ʱ��
    static uint8_t repeatCnt = 0;   //�ظ�����
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
            sanit.step = 1;   //��ʼ����
            repeatCnt = 0;
            deviceRunState = 1;   //XXX ����״̬
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
            waterFlag = waterInletControl(SANIT_VOLUME);   //��ˮ
            if(waterFlag == 2)//��ˮ��ʱ
            {
                sanit.step = 0xFF;   //��ˮ��ʱ
                log_d("sanit inlet over time");
            }
            else if(waterFlag == 1) //��ˮ���
            {
                log_d("sanit inlet end");
                sanit.step = 2;   //��ˮ���
                sanit.time = Timer.system1Sec-sanit.pauseTime;   //��¼��ʼʱ��
                sanit.startTime = Timer.system1Sec-sanit.pauseTime;   //��¼��ʼʱ��
                sanit.time = sanit.startTime;   
                repeatCnt = 1;
            }
            else //��ˮ��
            {
                if(waterLevel.vol>20 && outputState.circulationPump == 0)   //�����趨ˮ������ˮ��
                {
                    log_d("sanit open pump");
                    openCirculationPump();   //����ѭ����
                    startCooling();   //������ȴ ����
                }
            }
            break;
        case 2:
            if(repeatCnt <= SANIT_STEP1_NUM)
            {
                if(Timer.system1Sec-sanit.startTime-sanit.pauseTime < (SANIT_SINGLE_TIME*repeatCnt)+(SANIT_PAUSE_TIME*(repeatCnt-1)))
                {
                    //����ˮ�ó�ϴ
                    if(outputState.circulationPump == 0)
                    {
                        openCirculationPump();
                        log_d("sanit open pump");
                    }
                    timeLeft = SANIT_TIME-(Timer.system1Sec-sanit.startTime-sanit.pauseTime);   //����ʣ��ʱ��
                    if(lastTimeLeft != timeLeft)
                    {
                        lastTimeLeft = timeLeft;
                        sanitShowTime(SANIT_TIME,timeLeft);
                    }
                }
                else if(Timer.system1Sec-sanit.startTime-sanit.pauseTime < ((SANIT_SINGLE_TIME+SANIT_PAUSE_TIME)*repeatCnt))
                {
                    //�ر�ˮ��
                    if(outputState.circulationPump == 1)
                    {
                        log_d("sanit close pump");
                        closeCirculationPump();
                        timeLeft = SANIT_TIME-(Timer.system1Sec-sanit.startTime-sanit.pauseTime);   //����ʣ��ʱ��
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
                sanit.step = 3;   //�������
                repeatCnt = 1;
                stopCooling();   //ֹͣ����
            }
            break;
        case 3://��ˮ
            timeLeft = SANIT_TIME-(Timer.system1Sec-sanit.startTime-sanit.pauseTime);   //����ʣ��ʱ��
            if(lastTimeLeft != timeLeft)
            {
                lastTimeLeft = timeLeft;
                sanitShowTime(SANIT_TIME,timeLeft);
            }
            waterLevel.drainOption = 0; //�ŷ�ˮ
            waterFlag = waterdrainControl();
            if(waterFlag == 1)
            {
                log_d("sanit drain end");
                sanit.repeatCnt++;
                sanit.step = 4;
            }
            break;
        case 4: //������ˮ
            waterFlag = waterInletControl(SANIT_VOLUME);   //��ˮ
            if(waterFlag == 1)
            {
                log_d("sanit inlet end");
                sanit.time = Timer.system1Sec-sanit.pauseTime;   //��¼��ʼʱ��
                repeatCnt = 1;
                sanit.step = 5;   //��ˮ���
            }
            else if(waterFlag == 2)//��ˮ��ʱ
            {
                sanit.step = 0xFF;   //��ˮ��ʱ
                log_d("sanit inlet over time");
            }
            else //��ˮ��
            {
                if(waterLevel.vol>20 && outputState.circulationPump == 0)   //�����趨ˮ������ˮ��
                {
                    log_d("sanit open pump");
                    openCirculationPump();   //����ѭ����
                    startCooling();   //������ȴ ����
                }
            }
            timeLeft = SANIT_TIME-(Timer.system1Sec-sanit.startTime-sanit.pauseTime);   //����ʣ��ʱ��
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
                    //����ˮ�ó�ϴ
                    if(outputState.circulationPump == 0)
                    {
                        openCirculationPump();
                        log_d("sanit open pump");
                    }
                    timeLeft = SANIT_TIME-(Timer.system1Sec-sanit.startTime-sanit.pauseTime);   //����ʣ��ʱ��
                    if(lastTimeLeft != timeLeft)
                    {
                        lastTimeLeft = timeLeft;
                        sanitShowTime(SANIT_TIME,timeLeft);
                    }
                }
                else if(Timer.system1Sec-sanit.startTime-sanit.pauseTime < ((SANIT_SINGLE_TIME+SANIT_PAUSE_TIME)*repeatCnt))
                {
                    //�ر�ˮ��
                    if(outputState.circulationPump == 1)
                    {
                        log_d("sanit close pump");
                        closeCirculationPump();
                        timeLeft = SANIT_TIME-(Timer.system1Sec-sanit.startTime-sanit.pauseTime);   //����ʣ��ʱ��
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
                sanit.step = 6;   //�������
                repeatCnt = 1;
                stopCooling();   //ֹͣ����
            }
            break;
        case 6:
            timeLeft = SANIT_TIME-(Timer.system1Sec-sanit.startTime-sanit.pauseTime);   //����ʣ��ʱ��
            if(lastTimeLeft != timeLeft)
            {
                lastTimeLeft = timeLeft;
                sanitShowTime(SANIT_TIME,timeLeft);
            }
            waterLevel.drainOption = 0; //�ŷ�ˮ
            waterFlag = waterdrainControl();
            if(waterFlag == 1)
            {
                log_d("sanit drain end");
                sanit.repeatCnt++;
                sanitShowTime(config.data.washTime,0);
                sanit.step = 7;
            }
            break;
        case 7: //����
            timeLeft = 0;
            sanit.state = 0;
            sanit.step = 0;  
            sanit.startTime = 0;
            sanit.time = 0;
            sanit.pauseTime = 0;
            sanit.pauseStartTime = 0;
            sanit.pauseEndTime = 0;
            sanit.repeatCnt = 0;
            deviceRunState = 0;   //XXX ����״̬
        
            record.data.sanitCnt++;
            write_record_data(RECORD_ALL);   
            SetControlEnable(sanit_page,4,1,UART2_ID);//
			AnimationPlayFrame(sanit_page,2,0,UART2_ID);   //����
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
            delete_task(sanitProcessControl);        //[ ]ɾ������
            log_v("sanit end");
            break;
        case 0xFF:
            
            deviceRunState = 2;   //XXX ����״̬
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
            AnimationPlayFrame(sanit_page,2,0,UART2_ID);   //����
            SetControlEnable(sanit_page,4,1,UART2_ID);   //ʹ�ܷ��ؼ�
            SetScreen(water_ingress_page,UART2_ID);     //��ˮ����ҳ��
            sanitReset();   //��λ
            delete_task(sanitProcessControl);        //[ ]ɾ������
            log_d("delete task sanitProcessControl");
            break;
        default:
            break;
    }
}


/**
 * ************************************************************************
 * @brief ��ͣ����
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
    waterInletStop();   //��ˮֹͣ
    waterDrainStop();   //��ˮֹͣ
}

/**
 * ************************************************************************
 * @brief ��������λ
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void sanitReset(void)   //������λ
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
    closeCirculationPump(); //�ر�ѭ����
    waterInletStop();   //��ˮֹͣ
    waterDrainStop();   //��ˮֹͣ
}


