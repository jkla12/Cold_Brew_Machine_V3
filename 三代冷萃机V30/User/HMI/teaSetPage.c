/**
 * 
 * @file teaSetPage.c
 * @author jiaokai 
 * @brief ����ȡ����ҳ����Ӧ
 * 
 * @copyright Copyright (c) 2025
 */
#include "teaSetPage.h"
#include "lcd_data_process.h"
#include "waterLevel.h"
#include "uart.h"
#include "string.h"
#include "teaMake.h"
#include "teaExtractionPage.h"
#include "work.h"
#include "stdlib.h"
#include "eeprom.h"
#include "config.h"
#include "elog.h"
#include <stddef.h> // For offsetof macro


#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "teaSetPage"

uint8_t saveTeaDataChangeFlag = 0; //���ݱ����־

/**
 * ************************************************************************
 * @brief ������ҳ�水ť��Ӧ����
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
void teaSetPageButton(uint8_t num,uint8_t state)
{
    char str[10];
    switch(num)
    {
        case 4://������ҳ��
            //������ݱ��,�������ݵ�EEPROM
            if(saveTeaDataChangeFlag == 1)
            {
                saveTeaDataChangeFlag = 0; //������ݱ����־
                write_config_data(CONFIG_TEA_MAKE);
            }
            else if(config.data.teaMake.currentNumber != tea.num)
            {
                config.data.teaMake.currentNumber = tea.num;
                saveTeaCurrentNum();
            }
            SetScreen(main_page,UART2_ID);
            break;
        case 5: //��������ҳ��
            if(saveTeaDataChangeFlag == 1)
            {
                saveTeaDataChangeFlag = 0; //������ݱ����־
                write_config_data(CONFIG_TEA_MAKE);
            }
            else if(config.data.teaMake.currentNumber != tea.num)
            {
                config.data.teaMake.currentNumber = tea.num;
                saveTeaCurrentNum();
            }
            if(waterLevel.sensorErr == 1)   //����������
            {
                SetScreen(water_level_page,UART2_ID);
            }
            else
            {
                enterTeaExtractionPage();
                SetScreen(make_tea_page,UART2_ID);
            }
            break;
        case 9:     //Һ������
            if(config.data.teaMake.vol[tea.num]> config.data.waterVolumeLow)
            {
                int16_t newVol = config.data.teaMake.vol[tea.num] - config.data.waterChangeVal;
                config.data.teaMake.vol[tea.num] = (newVol >= config.data.waterVolumeLow) ? newVol : config.data.waterVolumeLow;
            }
            sprintf(str,"%d.%d",config.data.teaMake.vol[tea.num]/10,config.data.teaMake.vol[tea.num]%10);
            SetTextValue(make_tea_set_page,vol_addr,(unsigned char *)str,UART2_ID);
            saveTeaDataChangeFlag = 1; //���ݱ����־
            break;
        case 10:    //Һ������
            if(config.data.teaMake.vol[tea.num]< config.data.waterVolumeHigh)
            {
                int16_t newVol = config.data.teaMake.vol[tea.num] + config.data.waterChangeVal;
                config.data.teaMake.vol[tea.num] = (newVol <= config.data.waterVolumeHigh) ? newVol : config.data.waterVolumeHigh;
            }
            sprintf(str,"%d.%d",config.data.teaMake.vol[tea.num]/10,config.data.teaMake.vol[tea.num]%10);
                SetTextValue(make_tea_set_page,vol_addr,(unsigned char *)str,UART2_ID);
            saveTeaDataChangeFlag = 1; //���ݱ����־
            break;
        case 11:    //ʱ�����
            if(config.data.teaMake.time[tea.num] > EXTRACTION_MIN_TIME)
            {
                int16_t newTime = config.data.teaMake.time[tea.num] - config.data.makeChangeTime;
                config.data.teaMake.time[tea.num] = (newTime >= EXTRACTION_MIN_TIME) ? newTime : EXTRACTION_MIN_TIME;
            }
            log_v("time:%d",config.data.teaMake.time[tea.num]);
            sprintf(str,"%d",config.data.teaMake.time[tea.num]);
            SetTextValue(make_tea_set_page,time_addr,(unsigned char *)str,UART2_ID);
            saveTeaDataChangeFlag = 1; //���ݱ����־
            break;
        case 12:    //ʱ������
            if(config.data.teaMake.time[tea.num] < EXTRACTION_MAX_TIME)
            {
                int16_t newTime = config.data.teaMake.time[tea.num] + config.data.makeChangeTime;
                config.data.teaMake.time[tea.num] = (newTime <= EXTRACTION_MAX_TIME) ? newTime : EXTRACTION_MAX_TIME;
            }
            log_v("time:%d",config.data.teaMake.time[tea.num]);
            sprintf(str,"%d",config.data.teaMake.time[tea.num]);
            SetTextValue(make_tea_set_page,time_addr,(unsigned char *)str,UART2_ID);
            saveTeaDataChangeFlag = 1; //���ݱ����־
            break;
        case 18:  //�ղ�
            teaCollectChange(tea.num,state);
            break;
        case 19:  //��ˮѡ��
            if(state == 0)  //�����Զ���ˮ
            {
                config.data.teaMake.autoDrainangeFlag[tea.num] = 1;
                log_v("tea auto drain:%d",config.data.teaMake.autoDrainangeFlag[tea.num]);
            }
            else    //�ر��Զ���ˮ
            {
                config.data.teaMake.autoDrainangeFlag[tea.num] = 0;
                log_v("tea auto drain:%d",config.data.teaMake.autoDrainangeFlag[tea.num]);
            }
            saveTeaDataChangeFlag = 1; //���ݱ����־
            break;
        case 20:  //��ˮѡ��
            if(state == 0)  //�ر��Զ���ˮ
            {
                config.data.teaMake.autoDrainangeFlag[tea.num] = 0;
                log_v("tea auto drain:%d",config.data.teaMake.autoDrainangeFlag[tea.num]);
            }
            else    //�����Զ���ˮ
            {
                config.data.teaMake.autoDrainangeFlag[tea.num] = 1;
                log_v("tea auto drain:%d",config.data.teaMake.autoDrainangeFlag[tea.num]);
            }
            saveTeaDataChangeFlag = 1; //���ݱ����־
            break;
        case collect1_button_addr:
            teaCollectChoose(collect1_button_addr);
            break;
        case collect2_button_addr:
            teaCollectChoose(collect2_button_addr);
            break;
        case collect3_button_addr:
            teaCollectChoose(collect3_button_addr);
            break;
        case collect4_button_addr:
            teaCollectChoose(collect4_button_addr);
            break;
        case 26: //���ؼ���
            if(config.data.teaMake.weight[tea.num] > config.data.weightMin)
            {
                int16_t newWeight = config.data.teaMake.weight[tea.num] - config.data.weightChangeVal;
                config.data.teaMake.weight[tea.num] = (newWeight >= config.data.weightMin) ? newWeight : config.data.weightMin;
            }
            log_v("weight:%d",config.data.teaMake.weight[tea.num]);
            sprintf(str,"%d",config.data.teaMake.weight[tea.num]);
            SetTextValue(make_tea_set_page,weight_addr,(unsigned char *)str,UART2_ID);
            saveTeaDataChangeFlag = 1; //���ݱ����־
            break;
        case 27: //��������
            if(config.data.teaMake.weight[tea.num] < config.data.weightMax)
            {
                int16_t newWeight = config.data.teaMake.weight[tea.num] + config.data.weightChangeVal;
                config.data.teaMake.weight[tea.num] = (newWeight <= config.data.weightMax) ? newWeight : config.data.weightMax;
            }
            log_v("weight:%d",config.data.teaMake.weight[tea.num]);
            sprintf(str,"%d",config.data.teaMake.weight[tea.num]);
            SetTextValue(make_tea_set_page,weight_addr,(unsigned char *)str,UART2_ID);
            saveTeaDataChangeFlag = 1; //���ݱ����־
            break;
        default:
            break;
    }
}

/**
 * ************************************************************************
 * @brief  ������ҳ���ı���Ӧ����
 * 
 * @param[in] num  �ؼ����
 * @param[in] str  �ؼ��ı�
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void teaSetPageText(uint8_t num,uint8_t *str)
{
   // char str1[10];
    uint8_t val[2];
    switch(num)
    {
        case show_addr:
            val[0] = atoi((char *)str); //���ַ���תΪ����
            //�޷�
            val[0] = (val[0] >=1 )? val[0] : 1;
            val[0] = (val[0] <=10 )? val[0] : 10;
            // config.data.teaMake.currentNumber = val[0]-1;
            // tea.num = config.data.teaMake.currentNumber;
            tea.num = val[0]-1;
            loadingTeaMakeSet();
            log_v("tea make num:%d",tea.num);
            break;
        default:
            break;
    }
}

void saveTeaCurrentNum(void)
{
    //���浱ǰ���
    eeprom_buffer_write_timeout((uint8_t *)&config.data.teaMake.currentNumber,offsetof(config_TypeDef,teaMake.currentNumber) ,sizeof(config.data.teaMake.currentNumber));
}

/**
 * ************************************************************************
 * @brief  ���²��ղ�״̬
 * 
 * @param[in] num  ���
 * @param[in] state  ״̬��0ȡ���ղأ�1�ղ�
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void teaCollectChange(uint8_t num,uint8_t state)
{
    if(state == 1)  //�ղ�
    {
        if(config.data.teaMake.collectFlag[NUMBER_OF_COLLECTIONS-1] == 0)   //���һ���ղ�λΪ��
        {
            config.data.teaMake.collectFlag[NUMBER_OF_COLLECTIONS-1] = num+1;
            log_v("tea collect num:%d",config.data.teaMake.collectFlag[NUMBER_OF_COLLECTIONS-1]);
        }
        else if(find_num(config.data.teaMake.collectFlag, num+1) == 0)   //û���ղع�
        {
            for(uint8_t i=0;i<NUMBER_OF_COLLECTIONS;i++)
            {
                config.data.teaMake.collectFlag[i] = config.data.teaMake.collectFlag[i+1];
            }
            config.data.teaMake.collectFlag[NUMBER_OF_COLLECTIONS-1] = num+1;
        }   
    }
    else    //ȡ���ղ�
    {
        if(find_num(config.data.teaMake.collectFlag, num+1) == 1) //����ղ��б��д��ڸ��ղ�
        {
            for(uint8_t i = 0;i<NUMBER_OF_COLLECTIONS;i++)
            {
                if(config.data.teaMake.collectFlag[i] == num+1)
                {
                    for(uint8_t j = i;j<NUMBER_OF_COLLECTIONS-1;j++)
                    {
                        config.data.teaMake.collectFlag[j] = config.data.teaMake.collectFlag[j+1];
                    }
                    config.data.teaMake.collectFlag[NUMBER_OF_COLLECTIONS-1] = 0;
                }
            }
        }
    }
    updateTeaCollect();
    saveTeaDataChangeFlag = 1;
}

/**
 * ************************************************************************
 * @brief  �����ղ����ݵ���Ļ
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void updateTeaCollect(void)
{
    char str[10];
    uint8_t len;
    uint8_t strLen;
    uart2_struct.tx_buf[0] = 0xEE;
    uart2_struct.tx_buf[1] = 0xB1;
    uart2_struct.tx_buf[2] = 0x12;
    uart2_struct.tx_buf[3] = 0x00;
    uart2_struct.tx_buf[4] = make_tea_set_page;
    len = 5;
    for(uint8_t i=0;i<NUMBER_OF_COLLECTIONS;i++)
    {
        uart2_struct.tx_buf[len] = 0x00;
        uart2_struct.tx_buf[len+1] = collect1_addr+i;
        uart2_struct.tx_buf[len+2] = 0x00;
        memset(str,0,sizeof(str));
        sprintf(str,"%d",config.data.teaMake.collectFlag[i]);
        strLen = strlen(str);
        uart2_struct.tx_buf[len+3] = strLen;
        for(uint8_t j=0;j<strLen;j++)
        {
            uart2_struct.tx_buf[len+4+j] = str[j];
        }
        len = len+4+strLen;
    }
    uart2_struct.tx_buf[len] = 0xFF;    //֡β
    uart2_struct.tx_buf[len+1] = 0xFC;
    uart2_struct.tx_buf[len+2] = 0xFF;
    uart2_struct.tx_buf[len+3] = 0xFF;
    uart2_struct.tx_count = len+4;
    uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);
}

/**
 * ************************************************************************
 * @brief ������ղ����ݵ�EEPROM
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void saveTeaCollect(void)
{
    //������ղ�����
    eeprom_buffer_write_timeout((uint8_t *)&config.data.teaMake.collectFlag[0],offsetof(config_TypeDef,teaMake.collectFlag) ,sizeof(config.data.teaMake.collectFlag));
}

/**
 * ************************************************************************
 * @brief  ѡ���ղر��
 * 
 * @param[in] num  ���
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void teaCollectChoose(uint8_t num)
{
    char str[10];
    tea.num = config.data.teaMake.collectFlag[num-collect1_button_addr]-1;
    //config.data.teaMake.currentNumber = tea.num;
    sprintf(str,"%d",tea.num+1);
    SetTextValue(make_tea_set_page,show_addr,(unsigned char *)str,UART2_ID);
    loadingTeaMakeSet();
}

/**
 * ************************************************************************
 * @brief  ���������ҳ������
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-13
 * 
 * ************************************************************************
 */
void loadingTeaMakeSet(void)
{
    char str[10];
    uint8_t len;
    uint8_t strLen;
    uart2_struct.tx_buf[0] = 0xEE;
    uart2_struct.tx_buf[1] = 0xB1;
    uart2_struct.tx_buf[2] = 0x12;
    uart2_struct.tx_buf[3] = 0x00;
    uart2_struct.tx_buf[4] = make_tea_set_page;
    uart2_struct.tx_buf[5] = 0x00;
    uart2_struct.tx_buf[6] = vol_addr;
    uart2_struct.tx_buf[7] = 0x00;
    uart2_struct.tx_buf[8] = 0x03;  //���ݳ���
    len = 9;
    /************************************д��ˮ��************************************ */
    //�����޷�
    config.data.teaMake.vol[tea.num] = (config.data.teaMake.vol[tea.num]>=config.data.waterVolumeLow)?config.data.teaMake.vol[tea.num]:config.data.waterVolumeLow;
    config.data.teaMake.vol[tea.num] = (config.data.teaMake.vol[tea.num]<=config.data.waterVolumeHigh)?config.data.teaMake.vol[tea.num]:config.data.waterVolumeHigh;
    sprintf(str,"%d.%d",config.data.teaMake.vol[tea.num]/10,config.data.teaMake.vol[tea.num]%10);
    uart2_struct.tx_buf[9] = str[0];
    uart2_struct.tx_buf[10] = str[1];
    uart2_struct.tx_buf[11] = str[2];
    len = len+3;
    /************************************д��ʱ��************************************ */
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len+1] = time_addr;
    uart2_struct.tx_buf[len+2] = 0x00;
    //�����޷�
    config.data.teaMake.time[tea.num] = (config.data.teaMake.time[tea.num]>=EXTRACTION_MIN_TIME)?config.data.teaMake.time[tea.num]:EXTRACTION_MIN_TIME;
    config.data.teaMake.time[tea.num] = (config.data.teaMake.time[tea.num]<=EXTRACTION_MAX_TIME)?config.data.teaMake.time[tea.num]:EXTRACTION_MAX_TIME;
    memset(str,0,sizeof(str));
    sprintf(str,"%d",config.data.teaMake.time[tea.num]);
    strLen = strlen(str);
    uart2_struct.tx_buf[len+3] = strLen;
    for(uint8_t i=0;i<strLen;i++)
    {
        uart2_struct.tx_buf[len+4+i] = str[i];
    }
    len = len+4+strLen;
    /************************************д�����************************************ */
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len+1] = weight_addr;
    uart2_struct.tx_buf[len+2] = 0x00;
    //�����޷�
    config.data.teaMake.weight[tea.num] = (config.data.teaMake.weight[tea.num]<=WEIGHT_MAX)?config.data.teaMake.weight[tea.num]:WEIGHT_MAX;
    memset(str,0,sizeof(str));
    sprintf(str,"%d",config.data.teaMake.weight[tea.num]);
    strLen = strlen(str);
    uart2_struct.tx_buf[len+3] = strLen;
    for(uint8_t i=0;i<strLen;i++)
    {
        uart2_struct.tx_buf[len+4+i] = str[i];
    }
    len = len+4+strLen;
    /************************************д���Ƿ��ղ�************************************ */
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len+1] = 0x12;
    uart2_struct.tx_buf[len+2] = 0x00;
    uart2_struct.tx_buf[len+3] = 0x01;
    if(find_num(config.data.teaMake.collectFlag, tea.num+1) == 1) //����ղ��б��д��ڸ��ղ�
    {
        uart2_struct.tx_buf[len+4] = 0x01;
    }
    else
    {
        uart2_struct.tx_buf[len+4] = 0x00;
    }
    len = len+5;
    /************************************д���Ƿ��Զ���ˮ************************************ */
    uart2_struct.tx_buf[len] = 0x00;
    uart2_struct.tx_buf[len+1] = 0x13;
    uart2_struct.tx_buf[len+2] = 0x00;
    uart2_struct.tx_buf[len+3] = 0x01;
    if(config.data.teaMake.autoDrainangeFlag[tea.num] == 1) //����ղ��б��д��ڸ��ղ�
    {
        uart2_struct.tx_buf[len+4] = 0x00;
        uart2_struct.tx_buf[len+5] = 0x00;
        uart2_struct.tx_buf[len+6] = 0x14;
        uart2_struct.tx_buf[len+7] = 0x00;
        uart2_struct.tx_buf[len+8] = 0x01;
        uart2_struct.tx_buf[len+9] = 0x01;
    }
    else
    {
        uart2_struct.tx_buf[len+4] = 0x01;
        uart2_struct.tx_buf[len+5] = 0x00;
        uart2_struct.tx_buf[len+6] = 0x14;
        uart2_struct.tx_buf[len+7] = 0x00;
        uart2_struct.tx_buf[len+8] = 0x01;
        uart2_struct.tx_buf[len+9] = 0x00;
    }
    len = len+10;
    uart2_struct.tx_buf[len] = 0xFF;    //֡β
    uart2_struct.tx_buf[len+1] = 0xFC;
    uart2_struct.tx_buf[len+2] = 0xFF;
    uart2_struct.tx_buf[len+3] = 0xFF;
    uart2_struct.tx_count = len+4;
    uart2_dma_send(uart2_struct.tx_buf,uart2_struct.tx_count);
}
