#ifndef __WATERLEVEL_H__
#define __WATERLEVEL_H__

#include "gd32f30x.h"

#define WATER_LEVEL_DEBUG 0


typedef struct 
{
    uint8_t state;      //状态，0停止，1进水，2排水
    uint8_t err;        //错误，0无，1进水错误，2排水错误,
    uint8_t sensorErr;  //传感器故障
    uint16_t vol;       //液量
    uint32_t inletstartTime;    //进水开始时间
    uint32_t inletendTime;      //进水结束时间
    uint32_t drainstartTime;    //排水开始时间
    uint32_t draindelayTime;    //排水延迟时间
    uint8_t inlteStep;          //进水步骤
    uint8_t drainStep;          //排水步骤
    uint8_t drainOption;        //排水选择，0废水，1萃取液
}waterLevel_TypeDef;

extern waterLevel_TypeDef waterLevel;

void waterLevel_init(void);
void waterLevel_update(void);
uint8_t waterInletControl(uint16_t vol);
uint8_t  waterdrainControl(void);

void waterInletStop(void);
void waterDrainStop(void);

#endif

