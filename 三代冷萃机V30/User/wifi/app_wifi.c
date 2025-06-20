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



// 定义一个环形缓冲区接收UART数据

volatile uint8_t rxBuffer[1024];
volatile uint16_t rxHead = 0;
volatile uint16_t rxTail = 0;

// 用于存储和处理事件消息
char lastEventMessage[256] = {0};  // 存储最近一次接收到的事件消息
volatile uint8_t hasNewEvent = 0;  // 标记是否有新的事件需要处理

/**********************************************************************
**函数原型:	void UART4_IRQHandler(void)
**函数作用:	串口4中断函数
**入口参数:	无
**
**出口参数:	无
**备    注:	无
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
        { // 缓冲区未满
            rxBuffer[rxHead] = data;
            rxHead = nextHead;
        }
        // 2. 同时检测事件字符串
        if (data == '+' && eventIndex == 0) {
            // 可能是事件开始
            eventBuffer[eventIndex++] = data;
        }
        else if (eventIndex > 0) {
            // 已经在收集事件
            if (eventIndex < sizeof(eventBuffer) - 1) {
                eventBuffer[eventIndex++] = data;
            }
            
            // 检查是否事件结束（遇到换行符）
            if (data == '\n') {
                eventBuffer[eventIndex] = '\0';
                
                // 处理事件
                if (strstr(eventBuffer, "+EVENT:")) {
                    // 通过全局标志或队列通知主循环处理事件
                    memcpy(lastEventMessage, eventBuffer, eventIndex + 1);
                    hasNewEvent = true;
                }
                
                // 重置事件缓冲区
                eventIndex = 0;
            }
        }
    }
    if (RESET != usart_interrupt_flag_get(UART4, USART_INT_FLAG_IDLE))
    {
        // 清除IDLE中断标志
        usart_data_receive(UART4);
    }
}


/**
 * ************************************************************************
 * @brief 检测WIFI模组状态
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


