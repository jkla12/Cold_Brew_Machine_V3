#ifndef __COFFEE_SET_PAGE_H__
#define __COFFEE_SET_PAGE_H__

#include "gd32f30x.h"

//������ȡ����ҳ�水ť��Ӧ����
void coffeeSetPageButton(uint8_t num,uint8_t state);
//������ȡ����ҳ���ı�����
void coffeeSetPageText(uint16_t num,uint8_t *str);
//�����ղر������
void coffeeCollectChange(uint8_t num,uint8_t state);
//���¿����ղ�
void UpdateCoffeeCollect(void);
//���浱ǰ���ȱ��
void saveCoffeeCurrentNum(void);
 //���濧���ղ�
void saveCoffeeCollect(void);
//���ؿ����ղ�
void coffeeCollectChoose(uint8_t num);
//���ؿ�����ȡ����
void loadingCoffeeMakeSet(void);
#endif

