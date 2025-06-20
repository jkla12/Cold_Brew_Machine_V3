#ifndef __TIMER_H
#define __TIMER_H

#include "gd32f30x.h"

typedef struct
{
	uint32_t systemTime;		//系统时间, 1ms基数
	uint32_t system1Sec;		//系统时间，1s基数
	uint8_t _1sFlag;		    //定时器1_1S_触发标志位
	uint8_t _100msFlag;	        //定时器1_100ms_触发标志位
	uint8_t _10msFlag;	        //定时器1_10ms_触发标志位
	uint8_t flag;			    //定时器1_1ms_触发标志位
	uint16_t cnt;		        //定时器1计数标志位
}Timer_TypeDef;

extern Timer_TypeDef Timer;

void timer1_config(void);

#endif


