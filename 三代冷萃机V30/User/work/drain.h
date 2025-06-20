#ifndef __DRAIN_H__
#define __DRAIN_H__

#include "gd32f30x.h"

typedef struct
{
    uint8_t state;      //״̬��0ֹͣ,1 ��ˮ��2��ͣ
    uint8_t step;      //����
    uint32_t startTime; //��ʼʱ��
    uint32_t pauseTime; //��ͣʱ��
    uint32_t pauseStartTime; //��ͣ��ʼʱ��
    uint32_t pauseEndTime;   //��ͣ����ʱ��
}drain_TypeDef;

extern drain_TypeDef drain;

void drainProcessControl(void);
void drainReset(void);
#endif

