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
 * @brief WIFIģ�����ݽ���
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

//             // ȷ��������null��β
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
//                 ret = strstr((char *)WIFI.receiveBuffer[i], "ble_mac"); // ��ȡBLE MAC��ַ
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
//                 ret = strstr((char *)WIFI.receiveBuffer[i], "wifi_mac"); // ��ȡMAC��ַ
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
//                 ret = strstr((char *)WIFI.receiveBuffer[i], "ready"); // ��������ɹ�
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
    // ��黷�λ��������Ƿ�������
    if (rxHead != rxTail)
    {
        char tempBuf[RX_BUFFER_SIZE];
        uint16_t dataLen = 0;

        // �ӻ��λ������ж�ȡ���ݵ���ʱ������
        while (rxHead != rxTail && dataLen < RX_BUFFER_SIZE - 1)
        {
            tempBuf[dataLen++] = rxBuffer[rxTail];
            rxTail = (rxTail + 1) % 1024;
        }

        tempBuf[dataLen] = '\0'; // ȷ���ַ�������

        // ������յ�������
        char *ret;

        // ����Ƿ���BLE MAC��ַ
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

        // ����Ƿ���WiFi MAC��ַ
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

        // ���WiFi�Ƿ��ʼ��
        if (WIFI.isInitialized == 0)
        {
            ret = strstr(tempBuf, "ready");
            if (ret)
            {
                WIFI.isInitialized = 1;
                log_v("WIFI Ready");
            }
        }

        // ����Ƿ����"OK"
        ret = strstr(tempBuf, "OK");
        if (ret)
        {
            WIFI.isOK = 1;
        }

        // ��ӵ������
        log_d("Received data: %s", tempBuf);
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

/**
 * ************************************************************************
 * @brief WIFI��������
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
//  * @brief ����AT����
//  *
//  * @param[in] cmd  AT�����ַ���
//  * @param[in] timeoutMs  ������Ӧ�ĳ�ʱʱ�䣬��λΪ����
//  * @param[in] callback  �������ɺ��ʱ�Ļص�����������Ϊ״̬��������ַ���
//  *
//  * @return ���ط���״̬��0��ʾ����ʧ�ܣ���0��ʾ���ͳɹ�
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
//         return false; // ���������ڴ���
//     }

//     strncpy(currentCmd.command, cmd, sizeof(currentCmd.command) - 1);
//     currentCmd.responseIndex = 0;
//     currentCmd.timeoutMs = timeoutMs;
//     currentCmd.startTime = Timer.systemTime; // ��ȡϵͳʱ��
//     currentCmd.callback = callback;
//     currentCmd.state = AT_SENDING;

//     return true;
// }

// /**
//  * ************************************************************************
//  * @brief ATָ��״̬������
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
//         // ����״̬�����账��
//         break;

//     case AT_SENDING:
//         // ����AT����
//         // ������ĩβ��ӻ��з�
//         strcat(currentCmd.command, "\r\n");
//         uart4_send_buf((uint8_t *)currentCmd.command, strlen(currentCmd.command));
//         // uart4_send_str(currentCmd.command);
//         // uart4_send_str("\r\n");
//         log_v("AT command: %s", currentCmd.command);
//         currentCmd.state = AT_WAITING;
//         break;

//     case AT_WAITING:
//         // ����Ƿ���������
//         while (rxHead != rxTail)
//         {
//             char c = rxBuffer[rxTail];
//             rxTail = (rxTail + 1) % BUFFER_SIZE;

//             currentCmd.response[currentCmd.responseIndex++] = c;
//             if (currentCmd.responseIndex >= sizeof(currentCmd.response) - 1)
//             {
//                 currentCmd.responseIndex = sizeof(currentCmd.response) - 1;
//             }

//             // ����Ƿ���յ�������Ӧ����"OK"��"ERROR"��
//             if (strstr(currentCmd.response, "OK") ||
//                 strstr(currentCmd.response, "ERROR"))
//             {
//                 currentCmd.state = AT_PROCESSING;
//                 break;
//             }
//         }

//         // ��鳬ʱ
//         if (Timer.systemTime - currentCmd.startTime > currentCmd.timeoutMs)
//         {
//             currentCmd.state = AT_ERROR;
//         }
//         break;

//     case AT_PROCESSING:
//         // ������Ӧ
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
//         // �������
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
 * @brief ATָ��״̬������
 *
 * �Ľ���ATָ�����������׳�ش�����Ӧ
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
    // ��ӳ�ʱ���
    if (currentCmd.state != AT_IDLE && 
        Timer.systemTime - currentCmd.startTime > 5000) { // 5�볬ʱ
        
        log_e("AT command timeout, forcing reset");
        
        // �����RAW����ͷŻ�����
        if (currentCmd.needRawSend && currentCmd.rawData && 
            currentCmd.bufferIndex < PAYLOAD_BUFFER_COUNT) {
            payloadBuffers[currentCmd.bufferIndex].inUse = 0;
        }
        
        // ��������״̬
        currentCmd.state = AT_IDLE;
        currentCmd.rawData = NULL;
    }
    switch (currentCmd.state)
    {
    case AT_IDLE:
        // ����״̬�����账��
        
        break;

    case AT_SENDING:
    {
        // ����AT����
        // �����Ӧ������
        memset(currentCmd.response, 0, sizeof(currentCmd.response));
        currentCmd.responseIndex = 0;

        // ������ĩβ��ӻ��з������û�еĻ���
        size_t cmdLen = strlen(currentCmd.command);
        if (cmdLen >= 2 && (currentCmd.command[cmdLen - 2] != '\r' || currentCmd.command[cmdLen - 1] != '\n'))
        {
            strcat(currentCmd.command, "\r\n");
        }

        uart4_send_buf((uint8_t *)currentCmd.command, strlen(currentCmd.command));
        log_v("AT command sent: %s", currentCmd.command);
        currentCmd.state = AT_WAITING;
        // �������Ҫ����RAW���ݵ������MQTTPUBRAW��
        if (currentCmd.needRawSend && currentCmd.rawData && currentCmd.rawDataLen > 0)
        {
            // �ȴ���ʾ�� ">" ����
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
        // ����һ�����������Ƿ���յ�������
        // uint8_t dataReceived = 0;

        // ����Ƿ���������
        while (rxHead != rxTail)
        {
            // dataReceived = 1;
            char c = rxBuffer[rxTail];
            rxTail = (rxTail + 1) % 1024;

            // ��¼���յ����ַ��������ã�
            // log_d("Rx char: 0x%02X (%c)", c, (c >= 32 && c < 127) ? c : '.');

            // ��ֹ���������
            if (currentCmd.responseIndex < sizeof(currentCmd.response) - 1)
            {
                currentCmd.response[currentCmd.responseIndex++] = c;
                currentCmd.response[currentCmd.responseIndex] = '\0'; // ȷ��������null��β
            }

            // �����Ӧ�а���"OK"��"ERROR"����Ϊ����ִ�����
            if ((c == '\n' || c == 'K' || c == 'R') && // �Ż���ֻ�ڿ����γɹؼ��ʵ��ַ�ʱ�ż��
                (strstr(currentCmd.response, "OK") || strstr(currentCmd.response, "ERROR")))
            {
                log_d("Found command completion marker in response");

                // ֱ�ӽ��봦��״̬����������������
                currentCmd.state = AT_PROCESSING;
                break;
            }
        }

        // // ������յ���һЩ���ݵ���û�������������Ϣ
        // if (dataReceived && currentCmd.state == AT_WAITING &&
        //     Timer.systemTime - currentCmd.startTime > 200)
        // {
        //     log_d("Partial response: [%s]", currentCmd.response);
        // }

        // ��鳬ʱ
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
        // ��AT_PROCESSING������ǿ����߼�
        // ��AT_PROCESSING�������⴦��HTTP��Ӧ
    case AT_PROCESSING:
        // ������Ӧ
        currentCmd.response[currentCmd.responseIndex] = '\0'; // ȷ���ַ�������

        log_d("AT response (text): [%s]", currentCmd.response);

        if (currentCmd.callback)
        {
            // ����Ƿ����HTTP��Ӧ������Ƿ���JSON��ʽ���ݣ�
            if (strstr(currentCmd.response, "{") && strstr(currentCmd.response, "}"))
            {
                // ����HTTP��Ӧ
                char *jsonStart = strstr(currentCmd.response, "{");
                char *jsonEnd = strrchr(currentCmd.response, '}');

                if (jsonStart && jsonEnd && jsonEnd > jsonStart)
                {
                    // ��ȡ������JSON����
                    size_t jsonLen = jsonEnd - jsonStart + 1;
                    char *jsonData = (char *)malloc(jsonLen + 1);
                    if (jsonData)
                    {
                        memcpy(jsonData, jsonStart, jsonLen);
                        jsonData[jsonLen] = '\0';

                        log_i("Extracted JSON: %s", jsonData);

                        // ���ûص������ݳɹ�״̬��JSON����
                        currentCmd.callback(1, jsonData);
                        free(jsonData);
                    }
                    else
                    {
                        // �ڴ����ʧ�ܣ���Ȼ���ûص���ʹ��ԭʼ��Ӧ
                        log_e("Failed to allocate memory for JSON extraction");
                        currentCmd.callback(1, currentCmd.response);
                    }
                }
                else
                {
                    // JSON���ݸ�ʽ��������ʹ��ԭʼ��Ӧ
                    currentCmd.callback(1, currentCmd.response);
                }
            }
            else
            {
                // ����HTTP JSON��Ӧ��ʹ�ñ�׼���
                uint8_t success = (strstr(currentCmd.response, "OK") != NULL);
                currentCmd.callback(success, currentCmd.response);
            }
        }

        currentCmd.state = AT_IDLE;
        break;
    case AT_WAITING_PROMPT:
    {
        // ����Ƿ���������
//        uint8_t dataReceived = 0;
        while (rxHead != rxTail)
        {
//            dataReceived = 1;
            char c = rxBuffer[rxTail];
            rxTail = (rxTail + 1) % 1024;

            // ��ֹ���������
            if (currentCmd.responseIndex < sizeof(currentCmd.response) - 1)
            {
                currentCmd.response[currentCmd.responseIndex++] = c;
                currentCmd.response[currentCmd.responseIndex] = '\0';
            }

            // ����Ƿ��յ���ʾ�� ">"
            if (c == '>')
            {
                log_d("Received prompt '>' for raw data");

                // ����RAW����
                if (currentCmd.rawData && currentCmd.rawDataLen > 0)
                {
                    // �첽����ԭʼ����
                    uart4_send_buf((uint8_t *)currentCmd.rawData, currentCmd.rawDataLen);
                    log_d("Sent %zu bytes of raw data", currentCmd.rawDataLen);

                    currentCmd.rawSent = 1;
                    currentCmd.state = AT_WAITING; // �л����ȴ���Ӧ״̬
                }
                else
                {
                    log_e("Raw data pointer is NULL or length is 0");
                    currentCmd.state = AT_ERROR;
                }
                break;
            }
        }

        // ��鳬ʱ
        if (Timer.systemTime - currentCmd.startTime > currentCmd.timeoutMs)
        {
            log_w("Timeout waiting for prompt '>'");
            currentCmd.state = AT_ERROR;
        }
        break;
    }
    case AT_ERROR:
        // �����RAW�������ѷ����ڴ棬��Ҫ�ͷ�
        if (currentCmd.needRawSend && currentCmd.rawData != NULL) {
            free((void*)currentCmd.rawData);
            currentCmd.rawData = NULL;
        }
        // ����ʱ�����
        if (currentCmd.callback)
        {
            currentCmd.callback(0, currentCmd.response);
        }

        // ��ջ�����
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
 * @brief ����AT����ȴ���Ӧ
 *
 * @param[in] cmd  AT�����ַ���
 * @param[in] timeoutMs  ������Ӧ�ĳ�ʱʱ�䣬��λΪ����
 * @param[in] callback  �������ɺ��ʱ�Ļص�����������Ϊ״̬��������ַ���
 *
 * @return ���ط���״̬��0��ʾ����ʧ�ܣ���0��ʾ���ͳɹ�
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
        return 0; // ���������ڴ���
    }

    // ��ȫ�ظ�������
    strncpy(currentCmd.command, cmd, sizeof(currentCmd.command) - 3); // ����\r\n��\0�Ŀռ�
    currentCmd.command[sizeof(currentCmd.command) - 3] = '\0';

    currentCmd.timeoutMs = timeoutMs;
    currentCmd.startTime = Timer.systemTime;
    currentCmd.callback = callback;
    currentCmd.state = AT_SENDING;
    currentCmd.responseIndex = 0;

    return 1; // �����ѽ���
}
// ...existing code...
