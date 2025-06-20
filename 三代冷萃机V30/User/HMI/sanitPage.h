#ifndef __SANIT_PAGE_H__
#define __SANIT_PAGE_H__

#include "gd32f30x.h"

//消毒页面按钮响应函数
void sanitPageButton(uint8_t num,uint8_t state);
void sanitShowTime(uint32_t setTime,uint32_t remainTime);   //更新消毒进度条


#endif

