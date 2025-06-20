/**
 *
 * @file app_mqtt.c
 * @author jiaokai
 * @brief MQTTӦ�ó���
 *
 * @copyright Copyright (c) 2025
 */
#include "app_mqtt.h"
#include "app_wifi.h"
#include "coffeeMake.h"
#include "teaMake.h"
#include "cJSON.h"
#include "eeprom.h"
#include "config.h"
#include "string.h"
#include "elog.h"
#include "app_wifi.h"
#include "app_wifi_set.h"
#include "bsp_wifi.h"
#include "work.h"
#include "stdlib.h"
#include "systick.h"
#include "timer.h"
#include "lcd_data_process.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "app_mqtt"


// ��ȡ��һ�����û���������
int getNextAvailableBuffer(void)
{
    // �ȳ��Դӵ�ǰ������ʼ����
    for (int i = 0; i < PAYLOAD_BUFFER_COUNT; i++) {
        int idx = (currentBufferIndex + i) % PAYLOAD_BUFFER_COUNT;
        if (!payloadBuffers[idx].inUse) {
            currentBufferIndex = (idx + 1) % PAYLOAD_BUFFER_COUNT;
            return idx;
        }
    }
    return -1; // û�п��û�����
}

/**
 * ************************************************************************
 * @brief MQTT���ú���
 *
 *
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-15
 *  10������һ��
 * ************************************************************************
 */
void mqttConfig(void)
{
    // ÿ10����һ��WIFI״̬
    if (deviceInfo.data.MQTT.isRegistered != 1) // �豸δע��
    {
        // ���WIFIģ���Ƿ��ѳ�ʼ��
        if (WIFI.isInitialized)
        {
            // WIFI�����ӣ�����ִ���豸ע��
            if (WIFI.isConnected)
            {
                // ����豸δע�ᣬ�����ע��
                log_d("device not registered, start register");
                if (deviceInfo.data.MQTT.isRegistered != 1)
                {
                    registerDevice();
                }
            }
            // WIFIδ���ӣ���������
            else
            {
                // wifiConnectionConfiguration();
            }
        }
    }
    else // �豸��ע��
    {

        // ���MQTT��������
        if (WIFI.runWIFIConfigEnd == 1)
        {
            add_task(MQTT_Connect, NULL, 1000, true);
            log_i("devcice registered");
            delete_task(mqttConfig); // ɾ��ע������
        }
    }
}

/**
 * ************************************************************************
 * @brief �豸ע�ᵽ������
 *
 * @return �Ƿ�ɹ�����ע������
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-15
 *
 * ************************************************************************
 */
uint8_t registerDevice(void)
{
    if (!WIFI.isInitialized || !WIFI.isConnected)
    {
        log_e("WIFIδ��ʼ����δ����,�޷�ע���豸");
        return false;
    }

    // // �����豸��ϢJSON����
    // char jsonData[128];
    // snprintf(jsonData, sizeof(jsonData),
    //         "{\\\"ETID\\\":\\\"%s\\\",\\\"DV\\\":\\\"%d.%d\\\",\\\"ESN\\\":\\\"%s\\\"}",
    //         deviceInfo.data.MQTT.ETID,
    //         deviceInfo.data.FirmwareVersion/10,deviceInfo.data.FirmwareVersion%10,
    //         deviceInfo.data.MQTT.ESN);
    // log_d("jsonData: %s", jsonData);
    // ����HTTP POST��������

    char atCommand[256];
    float firmwareVersion = deviceInfo.data.FirmwareVersion / 10.0f;
    memset(atCommand, 0, sizeof(atCommand));
    snprintf(atCommand, sizeof(atCommand),
             "AT+HTTPCLIENTLINE=1,3,application/x-www-form-urlencoded,www.sparkinger.com,30020,/api/Equipment/AddDevice?ETID=%s&DV=%.1f&ESN=%s",
             deviceInfo.data.MQTT.ETID,
             firmwareVersion,
             deviceInfo.data.MQTT.ESN);

    // ����HTTP����
    return AT_SendCommand(atCommand, 15000, registerDeviceCallback);
}

/**
 * ************************************************************************
 * @brief �豸ע��ص����� - ������Ӧ
 *
 * @param[in] success AT�����Ƿ�ִ�гɹ�
 * @param[in] response ��������Ӧ����
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-15
 *
 * ************************************************************************
 */
void registerDeviceCallback(uint8_t success, const char *response)
{
    if (!success)
    {
        log_e("�豸ע������ʧ��,�����AT�������");
        return;
    }

    log_d("ע����Ӧԭʼ����: %s", response);

    // // ���� - ��ʾԭʼ��Ӧ��ʮ����������
    // log_d("��Ӧʮ�����Ʊ�ʾ:");
    // for (size_t i = 0; i < strlen(response) && i < 100; i++) {
    //     log_d("  byte[%d]: 0x%02X (%c)", i, (unsigned char)response[i],
    //           (response[i] >= 32 && response[i] < 127) ? response[i] : '.');
    // }

    // ��AT��Ӧ�в���JSON��Ӧ�� - ���ϸ�Ĳ���
    // ���������ܵĳ���ǰ׺��ʮ���������ͻ��з���
    const char *potentialStart = response;
    while (*potentialStart && !(*potentialStart == '{' ||
                                (*potentialStart >= 'a' && *potentialStart <= 'f') ||
                                (*potentialStart >= '0' && *potentialStart <= '9')))
    {
        potentialStart++;
    }

    // ����Ƿ���ʮ�����Ƴ���ǰ׺
    const char *jsonStart = strchr(potentialStart, '{');
    if (!jsonStart)
    {
        log_e("�޷��ҵ�JSON��Ӧ��ʼλ�� '{'");
        return;
    }

    // �ҵ�JSON����λ��
    const char *jsonEnd = strrchr(jsonStart, '}');
    if (!jsonEnd)
    {
        log_e("�޷��ҵ�JSON����λ�� '}'");
        return;
    }

    // ��ȡ����JSON
    size_t jsonLen = jsonEnd - jsonStart + 1;
    char *jsonData = (char *)malloc(jsonLen + 1);
    if (!jsonData)
    {
        log_e("�ڴ����ʧ��, �����С: %d bytes", (int)jsonLen + 1);
        return;
    }

    // ����JSON�������ֹ��
    memcpy(jsonData, jsonStart, jsonLen);
    jsonData[jsonLen] = '\0';

    log_d("��ȡ��JSON (���� %d): %s", (int)jsonLen, jsonData);

    // ���Խ���JSON
    cJSON *root = cJSON_Parse(jsonData);
    if (!root)
    {
        // ����ʧ�ܣ���ȡ������Ϣ
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr)
        {
            log_e("cJSON������������: %.20s", error_ptr);
        }
        else
        {
            log_e("δ֪cJSON��������");
        }

        // ����������ܰ����Ĳ��ɼ��ַ�
        for (size_t i = 0; i < jsonLen; i++)
        {
            if ((unsigned char)jsonData[i] < 32 && jsonData[i] != '\t' && jsonData[i] != '\n' && jsonData[i] != '\r')
            {
                jsonData[i] = ' '; // �滻Ϊ�ո�
            }
        }

        // ���Խ���
        log_d("��������Խ���: %s", jsonData);
        root = cJSON_Parse(jsonData);
        if (!root)
        {
            log_e("�ڶ��ν���ʧ�ܣ�����");
            free(jsonData);
            return;
        }
    }

    // �ɹ�����JSON
    log_d("cJSON�����ɹ�");

    // ��ȡcode�ֶ�
    cJSON *codeItem = cJSON_GetObjectItem(root, "code");
    if (!codeItem || !cJSON_IsString(codeItem))
    {
        log_e("�޷��ҵ���Ч��code�ֶ�");
        cJSON_Delete(root);
        free(jsonData);
        return;
    }

    const char *code = codeItem->valuestring;
    log_d("������codeֵ: %s", code);

    // ����code
    if (strcmp(code, "0") == 0 || strcmp(code, "401") == 0)
    {
        // �ɹ����豸��ע��
        const char *status = strcmp(code, "401") == 0 ? "�Ѵ���" : "�ɹ�";
        log_i("�豸ע��%s", status);

        // ��ȡdata����
        cJSON *data = cJSON_GetObjectItem(root, "data");
        if (data)
        {
            // ��ȡ������Ϣ
            cJSON *clientID = cJSON_GetObjectItem(data, "clientID");
            cJSON *userName = cJSON_GetObjectItem(data, "userName");
            cJSON *password = cJSON_GetObjectItem(data, "password");

            // ����ֶ��Ƿ���������ַ�������
            if (clientID && cJSON_IsString(clientID) &&
                userName && cJSON_IsString(userName) &&
                password && cJSON_IsString(password))
            {

                // ��ȫ������ȷ���ַ�����ֹ
                memset(deviceInfo.data.MQTT.ClientID, 0, sizeof(deviceInfo.data.MQTT.ClientID));
                memset(deviceInfo.data.MQTT.UserName, 0, sizeof(deviceInfo.data.MQTT.UserName));
                memset(deviceInfo.data.MQTT.Password, 0, sizeof(deviceInfo.data.MQTT.Password));

                strncpy(deviceInfo.data.MQTT.ClientID, clientID->valuestring,
                        sizeof(deviceInfo.data.MQTT.ClientID) - 1);
                strncpy(deviceInfo.data.MQTT.UserName, userName->valuestring,
                        sizeof(deviceInfo.data.MQTT.UserName) - 1);
                strncpy(deviceInfo.data.MQTT.Password, password->valuestring,
                        sizeof(deviceInfo.data.MQTT.Password) - 1);

                // �������
                log_d("�����ClientID: %s", deviceInfo.data.MQTT.ClientID);
                log_d("�����UserName: %s", deviceInfo.data.MQTT.UserName);
                log_d("�����Password: %s", deviceInfo.data.MQTT.Password);

                // ����ע���־
                deviceInfo.data.MQTT.isRegistered = 1;

                // ���浽EEPROM
                eeprom_buffer_write_timeout(deviceInfo.bytes, DEVICE_INFO_ADDRESS, sizeof(deviceInfo_TypeDef));
                log_i("MQTT������Ϣ�ѱ��浽EEPROM");
            }
            else
            {
                log_e("JSON������ȱ�ٱ�Ҫ��MQTT�����ֶλ��ֶ����ʹ���");
            }
        }
        else
        {
            log_e("JSON��ȱ��data����");
        }
    }
    else
    {
        log_e("�豸ע��ʧ�ܣ�������: %s", code);
    }

    // ������Դ
    cJSON_Delete(root);
    free(jsonData);
}

/**
 * ************************************************************************
 * @brief ��JSON�ַ�������ȡָ���ֶ�ֵ
 *
 * @param[in] json JSON�ַ���
 * @param[in] field �ֶ���
 * @param[out] value ��ȡ����ֵ
 * @param[in] maxLen ֵ��������󳤶�
 *
 * @return �Ƿ�ɹ���ȡ
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-15
 *
 * ************************************************************************
 */
uint8_t extractJsonField(const char *json, const char *field, char *value, uint16_t maxLen)
{
    char fieldPattern[32];
    snprintf(fieldPattern, sizeof(fieldPattern), "\"%s\":\"", field);

    const char *fieldStart = strstr(json, fieldPattern);
    if (!fieldStart)
    {
        // ���Բ��ҷ��ַ��������ֶ�
        snprintf(fieldPattern, sizeof(fieldPattern), "\"%s\":", field);
        fieldStart = strstr(json, fieldPattern);
        if (!fieldStart)
        {
            return false;
        }
        fieldStart += strlen(fieldPattern);
    }
    else
    {
        fieldStart += strlen(fieldPattern);
    }

    uint16_t i = 0;
    // ��ȡ�ֶ�ֵ��ֱ���������Ż򶺺Ż��������
    while (i < maxLen - 1 && fieldStart[i] != '"' && fieldStart[i] != ',' &&
           fieldStart[i] != '}' && fieldStart[i] != '\0')
    {
        value[i] = fieldStart[i];
        i++;
    }
    value[i] = '\0';

    return true;
}

/**
 * ************************************************************************
 * @brief ��JSONǶ�׶�������ȡָ���ֶ�ֵ
 *
 * @param[in] json JSON�ַ���
 * @param[in] object ������
 * @param[in] field �ֶ���
 * @param[out] value ��ȡ����ֵ
 * @param[in] maxLen ֵ��������󳤶�
 *
 * @return �Ƿ�ɹ���ȡ
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-15
 *
 * ************************************************************************
 */
uint8_t extractJsonNestedField(const char *json, const char *object, const char *field, char *value, uint16_t maxLen)
{
    char objectPattern[32];
    snprintf(objectPattern, sizeof(objectPattern), "\"%s\":{", object);

    const char *objectStart = strstr(json, objectPattern);
    if (!objectStart)
    {
        return false;
    }

    // ����������λ��
    objectStart += strlen(objectPattern);

    // �ڶ����ڲ����ֶ�
    char fieldPattern[32];
    snprintf(fieldPattern, sizeof(fieldPattern), "\"%s\":\"", field);

    const char *fieldStart = strstr(objectStart, fieldPattern);
    if (!fieldStart)
    {
        return false;
    }

    fieldStart += strlen(fieldPattern);

    uint16_t i = 0;
    // ��ȡ�ֶ�ֵ��ֱ����������
    while (i < maxLen - 1 && fieldStart[i] != '"')
    {
        value[i] = fieldStart[i];
        i++;
    }
    value[i] = '\0';

    return true;
}

/**
 * ************************************************************************
 * @brief ���ò����ӵ�MQTT������
 *
 * ����BW16ģ��ATָ������MQTT��������������
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-17
 *
 * ************************************************************************
 */
void MQTT_Connect(void)
{
    static uint8_t step = 0;
    char atCommand[256] = {0};
    if (!WIFI.isInitialized || !WIFI.isConnected || deviceInfo.data.MQTT.isRegistered != 1)
    {
        log_e("WIFI is not initialized or connected, or device is not registered");
        AT_SendCommand("AT+WJAP?", 500, AT_WJAPCallback);
        return;
    }
    switch (step)
    {
    case 0:
        log_i("start config MQTT");
        // ����MQTT��������ַ

        snprintf(atCommand, sizeof(atCommand), "AT+MQTT=1,%s", deviceInfo.data.MQTT.serverAddress);
        if (AT_SendCommand(atCommand, 500, mqttConfigCallback))
        {
            log_i("config MQTT server address success");
            step++;
        }
        else
        {
            log_e("config MQTT server address failed");
        }
        break;
    case 1:
        // ����MQTT�˿ں�
        memset(atCommand, 0, sizeof(atCommand));
        snprintf(atCommand, sizeof(atCommand), "AT+MQTT=2,%d", MQTT_PORT); // Ĭ��MQTT�˿�
        if (AT_SendCommand(atCommand, 500, mqttConfigCallback))
        {
            log_i("config MQTT port success");
            step++;
        }
        else
        {
            log_e("config MQTT port failed");
        }
        break;
    case 2:
        // ����MQTT���ӷ�ʽ��Ĭ��TCP��
        memset(atCommand, 0, sizeof(atCommand));
        snprintf(atCommand, sizeof(atCommand), "AT+MQTT=3,1");
        if (AT_SendCommand(atCommand, 500, mqttConfigCallback))
        {
            log_i("config MQTT connection type success");
            step++;
        }
        else
        {
            log_e("config MQTT connection type failed");
        }
        break;
    case 3:
        // ����MQTT�ͻ���ID
        memset(atCommand, 0, sizeof(atCommand));
        snprintf(atCommand, sizeof(atCommand), "AT+MQTT=4,%s", deviceInfo.data.MQTT.ClientID);
        if (AT_SendCommand(atCommand, 500, mqttConfigCallback))
        {
            log_i("config MQTT client ID success");
            step++;
        }
        else
        {
            log_e("config MQTT client ID failed");
        }
        break;
    case 4:
        // ����MQTT�û���
        memset(atCommand, 0, sizeof(atCommand));
        snprintf(atCommand, sizeof(atCommand), "AT+MQTT=5,%s", deviceInfo.data.MQTT.UserName);
        if (AT_SendCommand(atCommand, 500, mqttConfigCallback))
        {
            log_i("config MQTT username success");
            step++;
        }
        else
        {
            log_e("config MQTT username failed");
        }
        break;
    case 5:
        // ����MQTT����
        memset(atCommand, 0, sizeof(atCommand));
        snprintf(atCommand, sizeof(atCommand), "AT+MQTT=6,%s", deviceInfo.data.MQTT.Password);
        if (AT_SendCommand(atCommand, 500, mqttConfigCallback))
        {
            log_i("config MQTT password success");
            step++;
        }
        else
        {
            log_e("config MQTT password failed");
        }
        break;
    case 6:
        // ����MQTT��������ѡ��
        memset(atCommand, 0, sizeof(atCommand));
        // ʾ������������������Ϊ�豸ID��QoSΪ0������������ϢΪ"offline"
        snprintf(atCommand, sizeof(atCommand), "AT+MQTT=7,\"\",0,0,\"\"\r\n");
        if (AT_SendCommand(atCommand, 500, mqttConfigCallback))
        {
            log_i("config MQTT will message success");
            step++;
        }
        else
        {
            log_e("config MQTT will message failed");
        }
        break;
    case 7:
        // ������ӵ�MQTT������
        log_i("��ʼ����MQTT������...");
        if (AT_SendCommand("AT+MQTT", 500, mqttConnectCallback))
        {
            log_i("MQTT connect command success");
            step++;
        }
        else
        {
            log_e("MQTT connect command failed");
        }
        break;
    case 8:
        if (AT_SendCommand("AT+MQTT?", 500, mqttQueryCallback))
        {
            log_i("MQTT query command success");
            step++;
        }
        else
        {
            log_e("MQTT query command failed");
        }
        break;
    case 9:
        // �ȴ�MQTT���ӳɹ�
        if (WIFI.mqttConnected == 1)
        {
            log_i("MQTT connected successfully");
            step++;
        }
        else if (WIFI.mqttConnected == 0)
        {
            step = 8; // ���ò���
        }
        break;
    case 10:
        step++;
        break;
    case 11:
        step++;
        break;
    case 12:
        step++;
        break;
    case 13:
        if (MQTT_Subscribe())
        {
            log_i("MQTT subscribe success");
            step++;
        }
        else
        {
            log_e("MQTT subscribe failed");
        }
        break;
    case 14:
        step++;
        break;
    case 15:
        if (WIFI.mqttConnected == 1) // ���÷���
        {
            if (publish_allData())
            {
                log_i("MQTT subscribe success");
                step++;
            }
            else
            {
                log_e("MQTT subscribe failed");
            }
        }
        break;
    case 16:
        delete_task(MQTT_Connect); // ɾ��MQTT��������
        log_i("delete MQTT connect task");
        break;
    default:
        break;
    }
}

/**
 * ************************************************************************
 * @brief MQTT�������ûص�����
 *
 * ����MQTT���������������Ӧ
 *
 * @param[in] success AT�����Ƿ�ִ�гɹ�
 * @param[in] response ��Ӧ�ַ���
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-17
 *
 * ************************************************************************
 */
void mqttConfigCallback(uint8_t success, const char *response)
{
    if (success)
    {
        log_d("MQTT config success: %s", response);
    }
    else
    {
        log_e("MQTT config failed: %s", response);
    }
}

/**
 * ************************************************************************
 * @brief MQTT���ӻص�����
 *
 * ����MQTT�����������Ӧ
 *
 * @param[in] success AT�����Ƿ�ִ�гɹ�
 * @param[in] response ��Ӧ�ַ���
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-17
 *
 * ************************************************************************
 */
void mqttConnectCallback(uint8_t success, const char *response)
{
    if (success)
    {
        log_i("MQTT connect command executed successfully: %s", response);

        // ע�⣺��ʱֻ��ʾAT����ͳɹ���ʵ�����ӿ��ܻ��ڽ�����
        // ��Ҫͨ��AT_Update��������"+EVENT:MQTT_CONNECT"�¼���ȷ�����ӳɹ�

        // ��ʼ��MQTT���Ӽ���
        // WIFI.mqttConnected = 0;
        WIFI.mqttConnectionStartTime = Timer.systemTime;
    }
    else
    {
        log_e("MQTT connect command failed: %s", response);
    }
}






/**
 * ************************************************************************
 * @brief MQTT���ⶩ��
 *
 * �����豸��ص�����
 *
 * @return �Ƿ�ɹ���������
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-17
 *
 * ************************************************************************
 */
uint8_t MQTT_Subscribe(void)
{
    if (!WIFI.mqttConnected)
    {
        log_e("MQTT is not connected, cannot subscribe topic");
        return 0;
    }
    char atCommand[256] = {0};
    // AT+MQTTSUB=<topic>,<qos>
    snprintf(atCommand, sizeof(atCommand), "AT+MQTTSUB=\"%s\",0", deviceInfo.data.MQTT.subTopic);

    return AT_SendCommand(atCommand, 1000, mqttSubscribeCallback);
}



/**
 * ************************************************************************
 * @brief MQTT���Ļص�����
 *
 * @param[in] success AT�����Ƿ�ִ�гɹ�
 * @param[in] response ��Ӧ�ַ���
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-17
 *
 * ************************************************************************
 */
void mqttSubscribeCallback(uint8_t success, const char *response)
{
    if (success)
    {
        log_i("MQTT topic subscription succeeded");
    }
    else
    {
        log_e("MQTT topic subscription failed: %s", response);
    }
}

/**
 * ************************************************************************
 * @brief MQTT��ѯ���ûص�����
 *
 * ����MQTT��ѯ�������Ӧ������MQTT״̬�����ò���
 *
 * @param[in] success AT�����Ƿ�ִ�гɹ�
 * @param[in] response ��Ӧ�ַ���
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-17
 *
 * ************************************************************************
 */
void mqttQueryCallback(uint8_t success, const char *response)
{
    if (!success)
    {
        log_e("MQTT query command failed: %s", response);
        return;
    }

    log_i("MQTT query command executed successfully: %s", response);

    // ������Ӧǰ׺ "+MQTT:"
    const char *mqttInfo = strstr(response, "+MQTT:");
    if (!mqttInfo)
    {
        log_e("Invalid MQTT query response format");
        return;
    }

    // ���� "+MQTT:" ǰ׺
    mqttInfo += 6;

    // ����MQTT״̬�Ͳ���
    int mqtt_status = 0;
    char host_name[64] = {0};
    int port = 0;
    int scheme = 0;
    char client_id[64] = {0};
    char username[64] = {0};
    char password[64] = {0};
    char lwt_topic[64] = {0};
    int lwt_qos = 0;
    int lwt_retained = 0;
    char lwt_payload[128] = {0};

    // ʹ��sscanf����MQTT��Ϣ
    // ��ʽ: <MQTT_status>,<Host_name>,<Port>,<scheme>,<client_id>,<username>,<password>,<LWT_topic>,<LWT_qos>,<LWT_Retained>,<LWTpayload>
    int parsed = sscanf(mqttInfo, "%d,%63[^,],%d,%d,%63[^,],%63[^,],%63[^,],%63[^,],%d,%d,%127[^\r\n]",
                        &mqtt_status, host_name, &port, &scheme, client_id, username, password,
                        lwt_topic, &lwt_qos, &lwt_retained, lwt_payload);

    if (parsed < 4)
    {
        log_e("Failed to parse MQTT query response, only %d fields parsed", parsed);
        return;
    }

    // ����MQTT����״̬
    switch (mqtt_status)
    {
    case 0:
        log_i("MQTT Status: Initial state");
        WIFI.mqttConnected = 0;
        break;
    case 1:
        log_i("MQTT Status: Connecting");
        WIFI.mqttConnected = 0;
        break;
    case 2:
        log_i("MQTT Status: Subscribing");
        // �����ӵ���δ��ɶ���
        WIFI.mqttConnected = 1;
        break;
    case 3:
        log_i("MQTT Status: Connected");
        WIFI.mqttConnected = 1;
        break;
    default:
        log_w("Unknown MQTT status: %d", mqtt_status);
        break;
    }

    // �����ϸ��MQTT������Ϣ
    log_d("MQTT Configuration:");
    log_d("  Host: %s", host_name);
    log_d("  Port: %d", port);
    log_d("  Scheme: %d", scheme);
    log_d("  Client ID: %s", client_id);
    log_d("  Username: %s", username);
}

/**
 * ************************************************************************
 * @brief ��ʼ��MQTT���⣬��DeviceIDƴ�ӵ�������
 *
 * ��MQTT_PUBTOPIC��MQTT_SUBTOPIC�е�%s�滻Ϊ�豸ID
 *
 * @return ��
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-17
 *
 * ************************************************************************
 */
void initMQTTTopics(void)
{
    char tempBuffer[128] = {0};

    // ������е������ַ���
    memset(deviceInfo.data.MQTT.pubTopic, 0, sizeof(deviceInfo.data.MQTT.pubTopic));
    memset(deviceInfo.data.MQTT.subTopic, 0, sizeof(deviceInfo.data.MQTT.subTopic));

    // ʹ��snprintf��DeviceID��䵽����������
    snprintf(tempBuffer, sizeof(tempBuffer), MQTT_PUBTOPIC, deviceInfo.data.DeviceID);

    // ��ȫ�ؽ���ʽ������ַ������Ƶ�Ŀ�����
    strncpy(deviceInfo.data.MQTT.pubTopic, tempBuffer, sizeof(deviceInfo.data.MQTT.pubTopic) - 1);
    deviceInfo.data.MQTT.pubTopic[sizeof(deviceInfo.data.MQTT.pubTopic) - 1] = '\0'; // ȷ������ֹ��

    // ʹ��snprintf��DeviceID��䵽����������
    memset(tempBuffer, 0, sizeof(tempBuffer)); // �����ʱ������
    snprintf(tempBuffer, sizeof(tempBuffer), MQTT_SUBTOPIC, deviceInfo.data.DeviceID);

    // ��ȫ�ؽ���ʽ������ַ������Ƶ�Ŀ�����
    strncpy(deviceInfo.data.MQTT.subTopic, tempBuffer, sizeof(deviceInfo.data.MQTT.subTopic) - 1);
    deviceInfo.data.MQTT.subTopic[sizeof(deviceInfo.data.MQTT.subTopic) - 1] = '\0'; // ȷ������ֹ��

    // ��ӡ��־����֤�����ʽ���Ƿ���ȷ
    log_i("MQTT pubTopic: %s", deviceInfo.data.MQTT.pubTopic);
    log_i("MQTT subTopic: %s", deviceInfo.data.MQTT.subTopic);
}

/**
 * ************************************************************************
 * @brief  ����ѭ�������д���
 *
 *
 * @return
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-17
 *
 * ************************************************************************
 */
uint8_t publish_circulationValveCnt(void)
{
    // �� cJSON ����JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *services = cJSON_AddArrayToObject(root, "services");
    cJSON *svc = cJSON_CreateObject();
    cJSON_AddItemToArray(services, svc);
    cJSON_AddStringToObject(svc, "service_id", "ser_DataReport");
    cJSON *props = cJSON_AddObjectToObject(svc, "properties");
    cJSON_AddNumberToObject(props, "Da08", record.data.circulationValveCnt);

    // ���л�������
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // ֱ��ʹ���޸ĺ��MQTT_PublishStatus����
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}

/**
 * ************************************************************************
 * @brief  ������ˮ�����д���
 *
 *
 * @return
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-17
 *
 * ************************************************************************
 */
uint8_t publish_drainValveCnt(void)
{
    // �� cJSON ����JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *services = cJSON_AddArrayToObject(root, "services");
    cJSON *svc = cJSON_CreateObject();
    cJSON_AddItemToArray(services, svc);
    cJSON_AddStringToObject(svc, "service_id", "ser_DataReport");
    cJSON *props = cJSON_AddObjectToObject(svc, "properties");
    cJSON_AddNumberToObject(props, "Da09", record.data.drainValveCnt);

    // ���л�������
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // ֱ��ʹ���޸ĺ��MQTT_PublishStatus����
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}
/**
 * ************************************************************************
 * @brief  ������������,��������һ��
 *
 *
 * @return
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-17
 *
 * ************************************************************************
 */
uint8_t publish_allData(void)
{
    // �� cJSON ����JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *services = cJSON_AddArrayToObject(root, "services");
    cJSON *svc = cJSON_CreateObject();
    cJSON_AddItemToArray(services, svc);
    cJSON_AddStringToObject(svc, "service_id", "ser_DataReport");
    cJSON *props = cJSON_AddObjectToObject(svc, "properties");
    cJSON_AddNumberToObject(props, "Da01", record.data.coffeeMakeCnt + record.data.teaMakeCnt);
    cJSON_AddNumberToObject(props, "Da02", record.data.washCnt);
    cJSON_AddNumberToObject(props, "Da08", record.data.circulationValveCnt);
    cJSON_AddNumberToObject(props, "Da09", record.data.drainValveCnt);
    cJSON_AddNumberToObject(props, "Da10", record.data.circulationPumpRunTime);
    cJSON_AddNumberToObject(props, "Da11", record.data.waterInletValveCnt);
    cJSON_AddNumberToObject(props, "Da12", record.data.washValveCnt);
    cJSON_AddNumberToObject(props, "Da13", record.data.DCPumpRunTime);
    cJSON_AddNumberToObject(props, "Da15", record.data.waterTapwashCnt);
    cJSON_AddNumberToObject(props, "Da16", deviceRunState);
    cJSON_AddNumberToObject(props, "Da19", 0);
    cJSON_AddNumberToObject(props, "Ps01", 0);
    cJSON_AddNumberToObject(props, "Ps02", config.data.isLock);
    if(config.data.isLock == 1)
    {
        cJSON_AddNumberToObject(props, "Ps03", 0);
    }
    else
    {
        cJSON_AddNumberToObject(props, "Ps03", 1);
    }
    // ���л�������
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // ֱ��ʹ���޸ĺ��MQTT_PublishStatus����
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}
/**
 * ************************************************************************
 * @brief �����豸����״̬
 *
 *
 * @return
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-19
 *
 * ************************************************************************
 */
uint8_t publish_deviceRunState(void)
{
    // �� cJSON ����JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *services = cJSON_AddArrayToObject(root, "services");
    cJSON *svc = cJSON_CreateObject();
    cJSON_AddItemToArray(services, svc);
    cJSON_AddStringToObject(svc, "service_id", "ser_DataReport");
    cJSON *props = cJSON_AddObjectToObject(svc, "properties");
    cJSON_AddNumberToObject(props, "Da16", deviceRunState);
    if(deviceRunState == 2)
    {
        cJSON_AddNumberToObject(props, "Da19", 4);
    }
    else
    {
        cJSON_AddNumberToObject(props, "Da19", 0);
    }
    // ���л�������
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // ֱ��ʹ���޸ĺ��MQTT_PublishStatus����
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}

/**
 * ************************************************************************
 * @brief �ϱ���ϴ����
 *
 *
 * @return
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-19
 *
 * ************************************************************************
 */
uint8_t publish_washCnt(void)
{
    // �� cJSON ����JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *services = cJSON_AddArrayToObject(root, "services");
    cJSON *svc = cJSON_CreateObject();
    cJSON_AddItemToArray(services, svc);
    cJSON_AddStringToObject(svc, "service_id", "ser_DataReport");
    cJSON *props = cJSON_AddObjectToObject(svc, "properties");
    cJSON_AddNumberToObject(props, "Da02", record.data.washCnt);
    cJSON_AddNumberToObject(props, "Da16", deviceRunState);

    // ���л�������
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // ֱ��ʹ���޸ĺ��MQTT_PublishStatus����
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}
/**
 * ************************************************************************
 * @brief ������ˮ�������
 *
 *
 * @return
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-19
 *
 * ************************************************************************
 */
uint8_t publish_drainRelatData(void)
{
    // �� cJSON ����JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *services = cJSON_AddArrayToObject(root, "services");
    cJSON *svc = cJSON_CreateObject();
    cJSON_AddItemToArray(services, svc);
    cJSON_AddStringToObject(svc, "service_id", "ser_DataReport");
    cJSON *props = cJSON_AddObjectToObject(svc, "properties");
    cJSON_AddNumberToObject(props, "Da08", record.data.circulationValveCnt);
    cJSON_AddNumberToObject(props, "Da09", record.data.drainValveCnt);
    cJSON_AddNumberToObject(props, "Da10", record.data.circulationPumpRunTime);
    cJSON_AddNumberToObject(props, "Da16", deviceRunState);

    // ���л�������
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // ֱ��ʹ���޸ĺ��MQTT_PublishStatus����
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}

/**
 * ************************************************************************
 * @brief ����������������
 *
 *
 * @return
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-19
 *
 * ************************************************************************
 */
uint8_t publish_coffeeMakeCnt(void)
{
    // �� cJSON ����JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *services = cJSON_AddArrayToObject(root, "services");
    cJSON *svc = cJSON_CreateObject();
    cJSON_AddItemToArray(services, svc);
    cJSON_AddStringToObject(svc, "service_id", "ser_DataReport");
    cJSON *props = cJSON_AddObjectToObject(svc, "properties");
    cJSON_AddNumberToObject(props, "Da01", record.data.coffeeMakeCnt + record.data.teaMakeCnt);
    cJSON_AddNumberToObject(props, "Da03", config.data.coffeeMake.time[coffee.num]);
    cJSON_AddNumberToObject(props, "Da04", config.data.coffeeMake.weight[coffee.num]);
    cJSON_AddNumberToObject(props, "Da05", config.data.coffeeMake.vol[coffee.num]);
    cJSON_AddNumberToObject(props, "Da06", 0);
    cJSON_AddNumberToObject(props, "Da08", record.data.circulationValveCnt);
    cJSON_AddNumberToObject(props, "Da09", record.data.drainValveCnt);
    cJSON_AddNumberToObject(props, "Da10", record.data.circulationPumpRunTime);
    cJSON_AddNumberToObject(props, "Da11", record.data.waterInletValveCnt);
    cJSON_AddNumberToObject(props, "Da16", deviceRunState);
    // ���л�������
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // ֱ��ʹ���޸ĺ��MQTT_PublishStatus����
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}

/**
 * ************************************************************************
 * @brief ��������������
 *
 *
 * @return
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-19
 *
 * ************************************************************************
 */
uint8_t publish_teaMakeCnt(void)
{
    // �� cJSON ����JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *services = cJSON_AddArrayToObject(root, "services");
    cJSON *svc = cJSON_CreateObject();
    cJSON_AddItemToArray(services, svc);
    cJSON_AddStringToObject(svc, "service_id", "ser_DataReport");
    cJSON *props = cJSON_AddObjectToObject(svc, "properties");
    cJSON_AddNumberToObject(props, "Da01", record.data.coffeeMakeCnt + record.data.teaMakeCnt);
    cJSON_AddNumberToObject(props, "Da03", config.data.teaMake.time[tea.num]);
    cJSON_AddNumberToObject(props, "Da04", config.data.teaMake.weight[tea.num]);
    cJSON_AddNumberToObject(props, "Da05", config.data.teaMake.vol[tea.num]);
    cJSON_AddNumberToObject(props, "Da06", 1);
    cJSON_AddNumberToObject(props, "Da08", record.data.circulationValveCnt);
    cJSON_AddNumberToObject(props, "Da09", record.data.drainValveCnt);
    cJSON_AddNumberToObject(props, "Da10", record.data.circulationPumpRunTime);
    cJSON_AddNumberToObject(props, "Da11", record.data.waterInletValveCnt);
    cJSON_AddNumberToObject(props, "Da16", deviceRunState);

    // ���л�������
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // ֱ��ʹ���޸ĺ��MQTT_PublishStatus����
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}

/**
 * ************************************************************************
 * @brief �ӳٷ�������
 *
 * @param[in] type  ��������
 *
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-20
 *
 * ************************************************************************
 */
void delayedPublish(void *type)
{
    // ��ȫ�ؽ�void*ת����ö������
    publish_type_t publish_type = (publish_type_t)(uintptr_t)type;

    //    // ��֤ö��ֵ����Ч��Χ
    //    if (publish_type < PUBLISH_WASH_CNT || publish_type > PUBLISH_DRAIN_RELAT_DATA) {
    //        log_e("Invalid publish type: %d", (int)publish_type);
    //        return;
    //    }
    //
    switch (publish_type)
    {
    case PUBLISH_COFFEE_MAKE_CNT:
        if (publish_coffeeMakeCnt())
        {
            log_d("publish coffee make cnt");
        }
        else
        {
            log_e("publish coffee make cnt failed");
        }
        delete_task_with_param(delayedPublish, (void *)PUBLISH_COFFEE_MAKE_CNT);
        break;
    case PUBLISH_TEA_MAKE_CNT:
        if (publish_teaMakeCnt())
        {
            log_d("publish tea make cnt");
        }
        else
        {
            log_e("publish tea make cnt failed");
        }
        delete_task_with_param(delayedPublish, (void *)PUBLISH_TEA_MAKE_CNT);
        break;
    case PUBLISH_CIRCULATION_VALVE_CNT:
        if (publish_circulationValveCnt())
        {
            log_d("publish circulation valve cnt");
        }
        else
        {
            log_e("publish circulation valve cnt failed");
        }
        delete_task_with_param(delayedPublish, (void *)PUBLISH_CIRCULATION_VALVE_CNT);
        break;
    case PUBLISH_DRAIN_VALVE_CNT:
        if (publish_drainValveCnt())
        {
            log_d("publish drain valve cnt");
        }
        else
        {
            log_e("publish drain valve cnt failed");
        }
        delete_task_with_param(delayedPublish, (void *)PUBLISH_DRAIN_VALVE_CNT);
        break;
    case PUBLISH_DRAIN_RELAT_DATA:
        if (publish_drainRelatData())
        {
            log_d("publish drain relat data");
        }
        else
        {
            log_e("publish drain relat data failed");
        }
        delete_task_with_param(delayedPublish, (void *)PUBLISH_DRAIN_RELAT_DATA);
        break;
    case PUBLISH_WASH_CNT:
        if (publish_washCnt())
        {
            log_d("publish wash cnt");
        }
        else
        {
            log_e("publish wash cnt failed");
        }
        delete_task_with_param(delayedPublish, (void *)PUBLISH_WASH_CNT);
        break;
    case PUBLISH_DEVICE_RUN_STATE:
        if (publish_deviceRunState())
        {
            log_d("publish device run state");
        }
        else
        {
            log_e("publish device run state failed");
        }
        // delete_task_with_param(delayedPublish, (void*)PUBLISH_DEVICE_RUN_STATE);
        break;
    case PUBLISH_ALL_DATA:
        if (publish_allData())
        {
            log_d("publish all data");
        }
        else
        {
            log_e("publish all data failed");
        }
        delete_task_with_param(delayedPublish, (void *)(uintptr_t)PUBLISH_ALL_DATA);
        break;
    default:
        log_e("Unknown publish type: %d", publish_type);
        break;
    }
}


/**
 * ************************************************************************
 * @brief ����WIFI��MQTT�¼���Ϣ
 * 
 * ����ģ���ϱ����¼�֪ͨ
 * 
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-20
 * 
 * ************************************************************************
 */
void processEventMessage(const char* message)
{
   // log_d("Processing event: %s", message);
    
    if (strstr(message, "+EVENT:MQTT_CONNECT"))
    {
        log_i("MQTT connected event received");
        WIFI.mqttConnected = 1;
    }
    else if (strstr(message, "+EVENT:MQTT_DISCONNECT"))
    {
        log_i("MQTT disconnected event received");
        WIFI.mqttConnected = 0;
    }
    else if (strstr(message, "+EVENT:WIFI_CONNECT"))
    {
        log_i("WiFi connected event received");
        WIFI.isConnected = 1;
        SetControlVisiable(main_page, 7, 0, UART2_ID);  // ����δ����ͼ��
        SetControlVisiable(main_page, 8, 1, UART2_ID);  // ��ʾ������ͼ��

    }
    else if (strstr(message, "+EVENT:WIFI DISCONNECT") || 
             strstr(message, "+EVENT:WIFI_DISCONNECT"))
    {
        log_i("WiFi disconnected event received");
        WIFI.isConnected = 0;
        WIFI.mqttConnected = 0; // WiFi�Ͽ�ʱMQTTҲ��Ȼ�Ͽ�
        SetControlVisiable(main_page, 7, 1, UART2_ID);  
        SetControlVisiable(main_page, 8, 0, UART2_ID); 
    }
    // ����MQTT������Ϣ
    else if (strstr(message, "+EVENT:MQTT_SUB"))
    {
        processMQTTSubscriptionMessage(message);
    }
}

/**
 * ************************************************************************
 * @brief ����MQTT������Ϣ
 * 
 * ������ʽ: +EVENT:MQTT_SUB,<topic>,<length>,<json_payload>
 * ����: +EVENT:MQTT_SUB,$oc/devices/CB11_25050031/sys/messages/down,78,{"name":null,"id":"2b61808c-f1fa-445c-8878-71549a39c649","content":{"Ps01":1}}
 * 
 * @param[in] message ������Ϣ�ַ���
 * 
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-20
 * 
 * ************************************************************************
 */
void processMQTTSubscriptionMessage(const char* message)
{
    log_i("Processing MQTT subscription message: %s", message);
    
    // ��ȡ���⡢���Ⱥ�JSON����
    char topic[256] = {0};
    int length = 0;
    char* jsonStart = NULL;
    
    // ʹ��ָ�����������Ϣ��ʽ
    const char* prefix = "+EVENT:MQTT_SUB,";
    char* topicStart = strstr(message, prefix);
    
    if (!topicStart) {
        log_e("Invalid MQTT_SUB message format");
        return;
    }
    
    // �ƶ������⿪ʼλ��
    topicStart += strlen(prefix);
    
    // �����������λ��(����)
    char* topicEnd = strchr(topicStart, ',');
    if (!topicEnd) {
        log_e("Cannot find topic end in MQTT_SUB message");
        return;
    }
    
    // ��������
    int topicLen = topicEnd - topicStart;
    if (topicLen >= sizeof(topic)) topicLen = sizeof(topic) - 1;
    strncpy(topic, topicStart, topicLen);
    topic[topicLen] = '\0';
    
    // ��������
    char* lengthStart = topicEnd + 1;
    char* lengthEnd = strchr(lengthStart, ',');
    if (!lengthEnd) {
        log_e("Cannot find length end in MQTT_SUB message");
        return;
    }
    
    // ת������Ϊ����
    char lengthStr[16] = {0};
    int lengthStrLen = lengthEnd - lengthStart;
    if (lengthStrLen >= sizeof(lengthStr)) lengthStrLen = sizeof(lengthStr) - 1;
    strncpy(lengthStr, lengthStart, lengthStrLen);
    length = atoi(lengthStr);
    
    // ��ȡJSON����
    jsonStart = lengthEnd + 1;
    
    log_i("MQTT Subscription: Topic=%s, Length=%d", topic, length);
    log_i("JSON payload: %s", jsonStart);
    
    // ����ͬ���͵�����
    if (strstr(topic, "/sys/messages/down"))
    {
        // ����������Ϣ
        processMQTTDownlinkMessage(jsonStart);
        if(config.data.isLock == 0)
        {
            publish_unlockState();
        }
        else if(config.data.isLock == 1)
        {
            publish_lockState();
        }
    }
    else if (strstr(topic, "/sys/commands/"))
    {
        // ����������Ϣ
        processMQTTCommandMessage(jsonStart);
    }
    else
    {
        log_w("Unknown topic type: %s", topic);
    }
}

/**
 * ************************************************************************
 * @brief ����MQTT������Ϣ
 * 
 * ����ϵͳ������Ϣ��ͨ������������
 * 
 * @param[in] jsonPayload JSON����
 * 
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-20
 * 
 * ************************************************************************
 */
void processMQTTDownlinkMessage(const char* jsonPayload)
{
    log_i("Processing downlink message: %s", jsonPayload);
    
    // ʹ��cJSON����JSON
    cJSON *root = cJSON_Parse(jsonPayload);
    if (!root) {
        log_e("Failed to parse JSON payload");
        return;
    }
    // ��ȡ��ϢID
    cJSON *messageId = cJSON_GetObjectItem(root, "id");
    char idStr[64] = "unknown";
    if (messageId && cJSON_IsString(messageId) && messageId->valuestring) {
        strncpy(idStr, messageId->valuestring, sizeof(idStr) - 1);
        idStr[sizeof(idStr) - 1] = '\0';
        log_i("Message ID: %s", idStr);
    }
    // ��ȡ��Ϣ����
    cJSON *content = cJSON_GetObjectItem(root, "content");
    if (content) {
        // ����Ƿ������������
        cJSON *ps01 = cJSON_GetObjectItem(content, "Ps01");
        if (ps01 && cJSON_IsNumber(ps01)) {
            int value = ps01->valueint;
            log_i("Received command Ps01 = %d", value);
            // ����ִֵ����Ӧ����
            if (value == 1) {

                
                // TODO: �����Ӧ�Ĳ�������
                // �������״̬��־�����������ִ�в���
            }
        }
        cJSON *ps02 = cJSON_GetObjectItem(content, "Ps02");
        if (ps02 && cJSON_IsNumber(ps02)) {
            int value = ps02->valueint;
            log_i("Received command Ps02 = %d", value);
            // ����ִֵ����Ӧ����
            if (value == 1) {
                // ����: ֹͣ�豸
                config.data.isLock = 1;
                lockDevice();
                log_i("Executing command: lock Device");
                write_config_data(CONFIG_IS_LOCK);
            }
        }
        cJSON *ps03 = cJSON_GetObjectItem(content, "Ps03");
        if (ps03 && cJSON_IsNumber(ps03)) {
            int value = ps03->valueint;
            log_i("Received command Ps03 = %d", value);
            // ����ִֵ����Ӧ����
            if (value == 1) {
                config.data.isLock = 0;
                unlockDevice();
                log_i("Executing command: unlock Device");
                write_config_data(CONFIG_IS_LOCK);
            }
        }

        // ��������������ԵĴ����߼�
    }

    // �ͷ�JSON����
    cJSON_Delete(root);
}

/**
 * ************************************************************************
 * @brief ����MQTT������Ϣ
 * 
 * �������ƽ̨���͵�����
 * 
 * @param[in] jsonPayload JSON����
 * 
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-20
 * 
 * ************************************************************************
 */
void processMQTTCommandMessage(const char* jsonPayload)
{
    log_i("Processing command message: %s", jsonPayload);
    
    // ʹ��cJSON����JSON
    cJSON *root = cJSON_Parse(jsonPayload);
    if (!root) {
        log_e("Failed to parse JSON payload");
        return;
    }
    
    // ��ȡ����ID
    cJSON *commandId = cJSON_GetObjectItem(root, "id");
    if (commandId && cJSON_IsString(commandId)) {
        log_i("Command ID: %s", commandId->valuestring);
    }
    
    // ��ȡ��������
    cJSON *commandName = cJSON_GetObjectItem(root, "name");
    if (commandName && cJSON_IsString(commandName)) {
        log_i("Command Name: %s", commandName->valuestring);
    }
    
    // ��ȡ��������
    cJSON *commandContent = cJSON_GetObjectItem(root, "content");
    if (commandContent) {
        // ����������������������ֶ�ȡ��������Ӧ��
        // ����ֻ��һ��ʾ��
        log_i("Processing command parameters");
    }
    
    // �ͷ�JSON����
    cJSON_Delete(root);
}

/**
 * ************************************************************************
 * @brief �ش��յ�����Ϣ
 * 
 * ����MQTT��Ϣ�Ļص�����
 * 
 * @param[in] topic ����
 * @param[in] payload ��Ϣ����
 * @param[in] payloadLen ��Ϣ����
 * 
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-20
 * 
 * ************************************************************************
 */
uint8_t mqttSubscribeUp(const char *payload)
{
    int bufferFound = -1;
    
    // �������
    if (!WIFI.mqttConnected) {
        log_e("MQTT is not connected, cannot publish reply");
        return 0;
    }

    if (payload == NULL) {
        log_e("Payload is NULL");
        return 0;
    }

    // ��ȡ���س���
    size_t payloadLen = strlen(payload);
    if (payloadLen == 0 || payloadLen >= MAX_PAYLOAD_SIZE) {
        log_e("Payload length invalid: %d", payloadLen);
        return 0;
    }

    // // ���ҿ��û�����
    // for (int i = 0; i < PAYLOAD_BUFFER_COUNT; i++) {
    //     if (!payloadBuffers[i].inUse) {
    //         bufferFound = i;
    //         break;
    //     }
    // }
    // ʹ���º�����ȡ���û�����
     bufferFound = getNextAvailableBuffer();
    // ���л���������ʹ����
    if (bufferFound < 0) {
        log_e("No free payload buffer available");
        return 0;
    }

    // ����payload��������
    strncpy(payloadBuffers[bufferFound].buffer, payload, MAX_PAYLOAD_SIZE - 1);
    payloadBuffers[bufferFound].buffer[MAX_PAYLOAD_SIZE - 1] = '\0';
    payloadBuffers[bufferFound].inUse = 1;

    char upTopic[256] = {0};
    sprintf(upTopic, "$oc/devices/%s/sys/messages/up", deviceInfo.data.DeviceID);
    
    // ����MQTTPUBRAW����
    char atCommand[256] = {0};
    snprintf(atCommand, sizeof(atCommand), "AT+MQTTPUBRAW=\"%s\",1,0,%zu",
             upTopic, payloadLen);

    // ��������ṹ��
    AT_Command_TypeDef rawCommand;
    memset(&rawCommand, 0, sizeof(AT_Command_TypeDef));
    strncpy(rawCommand.command, atCommand, sizeof(rawCommand.command) - 1);
    rawCommand.timeoutMs = 10000; // �ϳ��ĳ�ʱʱ��
    rawCommand.callback = mqttPublishCallback;
    rawCommand.rawData = payloadBuffers[bufferFound].buffer; // ָ��̬������
    rawCommand.rawDataLen = payloadLen;
    rawCommand.needRawSend = 1;
    rawCommand.bufferIndex = bufferFound; // ���滺��������

    // ��������
    if (!AT_SendCommandRaw(&rawCommand)) {
        log_e("Failed to send AT command");
        payloadBuffers[bufferFound].inUse = 0; // ���Ϊδʹ��
        return 0;
    }
    
    return 1;
}

uint8_t publish_lockState(void)
{
    // �� cJSON ����JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *services = cJSON_AddArrayToObject(root, "services");
    cJSON *svc = cJSON_CreateObject();
    cJSON_AddItemToArray(services, svc);
    cJSON_AddStringToObject(svc, "service_id", "ser_DataReport");
    cJSON *props = cJSON_AddObjectToObject(svc, "properties");
    cJSON_AddNumberToObject(props, "Ps02", 1);
    cJSON_AddNumberToObject(props, "Ps03", 0);

    // ���л�������
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // ֱ��ʹ���޸ĺ��MQTT_PublishStatus����
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}

uint8_t publish_unlockState(void)
{
    // �� cJSON ����JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *services = cJSON_AddArrayToObject(root, "services");
    cJSON *svc = cJSON_CreateObject();
    cJSON_AddItemToArray(services, svc);
    cJSON_AddStringToObject(svc, "service_id", "ser_DataReport");
    cJSON *props = cJSON_AddObjectToObject(svc, "properties");
    cJSON_AddNumberToObject(props, "Ps02", 0);
    cJSON_AddNumberToObject(props, "Ps03", 1);

    // ���л�������
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // ֱ��ʹ���޸ĺ��MQTT_PublishStatus����
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}

