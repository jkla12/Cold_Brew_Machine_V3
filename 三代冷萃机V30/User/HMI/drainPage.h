#ifndef __WASH_PAGE_H__
#define __WASH_PAGE_H__ 

#include "gd32f30x.h"

//��ϴҳ�水ť��Ӧ����

void drainPageButton(uint8_t num,uint8_t state);
void drainShowTime(uint32_t setTime,uint32_t remainTime);   //������ˮ������
void drainChoosePageButton(uint8_t num,uint8_t state);   //��ˮѡ��ҳ�水ť��Ӧ
#endif

