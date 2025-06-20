/**
 * 
 * @file teaExtractionPage.c
 * @author jiaokai 
 * @brief ����ȡҳ�水ť��Ӧ
 * 
 * @copyright Copyright (c) 2025
 */
#include "teaExtractionPage.h"
#include "lcd_data_process.h"
#include "waterLevel.h"
#include "uart.h"
#include "string.h"
#include "teaMake.h"
#include "timer.h"
#include "eeprom.h"
#include "config.h"
#include "work.h"
#include "drainPage.h"
#include "app_mqtt.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "teaExtractionPage"


/**
 * ************************************************************************
 * @brief ����ȡҳ�水ť��Ӧ
 * 
 * @param[in] num  ��ť���
 * @param[in] state  ��ť״̬
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void teaExtractionPageButton(uint8_t num,uint8_t state)
{
    switch(num)
    {
        case 3: //��ʼ��ͣ��ť
            if(tea.state == 0)   //֮ǰֹͣ
            {
                add_task(teaProcessControl,NULL,100, true); //[ ] ��Ӳ���������,100msִ��һ��
                log_d("add teaProcessControl task");
                AnimationPlayFrame(make_tea_page,2,1,UART2_ID);   //�л���ʼ��ͣͼ��
                SetControlEnable(make_tea_page,4,0,UART2_ID);     //���÷��ذ�ť
                SetControlVisiable(make_tea_page,14,0,UART2_ID);  //������ˮͼ��
                SetControlEnable(make_tea_page,15,0,UART2_ID);    //������ˮ��ť
                log_v("tea start");
            }
            else if(tea.state == 1)  //֮ǰ����
            {
                tea.state = 2;
                disable_task(teaProcessControl); //[ ] ��ͣ����������
                log_d("disable teaProcessControl task");
                AnimationPlayFrame(make_tea_page,2,0,UART2_ID);   //�л���ʼ��ͣͼ��
                SetControlEnable(make_tea_page,4,1,UART2_ID);     //ʹ�ܷ��ذ�ť
                tea.pauseStartTime = Timer.system1Sec;
                log_v("tea pause");
                teaMakePause();
            }
            else if(tea.state == 2)  //֮ǰֹͣ
            {
                tea.state = 1;
                enable_task(teaProcessControl); //[ ] ��������������
                log_d("enable teaProcessControl task");
                AnimationPlayFrame(make_tea_page,2,1,UART2_ID);   //�л���ʼ��ͣͼ��
                SetControlEnable(make_tea_page,4,0,UART2_ID);     //���÷��ذ�ť
                SetControlVisiable(make_tea_page,14,0,UART2_ID);  //������ˮͼ��
                SetControlEnable(make_tea_page,15,0,UART2_ID);    //������ˮ��ť
                tea.pauseEndTime = Timer.system1Sec;
                tea.pauseTime = tea.pauseTime+(tea.pauseEndTime-tea.pauseStartTime);
                log_v("tea continue");
            }
            break;
        case 4: //���ذ�ť
            
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
            SetScreen(make_tea_set_page,UART2_ID);   //��������ҳ��
            
            if(tea.state != 0)   //��ǰ����ͣ״̬
            {
                delete_task(teaProcessControl); //[ ] ɾ������������
                log_d("delete teaProcessControl task");
            }
            teaMakeReset();   //��������״̬
            break;
        case 15: //��ˮ��ť
            AnimationPlayFrame(make_tea_page,10,0,UART2_ID);   //�л���ˮͼ��
            if(waterLevel.sensorErr == 1)   //����������
            {
                SetScreen(water_level_page,UART2_ID);
            }
            else
            {
                SetScreen(drain_page,UART2_ID);   //������ˮҳ��
                drainShowTime(DRAIN_TIME,DRAIN_TIME);   //��ʾ��ˮʱ��
            }
            break;
        default:
            
            break;
    }
}

/**
 * ************************************************************************
 * @brief �������ȡҳ��
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void enterTeaExtractionPage(void)
{
    char str[10];
    uint8_t len;
    uint8_t strLen;
    float volVal;
    makeTeaShowTime(config.data.teaMake.time[tea.num]*60,
                    config.data.teaMake.time[tea.num]*60);   //��ʾ��ȡʱ��
    // ��ʾʱ�䣬ˮ������ˮģʽ
    len = 0;
    uart2_struct.tx_buf[0] = 0xEE;
    uart2_struct.tx_buf[1] = 0xB1;
    uart2_struct.tx_buf[2] = 0x12;
    uart2_struct.tx_buf[3] = 0x00;
    uart2_struct.tx_buf[4] = make_tea_page;
    /************************************д��ʱ��************************************ */
    uart2_struct.tx_buf[5] = 0x00;
    uart2_struct.tx_buf[6] = 0x0C;              //�ؼ���ַ
    uart2_struct.tx_buf[7] = 0x00;              //���ݳ���
    sprintf(str,"%d",config.data.teaMake.time[tea.num]);
    strLen = strlen(str);
    uart2_struct.tx_buf[8] = strLen;
    for(uint8_t i=0;i<strLen;i++)
    {
        uart2_struct.tx_buf[9+i] = str[i];
    }
    len = len+9+strLen;
    /************************************д��ˮ��************************************ */
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len+1] = 0x0D;              //�ؼ���ַ
    uart2_struct.tx_buf[len+2] = 0x00;              //���ݳ���
    volVal = (float)config.data.teaMake.vol[tea.num]/10.0f;
    sprintf(str,"%.1f",volVal);
    strLen = strlen(str);
    uart2_struct.tx_buf[len+3] = strLen;
    for(uint8_t i=0;i<strLen;i++)
    {
        uart2_struct.tx_buf[len+4+i] = str[i];
    }
    len = len+4+strLen;
    uart2_struct.tx_buf[len] = 0xFF;
    uart2_struct.tx_buf[len+1] = 0xFC;
    uart2_struct.tx_buf[len+2] = 0xFF;
    uart2_struct.tx_buf[len+3] = 0xFF;
    uart2_struct.tx_count = len+4;
    uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);   //��������
    //ͼ�����
    if(config.data.teaMake.autoDrainangeFlag[tea.num] == 1)
    {
        SetControlVisiable(make_tea_page,10,0,UART2_ID);    
        SetControlVisiable(make_tea_page,11,1,UART2_ID);    
    }
    else
    {
        SetControlVisiable(make_tea_page,10,1,UART2_ID);  
        SetControlVisiable(make_tea_page,11,0,UART2_ID);   
    }
    SetControlVisiable(make_tea_page,14,0,UART2_ID);  //������ˮͼ��
    SetControlEnable(make_tea_page,15,0,UART2_ID);    //������ˮ��ť
}

/**
 * ************************************************************************
 * @brief ����ȡҳ���������ʾ
 * 
 * @param[in] setTime  ����ʱ�� ��
 * @param[in] remainTime  ʣ��ʱ�� ��
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void makeTeaShowTime(uint32_t setTime,uint32_t remainTime)
{
    uint16_t lastMin,lastSec;   //ʣ�������
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
    uart2_struct.tx_buf[4] = make_tea_page;   //ҳ���ַ
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
    // �����벿��
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
        progressBarValue = ((uint32_t)(setTime-remainTime)*100)/setTime; //���������ֵ
    }
    //ȷ��������ֵ������255
    if(progressBarValue > 255)
    {
        progressBarValue = 255;
    }
    uart2_struct.tx_buf[len+7] = (uint8_t)progressBarValue; //������ֵ
    len = len+8;
    uart2_struct.tx_buf[len] = 0xFF;
    uart2_struct.tx_buf[len+1] = 0xFC;
    uart2_struct.tx_buf[len+2] = 0xFF;
    uart2_struct.tx_buf[len+3] = 0xFF;
    uart2_struct.tx_count = len+4;
    uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);   //��������
}

