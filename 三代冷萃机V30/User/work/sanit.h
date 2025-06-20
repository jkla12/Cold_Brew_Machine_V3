#ifndef __SANIT_H__
#define __SANIT_H__

#include "gd32f30x.h"

typedef struct
{
    uint8_t state;          //状态  0空闲，1开始，2暂停
    uint8_t step;           //步骤
    uint32_t startTime;     //开始时间 秒
    uint32_t time;
    uint32_t pauseTime;     //暂停时间 秒
    uint32_t pauseStartTime;//暂停开始时间 秒
    uint32_t pauseEndTime;  //暂停结束时间 秒
    uint8_t  repeatCnt;     //清洗重复计数  
    uint32_t endTime;      //结束时间
}sanit_TypeDef;

extern sanit_TypeDef sanit;

//消毒初始化
void sanitInit(void);
//消毒流程
void sanitProcessControl(void);
//暂停消毒
void sanitPause(void);
//消毒复位
void sanitReset(void);
#endif

