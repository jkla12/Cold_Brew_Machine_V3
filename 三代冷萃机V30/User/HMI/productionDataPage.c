/**
 * 
 * @file productionDataPage.c
 * @author jiaokai 
 * @brief 生成数据页面响应函数
 * 
 * @copyright Copyright (c) 2025
 */
#include "productionDataPage.h"
#include "lcd_data_process.h"
#include "uart.h"
#include "string.h"
#include "eeprom.h"
#include "config.h"
#include "work.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "productionDataPage"


/**
 * ************************************************************************
 * @brief  进入生产数据页面
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void enterProductionPage(void)
{
    char str[10];
    uint8_t strLen;
    uint8_t len;
    uint32_t makeCnt;
    uart2_struct.tx_buf[0] = 0xEE;
    uart2_struct.tx_buf[1] = 0xB1;
    uart2_struct.tx_buf[2] = 0x12;
    uart2_struct.tx_buf[3] = 0x00;
    uart2_struct.tx_buf[4] = production_page;   //页面地址
    uart2_struct.tx_buf[5] = 0x00;
    uart2_struct.tx_buf[6] = 0x02;              //萃取信息
    uart2_struct.tx_buf[7] = 0x00;
    if(record.data.coffeeMakeCnt > 99999)
    {
        record.data.coffeeMakeCnt = 99999;
    }
    if(record.data.teaMakeCnt > 99999)
    {
        record.data.teaMakeCnt = 99999;
    }
    makeCnt = record.data.coffeeMakeCnt + record.data.teaMakeCnt; //制作次数
    sprintf(str,"%d",makeCnt);
    strLen = strlen(str);
    uart2_struct.tx_buf[8] = strLen;
    for(uint8_t i=0;i<strLen;i++)
    {
        uart2_struct.tx_buf[9+i] = str[i];
    }
    len = 9+strLen;
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len+1] = 0x03;              //清洗信息
    uart2_struct.tx_buf[len+2] = 0x00;
    if(record.data.washCnt > 99999)
    {
        record.data.washCnt = 0;
    }
    sprintf(str,"%d",record.data.washCnt);
    strLen = strlen(str);
    uart2_struct.tx_buf[len+3] = strLen;
    for(uint8_t i=0;i<strLen;i++)
    {
        uart2_struct.tx_buf[len+4+i] = str[i];
    }
    len = len+4+strLen;
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len+1] = 0x05;              //消毒信息
    uart2_struct.tx_buf[len+2] = 0x00;
    if(record.data.sanitCnt > 99999)
    {
        record.data.sanitCnt = 0;
    }
    sprintf(str,"%d",record.data.sanitCnt);
    strLen = strlen(str);
    uart2_struct.tx_buf[len+3] = strLen;
    for(uint8_t i=0;i<strLen;i++)
    {
        uart2_struct.tx_buf[len+4+i] = str[i];
    }
    len = len+4+strLen;
    uart2_struct.tx_buf[len] = 0xFF;
    uart2_struct.tx_buf[len+1] = 0xFC;
    uart2_struct.tx_buf[len+2] = 0xFF;
    uart2_struct.tx_buf[len+3] = 0xFF;
    uart2_struct.tx_count = len+4;
    uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);   //发送数据
}


