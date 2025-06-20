#ifndef __WASH_PAGE_H__
#define __WASH_PAGE_H__ 

#include "gd32f30x.h"

//清洗页面按钮响应函数

void drainPageButton(uint8_t num,uint8_t state);
void drainShowTime(uint32_t setTime,uint32_t remainTime);   //更新排水进度条
void drainChoosePageButton(uint8_t num,uint8_t state);   //排水选择页面按钮响应
#endif

