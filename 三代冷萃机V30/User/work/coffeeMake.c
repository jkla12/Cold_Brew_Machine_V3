/**
 * 
 * @file coffeeMake.c
 * @author jiaokai 
 * @brief ������ȡ����
 * 
 * @copyright Copyright (c) 2025
 */

#include "coffeeMake.h"
#include "eeprom.h"
#include "relay.h"
#include "waterlevel.h"
#include "config.h"
#include "work.h"
#include "lcd_data_process.h"
#include "timer.h"
#include "uart.h"
#include "string.h"
#include "coffeeExtractionPage.h"
#include "coffeeSetPage.h"
#include "app_mqtt.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "coffeeMake"

coffee_TypeDef coffee;


/**
 * ************************************************************************
 * @brief ����������ʼ��
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-12
 * 
 * ************************************************************************
 */
void coffeeMakeInit(void)
{
    log_v("coffeeMakeInit");
    coffee.num = config.data.coffeeMake.currentNumber;  //��ȡ������ϴ��������
    coffee.state = 0;         
    coffee.isFinish = 0;       //�Ƿ����
    coffee.step = 0;           //����
    coffee.startTime = 0;     //��ʼʱ�� ��
    coffee.pauseTime = 0;     //��ͣʱ�� ��
    coffee.pauseStartTime = 0;//��ͣ��ʼʱ�� ��
    coffee.pauseEndTime = 0;  //��ͣ����ʱ�� ��
}

/**
 * ************************************************************************
 * @brief ������������
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-12
 * 
 * ************************************************************************
 */
void coffeeProcessControl(void)
{
    uint16_t timeLeft = 0;
    static uint16_t lastTimeLeft;  //�ϴ�ʣ��ʱ��
    static uint16_t makeTime;
    uint8_t waterFlag = 0;
    static uint8_t waterVol;
    switch (coffee.step)
    {
        case 0:
            coffee.state = 1; //��ʼ����
            coffee.pauseTime = 0;
            coffee.pauseStartTime = 0;
            coffee.pauseEndTime = 0;
            coffee.isFinish = 0;
            coffee.step = 1;
            makeTime = config.data.coffeeMake.time[coffee.num]*60; //��ȡ����ʱ��
            waterVol = config.data.coffeeMake.vol[coffee.num]+config.data.coffeeCompensateLevel; //��ȡ����ˮ��
            makeCoffeeShowTime(makeTime,makeTime);
            deviceRunState = 1;    //XXX����״̬
            if(WIFI.mqttConnected == 1)
            {
                if(publish_deviceRunState())
                {
                    log_i("publish device run state");
                }
                else
                {
                    log_e("publish device run state failed");
                }
            }
            log_v("coffee step 0");
            break;
        case 1: //��ˮ
            waterFlag = waterInletControl(waterVol);
            if(waterFlag == 1) //��ˮ���
            {
                coffee.step = 2;
                coffee.startTime = Timer.system1Sec; //��ȡ��ʼʱ��
                coffee.pauseStartTime = 0;
                coffee.pauseEndTime = 0;
                coffee.pauseTime = 0;
                log_v("coffee step 1 end");
            }
            else if(waterFlag == 2) //��ˮ��ʱ
            {
                log_e("coffee step 1 water inlet over time");
                coffee.step = 0xFF;
                waterInletStop();
            }
            break;
        case 2: //��ȡ
            if((Timer.system1Sec-coffee.startTime-coffee.pauseTime) >= makeTime) //��ȡʱ�䵽
            {
                log_v("coffee step 2 end");
                if(config.data.coffeeMake.autoDrainangeFlag[coffee.num] == 1) //�Զ���ˮ
                {
                    coffee.step = 3;//��ˮ
                    waterLevel.drainOption = 1; //����ȡҺ
                    log_v("coffee auto drain");
                }
                else    //����ˮ
                {
                   coffee.step = 4; //��ȡ���
                   waterLevel.drainOption = 1; //����ȡҺ
                    
                }
                closeCirculationPump();   //�ر�ѭ����
                stopCooling();  //�رհ뵼������
                //XXX ������벻֪��Ϊɶ���ϣ����Կ�����˵
                uart2_struct.tx_buf[0] = 0xEE;
                uart2_struct.tx_buf[1] = 0xB1;
                uart2_struct.tx_buf[2] = 0x12;
                uart2_struct.tx_buf[3] = 0x00;
                uart2_struct.tx_buf[4] = make_coffe_page;
                uart2_struct.tx_buf[5] = 0x00;
                uart2_struct.tx_buf[6] = 0x08;
                uart2_struct.tx_buf[7] = 0x00;
                uart2_struct.tx_buf[8] = 0x01;
                uart2_struct.tx_buf[9] = 0x30;
                uart2_struct.tx_buf[10] = 0x00;
                uart2_struct.tx_buf[11] = 0x09;
                uart2_struct.tx_buf[12] = 0x00;
                uart2_struct.tx_buf[13] = 0x01;
                uart2_struct.tx_buf[14] = 0x30;
                uart2_struct.tx_buf[15] = 0xFF;
                uart2_struct.tx_buf[16] = 0xFC;
                uart2_struct.tx_buf[17] = 0xFF;
                uart2_struct.tx_buf[18] = 0xFF;
                uart2_struct.tx_count = 19;
                uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);
                
            }
            else
            {
                if(outputState.circulationPump == 0)   //����ѭ����
                {
                    openCirculationPump();
                }
                if(outputState.cool == 0)   //��������
                {
                    startCooling();
                }
                timeLeft = makeTime - (Timer.system1Sec-coffee.startTime-coffee.pauseTime);
                if(timeLeft != lastTimeLeft)
                {
                    lastTimeLeft = timeLeft;
                    makeCoffeeShowTime(makeTime,timeLeft);
                }
            }
            break;
        case 3: //��ˮ
            if(waterdrainControl() == 1)
            {
                log_v("coffee step 3 end");
                coffee.step = 4; //��ȡ���
            }
            break;
        case 4: //��ȡ���
            AnimationPlayFrame(make_coffe_page,2,0,UART2_ID);   
            SetControlEnable(make_coffe_page,4,1,UART2_ID);   //ʹ�ܷ��ؼ�
            if(config.data.coffeeMake.autoDrainangeFlag[coffee.num] == 0) //�ֶ���ˮ
            {
                SetControlVisiable(make_coffe_page,14,1,UART2_ID);//��ʾ��ˮͼ��
                SetControlEnable(make_coffe_page,15,1,UART2_ID);//ʹ����ˮ��ť
            }
            coffeeMakeReset();
            deviceRunState = 0;   //XXX ����״̬
            record.data.coffeeMakeCnt++;
            if(WIFI.mqttConnected == 1)
            {
                if(publish_coffeeMakeCnt())
                {
                    log_i("publish coffee make count");
                }
                else
                {
                    log_e("publish coffee make count failed");
                }
            }
            write_record_data(RECORD_ALL);   //������������
            log_v("coffee step 4 end");
            delete_task(coffeeProcessControl);        //[ ]ɾ������
            log_d("delete task coffeeProcessControl");
            break;
        case 0xFF: //��ˮ��ʱ
            coffeeMakeReset();   //��λ
            deviceRunState = 2;   //XXX ����״̬
            if(WIFI.mqttConnected == 1)
            {
                if(publish_deviceRunState())
                {
                    log_i("publish device run state");
                }
                else
                {
                    log_e("publish device run state failed");
                }
            }
            AnimationPlayFrame(make_coffe_page,2,0,UART2_ID);   
            SetControlEnable(make_coffe_page,4,1,UART2_ID);   //ʹ�ܷ��ؼ�
            SetScreen(water_ingress_page,UART2_ID);     //��ˮ����ҳ��
            delete_task(coffeeProcessControl);        //[ ]ɾ������
            log_d("delete task coffeeProcessControl");
            break;
        default:
            break;
    }
}




/**
 * ************************************************************************
 * @brief ����������ͣ
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-12
 * 
 * ************************************************************************
 */
void coffeeMakePause(void)
{
    //�ر�ˮ��
    closeCirculationPump();
    //�رս�ˮ���ر���ˮ
    if(waterLevel.state == 1)   //��ǰ�ǽ�ˮ
    {
        waterInletStop();    //�رս�ˮ
        
    }
    else if(waterLevel.state == 2)   //��ǰ����ˮ
    {
         waterDrainStop();   //�ر���ˮ
    }
    stopCooling();  //�رհ뵼������
}

/**
 * ************************************************************************
 * @brief ����������λ
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void coffeeMakeReset(void)
{
    coffee.state = 0;
    coffee.pauseEndTime = 0;
    coffee.pauseStartTime = 0;
    coffee.pauseTime = 0;
    coffee.isFinish = 0;
    coffee.step = 0;
    coffee.startTime = 0;
    if(waterLevel.state == 1)
    {
        waterInletStop();
    }
    else if(waterLevel.state == 2)
    {
        waterDrainStop();
    }
    stopCooling();  //�رհ뵼������
}
