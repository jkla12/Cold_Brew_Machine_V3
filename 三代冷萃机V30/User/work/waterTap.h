#ifndef __WATER_TAP_H__
#define __WATER_TAP_H__

#include "gd32f30x.h"

typedef struct
{
    uint16_t vol[3];            //ˮ��
    uint8_t runFlag;            //���б�־
    uint8_t buttonState[5];     //��ť״̬
    uint8_t setVol[3];          //����ˮ�� �޸�ʱ����
    uint8_t pumpState;          //ˮ��״̬
    uint8_t valveState;         //����״̬
    uint8_t setOpenTime;        //���ÿ���ʱ��  ��λ0.1��
    uint8_t nowOpenTime;        //��ǰ����ʱ�� ��λ0.1��
    uint8_t nowVolumeOption;   //��ǰˮ��ѡ�� 0:����ˮ��1����ť1ˮ����2����ť2ˮ����3����ť3ˮ����4��������5����ϴ
}waterTapWork_TypeDef;

extern waterTapWork_TypeDef waterTap_;

//ˮ��ͷ��ʼ��
void waterTapInit(void);
//ˮ��ͷ�������̿���
void waterTapProcessControl(void);
#endif
