#ifndef __COFFEEMAKE_H__
#define __COFFEEMAKE_H__

#include "gd32f30x.h"

//咖啡萃取流程

typedef struct 
{
    uint8_t num;                //当前制作编号
    uint8_t state;              //当前制作状态 0停止，1运行,2暂停
    uint8_t isFinish;           //是否完成
    uint8_t step;               //当前步骤
    uint32_t startTime;         //开始时间 秒
    uint32_t pauseTime;         //暂停时间 秒
    uint32_t pauseStartTime;    //暂停开始时间
    uint32_t pauseEndTime;      //暂停结束时间
}coffee_TypeDef;

extern coffee_TypeDef coffee;

void coffeeMakeInit(void);  //咖啡制作初始化
void coffeeProcessControl(void); //咖啡制作流程
void coffeeMakePause(void); //咖啡制作暂停
void coffeeMakeReset(void); //咖啡制作重置
#endif

