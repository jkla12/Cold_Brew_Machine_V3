/**
 *
 * @file debug.c
 * @author jiaokai
 * @brief  调试程序
 *
 * @copyright Copyright (c) 2025
 */
#include "debug.h"
#include "eeprom.h"
#include "uart.h"
#include "config.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "app_mqtt.h"
#include "elog.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "debug"

/**
 * ************************************************************************
 * @brief 处理上位机发送的配置命令
 *
 *
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-16
 * 10 ms执行一次
 * ************************************************************************
 */
void debugProcess(void)
{
    if (uart0_struct.rx_flag == 1)
    {
        uart0_struct.rx_flag = 0;
        /* ----------------- 第1步：基础帧校验 ---------------- */
        if (uart0_struct.rx_count < 6 || uart0_struct.rx_buf[0] != FRAME_HEADER_WRITE || uart0_struct.rx_buf[uart0_struct.rx_count - 1] != FRAME_END_WRITE) // 数据帧不完整
        {
            uart0_struct.rx_count = 0;
            return;
        }
        /* ----------------- 第2步：提取帧字段 ---------------- */
        uint8_t dataType = uart0_struct.rx_buf[1]; // 数据类型: 0x01-SN码, 0x02-硬件版本, 0x03-软件版本, 0x04-日期
        uint8_t opType = uart0_struct.rx_buf[2];   // 操作类型: 0x01-写, 0x02-读
        uint8_t dataLen = uart0_struct.rx_buf[3];  // 数据域长度
        if (4 + dataLen + 2 > uart0_struct.rx_count)
            return;

        /* ----------------- 第3步：校验和验证 ---------------- */
        uint8_t checksum = uart0_struct.rx_buf[4 + dataLen];
        if (checksum != calc_checksum(uart0_struct.rx_buf, 4 + dataLen))
            return;

        /* ----------------- 第4步：构造响应帧头 ---------------- */
        uint8_t tx[64] = {0}; // 响应缓冲区
        int pos = 0;
        tx[pos++] = FRAME_HEADER_READ; // 响应帧头
        tx[pos++] = dataType;          // 回显数据类型
        tx[pos++] = opType;            // 回显操作类型
        /* ----------------- 第五步：根据数据类型处理 ------------ */
        // SN码处理
        if (dataType == DATA_TYPE_SN)
        {
            if (opType == OPERATION_WRITE)
            {
                // 写入SN码到EEPROM
                if (dataLen >= SN_MAX_LEN) {
                    dataLen = SN_MAX_LEN - 1; // 防止溢出
                }
                char sn[SN_MAX_LEN];
                memcpy(sn, &uart0_struct.rx_buf[4], dataLen);
                sn[dataLen] = '\0';
                memcpy(deviceInfo.data.DeviceID, sn, dataLen+1);
                memcpy(deviceInfo.data.MQTT.ESN, sn, dataLen+1);
                initMQTTTopics();
                write_device_info_data(DEVICE_INFO_DEVICE_ID);
                write_device_info_data(DEVICE_INFO_MQTT);
                // 返回原始数据长度和数据内容
                tx[pos++] = dataLen;
                memcpy(&tx[pos], deviceInfo.data.DeviceID, dataLen);
                pos += dataLen;
            }
            else
            {
                // 从EEPROM读取SN码
                read_device_info_data(DEVICE_INFO_DEVICE_ID);
                uint8_t sn_len = strlen(deviceInfo.data.DeviceID);  // 实际有效长度
                tx[pos++] = sn_len;                                 // 数据长度字段
                memcpy(&tx[pos], deviceInfo.data.DeviceID, sn_len); // 数据内容
                pos += sn_len;
            }
        }
        // 硬件版本处理
        else if (dataType == DATA_TYPE_HW)
        {
            if (opType == OPERATION_WRITE)
            {
                // 写入数字版本号（如"10"）
                uint8_t ver_num = atoi((char *)&uart0_struct.rx_buf[4]);
                deviceInfo.data.HardwareVersion = ver_num;
                write_device_info_data(DEVICE_INFO_HARDWARE_VERSION);
                // 返回格式化字符串（如"V1.0"）
                char ver_str[16];
                snprintf(ver_str, sizeof(ver_str), "V%d.%d", ver_num / 10, ver_num % 10);
                tx[pos++] = strlen(ver_str);
                memcpy(&tx[pos], ver_str, strlen(ver_str));
                pos += strlen(ver_str);
            }
            else if (opType == OPERATION_READ)
            {
                read_device_info_data(DEVICE_INFO_HARDWARE_VERSION);
                uint8_t ver_num = deviceInfo.data.HardwareVersion;
                if (ver_num != 0xFF)
                {
                    char ver_str[16];
                    snprintf(ver_str, sizeof(ver_str), "V%d.%d", ver_num / 10, ver_num % 10);
                    tx[pos++] = strlen(ver_str);
                    memcpy(&tx[pos], ver_str, strlen(ver_str));
                    pos += strlen(ver_str);
                }
                else
                {
                    tx[pos++] = 1;
                    tx[pos++] = 0xFF; // 未设置标志
                }
            }
        }
        // 软件版本处理
        else if (dataType == DATA_TYPE_SW)
        {
            if (opType == OPERATION_WRITE)
            {
                // 写入数字版本号（如"10"）
                uint8_t ver_num = atoi((char *)&uart0_struct.rx_buf[4]);
                deviceInfo.data.FirmwareVersion = ver_num;
                write_device_info_data(DEVICE_INFO_FIRMWARE_VERSION);
                // 返回格式化字符串（如"V1.0"）
                char ver_str[16];
                snprintf(ver_str, sizeof(ver_str), "V%d.%d", ver_num / 10, ver_num % 10);
                tx[pos++] = strlen(ver_str);
                memcpy(&tx[pos], ver_str, strlen(ver_str));
                pos += strlen(ver_str);
            }
            else if (opType == OPERATION_READ)
            {
                read_device_info_data(DEVICE_INFO_FIRMWARE_VERSION);
                uint8_t ver_num = deviceInfo.data.FirmwareVersion;
                if (ver_num != 0xFF)
                {
                    char ver_str[16];
                    snprintf(ver_str, sizeof(ver_str), "V%d.%d", ver_num / 10, ver_num % 10);
                    tx[pos++] = strlen(ver_str);
                    memcpy(&tx[pos], ver_str, strlen(ver_str));
                    pos += strlen(ver_str);
                }
                else
                {
                    tx[pos++] = 1;
                    tx[pos++] = 0xFF; // 未设置标志
                }
            }
        }
        // 日期处理
        else if (dataType == DATA_TYPE_DATE)
        {
            if (opType == OPERATION_WRITE)
            {
                if (dataLen > DATE_LEN) {
                    dataLen = DATE_LEN; // 防止溢出
                }
                char date[DATE_LEN + 1] = {0};
                memcpy(date, &uart0_struct.rx_buf[4], dataLen);
                date[dataLen] = '\0';
                memcpy(deviceInfo.data.DateOfManufacture, date, dataLen + 1);
                write_device_info_data(DEVICE_INFO_DATE_OF_MANUFACTURE);
                tx[pos++] = dataLen;
                memcpy(&tx[pos], date, dataLen);
                pos += dataLen;
            }
            else if (opType == OPERATION_READ)
            {
                char date[DATE_LEN + 1] = {0};
                read_device_info_data(DEVICE_INFO_DATE_OF_MANUFACTURE);
                memcpy(date, deviceInfo.data.DateOfManufacture, DATE_LEN);
                date[DATE_LEN] = '\0'; // 确保字符串以'\0'结尾
                tx[pos++] = strlen(date);
                memcpy(&tx[pos], date, strlen(date));
                pos += strlen(date);
            }
        }
        // 确保不会溢出tx缓冲区
        if (pos + 2 > sizeof(tx)) {
            log_e("Response too large to fit in buffer");
            return;
        }
        // ----------------- 第5步：填充校验和及帧尾 -----------------
        tx[pos++] = calc_checksum(tx, pos);
        tx[pos++] = FRAME_END_READ;
        // 通过DMA发送响应
        uart0_dma_send(tx, pos);
    }
}

/**
 * ************************************************************************
 * @brief  计算校验和
 *
 * @param[in] buf  待计算校验和的缓冲区
 * @param[in] len  缓冲区长度
 *
 * @return 校验和
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-16
 *
 * ************************************************************************
 */
static uint8_t calc_checksum(const uint8_t *buf, uint16_t len)
{
    uint8_t sum = 0;
    for (uint16_t i = 0; i < len; i++)
        sum += buf[i];
    return sum;
}
