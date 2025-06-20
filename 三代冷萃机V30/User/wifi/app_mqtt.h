#ifndef __APP_MQTT_H
#define __APP_MQTT_H


#include "gd32f30x.h"
#include "app_wifi.h"

// 定义静态内存池
#define MAX_PAYLOAD_SIZE 256  // 最大负载大小
#define PAYLOAD_BUFFER_COUNT 5 // 缓冲区数量

// 内存池结构
typedef struct {
    char buffer[MAX_PAYLOAD_SIZE];
    uint8_t inUse;
} PayloadBuffer_t;

// 内存池和管理变量
static PayloadBuffer_t payloadBuffers[PAYLOAD_BUFFER_COUNT] = {0};
static uint8_t currentBufferIndex = 0;


typedef enum {
    PUBLISH_WASH_CNT,
    PUBLISH_COFFEE_MAKE_CNT,
    PUBLISH_TEA_MAKE_CNT,
    PUBLISH_DRAIN_VALVE_CNT,
    PUBLISH_CIRCULATION_VALVE_CNT,
    PUBLISH_ALL_DATA,
    PUBLISH_DEVICE_RUN_STATE,
    PUBLISH_DRAIN_RELAT_DATA,
} publish_type_t;
int getNextAvailableBuffer(void);
//MQTT配置
void mqttConfig(void);

// 设备注册相关函数
uint8_t registerDevice(void);
void registerDeviceCallback(uint8_t success, const char* response);
uint8_t extractJsonField(const char* json, const char* field, char* value, uint16_t maxLen);
uint8_t extractJsonNestedField(const char* json, const char* object, const char* field, char* value, uint16_t maxLen);

void MQTT_Connect(void);
void mqttConfigCallback(uint8_t success, const char* response);
void mqttQueryCallback(uint8_t success, const char* response);
void mqttConnectCallback(uint8_t success, const char* response);
void MQTT_ConnectionMonitor(void);
void mqttPublishCallback(uint8_t success, const char* response);
void mqttSubscribeCallback(uint8_t success, const char* response);
void initMQTTTopics(void);

uint8_t MQTT_Subscribe(void);
uint8_t AT_SendCommandRaw(AT_Command_TypeDef* command);

uint8_t publish_circulationValveCnt(void);
uint8_t publish_drainValveCnt(void);
uint8_t publish_allData(void);
uint8_t publish_deviceRunState(void);
uint8_t publish_washCnt(void);
uint8_t publish_drainRelatData(void);
uint8_t publish_coffeeMakeCnt(void);
uint8_t publish_teaMakeCnt(void);
void delayedPublish(void* type);
void processEventMessage(const char* message);
// 处理MQTT订阅消息
void processMQTTSubscriptionMessage(const char* message);

// 处理MQTT下行消息
void processMQTTDownlinkMessage(const char* jsonPayload);

// 处理MQTT命令消息
void processMQTTCommandMessage(const char* jsonPayload);
uint8_t mqttSubscribeUp(const char *payload);
uint8_t publish_lockState(void);
uint8_t publish_unlockState(void);
#endif

