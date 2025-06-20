#ifndef __APP_WIFI_SET_H__
#define __APP_WIFI_SET_H__

#include "app_wifi.h"




typedef struct
{
    uint8_t connectionStatus;  //����״̬ 0:û������WIFI��1:��������WIFI,2:������WIFI��δ��ȡIP��
                            //3��������WFII���ѻ�ȡIP��4��WIFI����ʧ��
    char ssid[32];          //WIFI��SSID
    char pwd[64];           //WIFI������
    char MAC[32];           //WIFI��MAC��ַ
    char IP[32];            //WIFI��IP��ַ
    char gateway[32];       //WIFI������



    uint8_t isAutoConnect;  //�Զ�����
    uint8_t WMODE;          //WIFI����ģʽ��0��δ��ʼ����1��STA��2��AP��3��AP+STA
    uint8_t isWCONFIG;      //WIFI����,0�ر�������1��������
}NetworkTypeDef;

extern NetworkTypeDef Network;

void WIFIConfiguration(void);
void ATCommandCallback(uint8_t success, const char *response);
void AT_WJAPCallback(uint8_t success, const char *response);

void WIFIAutoConfigNetWork(void);   //WIFI�Զ�����
void AT_WCONFIGCallback(uint8_t success,const char *response);      //�ֻ�������ʼ����
void AT_WCONFIGSTOPCallback(uint8_t success,const char *response);  //�ֻ�����ֹͣ����
void AT_WMODECallback(uint8_t success,const char *response);        //����WIFI����ģʽ����
void AT_WAUTOCONNCallback(uint8_t success, const char *response);  //�����Զ����ӷ���


#endif

