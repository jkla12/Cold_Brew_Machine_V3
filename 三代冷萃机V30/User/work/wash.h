#ifndef __WASH_H
#define __WASH_H


#include "gd32f30x.h"

#define WASH_DEBUG 0


typedef struct 
{
    uint8_t state;          //״̬  0���У�1��ʼ��2��ͣ
    uint8_t step;           //����
    uint32_t startTime;     //��ʼʱ�� ��
    uint32_t time;
    uint32_t pauseTime;     //��ͣʱ�� ��
    uint32_t pauseStartTime;//��ͣ��ʼʱ�� ��
    uint32_t pauseEndTime;  //��ͣ����ʱ�� ��
    uint8_t  repeatCnt;     //��ϴ�ظ�����  
    uint32_t endTime;      //����ʱ��
}wash_TypeDef;

extern wash_TypeDef wash;

void washInit(void);
void washProcessControl(void);
void washProcess2Control(void);
void washReset(void);
void washPause(void);
void washShowTime(uint32_t Set_time,uint32_t Remain_time);


#endif

