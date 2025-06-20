/**
 * 
 * @file drain.c
 * @author jiaokai 
 * @brief �ֶ���ˮ��ؿ��ƴ��� 
 * 
 * @copyright Copyright (c) 2025
 */
#include "drain.h"
#include "eeprom.h"
#include "config.h"
#include "lcd_data_process.h"
#include "uart.h"
#include "work.h"
#include "waterLevel.h"
#include "timer.h"
#include "drainPage.h"
#include "app_mqtt.h"
#include "elog.h"
#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "drain"

drain_TypeDef drain;


/**
 * ************************************************************************
 * @brief  �ֶ���ˮֹͣ
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void drainReset(void)
{
    drain.state = 0;      //״̬��0ֹͣ
    drain.step = 0;       //����
    drain.startTime = 0; //��ʼʱ��
    drain.pauseTime = 0; //��ͣʱ��
    drain.pauseStartTime = 0; //��ͣ��ʼʱ��
    drain.pauseEndTime = 0;   //��ͣ����ʱ��
    waterDrainStop(); //ֹͣ��ˮ
    SetControlEnable(drain_page,11,1,UART2_ID);//ʹ����ˮѡ��ť
}

/**
 * ************************************************************************
 * @brief �ֶ���ˮ���̿���
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void drainProcessControl(void)
{
    uint16_t lastTime;
    uint8_t waterFlag;
        switch(drain.step)
        {
            case 0: //��ʼ��
            
                log_v("drain init");
                drain.state = 1; //����Ϊ��ˮ״̬
                drain.startTime = Timer.system1Sec; //��¼��ʼʱ��
                drain.pauseTime = 0; //�����ͣʱ��
                drain.pauseStartTime = 0; //�����ͣ��ʼʱ��
                drain.pauseEndTime = 0; //�����ͣ����ʱ��
                drain.step = 1; //����Ϊ��ˮ��ʼ״̬
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
                break;
            case 1: //��ˮ��ʼ
                lastTime = config.data.drainTime-(Timer.system1Sec-drain.startTime-drain.pauseTime);   //����ʣ��ʱ��
                if(lastTime>config.data.drainTime)
                {
                    lastTime = 1;
                }
                
                drainShowTime(config.data.drainTime,lastTime);
                waterFlag = waterdrainControl(); //��ˮ����
                if(waterFlag == 1)//��ˮ���
                {
                    log_d("drain end");
                    drain.step = 2;
                    drainShowTime(config.data.drainTime,0);
                }
                else if(waterFlag == 2) //��ˮ��ʱ
                {
                    log_d("drain timeout");
                    drain.step = 2;
                }
                break;
            case 2: //��ˮ����
                drain.state = 0; //����Ϊֹͣ״̬
                drain.step = 0; //����Ϊ��ʼ��״̬
                lastTime = 0;
                drain.pauseTime = 0;
                drain.pauseStartTime = 0;
                drain.pauseEndTime = 0;
                write_record_data(RECORD_ALL);   //��¼��������
                AnimationPlayFrame(drain_page,2,0,UART2_ID);
                SetControlEnable(drain_page,4,1,UART2_ID);//ʹ�ܷ��ؼ�
                SetControlEnable(drain_page,11,1,UART2_ID);//ʹ����ˮѡ��ť
                 
                log_d("delete task drainProcessControl");
                log_v("drain end");   
                deviceRunState = 0;    //XXX����״̬
                if(WIFI.mqttConnected == 1)
                {
                    if(publish_drainRelatData())
                    {
                        log_i("publish drain related data");
                    }
                    else
                    {
                        log_e("publish drain related data failed");
                    }
                }
                

                delete_task(drainProcessControl); //FIXME ɾ������    
                break;
            default:
                break;
        }
}

