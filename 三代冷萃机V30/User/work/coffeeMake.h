#ifndef __COFFEEMAKE_H__
#define __COFFEEMAKE_H__

#include "gd32f30x.h"

//������ȡ����

typedef struct 
{
    uint8_t num;                //��ǰ�������
    uint8_t state;              //��ǰ����״̬ 0ֹͣ��1����,2��ͣ
    uint8_t isFinish;           //�Ƿ����
    uint8_t step;               //��ǰ����
    uint32_t startTime;         //��ʼʱ�� ��
    uint32_t pauseTime;         //��ͣʱ�� ��
    uint32_t pauseStartTime;    //��ͣ��ʼʱ��
    uint32_t pauseEndTime;      //��ͣ����ʱ��
}coffee_TypeDef;

extern coffee_TypeDef coffee;

void coffeeMakeInit(void);  //����������ʼ��
void coffeeProcessControl(void); //������������
void coffeeMakePause(void); //����������ͣ
void coffeeMakeReset(void); //������������
#endif

