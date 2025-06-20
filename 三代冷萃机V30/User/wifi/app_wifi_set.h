#ifndef __APP_WIFI_SET_H__
#define __APP_WIFI_SET_H__

#include "app_wifi.h"




typedef struct
{
    uint8_t connectionStatus;  //连接状态 0:没有连接WIFI，1:正在连接WIFI,2:已连接WIFI，未获取IP，
                            //3：已连接WFII，已获取IP，4：WIFI连接失败
    char ssid[32];          //WIFI的SSID
    char pwd[64];           //WIFI的密码
    char MAC[32];           //WIFI的MAC地址
    char IP[32];            //WIFI的IP地址
    char gateway[32];       //WIFI的网关



    uint8_t isAutoConnect;  //自动连接
    uint8_t WMODE;          //WIFI工作模式，0：未初始化，1：STA，2：AP，3：AP+STA
    uint8_t isWCONFIG;      //WIFI配置,0关闭配网，1开启配网
}NetworkTypeDef;

extern NetworkTypeDef Network;

void WIFIConfiguration(void);
void ATCommandCallback(uint8_t success, const char *response);
void AT_WJAPCallback(uint8_t success, const char *response);

void WIFIAutoConfigNetWork(void);   //WIFI自动配网
void AT_WCONFIGCallback(uint8_t success,const char *response);      //手机配网开始返回
void AT_WCONFIGSTOPCallback(uint8_t success,const char *response);  //手机配网停止返回
void AT_WMODECallback(uint8_t success,const char *response);        //设置WIFI工作模式返回
void AT_WAUTOCONNCallback(uint8_t success, const char *response);  //设置自动连接返回


#endif

