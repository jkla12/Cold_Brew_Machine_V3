/**
 * 
 * @file deviceInfoPage.c
 * @author jiaokai 
 * @brief 设备信息页面响应函数
 * 
 * @copyright Copyright (c) 2025
 */
#include "deviceInfoPage.h"
#include "lcd_data_process.h"
#include "uart.h"
#include "eeprom.h"
#include "config.h"
#include "string.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "deviceInfoPage"

/**
 * ************************************************************************
 * @brief 更新设备信息页面
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void updateDeviceInfoPage(void)
{
    char str[50];
    uint8_t len;
    uint8_t strLen;
    uart2_struct.tx_buf[0] = 0xEE;
    uart2_struct.tx_buf[1] = 0xB1;
    uart2_struct.tx_buf[2] = 0x12;
    uart2_struct.tx_buf[3] = 0x00;
    uart2_struct.tx_buf[4] = device_info_page;
    // 设备硬件版本号
    uart2_struct.tx_buf[5] = 0x00;
    uart2_struct.tx_buf[6] = 0x02;
    sprintf(str,"V%d.%d",deviceInfo.data.HardwareVersion/10,deviceInfo.data.HardwareVersion%10);
    strLen = strlen(str);
    uart2_struct.tx_buf[7] = 0;
    uart2_struct.tx_buf[8] = strLen;
    for(uint8_t i = 0; i < strLen; i++)
    {
        uart2_struct.tx_buf[9 + i] = str[i];
    }
    len = 9 + strLen;
    // 设备软件版本号
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len + 1] = 0x03;
    sprintf(str,"V%d.%d",deviceInfo.data.FirmwareVersion/10,deviceInfo.data.FirmwareVersion%10);
    strLen = strlen(str);
    uart2_struct.tx_buf[len + 2] = 0;
    uart2_struct.tx_buf[len + 3] = strLen;
    for(uint8_t i = 0; i < strLen; i++)
    {
        uart2_struct.tx_buf[len + 4 + i] = str[i];
    }
    len = len+4+strLen;
    //设备ID
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len + 1] = 0x05;
    strLen = strlen(deviceInfo.data.DeviceID);
    uart2_struct.tx_buf[len + 2] = 0;
    uart2_struct.tx_buf[len + 3] = strLen;
    for(uint8_t i = 0; i < strLen; i++)
    {
        uart2_struct.tx_buf[len + 4 + i] = deviceInfo.data.DeviceID[i];
    }
    len = len+4+strLen;
    //出厂日期
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len + 1] = 0x06;
    strLen = strlen(deviceInfo.data.DateOfManufacture);
    uart2_struct.tx_buf[len + 2] = 0;
    uart2_struct.tx_buf[len + 3] = strLen;
    for(uint8_t i = 0; i < strLen; i++)
    {
        uart2_struct.tx_buf[len + 4 + i] = deviceInfo.data.DateOfManufacture[i];
    }
    len = len+4+strLen;
    uart2_struct.tx_buf[len] = 0xFF;    //帧尾
    uart2_struct.tx_buf[len + 1] = 0xFC;
    uart2_struct.tx_buf[len + 2] = 0xFF;
    uart2_struct.tx_buf[len + 3] = 0xFF;
    uart2_struct.tx_count = len + 4;
    uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
}

/**
 * ************************************************************************
 * @brief   设置页面按钮响应函数
 *
 * @param[in] num  Button number
 * @param[in] state  Button state
 *
 *
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-16
 * 
 * ************************************************************************
 */
void settingPageButton(uint8_t num,uint8_t state)
{
    switch(num)
    {
        case 8:
            if(state == 1)
            {
                updateDeviceInfoPage(); //进入设备信息页面
            }
            break;
        default:
            break;
    }
}

