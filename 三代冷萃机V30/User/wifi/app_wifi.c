#include "app_wifi.h"
#include "uart.h"
#include "string.h"
#include "stdio.h"
#include "timer.h"
#include "app_wifi_set.h"
#include "eeprom.h"
#include "stdlib.h"
#include "systick.h"
#include "app_mqtt.h"
#include "elog.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "app_wifi"

WIFI_TypeDef WIFI;



// ����һ�����λ���������UART����

volatile uint8_t rxBuffer[1024];
volatile uint16_t rxHead = 0;
volatile uint16_t rxTail = 0;

// ���ڴ洢�ʹ����¼���Ϣ
char lastEventMessage[256] = {0};  // �洢���һ�ν��յ����¼���Ϣ
volatile uint8_t hasNewEvent = 0;  // ����Ƿ����µ��¼���Ҫ����

/**********************************************************************
**����ԭ��:	void UART4_IRQHandler(void)
**��������:	����4�жϺ���
**��ڲ���:	��
**
**���ڲ���:	��
**��    ע:	��
************************************************************************/
void UART4_IRQHandler(void)
{
    static char eventBuffer[256] = {0};
    static uint8_t eventIndex = 0;
    uint8_t data;
    if (RESET != usart_interrupt_flag_get(UART4, USART_INT_FLAG_RBNE))
    {
        data = usart_data_receive(UART4);
        uint16_t nextHead = (rxHead + 1) % 1024;
        if (nextHead != rxTail)
        { // ������δ��
            rxBuffer[rxHead] = data;
            rxHead = nextHead;
        }
        // 2. ͬʱ����¼��ַ���
        if (data == '+' && eventIndex == 0) {
            // �������¼���ʼ
            eventBuffer[eventIndex++] = data;
        }
        else if (eventIndex > 0) {
            // �Ѿ����ռ��¼�
            if (eventIndex < sizeof(eventBuffer) - 1) {
                eventBuffer[eventIndex++] = data;
            }
            
            // ����Ƿ��¼��������������з���
            if (data == '\n') {
                eventBuffer[eventIndex] = '\0';
                
                // �����¼�
                if (strstr(eventBuffer, "+EVENT:")) {
                    // ͨ��ȫ�ֱ�־�����֪ͨ��ѭ�������¼�
                    memcpy(lastEventMessage, eventBuffer, eventIndex + 1);
                    hasNewEvent = true;
                }
                
                // �����¼�������
                eventIndex = 0;
            }
        }
    }
    if (RESET != usart_interrupt_flag_get(UART4, USART_INT_FLAG_IDLE))
    {
        // ���IDLE�жϱ�־
        usart_data_receive(UART4);
    }
}


/**
 * ************************************************************************
 * @brief ���WIFIģ��״̬
 *
 *
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-04-22
 *
 * ************************************************************************
 */
void checkWifiStatus(void)
{
    if (WIFI.isConnected == 0)
    {
        uart4_send_buf((uint8_t *)"AT\r\n", 4);
    }
}


