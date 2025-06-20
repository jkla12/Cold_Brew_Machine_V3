/**
 * 
 * @file wifiSetPage.c
 * @author jiaokai 
 * @brief WIFI����ҳ����Ӧ����
 * 
 * @copyright Copyright (c) 2025
 */
#include "wifiSetPage.h"
#include "lcd_data_process.h"
#include "uart.h"
#include "eeprom.h"
#include "config.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "work.h"
#include "app_wifi.h"
#include "app_wifi_set.h"
#include "app_mqtt.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "wifiSetPage"

uint8_t wifiSetState = 0; //wifi����״̬ 0,δ������1�����У�2������ɣ�3����ʧ��

/**
 * ************************************************************************
 * @brief WIFI����ҳ�水ť��Ӧ����
 * 
 * 
 * 
 * @param[in] num  ��ť���
 * @param[in] state  ��ť״̬
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-16
 * 
 * ************************************************************************
 */
void wifiSetPageButton(uint8_t num,uint8_t state)
{
   switch(num)
   {
        case 4: //���ذ�ť
            if(state == 1 && wifiSetState == 1)
            {
                wifiSetState = 0; //ֹͣ��������
                AnimationPlayFrame(wifi_setting_page,1,WIFI_SET_START,UART2_ID);
                delete_task(WIFIAutoConfigNetWork);
                //����ֹͣ����ָ��
                AT_SendCommand("AT+WCONFIG=0",1000,AT_WCONFIGSTOPCallback);
                log_v("stop wifi config return set page");
                if(Network.connectionStatus != 3)
                {
                    add_task(WIFIConfiguration,NULL,1000,true);
                    add_task(mqttConfig,NULL,1000,true);
                    log_v("add task wifi config");
                }
            }
            break;
        case 5:
            if(state == 1 && wifiSetState == 0)
            {
                wifiSetState = 1; //������������
                AnimationPlayFrame(wifi_setting_page,1,WIFI_SET_IN_PROGRESS,UART2_ID);
                add_task(WIFIAutoConfigNetWork,NULL,2000,true);
                log_d("start wifi config");
            }
            else if(state == 1 && wifiSetState == 1)
            {
                wifiSetState = 0; //ֹͣ��������
                delete_task(WIFIAutoConfigNetWork);
                AnimationPlayFrame(wifi_setting_page,1,WIFI_SET_START,UART2_ID);
                //����ֹͣ����ָ��
                AT_SendCommand("AT+WCONFIG=0",1000,AT_WCONFIGSTOPCallback);
                log_d("stop wifi config");
            }
            break;
        default:
            break;
   }
}
