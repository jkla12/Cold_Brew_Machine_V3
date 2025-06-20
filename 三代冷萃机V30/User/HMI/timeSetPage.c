/**
 * 
 * @file timeSetPage.c
 * @author jiaokai 
 * @brief 时间设置页面响应函数
 * 
 * @copyright Copyright (c) 2025
 */
#include "timeSetPage.h"
#include "lcd_data_process.h"
#include "uart.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "timeSetPage"

timeSet_TypeDef timeSet;

/**
 * ************************************************************************
 * @brief  时间设置页面按钮响应函数
 * 
 * @param[in] num  控件编号
 * @param[in] state  控件状态
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void timeSetPageButton(uint8_t num,uint8_t state)
{
    switch(num)
    {
        case 8:
            if(state == 1)
            {
                updateTimeSet();
            }
        default:
            break;
    }
}

/**
 * ************************************************************************
 * @brief 时间设置页面选择响应函数
 * 
 * @param[in] num  Comment
 * @param[in] item  Comment
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void timeSetPageSelector(uint8_t num,uint8_t item)
{
    switch(num)
    {
        case 2://年
            timeSet.year = 24+item;
            if(timeSet.year > 74)
            {
                timeSet.year = 74;
            }
            break;
        case 3://月
            timeSet.month = 1+item;
            if(timeSet.month > 12)
            {
                timeSet.month = 12;
            }
            break;
        case 5://日
            timeSet.day = 1+item;
            if(timeSet.day > 31)
            {
                timeSet.day = 31;
            }
            break;
        case 6://时
            timeSet.hour = item;
            if(timeSet.hour > 23)
            {
                timeSet.hour = 23;
            }
            break;
        case 7://分
            timeSet.minute = item;
            if(timeSet.minute > 59)
            {
                timeSet.minute = 59;
            }
            break;
        default:
            break;
    }
}

/**
 * ************************************************************************
 * @brief  更新时间到屏幕
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void updateTimeSet(void)
{
    //时间限幅
    timeSet.year = (timeSet.year > 99) ? 99 : timeSet.year;
   // timeSet.year = (timeSet.year <= 0) ? 0 : timeSet.year;
    timeSet.month = (timeSet.month > 12) ? 12 : timeSet.month;
    timeSet.month = (timeSet.month < 1) ? 1 : timeSet.month;
    timeSet.day = (timeSet.day > 31) ? 31 : timeSet.day;
    timeSet.day = (timeSet.day < 1) ? 1 : timeSet.day;
    timeSet.hour = (timeSet.hour > 23) ? 23 : timeSet.hour; 
    timeSet.minute = (timeSet.minute > 59) ? 59 : timeSet.minute;
    uart2_struct.tx_buf[0] = 0xEE;
    uart2_struct.tx_buf[1] = 0x81;
    uart2_struct.tx_buf[2] = 0x00;
    uart2_struct.tx_buf[3] = (timeSet.minute/10)<<4 | (timeSet.minute%10);
    uart2_struct.tx_buf[4] = (timeSet.hour/10)<<4 | (timeSet.hour%10);
    uart2_struct.tx_buf[5] = (timeSet.day/10)<<4 | (timeSet.day%10);
    uart2_struct.tx_buf[6] = 0;
    uart2_struct.tx_buf[7] = (timeSet.month/10)<<4 | (timeSet.month%10);
    uart2_struct.tx_buf[8] = (timeSet.year/10)<<4 | (timeSet.year%10);
    uart2_struct.tx_buf[9] = 0xFF;
    uart2_struct.tx_buf[10] = 0xFC;
    uart2_struct.tx_buf[11] = 0xFF;
    uart2_struct.tx_buf[12] = 0xFF;
    uart2_struct.tx_count = 13;
    uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);
    log_v("timeSet:%d-%d-%d %d:%d",timeSet.year,timeSet.month,timeSet.day,timeSet.hour,timeSet.minute);
}

