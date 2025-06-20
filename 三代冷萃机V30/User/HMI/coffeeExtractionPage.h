#ifndef __COFFEE_EXTRACTION_PAGE_H__
#define __COFFEE_EXTRACTION_PAGE_H__ 

#include "gd32f30x.h"

//咖啡萃取页面按钮响应函数
void coffeeExtractionPageButton(uint8_t num,uint8_t state);
//进入咖啡萃取页面，刷新页面数据
void enterCoffeeExtractionPage(void);
//咖啡萃取设置页面进度条函数
void makeCoffeeShowTime(uint32_t setTime,uint32_t remainTime);


#endif

