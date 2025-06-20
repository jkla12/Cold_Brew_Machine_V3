/**
 *
 * @file debug.c
 * @author jiaokai
 * @brief  ���Գ���
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
 * @brief ������λ�����͵���������
 *
 *
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-16
 * 10 msִ��һ��
 * ************************************************************************
 */
void debugProcess(void)
{
    if (uart0_struct.rx_flag == 1)
    {
        uart0_struct.rx_flag = 0;
        /* ----------------- ��1��������֡У�� ---------------- */
        if (uart0_struct.rx_count < 6 || uart0_struct.rx_buf[0] != FRAME_HEADER_WRITE || uart0_struct.rx_buf[uart0_struct.rx_count - 1] != FRAME_END_WRITE) // ����֡������
        {
            uart0_struct.rx_count = 0;
            return;
        }
        /* ----------------- ��2������ȡ֡�ֶ� ---------------- */
        uint8_t dataType = uart0_struct.rx_buf[1]; // ��������: 0x01-SN��, 0x02-Ӳ���汾, 0x03-����汾, 0x04-����
        uint8_t opType = uart0_struct.rx_buf[2];   // ��������: 0x01-д, 0x02-��
        uint8_t dataLen = uart0_struct.rx_buf[3];  // �����򳤶�
        if (4 + dataLen + 2 > uart0_struct.rx_count)
            return;

        /* ----------------- ��3����У�����֤ ---------------- */
        uint8_t checksum = uart0_struct.rx_buf[4 + dataLen];
        if (checksum != calc_checksum(uart0_struct.rx_buf, 4 + dataLen))
            return;

        /* ----------------- ��4����������Ӧ֡ͷ ---------------- */
        uint8_t tx[64] = {0}; // ��Ӧ������
        int pos = 0;
        tx[pos++] = FRAME_HEADER_READ; // ��Ӧ֡ͷ
        tx[pos++] = dataType;          // ������������
        tx[pos++] = opType;            // ���Բ�������
        /* ----------------- ���岽�������������ʹ��� ------------ */
        // SN�봦��
        if (dataType == DATA_TYPE_SN)
        {
            if (opType == OPERATION_WRITE)
            {
                // д��SN�뵽EEPROM
                if (dataLen >= SN_MAX_LEN) {
                    dataLen = SN_MAX_LEN - 1; // ��ֹ���
                }
                char sn[SN_MAX_LEN];
                memcpy(sn, &uart0_struct.rx_buf[4], dataLen);
                sn[dataLen] = '\0';
                memcpy(deviceInfo.data.DeviceID, sn, dataLen+1);
                memcpy(deviceInfo.data.MQTT.ESN, sn, dataLen+1);
                initMQTTTopics();
                write_device_info_data(DEVICE_INFO_DEVICE_ID);
                write_device_info_data(DEVICE_INFO_MQTT);
                // ����ԭʼ���ݳ��Ⱥ���������
                tx[pos++] = dataLen;
                memcpy(&tx[pos], deviceInfo.data.DeviceID, dataLen);
                pos += dataLen;
            }
            else
            {
                // ��EEPROM��ȡSN��
                read_device_info_data(DEVICE_INFO_DEVICE_ID);
                uint8_t sn_len = strlen(deviceInfo.data.DeviceID);  // ʵ����Ч����
                tx[pos++] = sn_len;                                 // ���ݳ����ֶ�
                memcpy(&tx[pos], deviceInfo.data.DeviceID, sn_len); // ��������
                pos += sn_len;
            }
        }
        // Ӳ���汾����
        else if (dataType == DATA_TYPE_HW)
        {
            if (opType == OPERATION_WRITE)
            {
                // д�����ְ汾�ţ���"10"��
                uint8_t ver_num = atoi((char *)&uart0_struct.rx_buf[4]);
                deviceInfo.data.HardwareVersion = ver_num;
                write_device_info_data(DEVICE_INFO_HARDWARE_VERSION);
                // ���ظ�ʽ���ַ�������"V1.0"��
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
                    tx[pos++] = 0xFF; // δ���ñ�־
                }
            }
        }
        // ����汾����
        else if (dataType == DATA_TYPE_SW)
        {
            if (opType == OPERATION_WRITE)
            {
                // д�����ְ汾�ţ���"10"��
                uint8_t ver_num = atoi((char *)&uart0_struct.rx_buf[4]);
                deviceInfo.data.FirmwareVersion = ver_num;
                write_device_info_data(DEVICE_INFO_FIRMWARE_VERSION);
                // ���ظ�ʽ���ַ�������"V1.0"��
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
                    tx[pos++] = 0xFF; // δ���ñ�־
                }
            }
        }
        // ���ڴ���
        else if (dataType == DATA_TYPE_DATE)
        {
            if (opType == OPERATION_WRITE)
            {
                if (dataLen > DATE_LEN) {
                    dataLen = DATE_LEN; // ��ֹ���
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
                date[DATE_LEN] = '\0'; // ȷ���ַ�����'\0'��β
                tx[pos++] = strlen(date);
                memcpy(&tx[pos], date, strlen(date));
                pos += strlen(date);
            }
        }
        // ȷ���������tx������
        if (pos + 2 > sizeof(tx)) {
            log_e("Response too large to fit in buffer");
            return;
        }
        // ----------------- ��5�������У��ͼ�֡β -----------------
        tx[pos++] = calc_checksum(tx, pos);
        tx[pos++] = FRAME_END_READ;
        // ͨ��DMA������Ӧ
        uart0_dma_send(tx, pos);
    }
}

/**
 * ************************************************************************
 * @brief  ����У���
 *
 * @param[in] buf  ������У��͵Ļ�����
 * @param[in] len  ����������
 *
 * @return У���
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
