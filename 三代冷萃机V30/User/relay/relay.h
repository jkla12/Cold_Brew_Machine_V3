#ifndef __RELAY_H
#define __RELAY_H

#include "gd32f30x.h"

#define Relay1_PORT GPIOA
#define Relay1_PIN  GPIO_PIN_6
#define Relay2_PORT GPIOB
#define Relay2_PIN  GPIO_PIN_13
#define Relay3_PORT GPIOA
#define Relay3_PIN  GPIO_PIN_7
#define Relay4_PORT GPIOA
#define Relay4_PIN  GPIO_PIN_8
#define Relay5_PORT GPIOC
#define Relay5_PIN  GPIO_PIN_4
#define Relay6_PORT GPIOC
#define Relay6_PIN  GPIO_PIN_6
#define Relay7_PORT GPIOC
#define Relay7_PIN  GPIO_PIN_5
#define Relay8_PORT GPIOA
#define Relay8_PIN  GPIO_PIN_1
#define Relay9_PORT GPIOB
#define Relay9_PIN  GPIO_PIN_7

#define MOTOR_PWM_PORT  GPIOB
#define MOTOR_PWM_PIN   GPIO_PIN_6


static uint32_t RELAY_GPIO_PORT[9] = {
    Relay1_PORT,Relay2_PORT,Relay3_PORT,Relay4_PORT,
    Relay5_PORT,Relay6_PORT,Relay7_PORT,Relay8_PORT,
    Relay9_PORT
};
static uint32_t RELAY_GPIO_PIN[9] = {
    Relay1_PIN,Relay2_PIN,Relay3_PIN,Relay4_PIN,
    Relay5_PIN,Relay6_PIN,Relay7_PIN,Relay8_PIN,
    Relay9_PIN
};


typedef enum{
    Relay1 = 0,	//ѭ����
    Relay2 = 1,	//Ԥ��
    Relay3 = 2,	//��ˮ��ŷ�
    Relay4 = 3,	//��ϴ��ŷ�
    Relay5 = 4,	//ѭ����ͨ
    Relay6 = 5,	//��Һ��ͨ
    Relay7 = 6,	//�뵼������
    Relay8 = 7,	//ֱ����1
    Relay9 = 8	//ֱ����2
}Relay_typedef_enum;



typedef struct
{
    uint8_t circulationPump;    //ѭ����
    uint32_t pumpOpeningTime;   //ѭ���ÿ���ʱ�� ��λ��
    uint32_t pumpCurrentRuntime;//ѭ���õ�ǰ����ʱ�� ��λ��
    uint8_t relay2;             //Ԥ��
    uint8_t inletValve;         //��ˮ��ŷ�
    uint8_t washValve;          //��ϴ��ŷ�
    uint8_t circulationValve;   //ѭ����ͨ
    uint8_t drainValve;         //��Һ��ͨ
    uint8_t cool;               //�뵼������
    uint32_t coolOpeningTime;   //�뵼�����俪��ʱ�� ��λ��
    uint32_t coolCurrentRuntime;//�뵼�����䵱ǰ����ʱ�� ��λ��
    uint8_t pump1;              //ֱ����1
    uint8_t pump2;              //ֱ����2
    uint8_t recordState[7];     //��¼ǰ7·���״̬
    uint32_t pump1OpeningTime;   //ֱ����1����ʱ�� ��λ��
    uint32_t pump1CurrentRuntime;//ֱ����1��ǰ����ʱ�� ��λ��
}OutputState_TypeDef;


extern OutputState_TypeDef outputState;





void relay_init(void);
void relay_ON(Relay_typedef_enum number);
void relay_OFF(Relay_typedef_enum number);


void openInletValve(void);
void closeInletValve(void);

void drainValveExtract(void);   //��ͨ���л�����ȡҺ
void drainValveSewage(void);    //��ͨ���л�����ˮ

void openCirculationValve(void);    //��ͨ���л�����ˮ
void closeCirculationValve(void);   //��ͨ���л���ѭ��

void openCirculationPump(void);     //����ѭ����
void closeCirculationPump(void);    //�ر�ѭ����

void startCooling(void);    //�����뵼������
void stopCooling(void);     //�رհ뵼������

void openPump1(void);      //����ֱ����1
void closePump1(void);     //�ر�ֱ����1

void openPump2(void);      //����ֱ����2
void closePump2(void);     //�ر�ֱ����2
void openWashValve(void);      //������ϴ��ŷ�
void closeWashValve(void);     //�ر���ϴ��ŷ�

void recordOutputState(void); //��¼���״̬
void restoreOutputState(void); //�ָ����״̬
#endif


