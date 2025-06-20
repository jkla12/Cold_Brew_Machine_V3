/**
 * 
 * @file teaMake.c
 * @author jiaokai 
 * @brief ����ȡ����
 * 
 * @copyright Copyright (c) 2025
 */

#include "teaMake.h"
#include "eeprom.h"
#include "relay.h"
#include "waterlevel.h"
#include "work.h"
#include "config.h"
#include "lcd_data_process.h"
#include "timer.h"
#include "uart.h"
#include "string.h"
#include "teaExtractionPage.h"
#include "teaSetPage.h"
#include "app_mqtt.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "teaMake"

tea_TypeDef tea;

/**
 * ************************************************************************
 * @brief ����ȡ�����ʼ��
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void teaMakeInit(void)
{
    log_v("teaMakeInit");
    tea.num = config.data.teaMake.currentNumber;  //��ȡ������ϴ��������
    tea.state = 0;         
    tea.isFinish = 0;       //�Ƿ����
    tea.step = 0;           //����
    tea.startTime = 0;     //��ʼʱ�� ��
    tea.pauseTime = 0;     //��ͣʱ�� ��
    tea.pauseStartTime = 0;//��ͣ��ʼʱ�� ��
    tea.pauseEndTime = 0;  //��ͣ����ʱ�� ��
}





/**
 * ************************************************************************
 * @brief ����������
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void teaProcessControl(void)
{
    uint16_t timeLeft;   //ʣ��ʱ��
    static uint16_t lastTime = 0; //�ϴ�ʱ��
    static uint16_t makeTime;
    uint8_t waterFlag;
    static uint8_t waterVol;
    switch (tea.step)
    {
        case 0:
            tea.state = 1; //��ʼ����
            tea.pauseTime = 0;
            tea.pauseStartTime = 0;
            tea.pauseEndTime = 0;
            makeTime= config.data.teaMake.time[tea.num]*60; //����ʱ��
            waterVol = config.data.teaMake.vol[tea.num]+config.data.teaCompensateLevel; //����ˮ��
            makeTeaShowTime(makeTime,makeTime); //��ʾ����ʱ��
            log_v("tea step 0");
            deviceRunState = 1;   //XXX ����״̬
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
            tea.step = 1; //��ˮ
            break;
        case 1: //��ˮ
            waterFlag = waterInletControl(waterVol);
            if(waterFlag == 1) //��ˮ���
            {
                tea.step = 2;
                tea.startTime = Timer.system1Sec; //��ȡ��ʼʱ��
                tea.pauseStartTime = 0;
                tea.pauseEndTime = 0;
                tea.pauseTime = 0;
                log_v("tea step 1 end");
            }
            else if(waterFlag == 2) //��ˮ��ʱ
            {
                log_e("tea step 1 water inlet over time");
                tea.step = 0xFF;
                waterInletStop();
            }
            break;
        case 2: //��ȡ
            if((Timer.system1Sec-tea.startTime-tea.pauseTime) >= makeTime) //��ȡʱ�䵽
            {
                log_v("coffee step 2 end");
                if(config.data.teaMake.autoDrainangeFlag[tea.num] == 1) //�Զ���ˮ
                {
                    tea.step = 3; //��ˮ
                    waterLevel.drainOption = 1; //�ŷ�ˮ
                    log_v("tea auto drain");
                }
                else
                {
                    tea.step = 4; //��ȡ���
                    waterLevel.drainOption = 1; //������Һ
                }
                closeCirculationPump(); //�ر�ѭ����
                uart2_struct.tx_buf[0] = 0xEE;
                uart2_struct.tx_buf[1] = 0xB1;
                uart2_struct.tx_buf[2] = 0x12;
                uart2_struct.tx_buf[3] = 0x00;
                uart2_struct.tx_buf[4] = make_tea_page;
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
                if(outputState.circulationPump == 0)
                {
                    openCirculationPump(); //��ѭ����
                }
                if(outputState.cool == 0)
                {
                    startCooling(); //�򿪰뵼������
                }
                timeLeft = makeTime - (Timer.system1Sec-tea.startTime-tea.pauseTime); //ʣ��ʱ��
                if(timeLeft != lastTime) //ʱ��仯
                {
                    lastTime = timeLeft;
                    makeTeaShowTime(makeTime,timeLeft); //��ʾ����ʱ��
                }
            }
            break;
        case 3: //��ˮ
            if(waterdrainControl() == 1)
            {
                log_v("tea step 3 end");
                tea.step = 4; //��ˮ���
            }
            break;
        case 4: //��ȡ���
            AnimationPlayFrame(make_tea_page,2,0,UART2_ID);   //����
            SetControlEnable(make_tea_page,4,1,UART2_ID);   //ʹ�ܷ��ؼ�
            if(config.data.teaMake.autoDrainangeFlag[tea.num] == 0)//�ֶ���ˮ
            {
                SetControlVisiable(make_tea_page,14,1,UART2_ID);//��ʾ��ˮͼ��
                SetControlEnable(make_tea_page,15,1,UART2_ID);//ʹ����ˮ��ť
            }
            deviceRunState = 0;   //XXX ������������״̬
            record.data.teaMakeCnt++;
            if(WIFI.mqttConnected == 1)
            {
                if(publish_teaMakeCnt())
                {
                    log_i("publish device run state");
                }
                else
                {
                    log_e("publish device run state failed");
                }
            }
            write_record_data(RECORD_ALL); //XXX ������������
            teaMakeReset();   //��λ
            log_v("tea step 4 end");
            delete_task(teaProcessControl);        //[ ]ɾ������
            log_d("delete task teaProcessControl");
            break;
        case 0xFF: //��ˮ��ʱ
            teaMakeReset();   //��λ
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
            AnimationPlayFrame(make_tea_page,2,0,UART2_ID);   //����
            SetControlEnable(make_tea_page,4,1,UART2_ID);   //ʹ�ܷ��ؼ�
            SetScreen(water_ingress_page,UART2_ID);     //��ˮ����ҳ��
            delete_task(teaProcessControl);        //[ ]ɾ������
            log_d("delete task teaProcessControl");
            break;
    }
}

/**
 * ************************************************************************
 * @brief ��ͣ������
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void teaMakePause(void)
{
    closeCirculationPump(); //�ر�ѭ����
    if(waterLevel.state == 1)   //��ǰ�ǽ�ˮ
    {
        waterInletStop();   //��ˮֹͣ
    }
    else if(waterLevel.state == 2)    //��ǰ����ˮ
    {
        waterDrainStop();
    }
    stopCooling();  //�رհ뵼������
}

/**
 * ************************************************************************
 * @brief ��������λ
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void teaMakeReset(void)
{
    tea.state = 0;         
    tea.isFinish = 0;       //�Ƿ����
    tea.step = 0;           //����
    tea.startTime = 0;     //��ʼʱ�� ��
    tea.pauseTime = 0;     //��ͣʱ�� ��
    tea.pauseStartTime = 0;//��ͣ��ʼʱ�� ��
    tea.pauseEndTime = 0;  //��ͣ����ʱ�� ��
    if(waterLevel.state == 1)   //��ǰ�ǽ�ˮ
    {
        waterInletStop();   //��ˮֹͣ
    }
    else    //��ǰ����ˮ
    {
        waterDrainStop();
    }
    stopCooling();  //�رհ뵼������
}


