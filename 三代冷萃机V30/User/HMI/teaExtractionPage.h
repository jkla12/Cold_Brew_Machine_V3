#ifndef __TEA_EXTRACTION_PAGE_H
#define __TEA_EXTRACTION_PAGE_H

#include "gd32f30x.h"

//����ȡҳ�水ť��Ӧ����
void teaExtractionPageButton(uint8_t num,uint8_t state);
//�������ȡҳ��
void enterTeaExtractionPage(void); 
//����ȡҳ���������ʾ
void makeTeaShowTime(uint32_t setTime,uint32_t remainTime);
#endif
