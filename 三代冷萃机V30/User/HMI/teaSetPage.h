#ifndef __TEA_SET_PAGE_H
#define __TEA_SET_PAGE_H

#include "gd32f30x.h"

//������ҳ�水ť��Ӧ����
void teaSetPageButton(uint8_t num,uint8_t state);
//������ҳ���ı�����
void teaSetPageText(uint8_t num,uint8_t *str);
//���ղر������
void teaCollectChange(uint8_t num,uint8_t state);
//���²��ղ�
void updateTeaCollect(void);
//������ղ�
void saveTeaCollect(void);
//���ز��ղ�
void teaCollectChoose(uint8_t num);
//���ز���ȡ����
void loadingTeaMakeSet(void);


#endif

