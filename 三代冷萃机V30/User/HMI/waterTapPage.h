#ifndef __WATER_TAP_PAGE_H__
#define __WATER_TAP_PAGE_H__

#include "gd32f30x.h"

//��ҳ��ť��Ӧ����
void waterTapMainPageButton(uint8_t num,uint8_t state);
//����ҳ�水ť��Ӧ����
void waterTapSetPageButton(uint8_t num,uint8_t state);
//����ҳ���ı����º���
void waterTapSetPageText(uint8_t num,uint8_t *str);
//����ˮ��ͷ��Ļ�ı�
void updataWaterTapPageText(uint8_t page);
//����ͼ��
void waterTapSetIcon(uint16_t screenID,uint16_t controlID,uint8_t value);
//ˮ��ͷ��Ļ����������ɫ
void waterTapSetForeColor(uint8_t num);
#endif


