/**
 * 
 * @file errPage.c
 * @author jiaokai 
 * @brief 故障页面响应函数
 * 
 * @copyright Copyright (c) 2025
 */
#include "errPage.h"
#include "lcd_data_process.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "errPage"



/**
 * ************************************************************************
 * @brief   故障页面按钮响应函数
 * 
 * @param[in] page  页面地址
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
void errPageButton(uint8_t page,uint8_t num,uint8_t state)
{
    switch(page)
    {
        case water_level_page: //
            SetScreen(main_page,UART2_ID);
            break;
        case water_ingress_page:
            SetScreen(main_page,UART2_ID);
            break;
        case water_drain_page:
            SetScreen(main_page,UART2_ID);
            break;
        default:
            break;
    }
}


