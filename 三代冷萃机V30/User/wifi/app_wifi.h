#ifndef __APP_WIFI_H__
#define __APP_WIFI_H__

#include "gd32f30x.h"
#include <stddef.h>
#define false 0
#define true 1

#define BUFFER_INDEX_MAX 3
#define RX_BUFFER_SIZE 512

#define WIFI_DEBUG  0

// 在app_wifi.h中添加
extern char lastEventMessage[256];
extern volatile uint8_t hasNewEvent;

typedef struct
{
    uint8_t runWIFIConfigEnd;
    uint8_t receiveBuffer[BUFFER_INDEX_MAX][RX_BUFFER_SIZE];    //接收缓存数组
    uint8_t bufferIndex;                                        //接收缓存数组索引
    uint8_t isReceiveComplete[BUFFER_INDEX_MAX];                //接收完成标志
    uint16_t receiveCount;                                      //接收计数
    uint16_t receiveLen[BUFFER_INDEX_MAX];                      //接收数组长度
    uint8_t isInitialized;                                      //初始化完成标志
    uint8_t isConnected;                                        //WIFI连接成功标志位
    uint8_t mac[13];                                            //多一个字节用于null终止符
    uint8_t isMacAddressObtained;                               //MAC地址获取标志位
    uint8_t bleMac[13];                                         //多一个字节用于null终止符
    uint8_t isBleMacAddressObtained;                            //BLE MAC地址获取标志位
    uint8_t isOK;                                               //OK标志位
    uint8_t isConnectionConfiguration;                          //连接配置标志位
    uint8_t mqttConnected;  // MQTT是否已连接
    uint32_t mqttConnectionStartTime; // MQTT连接开始时间
}WIFI_TypeDef;

extern WIFI_TypeDef WIFI;




















void AT_Update(void);
uint8_t AT_SendCommand(const char* cmd, uint32_t timeoutMs, void (*callback)(uint8_t, const char*));



void WIFIMessageProcess(void);
void checkWifiStatus(void);
uint8_t  wifiConnectionConfiguration(void);


#endif


