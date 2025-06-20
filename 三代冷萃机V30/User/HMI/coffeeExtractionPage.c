/**
 * 
 * @file coffeeExtractionPage.c
 * @author jiaokai 
 * @brief ������ȡҳ����Ӧ
 * 
 * @copyright Copyright (c) 2025
 */
#include "coffeeExtractionPage.h"
#include "lcd_data_process.h"
#include "waterLevel.h"
#include "uart.h"
#include "string.h"
#include "coffeeMake.h"
#include "drainPage.h"
#include "eeprom.h"
#include "config.h"
#include "timer.h"
#include "uart.h"
#include "work.h"
#include "drainPage.h"
#include "app_mqtt.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "coffeeExtractionPage"


/**
 * ************************************************************************
 * @brief ������ȡҳ�水ť��Ӧ
 * 
 * @param[in] button ��ť���
 * @param[in] state ��ť״̬
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-10
 * 
 * ************************************************************************
 */
void coffeeExtractionPageButton(uint8_t num,uint8_t state)
{
    switch(num)
    {
        case 3: //��ʼ��ͣ��ť
            if(coffee.state == 0)   //֮ǰֹͣ
            {
                add_task(coffeeProcessControl,NULL,100, true); //[ ] ��ӿ�����������,100msִ��һ��
                log_d("add coffeeProcessControl task");
                AnimationPlayFrame(make_coffe_page,2,1,UART2_ID);   //�л���ʼ��ͣͼ��
                SetControlEnable(make_coffe_page,4,0,UART2_ID);     //���÷��ذ�ť
                SetControlVisiable(make_coffe_page,14,0,UART2_ID);  //������ˮͼ��
                SetControlEnable(make_coffe_page,15,0,UART2_ID);    //������ˮ��ť
                log_v("coffee start");
            }
            else if(coffee.state == 1)  //֮ǰ����
            {
                coffee.state = 2;
                disable_task(coffeeProcessControl); //[ ] ��ͣ������������
                log_d("disable coffeeProcessControl task");
                AnimationPlayFrame(make_coffe_page,2,0,UART2_ID);   //�л���ʼ��ͣͼ��
                SetControlEnable(make_coffe_page,4,1,UART2_ID);     //ʹ�ܷ��ذ�ť
                coffee.pauseStartTime = Timer.system1Sec;
                log_v("coffee pause");
                coffeeMakePause();
            }
            else if(coffee.state == 2)  //֮ǰֹͣ
            {
                coffee.state = 1;
                enable_task(coffeeProcessControl); //[ ] ����������������
                log_d("enable coffeeProcessControl task");
                AnimationPlayFrame(make_coffe_page,2,1,UART2_ID);   //�л���ʼ��ͣͼ��
                SetControlEnable(make_coffe_page,4,0,UART2_ID);     //���÷��ذ�ť
                SetControlVisiable(make_coffe_page,14,0,UART2_ID);  //������ˮͼ��
                SetControlEnable(make_coffe_page,15,0,UART2_ID);    //������ˮ��ť
                coffee.pauseEndTime = Timer.system1Sec;
                coffee.pauseTime = coffee.pauseTime+(coffee.pauseEndTime-coffee.pauseStartTime);
                log_v("coffee continue");
            }
            break;
        case 4: //���ذ�ť
            
            SetScreen(make_coffe_set_page,UART2_ID);
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
           
            if(coffee.state != 0)
            {
                delete_task(coffeeProcessControl); //[ ] ɾ��������������
                log_d("delete coffeeProcessControl task");
            }
            coffeeMakeReset();
            break;
        case 15:  ///��ˮ��ť
            AnimationPlayFrame(drain_page,10,0,UART2_ID);   //�л���ˮͼ��
            if(waterLevel.sensorErr == 1)   //�������������
            {
                SetScreen(water_level_page,UART2_ID);
            }
            else
            {
                SetScreen(drain_page,UART2_ID);                 //������ˮҳ��
                drainShowTime(DRAIN_TIME,DRAIN_TIME);   //��ʾ��ˮʱ��
            }
            break;
        default:
            
            break;
    }
}



/**
 * ************************************************************************
 * @brief ���뿧����ȡҳ�棬ˢ��ҳ������
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-10
 * 
 * ************************************************************************
 */
void enterCoffeeExtractionPage(void)
{
    char str[10] ;
    uint8_t strLen;
    uint8_t len;
    float volVal;
    //��ʾʱ��
    makeCoffeeShowTime(config.data.coffeeMake.time[coffee.num]*60,
            config.data.coffeeMake.time[coffee.num]*60);    
     // ��ʾʱ�䣬ˮ������ˮģʽ
    uart2_struct.tx_buf[0] = 0xEE;
    uart2_struct.tx_buf[1] = 0xB1;
    uart2_struct.tx_buf[2] = 0x12;
    uart2_struct.tx_buf[3] = 0x00;
    uart2_struct.tx_buf[4] = make_coffe_page;
    uart2_struct.tx_buf[5] = 0x00;
    uart2_struct.tx_buf[6] = 0x0C;//ʱ��
    uart2_struct.tx_buf[7] = 0x00;
    sprintf(str,"%d",config.data.coffeeMake.time[coffee.num]);  //XXX ֮ǰ��config.data.coffeeMake.currentNumber
    strLen = strlen(str);
    uart2_struct.tx_buf[8] = strLen;
    if(strLen == 1)
    {
        uart2_struct.tx_buf[9] = str[0];
        len = 10;
    }
    else if(strLen == 2)
    {
        uart2_struct.tx_buf[9] = str[0];
        uart2_struct.tx_buf[10] = str[1];
        len = 11;
    }
    else if(strLen == 3)
    {
        uart2_struct.tx_buf[9] = str[0];
        uart2_struct.tx_buf[10] = str[1];
        uart2_struct.tx_buf[12] = str[2];
        len = 13;
    }
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len+1] = 0x0D;//ˮ��
    uart2_struct.tx_buf[len+2] = 0x00;
    len = len+3;
    volVal = (float)config.data.coffeeMake.vol[coffee.num]/10;
    sprintf(str,"%.1f",volVal);
    strLen = strlen(str);
    uart2_struct.tx_buf[len] = strLen;
    if(strLen == 3)
    {
        uart2_struct.tx_buf[len+1] = str[0];
        uart2_struct.tx_buf[len+2] = str[1];
        uart2_struct.tx_buf[len+3] = str[2];
        len = len+4; 
    }
    uart2_struct.tx_buf[len] = 0xFF;
    uart2_struct.tx_buf[len+1] = 0xFC;
    uart2_struct.tx_buf[len+2] = 0xFF;
    uart2_struct.tx_buf[len+3] = 0xFF;
    uart2_struct.tx_count = len+4;
    uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);
     
    //ͼ�����
    if(config.data.coffeeMake.autoDrainangeFlag[coffee.num] == 1)
    {
        SetControlVisiable(make_coffe_page,10,0,UART2_ID);
        SetControlVisiable(make_coffe_page,11,1,UART2_ID);
    }
    else
    {
        SetControlVisiable(make_coffe_page,10,1,UART2_ID);
        SetControlVisiable(make_coffe_page,11,0,UART2_ID);
    }
    SetControlVisiable(make_coffe_page,14,0,UART2_ID);//������ˮͼ��
    SetControlEnable(make_coffe_page,15,0,UART2_ID);//������ˮ��ť
}

/**
 * ************************************************************************
 * @brief ������ȡҳ���������ʾ
 * 
 * @param[in] setTime  ����ʱ�� ��
 * @param[in] remainTime  ʣ��ʱ�� ��
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-10
 * 
 * ************************************************************************
 */
void makeCoffeeShowTime(uint32_t setTime,uint32_t remainTime)
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
    uart2_struct.tx_buf[4] = make_coffe_page;   //ҳ���ַ
    uart2_struct.tx_buf[5] = 0x00;
    uart2_struct.tx_buf[6] = 0x08;              //�ؼ���ַ
    uart2_struct.tx_buf[7] = 0x00;
    sprintf(str,"%d",lastMin);
    strLen = strlen(str);
    uart2_struct.tx_buf[8] = strLen;
    if(strLen == 1)
    {
        uart2_struct.tx_buf[9] = str[0];
        len=9;
    }
    else if(strLen == 2)
    {
        uart2_struct.tx_buf[9] = str[0];
        uart2_struct.tx_buf[10] = str[1];
        len=10;
    }
    uart2_struct.tx_buf[len+1] = 0x00;
    uart2_struct.tx_buf[len+2] = 0x09;
    uart2_struct.tx_buf[len+3] = 0x00;
    len = len+3;

    // �����벿��
    sprintf(str,"%d",lastSec);
    strLen = strlen(str);
    uart2_struct.tx_buf[len+1] = strLen;
    if(strLen == 1)
    {
        uart2_struct.tx_buf[len+2] = str[0];
        len+=2;
    }
    else if(strLen == 2)
    {
        uart2_struct.tx_buf[len+2] = str[0];
        uart2_struct.tx_buf[len+3] = str[1];
        len+=3;
    }
    uart2_struct.tx_buf[len+1] = 0x00;
    uart2_struct.tx_buf[len+2] = 0x05;
    uart2_struct.tx_buf[len+3] = 0x00;
    uart2_struct.tx_buf[len+4] = 0x04;
    uart2_struct.tx_buf[len+5] = 0x00;
    uart2_struct.tx_buf[len+6] = 0x00;
    uart2_struct.tx_buf[len+7] = 0x00;
    if(setTime == 0)
    {
        progressBarValue = 100;  // �� setTime Ϊ 0����������Ϊ 100%
    }
    else
    {
        progressBarValue = ((uint32_t)(setTime - remainTime) * 100) / setTime;
    }
     // ȷ��������ֵ������ 255
    if (progressBarValue > 255) 
    {
        progressBarValue = 255;
    }
    uart2_struct.tx_buf[len+8] = (uint8_t)progressBarValue;
    len += 8;

    uart2_struct.tx_buf[len+1] = 0xFF;  //֡β
    uart2_struct.tx_buf[len+2] = 0xFC;
    uart2_struct.tx_buf[len+3] = 0xFF;
    uart2_struct.tx_buf[len+4] = 0xFF;
    uart2_struct.tx_count = len+5;
    uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);
}   


