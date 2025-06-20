#ifndef __WASH_H
#define __WASH_H


#include "gd32f30x.h"

#define WASH_DEBUG 0


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
}wash_TypeDef;

extern wash_TypeDef wash;

void washInit(void);
void washProcessControl(void);
void washProcess2Control(void);
void washReset(void);
void washPause(void);
void washShowTime(uint32_t Set_time,uint32_t Remain_time);


#endif

