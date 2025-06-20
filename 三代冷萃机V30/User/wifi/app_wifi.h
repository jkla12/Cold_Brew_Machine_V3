#ifndef __APP_WIFI_H__
#define __APP_WIFI_H__

#include "gd32f30x.h"
#include <stddef.h>
#define false 0
#define true 1

#define BUFFER_INDEX_MAX 3
#define RX_BUFFER_SIZE 512

#define WIFI_DEBUG  0

// ��app_wifi.h�����
extern char lastEventMessage[256];
extern volatile uint8_t hasNewEvent;

typedef struct
{
    uint8_t runWIFIConfigEnd;
    uint8_t receiveBuffer[BUFFER_INDEX_MAX][RX_BUFFER_SIZE];    //���ջ�������
    uint8_t bufferIndex;                                        //���ջ�����������
    uint8_t isReceiveComplete[BUFFER_INDEX_MAX];                //������ɱ�־
    uint16_t receiveCount;                                      //���ռ���
    uint16_t receiveLen[BUFFER_INDEX_MAX];                      //�������鳤��
    uint8_t isInitialized;                                      //��ʼ����ɱ�־
    uint8_t isConnected;                                        //WIFI���ӳɹ���־λ
    uint8_t mac[13];                                            //��һ���ֽ�����null��ֹ��
    uint8_t isMacAddressObtained;                               //MAC��ַ��ȡ��־λ
    uint8_t bleMac[13];                                         //��һ���ֽ�����null��ֹ��
    uint8_t isBleMacAddressObtained;                            //BLE MAC��ַ��ȡ��־λ
    uint8_t isOK;                                               //OK��־λ
    uint8_t isConnectionConfiguration;                          //�������ñ�־λ
    uint8_t mqttConnected;  // MQTT�Ƿ�������
    uint32_t mqttConnectionStartTime; // MQTT���ӿ�ʼʱ��
}WIFI_TypeDef;

extern WIFI_TypeDef WIFI;




















void AT_Update(void);
uint8_t AT_SendCommand(const char* cmd, uint32_t timeoutMs, void (*callback)(uint8_t, const char*));



void WIFIMessageProcess(void);
void checkWifiStatus(void);
uint8_t  wifiConnectionConfiguration(void);


#endif


