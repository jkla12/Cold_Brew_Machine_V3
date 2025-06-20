#ifndef __WASH_PAGE_H
#define __WASH_PAGE_H

#include "gd32f30x.h"

//清洗页面按钮响应函数
void washPageButton(uint8_t num,uint8_t state);
void washShowTime(uint32_t setTime,uint32_t remainTime);//清洗倒计时

#endif

