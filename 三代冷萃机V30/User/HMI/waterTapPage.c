/**
 * 
 * @file waterTapPage.c
 * @author jiaokai 
 * @brief ˮ��ͷҳ����Ӧ���� 
 * 
 * @copyright Copyright (c) 2025
 */
#include "waterTapPage.h"
#include "lcd_data_process.h"
#include "uart.h"
#include "string.h"
#include "waterLevel.h"
#include "eeprom.h"
#include "config.h"
#include "timer.h"
#include "work.h"
#include "waterTap.h"
#include "stdlib.h"
#include "elog.h"
#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "waterTapPage"

/**
 * ************************************************************************
 * @brief ˮ��ͷ��ҳ��ť��Ӧ����
 * 
 * @param[in] num   �ؼ����
 * @param[in] state  �ؼ�״̬
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void waterTapMainPageButton(uint8_t num,uint8_t state)
{
    switch(num)
    {
        case 2://ˮ��1
            if(state == 1)
            {
                if(waterTap_.nowVolumeOption == 0)   //��ǰû��ѡ��
                {
                    if(waterTap_.buttonState[0] == 0) //��ǰ�ǹر�״̬
                    {
                        waterTap_.nowVolumeOption = 1; //ѡ��ˮ��1
                        waterTap_.buttonState[0] = 1; //����Ϊ��״̬
                    }
                }
                //��ť1����
                log_v("button1 press");  
            }
            else
            {
                waterTap_.buttonState[0] = 0; //����Ϊ�ر�״̬
                log_v("button1 release");
            }
            break;
        case 3://ˮ��2
            if(state == 1)
            {
                if(waterTap_.nowVolumeOption == 0)   //��ǰû��ѡ��
                {
                    if(waterTap_.buttonState[1] == 0) //��ǰ�ǹر�״̬
                    {
                        waterTap_.nowVolumeOption = 2; //ѡ��ˮ��2
                        waterTap_.buttonState[1] = 1; //����Ϊ��״̬
                    }
                }
                //��ť2����
                log_v("button2 press");
            }
            else
            {
                waterTap_.buttonState[1] = 0; //����Ϊ�ر�״̬
                log_v("button2 release");
            }
            break;
        case 4://ˮ��3
            if(state == 1)
            {
                if(waterTap_.nowVolumeOption == 0)   //��ǰû��ѡ��
                {
                    if(waterTap_.buttonState[2] == 0) //��ǰ�ǹر�״̬
                    {
                        waterTap_.nowVolumeOption = 3; //ѡ��ˮ��3
                        waterTap_.buttonState[2] = 1; //����Ϊ��״̬
                    }
                }
                //��ť3����
                log_v("button3 press");
            }
            else
            {
                waterTap_.buttonState[2] = 0; //����Ϊ�ر�״̬
                log_v("button3 release");
            }
            break;
        case 5://����
            if(state == 1)
            {
                if(waterTap_.nowVolumeOption == 0 || waterTap_.nowVolumeOption == 4)   //��ǰû��ѡ��
                {
                    if(waterTap_.buttonState[3] == 0) //��ǰ�ǹر�״̬
                    {
                        waterTap_.nowVolumeOption = 4; //ѡ�񳣿�
                        waterTap_.buttonState[3] = 1; //����Ϊ��״̬
                        waterTapSetIcon(WATERTAP_MAIN_PAGE,11,1);
                        SetControlEnable(0,2,0,UART1_ID);
                        SetControlEnable(0,3,0,UART1_ID);
                        SetControlEnable(0,4,0,UART1_ID);
                        SetControlEnable(0,6,0,UART1_ID);
                        SetControlEnable(0,7,0,UART1_ID);
                    }
                    else
                    {
                        waterTap_.buttonState[3] = 0;
                        waterTapSetIcon(WATERTAP_MAIN_PAGE,11,0);
                        SetControlEnable(0,2,1,UART1_ID);
                        SetControlEnable(0,3,1,UART1_ID);
                        SetControlEnable(0,4,1,UART1_ID);
                        SetControlEnable(0,6,1,UART1_ID);
                        SetControlEnable(0,7,1,UART1_ID);
                    }
                }
            }
            break;
        case 6://��ϴ
            if(state == 1)
            {
                if(waterTap_.nowVolumeOption == 0|| waterTap_.nowVolumeOption == 5)
                {
                    if(waterTap_.buttonState[4] == 0)//�����ǹر�״̬
                    {
                        waterTap_.nowVolumeOption = 5;
                        waterTap_.buttonState[4] = 1;
                        waterTapSetIcon(WATERTAP_MAIN_PAGE,1,1);
                        SetControlEnable(0,2,0,UART1_ID);
                        SetControlEnable(0,3,0,UART1_ID);
                        SetControlEnable(0,4,0,UART1_ID);
                        SetControlEnable(0,5,0,UART1_ID);
                        SetControlEnable(0,7,0,UART1_ID);
                    }
                     else    //�ر�
                    {
                        waterTap_.buttonState[4] = 0;
                        waterTapSetIcon(WATERTAP_MAIN_PAGE,1,0);
                        SetControlEnable(0,2,1,UART1_ID);
                        SetControlEnable(0,3,1,UART1_ID);
                        SetControlEnable(0,4,1,UART1_ID);
                        SetControlEnable(0,5,1,UART1_ID);
                        SetControlEnable(0,7,1,UART1_ID);
                    }
                }
            }
            break;
        case 7:
            if(state == 1)
            {
                SetScreen(1,UART1_ID);  //�л������û���
            }
            break;
        default:
            break;
    }
}

/**
 * ************************************************************************
 * @brief ˮ��ͷ����ҳ�水ť��Ӧ����
 * 
 * @param[in] num  �ؼ����
 * @param[in] state  �ؼ�״̬
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void waterTapSetPageButton(uint8_t num,uint8_t state)
{
    switch(num)
    {
        case 2:
            if(state == 1)
            {
                for(uint8_t i = 0; i < 3; i++)
                {
                    config.data.waterTap.time[i] = waterTap_.setVol[i];
                    waterTap_.vol[i] = waterTap_.setVol[i];
                }
                updataWaterTapPageText(WATERTAP_MAIN_PAGE);
                SetScreen(WATERTAP_MAIN_PAGE,UART1_ID);  //�л�����ҳ
                write_config_data(CONFIG_WATER_TAP);
                log_d("water tap set save");
            }
        default:
            break;
    }
}


/**
 * ************************************************************************
 * @brief ˮ��ͷ����ҳ���ı����º���
 * 
 * @param[in] num  �ؼ����
 * @param[in] str  �ı�����
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void waterTapSetPageText(uint8_t num,uint8_t *str)
{
    float vol;
    switch(num)
    {
        case SET_VOL1_ID:
            vol = atof((char *)str);
            vol = (vol >= WATERTAP_VOLUME_MIN) ? vol : WATERTAP_VOLUME_MIN;
            vol = (vol <= WATERTAP_VOLUME_MAX) ? vol : WATERTAP_VOLUME_MAX;
            waterTap_.setVol[0] = (uint16_t)(vol*10.0f);
            break;
        case SET_VOL2_ID: //ʱ������
            vol = atoi((char *)str);
            vol = (vol >= WATERTAP_VOLUME_MIN) ? vol : WATERTAP_VOLUME_MIN;
            vol = (vol <= WATERTAP_VOLUME_MAX) ? vol : WATERTAP_VOLUME_MAX;
            waterTap_.setVol[1] = (uint16_t)(vol*10.0f);
            break;
        case SET_VOL3_ID: //ʱ������
            vol = atoi((char *)str);
            vol = (vol >= WATERTAP_VOLUME_MIN) ? vol : WATERTAP_VOLUME_MIN;
            vol = (vol <= WATERTAP_VOLUME_MAX) ? vol : WATERTAP_VOLUME_MAX;
            waterTap_.setVol[2] = (uint16_t)(vol*10.0f);
            break;
        default:
            break;
    }
}

/**
 * ************************************************************************
 * @brief ����ˮ��ͷ��Ļ�ı�
 * 
 * @param[in] page  ҳ����
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void updataWaterTapPageText(uint8_t page)
{
    uint8_t ID1,ID2,ID3,page_addr;
    char str[10];
    uint8_t str_len;
    uint8_t len;
    float time;
    if(page == WATERTAP_MAIN_PAGE)
    {
        page_addr = WATERTAP_MAIN_PAGE;
        ID1 = MAIN_VOL1_ID;
        ID2 = MAIN_VOL2_ID;
        ID3 = MAIN_VOL3_ID;
        
    }
    else if(page == WATERTAP_SET_PAGE)
    {
        page_addr = WATERTAP_SET_PAGE;
        ID1 = SET_VOL1_ID;
        ID2 = SET_VOL2_ID;
        ID3 = SET_VOL3_ID;
    }
    uart1_struct.tx_buf[0] = 0xEE;
    uart1_struct.tx_buf[1] = 0xB1;
    uart1_struct.tx_buf[2] = 0x12;
    uart1_struct.tx_buf[3] = 0x00;//ҳ���ַ
    uart1_struct.tx_buf[4] = page_addr;

    //��ť1
    uart1_struct.tx_buf[5] = 0x00;
    uart1_struct.tx_buf[6] = ID1;
    time = (float)waterTap_.vol[0]/10.0f;
    sprintf(str, "%.1f",  time);
    str_len = strlen(str);
    uart1_struct.tx_buf[7] = 0x00;
    uart1_struct.tx_buf[8] = str_len;
    for(int i=0;i<str_len;i++)
    {
        uart1_struct.tx_buf[9+i] = str[i];
    }
    len = 9+str_len;

    //��ť2 
    uart1_struct.tx_buf[len] = 0x00;
    uart1_struct.tx_buf[len+1] = ID2;
    time = (float)waterTap_.vol[1]/10.0f;
    sprintf(str, "%.1f",  time);
    str_len = strlen(str);
    uart1_struct.tx_buf[len+2] = 0x00;
    uart1_struct.tx_buf[len+3] = str_len;
    for(int i=0;i<str_len;i++)
    {
        uart1_struct.tx_buf[len+4+i] = str[i];
    }
    len += 4+str_len;

    //��ť3
    uart1_struct.tx_buf[len] = 0x00;
    uart1_struct.tx_buf[len+1] = ID3;
    time = (float)waterTap_.vol[2]/10.0f;
    sprintf(str, "%.1f",  time);
    str_len = strlen(str);
    uart1_struct.tx_buf[len+2] = 0x00;
    uart1_struct.tx_buf[len+3] = str_len;
    for(int i=0;i<str_len;i++)
    {   
        uart1_struct.tx_buf[len+4+i] = str[i];
    }
    len += 4+str_len;
    uart1_struct.tx_buf[len] = 0xFF;
    uart1_struct.tx_buf[len+1] = 0xFC;
    uart1_struct.tx_buf[len+2] = 0xFF;
    uart1_struct.tx_buf[len+3] = 0xFF;
    uart1_struct.tx_count = len+4;
    uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
}

/**
 * ************************************************************************
 * @brief ˮ��ͷ��Ļ����ͼ��
 * 
 * @param[in] screenID  ҳ��ID
 * @param[in] controlID  �ؼ�ID
 * @param[in] value  �ؼ�ֵ
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void waterTapSetIcon(uint16_t screenID,uint16_t controlID,uint8_t value)
{
    uart1_struct.tx_buf[0] = 0xEE;
    uart1_struct.tx_buf[1] = 0xB1;
    uart1_struct.tx_buf[2] = 0x23;
    uart1_struct.tx_buf[3] = screenID>>8;
    uart1_struct.tx_buf[4] = screenID;
    uart1_struct.tx_buf[5] = controlID>>8;
    uart1_struct.tx_buf[6] = controlID;
    uart1_struct.tx_buf[7] = value;
    uart1_struct.tx_buf[8] = 0xFF;
    uart1_struct.tx_buf[9] = 0xFC;
    uart1_struct.tx_buf[10] = 0xFF;
    uart1_struct.tx_buf[11] = 0xFF;
    uart1_struct.tx_count = 12;
    uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
}

/**
 * ************************************************************************
 * @brief ˮ��ͷ��Ļ����������ɫ
 * 
 * @param[in] num  �ؼ����
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-14
 * 
 * ************************************************************************
 */
void waterTapSetForeColor(uint8_t num)
{
    uart1_struct.tx_buf[0] = 0xEE;
    uart1_struct.tx_buf[1] = 0xB1;
    uart1_struct.tx_buf[2] = 0x19;
    uart1_struct.tx_buf[3] = 0x00;
    uart1_struct.tx_buf[4] = 0x00;
    uart1_struct.tx_buf[5] = 0x00;
    uart1_struct.tx_buf[6] = num;
    uart1_struct.tx_buf[7] = 0x9C;
    uart1_struct.tx_buf[8] = 0x92;
    uart1_struct.tx_buf[9] = 0xFF;
    uart1_struct.tx_buf[10] = 0xFC;
    uart1_struct.tx_buf[11] = 0xFF;
    uart1_struct.tx_buf[12] = 0xFF;
    uart1_struct.tx_count = 13;
    uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
}



