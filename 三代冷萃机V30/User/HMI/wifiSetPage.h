#ifndef ___WIFFI_SET_PAGE_H__
#define ___WIFFI_SET_PAGE_H__

#include "gd32f30x.h"

//wifi����ҳ�水ť��Ӧ����

typedef enum
{
    //��ʼ����
    WIFI_SET_START = 0,
    //������
    WIFI_SET_IN_PROGRESS,
    //�������
    WIFI_SET_COMPLETE,
    //������ʱ
    WIFI_SET_TIMEOUT,
    //����ʧ��
    WIFI_SET_FAILED
} wifi_set_page_icon;

void wifiSetPageButton(uint8_t num,uint8_t state);

#endif


