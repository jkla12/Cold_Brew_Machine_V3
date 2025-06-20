#ifndef __TEA_SET_PAGE_H
#define __TEA_SET_PAGE_H

#include "gd32f30x.h"

//茶设置页面按钮响应函数
void teaSetPageButton(uint8_t num,uint8_t state);
//茶设置页面文本更新
void teaSetPageText(uint8_t num,uint8_t *str);
//茶收藏变更函数
void teaCollectChange(uint8_t num,uint8_t state);
//更新茶收藏
void updateTeaCollect(void);
//保存茶收藏
void saveTeaCollect(void);
//加载茶收藏
void teaCollectChoose(uint8_t num);
//加载茶萃取设置
void loadingTeaMakeSet(void);


#endif

