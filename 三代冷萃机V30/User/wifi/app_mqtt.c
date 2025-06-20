/**
 *
 * @file app_mqtt.c
 * @author jiaokai
 * @brief MQTT应用程序
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


// 获取下一个可用缓冲区索引
int getNextAvailableBuffer(void)
{
    // 先尝试从当前索引开始查找
    for (int i = 0; i < PAYLOAD_BUFFER_COUNT; i++) {
        int idx = (currentBufferIndex + i) % PAYLOAD_BUFFER_COUNT;
        if (!payloadBuffers[idx].inUse) {
            currentBufferIndex = (idx + 1) % PAYLOAD_BUFFER_COUNT;
            return idx;
        }
    }
    return -1; // 没有可用缓冲区
}

/**
 * ************************************************************************
 * @brief MQTT配置函数
 *
 *
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-15
 *  10秒运行一次
 * ************************************************************************
 */
void mqttConfig(void)
{
    // 每10秒检查一次WIFI状态
    if (deviceInfo.data.MQTT.isRegistered != 1) // 设备未注册
    {
        // 检查WIFI模块是否已初始化
        if (WIFI.isInitialized)
        {
            // WIFI已连接，可以执行设备注册
            if (WIFI.isConnected)
            {
                // 如果设备未注册，则进行注册
                log_d("device not registered, start register");
                if (deviceInfo.data.MQTT.isRegistered != 1)
                {
                    registerDevice();
                }
            }
            // WIFI未连接，尝试连接
            else
            {
                // wifiConnectionConfiguration();
            }
        }
    }
    else // 设备已注册
    {

        // 添加MQTT连接任务
        if (WIFI.runWIFIConfigEnd == 1)
        {
            add_task(MQTT_Connect, NULL, 1000, true);
            log_i("devcice registered");
            delete_task(mqttConfig); // 删除注册任务
        }
    }
}

/**
 * ************************************************************************
 * @brief 设备注册到服务器
 *
 * @return 是否成功发送注册请求
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
        log_e("WIFI未初始化或未连接,无法注册设备");
        return false;
    }

    // // 构建设备信息JSON数据
    // char jsonData[128];
    // snprintf(jsonData, sizeof(jsonData),
    //         "{\\\"ETID\\\":\\\"%s\\\",\\\"DV\\\":\\\"%d.%d\\\",\\\"ESN\\\":\\\"%s\\\"}",
    //         deviceInfo.data.MQTT.ETID,
    //         deviceInfo.data.FirmwareVersion/10,deviceInfo.data.FirmwareVersion%10,
    //         deviceInfo.data.MQTT.ESN);
    // log_d("jsonData: %s", jsonData);
    // 构建HTTP POST请求命令

    char atCommand[256];
    float firmwareVersion = deviceInfo.data.FirmwareVersion / 10.0f;
    memset(atCommand, 0, sizeof(atCommand));
    snprintf(atCommand, sizeof(atCommand),
             "AT+HTTPCLIENTLINE=1,3,application/x-www-form-urlencoded,www.sparkinger.com,30020,/api/Equipment/AddDevice?ETID=%s&DV=%.1f&ESN=%s",
             deviceInfo.data.MQTT.ETID,
             firmwareVersion,
             deviceInfo.data.MQTT.ESN);

    // 发送HTTP请求
    return AT_SendCommand(atCommand, 15000, registerDeviceCallback);
}

/**
 * ************************************************************************
 * @brief 设备注册回调函数 - 处理响应
 *
 * @param[in] success AT命令是否执行成功
 * @param[in] response 服务器响应内容
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
        log_e("设备注册请求失败,网络或AT命令错误");
        return;
    }

    log_d("注册响应原始数据: %s", response);

    // // 调试 - 显示原始响应的十六进制内容
    // log_d("响应十六进制表示:");
    // for (size_t i = 0; i < strlen(response) && i < 100; i++) {
    //     log_d("  byte[%d]: 0x%02X (%c)", i, (unsigned char)response[i],
    //           (response[i] >= 32 && response[i] < 127) ? response[i] : '.');
    // }

    // 在AT响应中查找JSON响应体 - 更严格的查找
    // 先跳过可能的长度前缀（十六进制数和换行符）
    const char *potentialStart = response;
    while (*potentialStart && !(*potentialStart == '{' ||
                                (*potentialStart >= 'a' && *potentialStart <= 'f') ||
                                (*potentialStart >= '0' && *potentialStart <= '9')))
    {
        potentialStart++;
    }

    // 检查是否有十六进制长度前缀
    const char *jsonStart = strchr(potentialStart, '{');
    if (!jsonStart)
    {
        log_e("无法找到JSON响应开始位置 '{'");
        return;
    }

    // 找到JSON结束位置
    const char *jsonEnd = strrchr(jsonStart, '}');
    if (!jsonEnd)
    {
        log_e("无法找到JSON结束位置 '}'");
        return;
    }

    // 提取完整JSON
    size_t jsonLen = jsonEnd - jsonStart + 1;
    char *jsonData = (char *)malloc(jsonLen + 1);
    if (!jsonData)
    {
        log_e("内存分配失败, 请求大小: %d bytes", (int)jsonLen + 1);
        return;
    }

    // 复制JSON并添加终止符
    memcpy(jsonData, jsonStart, jsonLen);
    jsonData[jsonLen] = '\0';

    log_d("提取的JSON (长度 %d): %s", (int)jsonLen, jsonData);

    // 尝试解析JSON
    cJSON *root = cJSON_Parse(jsonData);
    if (!root)
    {
        // 解析失败，获取错误信息
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr)
        {
            log_e("cJSON解析错误发生在: %.20s", error_ptr);
        }
        else
        {
            log_e("未知cJSON解析错误");
        }

        // 尝试清理可能包含的不可见字符
        for (size_t i = 0; i < jsonLen; i++)
        {
            if ((unsigned char)jsonData[i] < 32 && jsonData[i] != '\t' && jsonData[i] != '\n' && jsonData[i] != '\r')
            {
                jsonData[i] = ' '; // 替换为空格
            }
        }

        // 重试解析
        log_d("清理后重试解析: %s", jsonData);
        root = cJSON_Parse(jsonData);
        if (!root)
        {
            log_e("第二次解析失败，放弃");
            free(jsonData);
            return;
        }
    }

    // 成功解析JSON
    log_d("cJSON解析成功");

    // 获取code字段
    cJSON *codeItem = cJSON_GetObjectItem(root, "code");
    if (!codeItem || !cJSON_IsString(codeItem))
    {
        log_e("无法找到有效的code字段");
        cJSON_Delete(root);
        free(jsonData);
        return;
    }

    const char *code = codeItem->valuestring;
    log_d("解析到code值: %s", code);

    // 处理code
    if (strcmp(code, "0") == 0 || strcmp(code, "401") == 0)
    {
        // 成功或设备已注册
        const char *status = strcmp(code, "401") == 0 ? "已存在" : "成功";
        log_i("设备注册%s", status);

        // 提取data对象
        cJSON *data = cJSON_GetObjectItem(root, "data");
        if (data)
        {
            // 提取连接信息
            cJSON *clientID = cJSON_GetObjectItem(data, "clientID");
            cJSON *userName = cJSON_GetObjectItem(data, "userName");
            cJSON *password = cJSON_GetObjectItem(data, "password");

            // 检查字段是否存在且是字符串类型
            if (clientID && cJSON_IsString(clientID) &&
                userName && cJSON_IsString(userName) &&
                password && cJSON_IsString(password))
            {

                // 安全拷贝并确保字符串终止
                memset(deviceInfo.data.MQTT.ClientID, 0, sizeof(deviceInfo.data.MQTT.ClientID));
                memset(deviceInfo.data.MQTT.UserName, 0, sizeof(deviceInfo.data.MQTT.UserName));
                memset(deviceInfo.data.MQTT.Password, 0, sizeof(deviceInfo.data.MQTT.Password));

                strncpy(deviceInfo.data.MQTT.ClientID, clientID->valuestring,
                        sizeof(deviceInfo.data.MQTT.ClientID) - 1);
                strncpy(deviceInfo.data.MQTT.UserName, userName->valuestring,
                        sizeof(deviceInfo.data.MQTT.UserName) - 1);
                strncpy(deviceInfo.data.MQTT.Password, password->valuestring,
                        sizeof(deviceInfo.data.MQTT.Password) - 1);

                // 调试输出
                log_d("保存的ClientID: %s", deviceInfo.data.MQTT.ClientID);
                log_d("保存的UserName: %s", deviceInfo.data.MQTT.UserName);
                log_d("保存的Password: %s", deviceInfo.data.MQTT.Password);

                // 设置注册标志
                deviceInfo.data.MQTT.isRegistered = 1;

                // 保存到EEPROM
                eeprom_buffer_write_timeout(deviceInfo.bytes, DEVICE_INFO_ADDRESS, sizeof(deviceInfo_TypeDef));
                log_i("MQTT连接信息已保存到EEPROM");
            }
            else
            {
                log_e("JSON数据中缺少必要的MQTT连接字段或字段类型错误");
            }
        }
        else
        {
            log_e("JSON中缺少data对象");
        }
    }
    else
    {
        log_e("设备注册失败，错误码: %s", code);
    }

    // 清理资源
    cJSON_Delete(root);
    free(jsonData);
}

/**
 * ************************************************************************
 * @brief 从JSON字符串中提取指定字段值
 *
 * @param[in] json JSON字符串
 * @param[in] field 字段名
 * @param[out] value 提取到的值
 * @param[in] maxLen 值缓冲区最大长度
 *
 * @return 是否成功提取
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
        // 尝试查找非字符串类型字段
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
    // 提取字段值，直到遇到引号或逗号或结束括号
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
 * @brief 从JSON嵌套对象中提取指定字段值
 *
 * @param[in] json JSON字符串
 * @param[in] object 对象名
 * @param[in] field 字段名
 * @param[out] value 提取到的值
 * @param[in] maxLen 值缓冲区最大长度
 *
 * @return 是否成功提取
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

    // 计算对象结束位置
    objectStart += strlen(objectPattern);

    // 在对象内查找字段
    char fieldPattern[32];
    snprintf(fieldPattern, sizeof(fieldPattern), "\"%s\":\"", field);

    const char *fieldStart = strstr(objectStart, fieldPattern);
    if (!fieldStart)
    {
        return false;
    }

    fieldStart += strlen(fieldPattern);

    uint16_t i = 0;
    // 提取字段值，直到遇到引号
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
 * @brief 配置并连接到MQTT服务器
 *
 * 按照BW16模组AT指令配置MQTT参数并建立连接
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
        // 设置MQTT服务器地址

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
        // 设置MQTT端口号
        memset(atCommand, 0, sizeof(atCommand));
        snprintf(atCommand, sizeof(atCommand), "AT+MQTT=2,%d", MQTT_PORT); // 默认MQTT端口
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
        // 设置MQTT连接方式（默认TCP）
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
        // 设置MQTT客户端ID
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
        // 设置MQTT用户名
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
        // 设置MQTT密码
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
        // 配置MQTT遗嘱（可选）
        memset(atCommand, 0, sizeof(atCommand));
        // 示例：设置遗嘱，主题为设备ID，QoS为0，不保留，消息为"offline"
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
        // 最后，连接到MQTT服务器
        log_i("开始连接MQTT服务器...");
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
        // 等待MQTT连接成功
        if (WIFI.mqttConnected == 1)
        {
            log_i("MQTT connected successfully");
            step++;
        }
        else if (WIFI.mqttConnected == 0)
        {
            step = 8; // 重置步骤
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
        if (WIFI.mqttConnected == 1) // 配置发布
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
        delete_task(MQTT_Connect); // 删除MQTT连接任务
        log_i("delete MQTT connect task");
        break;
    default:
        break;
    }
}

/**
 * ************************************************************************
 * @brief MQTT参数配置回调函数
 *
 * 处理MQTT参数配置命令的响应
 *
 * @param[in] success AT命令是否执行成功
 * @param[in] response 响应字符串
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
 * @brief MQTT连接回调函数
 *
 * 处理MQTT连接命令的响应
 *
 * @param[in] success AT命令是否执行成功
 * @param[in] response 响应字符串
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

        // 注意：此时只表示AT命令发送成功，实际连接可能还在进行中
        // 需要通过AT_Update函数监听"+EVENT:MQTT_CONNECT"事件来确认连接成功

        // 初始化MQTT连接监听
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
 * @brief MQTT主题订阅
 *
 * 订阅设备相关的主题
 *
 * @return 是否成功发送请求
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
 * @brief MQTT订阅回调函数
 *
 * @param[in] success AT命令是否执行成功
 * @param[in] response 响应字符串
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
 * @brief MQTT查询配置回调函数
 *
 * 处理MQTT查询命令的响应，解析MQTT状态和配置参数
 *
 * @param[in] success AT命令是否执行成功
 * @param[in] response 响应字符串
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

    // 查找响应前缀 "+MQTT:"
    const char *mqttInfo = strstr(response, "+MQTT:");
    if (!mqttInfo)
    {
        log_e("Invalid MQTT query response format");
        return;
    }

    // 跳过 "+MQTT:" 前缀
    mqttInfo += 6;

    // 解析MQTT状态和参数
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

    // 使用sscanf解析MQTT信息
    // 格式: <MQTT_status>,<Host_name>,<Port>,<scheme>,<client_id>,<username>,<password>,<LWT_topic>,<LWT_qos>,<LWT_Retained>,<LWTpayload>
    int parsed = sscanf(mqttInfo, "%d,%63[^,],%d,%d,%63[^,],%63[^,],%63[^,],%63[^,],%d,%d,%127[^\r\n]",
                        &mqtt_status, host_name, &port, &scheme, client_id, username, password,
                        lwt_topic, &lwt_qos, &lwt_retained, lwt_payload);

    if (parsed < 4)
    {
        log_e("Failed to parse MQTT query response, only %d fields parsed", parsed);
        return;
    }

    // 更新MQTT连接状态
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
        // 已连接但尚未完成订阅
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

    // 输出详细的MQTT配置信息
    log_d("MQTT Configuration:");
    log_d("  Host: %s", host_name);
    log_d("  Port: %d", port);
    log_d("  Scheme: %d", scheme);
    log_d("  Client ID: %s", client_id);
    log_d("  Username: %s", username);
}

/**
 * ************************************************************************
 * @brief 初始化MQTT主题，将DeviceID拼接到主题中
 *
 * 将MQTT_PUBTOPIC和MQTT_SUBTOPIC中的%s替换为设备ID
 *
 * @return 无
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

    // 清空现有的主题字符串
    memset(deviceInfo.data.MQTT.pubTopic, 0, sizeof(deviceInfo.data.MQTT.pubTopic));
    memset(deviceInfo.data.MQTT.subTopic, 0, sizeof(deviceInfo.data.MQTT.subTopic));

    // 使用snprintf将DeviceID填充到发布主题中
    snprintf(tempBuffer, sizeof(tempBuffer), MQTT_PUBTOPIC, deviceInfo.data.DeviceID);

    // 安全地将格式化后的字符串复制到目标变量
    strncpy(deviceInfo.data.MQTT.pubTopic, tempBuffer, sizeof(deviceInfo.data.MQTT.pubTopic) - 1);
    deviceInfo.data.MQTT.pubTopic[sizeof(deviceInfo.data.MQTT.pubTopic) - 1] = '\0'; // 确保有终止符

    // 使用snprintf将DeviceID填充到订阅主题中
    memset(tempBuffer, 0, sizeof(tempBuffer)); // 清空临时缓冲区
    snprintf(tempBuffer, sizeof(tempBuffer), MQTT_SUBTOPIC, deviceInfo.data.DeviceID);

    // 安全地将格式化后的字符串复制到目标变量
    strncpy(deviceInfo.data.MQTT.subTopic, tempBuffer, sizeof(deviceInfo.data.MQTT.subTopic) - 1);
    deviceInfo.data.MQTT.subTopic[sizeof(deviceInfo.data.MQTT.subTopic) - 1] = '\0'; // 确保有终止符

    // 打印日志以验证主题格式化是否正确
    log_i("MQTT pubTopic: %s", deviceInfo.data.MQTT.pubTopic);
    log_i("MQTT subTopic: %s", deviceInfo.data.MQTT.subTopic);
}

/**
 * ************************************************************************
 * @brief  发布循环阀运行次数
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
    // 用 cJSON 构造JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *services = cJSON_AddArrayToObject(root, "services");
    cJSON *svc = cJSON_CreateObject();
    cJSON_AddItemToArray(services, svc);
    cJSON_AddStringToObject(svc, "service_id", "ser_DataReport");
    cJSON *props = cJSON_AddObjectToObject(svc, "properties");
    cJSON_AddNumberToObject(props, "Da08", record.data.circulationValveCnt);

    // 序列化并发布
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // 直接使用修改后的MQTT_PublishStatus发布
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}

/**
 * ************************************************************************
 * @brief  发布排水阀运行次数
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
    // 用 cJSON 构造JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *services = cJSON_AddArrayToObject(root, "services");
    cJSON *svc = cJSON_CreateObject();
    cJSON_AddItemToArray(services, svc);
    cJSON_AddStringToObject(svc, "service_id", "ser_DataReport");
    cJSON *props = cJSON_AddObjectToObject(svc, "properties");
    cJSON_AddNumberToObject(props, "Da09", record.data.drainValveCnt);

    // 序列化并发布
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // 直接使用修改后的MQTT_PublishStatus发布
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}
/**
 * ************************************************************************
 * @brief  发布所有数据,开机发布一次
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
    // 用 cJSON 构造JSON
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
    // 序列化并发布
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // 直接使用修改后的MQTT_PublishStatus发布
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}
/**
 * ************************************************************************
 * @brief 发布设备运行状态
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
    // 用 cJSON 构造JSON
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
    // 序列化并发布
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // 直接使用修改后的MQTT_PublishStatus发布
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}

/**
 * ************************************************************************
 * @brief 上报清洗次数
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
    // 用 cJSON 构造JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *services = cJSON_AddArrayToObject(root, "services");
    cJSON *svc = cJSON_CreateObject();
    cJSON_AddItemToArray(services, svc);
    cJSON_AddStringToObject(svc, "service_id", "ser_DataReport");
    cJSON *props = cJSON_AddObjectToObject(svc, "properties");
    cJSON_AddNumberToObject(props, "Da02", record.data.washCnt);
    cJSON_AddNumberToObject(props, "Da16", deviceRunState);

    // 序列化并发布
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // 直接使用修改后的MQTT_PublishStatus发布
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}
/**
 * ************************************************************************
 * @brief 发布排水相关数据
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
    // 用 cJSON 构造JSON
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

    // 序列化并发布
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // 直接使用修改后的MQTT_PublishStatus发布
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}

/**
 * ************************************************************************
 * @brief 发布咖啡制作次数
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
    // 用 cJSON 构造JSON
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
    // 序列化并发布
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // 直接使用修改后的MQTT_PublishStatus发布
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}

/**
 * ************************************************************************
 * @brief 发布茶制作次数
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
    // 用 cJSON 构造JSON
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

    // 序列化并发布
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // 直接使用修改后的MQTT_PublishStatus发布
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}

/**
 * ************************************************************************
 * @brief 延迟发布函数
 *
 * @param[in] type  发布类型
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
    // 安全地将void*转换回枚举类型
    publish_type_t publish_type = (publish_type_t)(uintptr_t)type;

    //    // 验证枚举值的有效范围
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
 * @brief 处理WIFI和MQTT事件消息
 * 
 * 处理模组上报的事件通知
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
        SetControlVisiable(main_page, 7, 0, UART2_ID);  // 隐藏未连接图标
        SetControlVisiable(main_page, 8, 1, UART2_ID);  // 显示已连接图标

    }
    else if (strstr(message, "+EVENT:WIFI DISCONNECT") || 
             strstr(message, "+EVENT:WIFI_DISCONNECT"))
    {
        log_i("WiFi disconnected event received");
        WIFI.isConnected = 0;
        WIFI.mqttConnected = 0; // WiFi断开时MQTT也必然断开
        SetControlVisiable(main_page, 7, 1, UART2_ID);  
        SetControlVisiable(main_page, 8, 0, UART2_ID); 
    }
    // 处理MQTT订阅消息
    else if (strstr(message, "+EVENT:MQTT_SUB"))
    {
        processMQTTSubscriptionMessage(message);
    }
}

/**
 * ************************************************************************
 * @brief 处理MQTT订阅消息
 * 
 * 解析格式: +EVENT:MQTT_SUB,<topic>,<length>,<json_payload>
 * 例如: +EVENT:MQTT_SUB,$oc/devices/CB11_25050031/sys/messages/down,78,{"name":null,"id":"2b61808c-f1fa-445c-8878-71549a39c649","content":{"Ps01":1}}
 * 
 * @param[in] message 订阅消息字符串
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
    
    // 提取主题、长度和JSON负载
    char topic[256] = {0};
    int length = 0;
    char* jsonStart = NULL;
    
    // 使用指针操作分析消息格式
    const char* prefix = "+EVENT:MQTT_SUB,";
    char* topicStart = strstr(message, prefix);
    
    if (!topicStart) {
        log_e("Invalid MQTT_SUB message format");
        return;
    }
    
    // 移动到主题开始位置
    topicStart += strlen(prefix);
    
    // 查找主题结束位置(逗号)
    char* topicEnd = strchr(topicStart, ',');
    if (!topicEnd) {
        log_e("Cannot find topic end in MQTT_SUB message");
        return;
    }
    
    // 复制主题
    int topicLen = topicEnd - topicStart;
    if (topicLen >= sizeof(topic)) topicLen = sizeof(topic) - 1;
    strncpy(topic, topicStart, topicLen);
    topic[topicLen] = '\0';
    
    // 解析长度
    char* lengthStart = topicEnd + 1;
    char* lengthEnd = strchr(lengthStart, ',');
    if (!lengthEnd) {
        log_e("Cannot find length end in MQTT_SUB message");
        return;
    }
    
    // 转换长度为整数
    char lengthStr[16] = {0};
    int lengthStrLen = lengthEnd - lengthStart;
    if (lengthStrLen >= sizeof(lengthStr)) lengthStrLen = sizeof(lengthStr) - 1;
    strncpy(lengthStr, lengthStart, lengthStrLen);
    length = atoi(lengthStr);
    
    // 获取JSON负载
    jsonStart = lengthEnd + 1;
    
    log_i("MQTT Subscription: Topic=%s, Length=%d", topic, length);
    log_i("JSON payload: %s", jsonStart);
    
    // 处理不同类型的主题
    if (strstr(topic, "/sys/messages/down"))
    {
        // 处理下行消息
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
        // 处理命令消息
        processMQTTCommandMessage(jsonStart);
    }
    else
    {
        log_w("Unknown topic type: %s", topic);
    }
}

/**
 * ************************************************************************
 * @brief 处理MQTT下行消息
 * 
 * 处理系统下行消息，通常是属性设置
 * 
 * @param[in] jsonPayload JSON负载
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
    
    // 使用cJSON解析JSON
    cJSON *root = cJSON_Parse(jsonPayload);
    if (!root) {
        log_e("Failed to parse JSON payload");
        return;
    }
    // 提取消息ID
    cJSON *messageId = cJSON_GetObjectItem(root, "id");
    char idStr[64] = "unknown";
    if (messageId && cJSON_IsString(messageId) && messageId->valuestring) {
        strncpy(idStr, messageId->valuestring, sizeof(idStr) - 1);
        idStr[sizeof(idStr) - 1] = '\0';
        log_i("Message ID: %s", idStr);
    }
    // 获取消息内容
    cJSON *content = cJSON_GetObjectItem(root, "content");
    if (content) {
        // 检查是否包含属性设置
        cJSON *ps01 = cJSON_GetObjectItem(content, "Ps01");
        if (ps01 && cJSON_IsNumber(ps01)) {
            int value = ps01->valueint;
            log_i("Received command Ps01 = %d", value);
            // 根据值执行相应动作
            if (value == 1) {

                
                // TODO: 添加相应的操作代码
                // 可以添加状态标志或命令队列来执行操作
            }
        }
        cJSON *ps02 = cJSON_GetObjectItem(content, "Ps02");
        if (ps02 && cJSON_IsNumber(ps02)) {
            int value = ps02->valueint;
            log_i("Received command Ps02 = %d", value);
            // 根据值执行相应动作
            if (value == 1) {
                // 例如: 停止设备
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
            // 根据值执行相应动作
            if (value == 1) {
                config.data.isLock = 0;
                unlockDevice();
                log_i("Executing command: unlock Device");
                write_config_data(CONFIG_IS_LOCK);
            }
        }

        // 可以添加其他属性的处理逻辑
    }

    // 释放JSON对象
    cJSON_Delete(root);
}

/**
 * ************************************************************************
 * @brief 处理MQTT命令消息
 * 
 * 处理从云平台发送的命令
 * 
 * @param[in] jsonPayload JSON负载
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
    
    // 使用cJSON解析JSON
    cJSON *root = cJSON_Parse(jsonPayload);
    if (!root) {
        log_e("Failed to parse JSON payload");
        return;
    }
    
    // 获取命令ID
    cJSON *commandId = cJSON_GetObjectItem(root, "id");
    if (commandId && cJSON_IsString(commandId)) {
        log_i("Command ID: %s", commandId->valuestring);
    }
    
    // 获取命令名称
    cJSON *commandName = cJSON_GetObjectItem(root, "name");
    if (commandName && cJSON_IsString(commandName)) {
        log_i("Command Name: %s", commandName->valuestring);
    }
    
    // 获取命令内容
    cJSON *commandContent = cJSON_GetObjectItem(root, "content");
    if (commandContent) {
        // 解析具体命令参数，具体字段取决于您的应用
        // 这里只是一个示例
        log_i("Processing command parameters");
    }
    
    // 释放JSON对象
    cJSON_Delete(root);
}

/**
 * ************************************************************************
 * @brief 回传收到的消息
 * 
 * 处理MQTT消息的回调函数
 * 
 * @param[in] topic 主题
 * @param[in] payload 消息负载
 * @param[in] payloadLen 消息长度
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
    
    // 参数检查
    if (!WIFI.mqttConnected) {
        log_e("MQTT is not connected, cannot publish reply");
        return 0;
    }

    if (payload == NULL) {
        log_e("Payload is NULL");
        return 0;
    }

    // 获取负载长度
    size_t payloadLen = strlen(payload);
    if (payloadLen == 0 || payloadLen >= MAX_PAYLOAD_SIZE) {
        log_e("Payload length invalid: %d", payloadLen);
        return 0;
    }

    // // 查找可用缓冲区
    // for (int i = 0; i < PAYLOAD_BUFFER_COUNT; i++) {
    //     if (!payloadBuffers[i].inUse) {
    //         bufferFound = i;
    //         break;
    //     }
    // }
    // 使用新函数获取可用缓冲区
     bufferFound = getNextAvailableBuffer();
    // 所有缓冲区都在使用中
    if (bufferFound < 0) {
        log_e("No free payload buffer available");
        return 0;
    }

    // 复制payload到缓冲区
    strncpy(payloadBuffers[bufferFound].buffer, payload, MAX_PAYLOAD_SIZE - 1);
    payloadBuffers[bufferFound].buffer[MAX_PAYLOAD_SIZE - 1] = '\0';
    payloadBuffers[bufferFound].inUse = 1;

    char upTopic[256] = {0};
    sprintf(upTopic, "$oc/devices/%s/sys/messages/up", deviceInfo.data.DeviceID);
    
    // 构建MQTTPUBRAW命令
    char atCommand[256] = {0};
    snprintf(atCommand, sizeof(atCommand), "AT+MQTTPUBRAW=\"%s\",1,0,%zu",
             upTopic, payloadLen);

    // 创建命令结构体
    AT_Command_TypeDef rawCommand;
    memset(&rawCommand, 0, sizeof(AT_Command_TypeDef));
    strncpy(rawCommand.command, atCommand, sizeof(rawCommand.command) - 1);
    rawCommand.timeoutMs = 10000; // 较长的超时时间
    rawCommand.callback = mqttPublishCallback;
    rawCommand.rawData = payloadBuffers[bufferFound].buffer; // 指向静态缓冲区
    rawCommand.rawDataLen = payloadLen;
    rawCommand.needRawSend = 1;
    rawCommand.bufferIndex = bufferFound; // 保存缓冲区索引

    // 发送命令
    if (!AT_SendCommandRaw(&rawCommand)) {
        log_e("Failed to send AT command");
        payloadBuffers[bufferFound].inUse = 0; // 标记为未使用
        return 0;
    }
    
    return 1;
}

uint8_t publish_lockState(void)
{
    // 用 cJSON 构造JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *services = cJSON_AddArrayToObject(root, "services");
    cJSON *svc = cJSON_CreateObject();
    cJSON_AddItemToArray(services, svc);
    cJSON_AddStringToObject(svc, "service_id", "ser_DataReport");
    cJSON *props = cJSON_AddObjectToObject(svc, "properties");
    cJSON_AddNumberToObject(props, "Ps02", 1);
    cJSON_AddNumberToObject(props, "Ps03", 0);

    // 序列化并发布
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // 直接使用修改后的MQTT_PublishStatus发布
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}

uint8_t publish_unlockState(void)
{
    // 用 cJSON 构造JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *services = cJSON_AddArrayToObject(root, "services");
    cJSON *svc = cJSON_CreateObject();
    cJSON_AddItemToArray(services, svc);
    cJSON_AddStringToObject(svc, "service_id", "ser_DataReport");
    cJSON *props = cJSON_AddObjectToObject(svc, "properties");
    cJSON_AddNumberToObject(props, "Ps02", 0);
    cJSON_AddNumberToObject(props, "Ps03", 1);

    // 序列化并发布
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!payload)
        return 0;

    // 直接使用修改后的MQTT_PublishStatus发布
    uint8_t result = MQTT_PublishStatus(payload);
    free(payload);
    return result;
}

