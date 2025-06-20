/**
 * 
 * @file drainPage.c
 * @author jiaokai 
 * @brief ��ˮҳ����Ӧ����
 * 
 * @copyright Copyright (c) 2025
 */
#include "drainPage.h"
#include "lcd_data_process.h"
#include "waterlevel.h"
#include "uart.h"
#include "drain.h"
#include "timer.h"
#include "work.h"
#include "drain.h"
#include "string.h"
#include "config.h"
#include "app_mqtt.h"
#include "elog.h"
#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "drainPage"

/**
 * ************************************************************************
 * @brief ��ˮҳ�水ť��Ӧ����
 * 
 * @param[in] num  Comment
 * @param[in] state  Comment
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void drainPageButton(uint8_t num,uint8_t state)
{
	switch(num)
	{
		case 3:
			if(state == 1)
			{
				if(drain.state == 0) //��ǰû����ˮ
				{
					add_task(drainProcessControl,NULL,100, true); //FIXME �����ˮ����,100msִ��һ��
					drain.startTime = Timer.system1Sec; //��¼��ʼʱ��
					//drain.state = 1; //����Ϊ��ˮ״̬
					AnimationPlayFrame(drain_page,2,1,UART2_ID);
                    SetControlEnable(drain_page,4,0,UART2_ID);//���÷��ؼ�
                    SetControlEnable(drain_page,11,0,UART2_ID);//������ˮѡ��ť
                    log_v("��ʼ��ˮ");
				}
				else if(drain.state == 1) //��ǰ����ˮ״̬
				{
					drain.state = 2; //����Ϊ��ͣ״̬
					AnimationPlayFrame(drain_page,2,0,UART2_ID);
                    SetControlEnable(drain_page,4,1,UART2_ID);//ʹ�ܷ��ؼ�
					drain.pauseStartTime = Timer.system1Sec; //��¼��ͣ��ʼʱ��
					waterDrainStop(); //ֹͣ��ˮ
					disable_task(drainProcessControl); //FIXME ��ͣ��ˮ����
					log_d("disable task drainProcessControl");
					log_v("��ͣ��ˮ");
				}
				else if(drain.state == 2) //��ǰ����ͣ״̬
				{
					drain.state = 1; //����Ϊ��ˮ״̬
					enable_task(drainProcessControl); //FIXME ������ˮ����
					log_d("enable task drainProcessControl");
					AnimationPlayFrame(drain_page,2,1,UART2_ID);
					SetControlEnable(drain_page,4,0,UART2_ID);//���÷��ؼ�
					drain.pauseEndTime = Timer.system1Sec; //��¼��ͣ����ʱ��
					drain.pauseTime += (drain.pauseEndTime - drain.pauseStartTime); //������ͣʱ��
					log_v("������ˮ");
				}
			}
			break;
		case 4:
			deviceRunState = 0;   //XXX ����״̬
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
			SetScreen(setting_page,UART2_ID);   //�л�������ҳ��
			if(drain.state != 0)
			{
				delete_task(drainProcessControl);   //ɾ����ˮ����
				log_d("delete task drainProcessControl");
			}
			drainReset();   //������ˮ״̬
			break;
		default:
			break;
	}
}


/**
 * ************************************************************************
 * @brief ��ˮѡ��ҳ�水ť��Ӧ
 * 
 * @param[in] num  Comment
 * @param[in] state  Comment
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void drainChoosePageButton(uint8_t num,uint8_t state)
{
	switch(num)
	{
		case 2:
			if(state == 1)
			{
				AnimationPlayFrame(drain_page,10,1,UART2_ID);
				waterLevel.drainOption = 0;   //ѡ����ˮ
				drainShowTime(DRAIN_TIME,DRAIN_TIME);   //��ʾ��ˮʱ��
				SetScreen(drain_page,UART2_ID);   //�л�����ˮҳ��
			}
			break;
		case 3:
			if(state == 1)
			{
				AnimationPlayFrame(drain_page,10,0,UART2_ID);
				waterLevel.drainOption = 1;   //ѡ��ȡˮ
				drainShowTime(DRAIN_TIME,DRAIN_TIME);   //��ʾ��ˮʱ��
				SetScreen(drain_page,UART2_ID);   //�л�����ˮҳ��
			}
			break;
		default:
			break;
	}
}



/**
 * ************************************************************************
 * @brief ��ˮ��������ʾ
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
void drainShowTime(uint32_t setTime,uint32_t remainTime)
{
    uint8_t lastMin,lastSec;
    uint8_t strLen;
    uint8_t len;
    char str[10];
    uint32_t progressBarVal;    //������ֵ
    lastMin = remainTime/60;
    lastSec = remainTime%60;
    uart2_struct.tx_buf[0] = 0xEE;
	uart2_struct.tx_buf[1] = 0xB1;
	uart2_struct.tx_buf[2] = 0x12;
	uart2_struct.tx_buf[3] = 0x00;
	uart2_struct.tx_buf[4] = drain_page;
	uart2_struct.tx_buf[5] = 0x00;
	uart2_struct.tx_buf[6] = 0x08;
	uart2_struct.tx_buf[7] = 0x00;
	sprintf(str,"%d",lastMin);
	strLen = strlen(str);
	uart2_struct.tx_buf[8] = strLen;
    for(uint8_t i = 0;i < strLen;i++)
    {
        uart2_struct.tx_buf[9+i] = str[i];
    }
    len  = strLen + 9;
    uart2_struct.tx_buf[len] = 0x00;
	uart2_struct.tx_buf[len+1] = 0x09;
	uart2_struct.tx_buf[len+2] = 0x00;
	sprintf(str,"%d",lastSec);
	strLen = strlen(str);
	uart2_struct.tx_buf[len+3] = strLen;
    len = len+4;
    for(uint8_t i = 0;i < strLen;i++)
    {
        uart2_struct.tx_buf[len+i] = str[i];
    }
    len = len+strLen;
    uart2_struct.tx_buf[len] = 0x00;
	uart2_struct.tx_buf[len+1] = 0x05;
	uart2_struct.tx_buf[len+2] = 0x00;
	uart2_struct.tx_buf[len+3] = 0x04;
	uart2_struct.tx_buf[len+4] = 0x00;
	uart2_struct.tx_buf[len+5] = 0x00;
	uart2_struct.tx_buf[len+6] = 0x00;
	progressBarVal = (uint32_t)(setTime-remainTime)*100;
    progressBarVal = progressBarVal/setTime;
	uart2_struct.tx_buf[len+7] = progressBarVal;
	len = len+8;
    uart2_struct.tx_buf[len] = 0xFF;
	uart2_struct.tx_buf[len+1] = 0xFC;
	uart2_struct.tx_buf[len+2] = 0xFF;
	uart2_struct.tx_buf[len+3] = 0xFF;
	uart2_struct.tx_count = len+4;
	uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);
}



