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

AT_Command_TypeDef currentCmd;

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
 * @brief WIFI模组数据解析
 *
 *
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-04-22
 *
 * ************************************************************************
 */
// void WIFIMessageProcess(void)
// {
//     char *ret;
//     uint16_t dataLen;

//     for (uint8_t i = 0; i < BUFFER_INDEX_MAX; i++)
//     {
//         if (WIFI.isReceiveComplete[i] == 1)
//         {
//             WIFI.isReceiveComplete[i] = 0;
//             dataLen = WIFI.receiveLen[i];

//             // 确保数据以null结尾
//             if (dataLen < RX_BUFFER_SIZE)
//             {
//                 WIFI.receiveBuffer[i][dataLen] = '\0';
//             }
//             else
//             {
//                 WIFI.receiveBuffer[i][RX_BUFFER_SIZE - 1] = '\0';
//                 dataLen = RX_BUFFER_SIZE - 1;
//             }

//             if (WIFI.isBleMacAddressObtained == 0)
//             {
//                 ret = strstr((char *)WIFI.receiveBuffer[i], "ble_mac"); // 获取BLE MAC地址
//                 if (ret && (ret + 8 + 12) <= ((char *)WIFI.receiveBuffer[i] + dataLen))
//                 {
//                     char *macStart = ret + 8;
//                     memcpy(WIFI.bleMac, macStart, 12);
//                     WIFI.isBleMacAddressObtained = 1;
//                     WIFI.bleMac[12] = '\0';
//                     log_v("BLE MAC: %s", WIFI.bleMac);
//                 }
//             }

//             if (WIFI.isMacAddressObtained == 0)
//             {
//                 ret = strstr((char *)WIFI.receiveBuffer[i], "wifi_mac"); // 获取MAC地址
//                 if (ret && (ret + 9 + 12) <= ((char *)WIFI.receiveBuffer[i] + dataLen))
//                 {
//                     char *macStart = ret + 9;
//                     memcpy(WIFI.mac, macStart, 12);
//                     WIFI.isMacAddressObtained = 1;
//                     WIFI.mac[12] = '\0';
//                     log_v("MAC: %s", WIFI.mac);
//                 }
//             }

//             if (WIFI.isInitialized == 0)
//             {
//                 ret = strstr((char *)WIFI.receiveBuffer[i], "ready"); // 检测启动成功
//                 if (ret)
//                 {
//                     WIFI.isInitialized = 1;
//                     log_v("WIFI Ready");
//                 }
//             }

//             ret = strstr((char *)WIFI.receiveBuffer[i], "OK");
//             if (ret)
//             {
//                 WIFI.isOK = 1;
//             }
//         }
//     }
// }
void WIFIMessageProcess(void)
{
    // 检查环形缓冲区中是否有数据
    if (rxHead != rxTail)
    {
        char tempBuf[RX_BUFFER_SIZE];
        uint16_t dataLen = 0;

        // 从环形缓冲区中读取数据到临时缓冲区
        while (rxHead != rxTail && dataLen < RX_BUFFER_SIZE - 1)
        {
            tempBuf[dataLen++] = rxBuffer[rxTail];
            rxTail = (rxTail + 1) % 1024;
        }

        tempBuf[dataLen] = '\0'; // 确保字符串结束

        // 处理接收到的数据
        char *ret;

        // 检查是否有BLE MAC地址
        if (WIFI.isBleMacAddressObtained == 0)
        {
            ret = strstr(tempBuf, "ble_mac");
            if (ret && (ret + 8 + 12) <= (tempBuf + dataLen))
            {
                char *macStart = ret + 8;
                memcpy(WIFI.bleMac, macStart, 12);
                WIFI.isBleMacAddressObtained = 1;
                WIFI.bleMac[12] = '\0';
                log_v("BLE MAC: %s", WIFI.bleMac);
            }
        }

        // 检查是否有WiFi MAC地址
        if (WIFI.isMacAddressObtained == 0)
        {
            ret = strstr(tempBuf, "wifi_mac");
            if (ret && (ret + 9 + 12) <= (tempBuf + dataLen))
            {
                char *macStart = ret + 9;
                memcpy(WIFI.mac, macStart, 12);
                WIFI.isMacAddressObtained = 1;
                WIFI.mac[12] = '\0';
                log_v("MAC: %s", WIFI.mac);
            }
        }

        // 检查WiFi是否初始化
        if (WIFI.isInitialized == 0)
        {
            ret = strstr(tempBuf, "ready");
            if (ret)
            {
                WIFI.isInitialized = 1;
                log_v("WIFI Ready");
            }
        }

        // 检查是否包含"OK"
        ret = strstr(tempBuf, "OK");
        if (ret)
        {
            WIFI.isOK = 1;
        }

        // 添加调试输出
        log_d("Received data: %s", tempBuf);
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

/**
 * ************************************************************************
 * @brief WIFI连接配置
 *
 *
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-04-22
 *
 * ************************************************************************
 */
uint8_t wifiConnectionConfiguration(void)
{
    static uint8_t step;
    switch (step)
    {
    case 0:
        WIFI.isOK = 0;
        uart4_send_buf((uint8_t *)"AT+WAUTOCONN=1\r\n", 17);
        step++;
        return 0;
    case 1:
        if (WIFI.isOK == 1)
        {
            uart4_send_buf((uint8_t *)"AT+WMODE=1,1\r\n", 15);
            step++;
        }
        return 0;
    case 2:
        if (WIFI.isOK == 1)
        {
            uart4_send_buf((uint8_t *)"AT+WCONFIG=9\r\n", 15);
            step++;
        }
        return 0;
    case 3:
        if (WIFI.isOK == 1)
        {
            uart0_dma_send((uint8_t *)"wifi connection config ok\r\n", 27);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}

// /**
//  * ************************************************************************
//  * @brief 发送AT命令
//  *
//  * @param[in] cmd  AT命令字符串
//  * @param[in] timeoutMs  命令响应的超时时间，单位为毫秒
//  * @param[in] callback  命令发送完成后或超时的回调函数，参数为状态码和命令字符串
//  *
//  * @return 返回发送状态，0表示发送失败，非0表示发送成功
//  *
//  * @version 1.0
//  * @author jiaokai
//  * @date 2025-04-22
//  *
//  * ************************************************************************
//  */
// uint8_t AT_SendCommand(const char *cmd, uint32_t timeoutMs, void (*callback)(uint8_t, const char *))
// {
//     if (currentCmd.state != AT_IDLE)
//     {
//         return false; // 有命令正在处理
//     }

//     strncpy(currentCmd.command, cmd, sizeof(currentCmd.command) - 1);
//     currentCmd.responseIndex = 0;
//     currentCmd.timeoutMs = timeoutMs;
//     currentCmd.startTime = Timer.systemTime; // 获取系统时间
//     currentCmd.callback = callback;
//     currentCmd.state = AT_SENDING;

//     return true;
// }

// /**
//  * ************************************************************************
//  * @brief AT指令状态机更新
//  *
//  *
//  *
//  * @version 1.0
//  * @author jiaokai
//  * @date 2025-04-22
//  *
//  * ************************************************************************
//  */
// void AT_Update(void)
// {
//     switch (currentCmd.state)
//     {
//     case AT_IDLE:
//         // 空闲状态，无需处理
//         break;

//     case AT_SENDING:
//         // 发送AT命令
//         // 在命令末尾添加换行符
//         strcat(currentCmd.command, "\r\n");
//         uart4_send_buf((uint8_t *)currentCmd.command, strlen(currentCmd.command));
//         // uart4_send_str(currentCmd.command);
//         // uart4_send_str("\r\n");
//         log_v("AT command: %s", currentCmd.command);
//         currentCmd.state = AT_WAITING;
//         break;

//     case AT_WAITING:
//         // 检查是否有新数据
//         while (rxHead != rxTail)
//         {
//             char c = rxBuffer[rxTail];
//             rxTail = (rxTail + 1) % BUFFER_SIZE;

//             currentCmd.response[currentCmd.responseIndex++] = c;
//             if (currentCmd.responseIndex >= sizeof(currentCmd.response) - 1)
//             {
//                 currentCmd.responseIndex = sizeof(currentCmd.response) - 1;
//             }

//             // 检查是否接收到完整响应（如"OK"或"ERROR"）
//             if (strstr(currentCmd.response, "OK") ||
//                 strstr(currentCmd.response, "ERROR"))
//             {
//                 currentCmd.state = AT_PROCESSING;
//                 break;
//             }
//         }

//         // 检查超时
//         if (Timer.systemTime - currentCmd.startTime > currentCmd.timeoutMs)
//         {
//             currentCmd.state = AT_ERROR;
//         }
//         break;

//     case AT_PROCESSING:
//         // 处理响应
// 		//uart0_dma_send((uint8_t *)"AT PROCESSING\r\n", 15);
//         currentCmd.response[currentCmd.responseIndex] = '\0';
//         if (currentCmd.callback)
//         {
//             uint8_t success = strstr(currentCmd.response, "OK") != NULL;
//             currentCmd.callback(success, currentCmd.response);
//         }
//         currentCmd.state = AT_IDLE;
//         break;

//     case AT_ERROR:
//         // 处理错误
//         log_e("AT command error");
//         if (currentCmd.callback)
//         {
//             currentCmd.callback(false, "ERROR");
//         }

//         currentCmd.state = AT_IDLE;
//         break;
//     }
// }

// ...existing code...

/**
 * ************************************************************************
 * @brief AT指令状态机更新
 *
 * 改进的AT指令解析，更健壮地处理响应
 *
 * @version 2.0
 * @author jiaokai
 * @date 2025-05-16
 *
 * ************************************************************************
 */
void AT_Update(void)
{
    if (hasNewEvent)
    {
        processEventMessage(lastEventMessage);
        hasNewEvent = 0;
    }
    // 添加超时检测
    if (currentCmd.state != AT_IDLE && 
        Timer.systemTime - currentCmd.startTime > 5000) { // 5秒超时
        
        log_e("AT command timeout, forcing reset");
        
        // 如果是RAW命令，释放缓冲区
        if (currentCmd.needRawSend && currentCmd.rawData && 
            currentCmd.bufferIndex < PAYLOAD_BUFFER_COUNT) {
            payloadBuffers[currentCmd.bufferIndex].inUse = 0;
        }
        
        // 重置命令状态
        currentCmd.state = AT_IDLE;
        currentCmd.rawData = NULL;
    }
    switch (currentCmd.state)
    {
    case AT_IDLE:
        // 空闲状态，无需处理
        
        break;

    case AT_SENDING:
    {
        // 发送AT命令
        // 清空响应缓冲区
        memset(currentCmd.response, 0, sizeof(currentCmd.response));
        currentCmd.responseIndex = 0;

        // 在命令末尾添加换行符（如果没有的话）
        size_t cmdLen = strlen(currentCmd.command);
        if (cmdLen >= 2 && (currentCmd.command[cmdLen - 2] != '\r' || currentCmd.command[cmdLen - 1] != '\n'))
        {
            strcat(currentCmd.command, "\r\n");
        }

        uart4_send_buf((uint8_t *)currentCmd.command, strlen(currentCmd.command));
        log_v("AT command sent: %s", currentCmd.command);
        currentCmd.state = AT_WAITING;
        // 如果是需要发送RAW数据的命令（如MQTTPUBRAW）
        if (currentCmd.needRawSend && currentCmd.rawData && currentCmd.rawDataLen > 0)
        {
            // 等待提示符 ">" 出现
            currentCmd.rawSent = 0;
            currentCmd.state = AT_WAITING_PROMPT;
        }
        else
        {
            currentCmd.state = AT_WAITING;
        }
        break;
    }
    case AT_WAITING:
    {
        // 定义一个变量跟踪是否接收到了数据
        // uint8_t dataReceived = 0;

        // 检查是否有新数据
        while (rxHead != rxTail)
        {
            // dataReceived = 1;
            char c = rxBuffer[rxTail];
            rxTail = (rxTail + 1) % 1024;

            // 记录接收到的字符（调试用）
            // log_d("Rx char: 0x%02X (%c)", c, (c >= 32 && c < 127) ? c : '.');

            // 防止缓冲区溢出
            if (currentCmd.responseIndex < sizeof(currentCmd.response) - 1)
            {
                currentCmd.response[currentCmd.responseIndex++] = c;
                currentCmd.response[currentCmd.responseIndex] = '\0'; // 确保总是以null结尾
            }

            // 如果响应中包含"OK"或"ERROR"就认为命令执行完成
            if ((c == '\n' || c == 'K' || c == 'R') && // 优化：只在可能形成关键词的字符时才检查
                (strstr(currentCmd.response, "OK") || strstr(currentCmd.response, "ERROR")))
            {
                log_d("Found command completion marker in response");

                // 直接进入处理状态，不区分命令类型
                currentCmd.state = AT_PROCESSING;
                break;
            }
        }

        // // 如果接收到了一些数据但还没处理，输出调试信息
        // if (dataReceived && currentCmd.state == AT_WAITING &&
        //     Timer.systemTime - currentCmd.startTime > 200)
        // {
        //     log_d("Partial response: [%s]", currentCmd.response);
        // }

        // 检查超时
        if (Timer.systemTime - currentCmd.startTime > currentCmd.timeoutMs)
        {
            log_w("AT command timeout: %s, received so far: [%s]",
                  currentCmd.command, currentCmd.response);
            currentCmd.state = AT_ERROR;
        }
        if (strstr(currentCmd.response, "+EVENT:MQTT_CONNECT"))
        {
 //           log_i("MQTT connected successfully");
            WIFI.mqttConnected = 1;
        }
        else if (strstr(currentCmd.response, "+EVENT:MQTT_DISCONNECT"))
        {
   //         log_i("MQTT disconnected");
            WIFI.mqttConnected = 0;
        }

        break;
    }
        // 在AT_PROCESSING部分增强检测逻辑
        // 在AT_PROCESSING部分特殊处理HTTP响应
    case AT_PROCESSING:
        // 处理响应
        currentCmd.response[currentCmd.responseIndex] = '\0'; // 确保字符串结束

        log_d("AT response (text): [%s]", currentCmd.response);

        if (currentCmd.callback)
        {
            // 检测是否包含HTTP响应（检查是否有JSON格式数据）
            if (strstr(currentCmd.response, "{") && strstr(currentCmd.response, "}"))
            {
                // 解析HTTP响应
                char *jsonStart = strstr(currentCmd.response, "{");
                char *jsonEnd = strrchr(currentCmd.response, '}');

                if (jsonStart && jsonEnd && jsonEnd > jsonStart)
                {
                    // 提取完整的JSON数据
                    size_t jsonLen = jsonEnd - jsonStart + 1;
                    char *jsonData = (char *)malloc(jsonLen + 1);
                    if (jsonData)
                    {
                        memcpy(jsonData, jsonStart, jsonLen);
                        jsonData[jsonLen] = '\0';

                        log_i("Extracted JSON: %s", jsonData);

                        // 调用回调，传递成功状态和JSON数据
                        currentCmd.callback(1, jsonData);
                        free(jsonData);
                    }
                    else
                    {
                        // 内存分配失败，仍然调用回调但使用原始响应
                        log_e("Failed to allocate memory for JSON extraction");
                        currentCmd.callback(1, currentCmd.response);
                    }
                }
                else
                {
                    // JSON数据格式不完整，使用原始响应
                    currentCmd.callback(1, currentCmd.response);
                }
            }
            else
            {
                // 不是HTTP JSON响应，使用标准检测
                uint8_t success = (strstr(currentCmd.response, "OK") != NULL);
                currentCmd.callback(success, currentCmd.response);
            }
        }

        currentCmd.state = AT_IDLE;
        break;
    case AT_WAITING_PROMPT:
    {
        // 检查是否有新数据
//        uint8_t dataReceived = 0;
        while (rxHead != rxTail)
        {
//            dataReceived = 1;
            char c = rxBuffer[rxTail];
            rxTail = (rxTail + 1) % 1024;

            // 防止缓冲区溢出
            if (currentCmd.responseIndex < sizeof(currentCmd.response) - 1)
            {
                currentCmd.response[currentCmd.responseIndex++] = c;
                currentCmd.response[currentCmd.responseIndex] = '\0';
            }

            // 检查是否收到提示符 ">"
            if (c == '>')
            {
                log_d("Received prompt '>' for raw data");

                // 发送RAW数据
                if (currentCmd.rawData && currentCmd.rawDataLen > 0)
                {
                    // 异步发送原始数据
                    uart4_send_buf((uint8_t *)currentCmd.rawData, currentCmd.rawDataLen);
                    log_d("Sent %zu bytes of raw data", currentCmd.rawDataLen);

                    currentCmd.rawSent = 1;
                    currentCmd.state = AT_WAITING; // 切换到等待响应状态
                }
                else
                {
                    log_e("Raw data pointer is NULL or length is 0");
                    currentCmd.state = AT_ERROR;
                }
                break;
            }
        }

        // 检查超时
        if (Timer.systemTime - currentCmd.startTime > currentCmd.timeoutMs)
        {
            log_w("Timeout waiting for prompt '>'");
            currentCmd.state = AT_ERROR;
        }
        break;
    }
    case AT_ERROR:
        // 如果有RAW数据且已分配内存，需要释放
        if (currentCmd.needRawSend && currentCmd.rawData != NULL) {
            free((void*)currentCmd.rawData);
            currentCmd.rawData = NULL;
        }
        // 处理超时或错误
        if (currentCmd.callback)
        {
            currentCmd.callback(0, currentCmd.response);
        }

        // 清空缓冲区
        memset(currentCmd.response, 0, sizeof(currentCmd.response));
        currentCmd.responseIndex = 0;
        currentCmd.state = AT_IDLE;
        break;
    default:
        break;
    }  
}

/**
 * ************************************************************************
 * @brief 发送AT命令并等待响应
 *
 * @param[in] cmd  AT命令字符串
 * @param[in] timeoutMs  命令响应的超时时间，单位为毫秒
 * @param[in] callback  命令发送完成后或超时的回调函数，参数为状态码和命令字符串
 *
 * @return 返回发送状态，0表示发送失败，非0表示发送成功
 *
 * @version 2.0
 * @author jiaokai
 * @date 2025-05-16
 *
 * ************************************************************************
 */
uint8_t AT_SendCommand(const char *cmd, uint32_t timeoutMs, void (*callback)(uint8_t, const char *))
{
    if (currentCmd.state != AT_IDLE)
    {
        log_w("AT command busy, cannot send: %s", cmd);
        return 0; // 有命令正在处理
    }

    // 安全地复制命令
    strncpy(currentCmd.command, cmd, sizeof(currentCmd.command) - 3); // 留出\r\n和\0的空间
    currentCmd.command[sizeof(currentCmd.command) - 3] = '\0';

    currentCmd.timeoutMs = timeoutMs;
    currentCmd.startTime = Timer.systemTime;
    currentCmd.callback = callback;
    currentCmd.state = AT_SENDING;
    currentCmd.responseIndex = 0;

    return 1; // 命令已接受
}
// ...existing code...
