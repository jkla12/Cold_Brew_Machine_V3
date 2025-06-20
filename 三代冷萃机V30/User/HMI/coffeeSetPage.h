#ifndef __COFFEE_SET_PAGE_H__
#define __COFFEE_SET_PAGE_H__

#include "gd32f30x.h"

//咖啡萃取设置页面按钮响应函数
void coffeeSetPageButton(uint8_t num,uint8_t state);
//咖啡萃取设置页面文本更新
void coffeeSetPageText(uint16_t num,uint8_t *str);
//咖啡收藏变更函数
void coffeeCollectChange(uint8_t num,uint8_t state);
//更新咖啡收藏
void UpdateCoffeeCollect(void);
//保存当前咖啡编号
void saveCoffeeCurrentNum(void);
 //保存咖啡收藏
void saveCoffeeCollect(void);
//加载咖啡收藏
void coffeeCollectChoose(uint8_t num);
//加载咖啡萃取设置
void loadingCoffeeMakeSet(void);
#endif

