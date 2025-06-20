#ifndef __TIME_SET_PAGE_H__
#define __TIME_SET_PAGE_H__

#include "gd32f30x.h"


typedef struct
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
}timeSet_TypeDef;


//时间设置页面按钮响应函数

void timeSetPageButton(uint8_t num,uint8_t state);

void timeSetPageSelector(uint8_t num,uint8_t item);
void updateTimeSet(void);
#endif


