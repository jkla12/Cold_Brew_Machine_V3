/**
 * 
 * @file languagePage.c
 * @author jiaokai 
 * @brief ��������ҳ����Ӧ����
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
 * @brief   ��������ҳ�水ť��Ӧ����
 * 
 * @param[in] num  �ؼ����
 * @param[in] state  �ؼ�״̬
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

