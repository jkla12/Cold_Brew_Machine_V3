#ifndef __TEA_MAKE_H
#define __TEA_MAKA_H

#include "gd32f30x.h"
//����ȡ����

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
}tea_TypeDef;
extern tea_TypeDef tea;

void teaMakeInit(void);  //��������ʼ��
void teaProcessControl(void); //���������̿���
void teaMakePause(void); //��������ͣ
void teaMakeReset(void); //����������
void saveTeaCurrentNum(void);

#endif




