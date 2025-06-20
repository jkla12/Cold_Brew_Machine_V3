#ifndef __DRAIN_H__
#define __DRAIN_H__

#include "gd32f30x.h"

typedef struct
{
    uint8_t state;      //状态，0停止,1 排水，2暂停
    uint8_t step;      //步骤
    uint32_t startTime; //开始时间
    uint32_t pauseTime; //暂停时间
    uint32_t pauseStartTime; //暂停开始时间
    uint32_t pauseEndTime;   //暂停结束时间
}drain_TypeDef;

extern drain_TypeDef drain;

void drainProcessControl(void);
void drainReset(void);
#endif

