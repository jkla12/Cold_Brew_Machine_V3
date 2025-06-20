#ifndef __TEA_MAKE_H
#define __TEA_MAKA_H

#include "gd32f30x.h"
//茶萃取流程

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
}tea_TypeDef;
extern tea_TypeDef tea;

void teaMakeInit(void);  //茶制作初始化
void teaProcessControl(void); //茶制作流程控制
void teaMakePause(void); //茶制作暂停
void teaMakeReset(void); //茶制作重置
void saveTeaCurrentNum(void);

#endif




