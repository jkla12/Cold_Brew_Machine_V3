/**
 * 
 * @file waterTap.c
 * @author jiaokai
 * @brief ˮ��ͷ��ؿ��ƴ���
 * 
 * @copyright Copyright (c) 2025
 */
#include "waterTap.h"
#include "lcd_data_process.h"
#include "uart.h"
#include "string.h"
#include "eeprom.h"
#include "config.h"
#include "timer.h"
#include "work.h"
#include "relay.h"
#include "waterTapPage.h"
#include "elog.h"
#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "waterTap"

waterTapWork_TypeDef waterTap_;

/**
 * ************************************************************************
 * @brief ˮ��ͷ��ʼ��
 *
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-14
 *
 * ************************************************************************
 */
void waterTapInit(void)
{
    waterTap_.buttonState[0] = 0;
    waterTap_.buttonState[1] = 0;
    waterTap_.buttonState[2] = 0;
    waterTap_.buttonState[3] = 0;
    waterTap_.buttonState[4] = 0;
    waterTap_.vol[0] = config.data.waterTap.time[0];
    waterTap_.vol[1] = config.data.waterTap.time[1];
    waterTap_.vol[2] = config.data.waterTap.time[2];
    waterTap_.runFlag = 0;
    waterTap_.setVol[0] = config.data.waterTap.time[0];
    waterTap_.setVol[1] = config.data.waterTap.time[1];
    waterTap_.setVol[2] = config.data.waterTap.time[2];
    waterTap_.setOpenTime = 0;
    waterTap_.nowOpenTime = 0;
    waterTap_.nowVolumeOption = 0;
    waterTap_.valveState = 0;
    waterTap_.pumpState = 0;
    updataWaterTapPageText(0);
    updataWaterTapPageText(1);
}

/**
 * ************************************************************************
 * @brief ˮ��ͷ�������̿���
 *
 *
 * 
 * @version 1.0
 * @author jiaokai
 * @date 2025-05-14
 *
 * ************************************************************************
 */
void waterTapProcessControl(void)
{
    switch(waterTap_.nowVolumeOption)
    {
        case 0: //����ˮ

            break;  
        case 1: //��ť1ˮ��
            if(waterTap_.pumpState == 0 && waterTap_.buttonState[0] == 1)   //��δ����
            {
                waterTap_.runFlag = 1;
                waterTap_.setOpenTime = waterTap_.vol[0];
                openPump1();
                log_v("water tap open pump");
                waterTap_.pumpState = 1;
                waterTap_.nowOpenTime = 0;
            }
            else
            {
                waterTap_.nowOpenTime++;
                if(waterTap_.nowOpenTime >= waterTap_.setOpenTime)
                {
                    log_v("water tap close pump");
                    closePump1();
                    waterTap_.pumpState = 0;
                    waterTap_.runFlag = 0;
                    waterTap_.nowVolumeOption = 0; //��ˮ���
                    waterTap_.buttonState[0] = 0; //����Ϊ�ر�״̬
                    waterTap_.nowOpenTime = 0;
                    SetButtonValue(WATERTAP_MAIN_PAGE,2,0,UART1_ID); //���ð�ť״̬
                    waterTapSetForeColor(8);
                    SetControlEnable(0,3,1,UART1_ID);
                    SetControlEnable(0,4,1,UART1_ID);
                    SetControlEnable(0,5,1,UART1_ID);
                    SetControlEnable(0,6,1,UART1_ID);
                    SetControlEnable(0,7,1,UART1_ID);
                }
                else if (waterTap_.buttonState[0] == 0)
                {
                    waterTap_.runFlag = 0;
                    waterTap_.nowOpenTime = 0;
                    waterTap_.pumpState = 0;
                    waterTap_.nowVolumeOption = 0; //��ˮ���
                    closePump1();
                    log_v("water tap close pump");
                }
            }
            break;
        case 2: //��ť2ˮ��
            if(waterTap_.pumpState == 0 && waterTap_.buttonState[1] == 1)   //��δ����
            {
                waterTap_.runFlag = 1;
                waterTap_.setOpenTime = waterTap_.vol[1];
                openPump1();
                log_v("water tap open pump");
                waterTap_.pumpState = 1;
                waterTap_.nowOpenTime = 0;
            }
            else
            {
                waterTap_.nowOpenTime++;
                if(waterTap_.nowOpenTime >= waterTap_.setOpenTime)
                {
                    closePump1();
                    log_v("water tap close pump");
                    waterTap_.pumpState = 0;
                    waterTap_.runFlag = 0;
                    waterTap_.nowVolumeOption = 0; //��ˮ���
                    waterTap_.buttonState[1] = 0; //����Ϊ�ر�״̬
                    waterTap_.nowOpenTime = 0;
                    SetButtonValue(WATERTAP_MAIN_PAGE,3,0,UART1_ID); //���ð�ť״̬
                    waterTapSetForeColor(9);
                    SetControlEnable(0,2,1,UART1_ID);
                    SetControlEnable(0,4,1,UART1_ID);
                    SetControlEnable(0,5,1,UART1_ID);
                    SetControlEnable(0,6,1,UART1_ID);
                    SetControlEnable(0,7,1,UART1_ID);
                }
                else if (waterTap_.buttonState[1] == 0)
                {
                    waterTap_.runFlag = 0;
                    waterTap_.nowOpenTime = 0;
                    waterTap_.pumpState = 0;
                    waterTap_.nowVolumeOption = 0; //��ˮ���
                    closePump1();
                    log_v("water tap close pump");
                }
            }
            break;
        case 3: //��ť3ˮ��
            if(waterTap_.pumpState == 0 && waterTap_.buttonState[2] == 1)   //��δ����
            {
                waterTap_.runFlag = 1;
                waterTap_.setOpenTime = waterTap_.vol[2];
                openPump1();
                waterTap_.pumpState = 1;
                waterTap_.nowOpenTime = 0;
            }
            else
            {
                waterTap_.nowOpenTime++;
                if(waterTap_.nowOpenTime >= waterTap_.setOpenTime)
                {
                    closePump1();
                    waterTap_.pumpState = 0;
                    waterTap_.runFlag = 0;
                    waterTap_.nowVolumeOption = 0; //��ˮ���
                    waterTap_.buttonState[2] = 0; //����Ϊ�ر�״̬
                    waterTap_.nowOpenTime = 0;
                    SetButtonValue(WATERTAP_MAIN_PAGE,4,0,UART1_ID); //���ð�ť״̬
                    waterTapSetForeColor(10);
                    SetControlEnable(0,2,1,UART1_ID);
                    SetControlEnable(0,3,1,UART1_ID);
                    SetControlEnable(0,5,1,UART1_ID);
                    SetControlEnable(0,6,1,UART1_ID);
                    SetControlEnable(0,7,1,UART1_ID);
                }
                else if (waterTap_.buttonState[2] == 0)
                {
                    waterTap_.runFlag = 0;
                    waterTap_.nowOpenTime = 0;
                    waterTap_.pumpState = 0;
                    waterTap_.nowVolumeOption = 0; //��ˮ���
                    closePump1();
                }
            }
            break;
        case 4: //����
            if(waterTap_.pumpState == 0 && waterTap_.buttonState[3] == 1)   //��δ����
            {
                waterTap_.runFlag = 1;
                openPump1();
                waterTap_.pumpState = 1;
            }
            else if(waterTap_.buttonState[3] == 0)
            {
                waterTap_.runFlag = 0;
                closePump1();
                waterTap_.pumpState = 0;
                waterTap_.nowVolumeOption = 0; //��ˮ���
            }   
            break;
        case 5: //��ϴ
            if(waterTap_.valveState == 0 && waterTap_.buttonState[4] == 1)   //��δ����
            {
                waterTap_.runFlag = 1;
                openWashValve();
                waterTap_.valveState = 1;
            }
            else if(waterTap_.buttonState[4] == 0)
            {
                waterTap_.runFlag = 0;
                closeWashValve();
                waterTap_.valveState = 0;
                waterTap_.nowVolumeOption = 0; //��ˮ���
            }
            break;
        default:
            break;
    }
}

