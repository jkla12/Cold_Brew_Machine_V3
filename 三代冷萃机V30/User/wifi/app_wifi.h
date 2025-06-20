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
    uint16_t receiveLen[BUFFER_INDEX_MAX];                                        //接收数组长度
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

typedef enum {
    AT_IDLE,
    AT_SENDING,
    AT_WAITING_PROMPT, // 等待提示符（用于RAW数据发送）
    AT_WAITING,
    AT_PROCESSING,
    AT_ERROR
} AT_CommandState_TypeDef;

typedef struct {
    AT_CommandState_TypeDef state;
    char command[256];
    char response[1024];
    uint16_t responseIndex;
    uint32_t timeoutMs;
    uint32_t startTime;
    void (*callback)(uint8_t success, const char* response);
    const char* rawData;        // RAW数据指针（用于MQTTPUBRAW等命令）
    size_t rawDataLen;          // RAW数据长度
    uint8_t needRawSend;        // 是否需要发送RAW数据
    uint8_t rawSent;            // RAW数据是否已发送
    uint8_t bufferIndex;       // 新增：缓冲区索引，用于静态内存池管理
} AT_Command_TypeDef;

extern AT_Command_TypeDef currentCmd;
















void AT_Update(void);
uint8_t AT_SendCommand(const char* cmd, uint32_t timeoutMs, void (*callback)(uint8_t, const char*));



void WIFIMessageProcess(void);
void checkWifiStatus(void);
uint8_t  wifiConnectionConfiguration(void);


#endif


