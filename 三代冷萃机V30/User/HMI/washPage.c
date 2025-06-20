/**
 * 
 * @file washPage.c
 * @author jiaokai 
 * @brief ��ϴҳ����Ӧ����
 * 
 * @copyright Copyright (c) 2025
 */

#include "washPage.h"
#include "lcd_data_process.h"
#include "uart.h"
#include "string.h"
#include "eeprom.h"
#include "work.h"
#include "timer.h"
#include "config.h"
#include "wash.h"
#include "waterLevel.h"
#include "relay.h"
#include "app_mqtt.h"
#include "app_wifi.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "washPage"

/**
 * ************************************************************************
 * @brief  ��ϴҳ�水ť��Ӧ����
 * 
 * @param[in] num  �ؼ����
 * @param[in] state  �ؼ�״̬
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void washPageButton(uint8_t num,uint8_t state)
{
    switch(num)
    {
        case 3:
                if(wash.state == 0) //��ǰ����
                {
                    //[ ] ע����ϴ����
                    add_task(washProcessControl, NULL,100, true);  //HACK ʹ�õ���ģʽ2����Ҫ����
                    log_d("add task washProcessControl");
                    waterLevel.drainOption = 0; //�ŷ�ˮ
                    AnimationPlayFrame(wash_page,2,1,UART2_ID);
                    SetControlEnable(wash_page,4,0,UART2_ID);//���÷��ؼ�
                    log_v("wash start");
                }
                else if(wash.state  == 1) //��ǰ������ϴ
                {
                    wash.state = 2; //����Ϊ��ͣ״̬
                    recordOutputState();    //XXX ��¼��ǰ״̬ 
                    disable_task(washProcessControl);  //HACK ������ϴ����
                    log_d("disable task washProcessControl");
                    washPause();   //��ͣ��ϴ
                    wash.pauseStartTime = Timer.system1Sec;   //��¼��ͣ��ʼʱ��
                    AnimationPlayFrame(wash_page,2,0,UART2_ID);
                    SetControlEnable(wash_page,4,1,UART2_ID);//ʹ�ܷ��ؼ�
                    log_v("wash pause");
                }
                else if(wash.state == 2)//��ǰ����ͣ
                {
                    wash.state = 1; //����Ϊ��ϴ״̬
                    AnimationPlayFrame(wash_page,2,1,UART2_ID);
                    SetControlEnable(wash_page,4,0,UART2_ID);//���÷��ؼ�
                    enable_task(washProcessControl);  //HACK ʹ����ϴ����
                    log_d("enable task washProcessControl");
                    wash.pauseEndTime = Timer.system1Sec;   //��¼��ͣ����ʱ��
                    wash.pauseTime += (wash.pauseEndTime - wash.pauseStartTime);   //������ͣʱ��
                    restoreOutputState();   //XXX �ָ����״̬  
                    log_v("wash continue");
                }
            break;
        case 4:
            if(state == 1)
            {
                SetScreen(main_page,UART2_ID);          //������ҳ��
                AnimationPlayFrame(19,1,1,UART2_ID);    //ͼ��ĳ���ȡҺ
                
                deviceRunState = 0;         //XXX ����״̬
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
                
                if(wash.state != 0) //��ǰ����ͣ״̬
                {
                    delete_task(washProcessControl);        //[ ]ɾ������
                    log_d("delete task washProcessControl");

                }
                washReset();   //��ϴ��λ
            }
            break;
        default:
            break;
    }
}

/**
 * ************************************************************************
 * @brief ��ʾ��ϴ����ʱ
 * 
 * @param[in] Set_time  ����ʱ��
 * @param[in] Remain_time  ʣ��ʱ��
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-09
 * 
 * ************************************************************************
 */
void washShowTime(uint32_t setTime,uint32_t remainTime)//��ϴ����ʱ
{
    uint8_t lastMin,lastSec;   //ʣ�������
    char str[10];               //�����ַ���
    uint8_t strLen;             //�ַ�������
    uint8_t len;
    uint32_t progressBarValue;  //������ֵ
    if(remainTime>setTime)  //ʣ��ʱ���������ʱ��
    {
        remainTime = 1;
    }
    lastMin = remainTime/60;
    lastSec = remainTime%60;
    uart2_struct.tx_buf[0] = 0xEE;
    uart2_struct.tx_buf[1] = 0xB1;
    uart2_struct.tx_buf[2] = 0x12;
    uart2_struct.tx_buf[3] = 0x00;
    uart2_struct.tx_buf[4] = wash_page;   //ҳ���ַ
    uart2_struct.tx_buf[5] = 0x00;
    uart2_struct.tx_buf[6] = 0x08;              //�ؼ���ַ
    uart2_struct.tx_buf[7] = 0x00;
    sprintf(str,"%d",lastMin);
    strLen = strlen(str);
    uart2_struct.tx_buf[8] = strLen;
    for(uint8_t i=0;i<strLen;i++)
    {
        uart2_struct.tx_buf[9+i] = str[i];
    }
    len = 9+strLen;
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
        progressBarValue = ((uint32_t)(setTime-remainTime)*100)/setTime;
    }
    if(progressBarValue > 100)
    {
        progressBarValue = 100;
    }
    uart2_struct.tx_buf[len+7] = (uint8_t)progressBarValue;
    len = len+8;
    uart2_struct.tx_buf[len] = 0xFF;
    uart2_struct.tx_buf[len+1] = 0xFC;
    uart2_struct.tx_buf[len+2] = 0xFF;
    uart2_struct.tx_buf[len+3] = 0xFF;
    uart2_struct.tx_count = len+4;
    uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);   //��������
}


