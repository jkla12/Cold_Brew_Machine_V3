#ifndef ___WIFFI_SET_PAGE_H__
#define ___WIFFI_SET_PAGE_H__

#include "gd32f30x.h"

//wifi设置页面按钮响应函数

typedef enum
{
    //开始配网
    WIFI_SET_START = 0,
    //配网中
    WIFI_SET_IN_PROGRESS,
    //配网完成
    WIFI_SET_COMPLETE,
    //配网超时
    WIFI_SET_TIMEOUT,
    //配网失败
    WIFI_SET_FAILED
} wifi_set_page_icon;

void wifiSetPageButton(uint8_t num,uint8_t state);

#endif


