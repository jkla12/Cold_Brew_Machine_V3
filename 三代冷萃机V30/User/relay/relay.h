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
    Relay1 = 0,	//循环泵
    Relay2 = 1,	//预留
    Relay3 = 2,	//进水电磁阀
    Relay4 = 3,	//清洗电磁阀
    Relay5 = 4,	//循环三通
    Relay6 = 5,	//出液三通
    Relay7 = 6,	//半导体制冷
    Relay8 = 7,	//直流泵1
    Relay9 = 8	//直流泵2
}Relay_typedef_enum;



typedef struct
{
    uint8_t circulationPump;    //循环泵
    uint32_t pumpOpeningTime;   //循环泵开启时间 单位秒
    uint32_t pumpCurrentRuntime;//循环泵当前运行时间 单位秒
    uint8_t relay2;             //预留
    uint8_t inletValve;         //进水电磁阀
    uint8_t washValve;          //清洗电磁阀
    uint8_t circulationValve;   //循环三通
    uint8_t drainValve;         //出液三通
    uint8_t cool;               //半导体制冷
    uint32_t coolOpeningTime;   //半导体制冷开启时间 单位秒
    uint32_t coolCurrentRuntime;//半导体制冷当前运行时间 单位秒
    uint8_t pump1;              //直流泵1
    uint8_t pump2;              //直流泵2
    uint8_t recordState[7];     //记录前7路输出状态
    uint32_t pump1OpeningTime;   //直流泵1开启时间 单位秒
    uint32_t pump1CurrentRuntime;//直流泵1当前运行时间 单位秒
}OutputState_TypeDef;


extern OutputState_TypeDef outputState;





void relay_init(void);
void relay_ON(Relay_typedef_enum number);
void relay_OFF(Relay_typedef_enum number);


void openInletValve(void);
void closeInletValve(void);

void drainValveExtract(void);   //三通球阀切换到萃取液
void drainValveSewage(void);    //三通球阀切换到污水

void openCirculationValve(void);    //三通球阀切换到排水
void closeCirculationValve(void);   //三通球阀切换到循环

void openCirculationPump(void);     //开启循环泵
void closeCirculationPump(void);    //关闭循环泵

void startCooling(void);    //开启半导体制冷
void stopCooling(void);     //关闭半导体制冷

void openPump1(void);      //开启直流泵1
void closePump1(void);     //关闭直流泵1

void openPump2(void);      //开启直流泵2
void closePump2(void);     //关闭直流泵2
void openWashValve(void);      //开启清洗电磁阀
void closeWashValve(void);     //关闭清洗电磁阀

void recordOutputState(void); //记录输出状态
void restoreOutputState(void); //恢复输出状态
#endif


