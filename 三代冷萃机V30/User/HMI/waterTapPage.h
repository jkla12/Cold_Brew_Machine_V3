#ifndef __WATER_TAP_PAGE_H__
#define __WATER_TAP_PAGE_H__

#include "gd32f30x.h"

//主页按钮响应函数
void waterTapMainPageButton(uint8_t num,uint8_t state);
//设置页面按钮响应函数
void waterTapSetPageButton(uint8_t num,uint8_t state);
//设置页面文本更新函数
void waterTapSetPageText(uint8_t num,uint8_t *str);
//更新水龙头屏幕文本
void updataWaterTapPageText(uint8_t page);
//设置图标
void waterTapSetIcon(uint16_t screenID,uint16_t controlID,uint8_t value);
//水龙头屏幕设置字体颜色
void waterTapSetForeColor(uint8_t num);
#endif


