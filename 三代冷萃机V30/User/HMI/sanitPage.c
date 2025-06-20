/**
 * 
 * @file sanit.c
 * @author jiaokai 
 * @brief ����ҳ����Ӧ����
 * 
 * @copyright Copyright (c) 2025
 */
#include "sanitPage.h"
#include "lcd_data_process.h"
#include "waterLevel.h"
#include "uart.h"
#include "string.h"
#include "sanit.h"
#include "timer.h"
#include "eeprom.h"
#include "config.h"
#include "work.h"
#include "relay.h"
#include "sanit.h"
#include "app_mqtt.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "SANIT"

/**
 * ************************************************************************
 * @brief  ����ҳ�水ť��Ӧ����
 * 
 * @param[in] num  ���
 * @param[in] state  ״̬
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void sanitPageButton(uint8_t num,uint8_t state)
{
    switch(num)
    {
        case 3:
            if(state == 1)
            {
                if(sanit.state == 0)//����״̬
                {
                    waterLevel.drainOption = 0;//�ŷ�ˮ
                    AnimationPlayFrame(sanit_page,2,1,UART2_ID);
                    SetControlEnable(sanit_page,4,0,UART2_ID); //���÷��ذ�ť������
                    add_task(sanitProcessControl,NULL,100,true); //[ ]�������
                    log_d("add sanitProcessControl task");
                    log_v("sanit start");
                }
                else if(sanit.state == 1) //��ǰ��������������
                {
                    sanit.state = 2; //����Ϊ��ͣ״̬
                    recordOutputState();    //��¼��ǰ���״̬
                    disable_task(sanitProcessControl); //[ ]��ͣ����
                    log_d("disable sanitProcessControl task");
                    sanitPause();   //��ͣ����
                    sanit.pauseStartTime = Timer.system1Sec;   //��¼��ͣ��ʼʱ��
                    AnimationPlayFrame(sanit_page,2,0,UART2_ID);
                    SetControlEnable(sanit_page,4,1,UART2_ID); //���÷��ذ�ť����
                    log_v("sanit pause");
                }
                else if(sanit.state == 2) //��ǰ������ͣ״̬
                {
                    sanit.state = 1; //����Ϊ����״̬
                    AnimationPlayFrame(sanit_page,2,1,UART2_ID);
                    SetControlEnable(sanit_page,4,0,UART2_ID); //���÷��ذ�ť������
                    enable_task(sanitProcessControl);  //[ ] ʹ������
                    log_d("enable sanitProcessControl task");
                    sanit.pauseEndTime = Timer.system1Sec;   //��¼��ͣ����ʱ��
                    sanit.pauseTime += (sanit.pauseEndTime - sanit.pauseStartTime);   //������ͣʱ��
                    restoreOutputState();   //�ָ����״̬
                    log_v("sanit continue");
                }
            }
            break;
        case 4:
            if(state == 1)
            {

                SetScreen(main_page,UART2_ID);          //������ҳ��
                deviceRunState = 0;   //XXX ����״̬
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
                if(sanit.state != 0) //��ǰ��������������
                {
                    delete_task(sanitProcessControl); //[ ] ��ͣ����
                    log_d("disable sanitProcessControl task");
                }
                sanitReset();   //������λ
                
            }
            break;
        default:
            break; 
            
    }
}

/**
 * ************************************************************************
 * @brief  ��ʾ��������ʱ
 * 
 * @param[in] setTime  ����ʱ��
 * @param[in] remainTime  ʣ��ʱ��
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void sanitShowTime(uint32_t setTime,uint32_t remainTime)
{
    uint8_t lastMin,lastSec;   //ʣ�������
    char str[10];               //�����ַ���
    uint8_t strLen;             //�ַ�������
    uint8_t len;
    uint32_t progressBarValue;  //������ֵ
    if(remainTime>SANIT_TIME)  //ʣ��ʱ���������ʱ��
    {
        remainTime = 1;
    }
    lastMin = remainTime/60;
    lastSec = remainTime%60;
    uart2_struct.tx_buf[0] = 0xEE;
    uart2_struct.tx_buf[1] = 0xB1;
    uart2_struct.tx_buf[2] = 0x12;
    uart2_struct.tx_buf[3] = 0x00;
    uart2_struct.tx_buf[4] = sanit_page;   //ҳ���ַ
    //��ʾ���Ӳ���
    uart2_struct.tx_buf[5] = 0x00;
    uart2_struct.tx_buf[6] = 0x08;
    uart2_struct.tx_buf[7] = 0x00;
    sprintf(str,"%d",lastMin);
    strLen = strlen(str);
    uart2_struct.tx_buf[8] = strLen;
    for(uint8_t i=0;i<strLen;i++)
    {
        uart2_struct.tx_buf[9+i] = str[i];
    }
    len = 9+strLen;
    //��ʾ�벿��
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len+1] = 0x09;
    uart2_struct.tx_buf[len+2] = 0x00;
    sprintf(str,"%d",lastSec);
    strLen = strlen(str);
    uart2_struct.tx_buf[len+3] = strLen;
    for(uint8_t i=0;i<strLen;i++)
    {
        uart2_struct.tx_buf[len+4+i] = str[i];
    }
    len = len+4+strLen;
    //��ʾ������
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len+1] = 0x05;
    uart2_struct.tx_buf[len+2] = 0x00;
    uart2_struct.tx_buf[len+3] = 0x04;
    uart2_struct.tx_buf[len+4] = 0x00;
    uart2_struct.tx_buf[len+5] = 0x00;
    uart2_struct.tx_buf[len+6] = 0x00;
    if(setTime == 0)
    {
        progressBarValue = 100; //����ʱ��Ϊ0��������Ϊ100%
    }
    else
    {
        progressBarValue = ((uint32_t)(setTime-remainTime)*100)/setTime; //���������ֵ
    }
    //ȷ��������ֵ������255
    if(progressBarValue > 255)
    {
        progressBarValue = 255;
    }
    uart2_struct.tx_buf[len+7] = (uint8_t)progressBarValue; //������ֵ
    len = len+8;
    uart2_struct.tx_buf[len] = 0xFF; //������
    uart2_struct.tx_buf[len+1] = 0xFC;
    uart2_struct.tx_buf[len+2] = 0xFF;
    uart2_struct.tx_buf[len+3] = 0xFF;
    uart2_struct.tx_count = len+4; //�������ݳ���
    uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);   //��������
}



