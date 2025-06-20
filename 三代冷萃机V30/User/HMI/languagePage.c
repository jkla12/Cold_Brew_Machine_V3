/**
 * 
 * @file languagePage.c
 * @author jiaokai 
 * @brief 语言设置页面响应函数
 * 
 * @copyright Copyright (c) 2025
 */
#include "languagePage.h"
#include "lcd_data_process.h"
#include "eeprom.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "languagePage"



/**
 * ************************************************************************
 * @brief   语言设置页面按钮响应函数
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
void languagePageButton(uint8_t num,uint8_t state)
{
    if(num == 2)
    {
        config.data.language = 0;
        log_v("language:chinese");
    }
    else if(num == 3)
    {
        config.data.language = 1;
        log_v("language:english");
    }
}

