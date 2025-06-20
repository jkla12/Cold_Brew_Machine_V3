#ifndef __SANIT_H__
#define __SANIT_H__

#include "gd32f30x.h"

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
}sanit_TypeDef;

extern sanit_TypeDef sanit;

//������ʼ��
void sanitInit(void);
//��������
void sanitProcessControl(void);
//��ͣ����
void sanitPause(void);
//������λ
void sanitReset(void);
#endif

