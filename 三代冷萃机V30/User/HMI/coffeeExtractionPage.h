#ifndef __COFFEE_EXTRACTION_PAGE_H__
#define __COFFEE_EXTRACTION_PAGE_H__ 

#include "gd32f30x.h"

//������ȡҳ�水ť��Ӧ����
void coffeeExtractionPageButton(uint8_t num,uint8_t state);
//���뿧����ȡҳ�棬ˢ��ҳ������
void enterCoffeeExtractionPage(void);
//������ȡ����ҳ�����������
void makeCoffeeShowTime(uint32_t setTime,uint32_t remainTime);


#endif

