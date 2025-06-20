#ifndef __TEA_EXTRACTION_PAGE_H
#define __TEA_EXTRACTION_PAGE_H

#include "gd32f30x.h"

//茶萃取页面按钮响应函数
void teaExtractionPageButton(uint8_t num,uint8_t state);
//进入茶萃取页面
void enterTeaExtractionPage(void); 
//茶萃取页面进度条显示
void makeTeaShowTime(uint32_t setTime,uint32_t remainTime);
#endif
