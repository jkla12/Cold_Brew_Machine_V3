#ifndef __WATERLEVEL_H__
#define __WATERLEVEL_H__

#include "gd32f30x.h"

#define WATER_LEVEL_DEBUG 0


typedef struct 
{
    uint8_t state;      //״̬��0ֹͣ��1��ˮ��2��ˮ
    uint8_t err;        //����0�ޣ�1��ˮ����2��ˮ����,
    uint8_t sensorErr;  //����������
    uint16_t vol;       //Һ��
    uint32_t inletstartTime;    //��ˮ��ʼʱ��
    uint32_t inletendTime;      //��ˮ����ʱ��
    uint32_t drainstartTime;    //��ˮ��ʼʱ��
    uint32_t draindelayTime;    //��ˮ�ӳ�ʱ��
    uint8_t inlteStep;          //��ˮ����
    uint8_t drainStep;          //��ˮ����
    uint8_t drainOption;        //��ˮѡ��0��ˮ��1��ȡҺ
}waterLevel_TypeDef;

extern waterLevel_TypeDef waterLevel;

void waterLevel_init(void);
void waterLevel_update(void);
uint8_t waterInletControl(uint16_t vol);
uint8_t  waterdrainControl(void);

void waterInletStop(void);
void waterDrainStop(void);

#endif

