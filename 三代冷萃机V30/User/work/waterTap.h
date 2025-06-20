#ifndef __WATER_TAP_H__
#define __WATER_TAP_H__

#include "gd32f30x.h"

typedef struct
{
    uint16_t vol[3];            //水量
    uint8_t runFlag;            //运行标志
    uint8_t buttonState[5];     //按钮状态
    uint8_t setVol[3];          //设置水量 修改时缓存
    uint8_t pumpState;          //水泵状态
    uint8_t valveState;         //阀门状态
    uint8_t setOpenTime;        //设置开启时间  单位0.1秒
    uint8_t nowOpenTime;        //当前开启时间 单位0.1秒
    uint8_t nowVolumeOption;   //当前水量选项 0:不出水，1：按钮1水量，2：按钮2水量，3：按钮3水量，4：常开，5：清洗
}waterTapWork_TypeDef;

extern waterTapWork_TypeDef waterTap_;

//水龙头初始化
void waterTapInit(void);
//水龙头工作流程控制
void waterTapProcessControl(void);
#endif
