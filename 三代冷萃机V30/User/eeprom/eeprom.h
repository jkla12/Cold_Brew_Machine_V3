#ifndef __EEPROM_H
#define __EEPROM_H

#include "gd32f30x.h"
#include "config.h"

typedef enum {
    I2C_START = 0,
    I2C_SEND_ADDRESS,
    I2C_CLEAR_ADDRESS_FLAG,
    I2C_TRANSMIT_DATA,
    I2C_TRANSMIT_DATA2,
    I2C_STOP,
} i2c_process_enum;




#pragma pack(push, 1)  // ǿ��1�ֽڶ���
typedef struct
{
    uint8_t     vol[NUMBER_OF_PARAMETERS];                  //ˮ�� ��λ.0��
    uint8_t     time[NUMBER_OF_PARAMETERS];                 //ʱ��  ��λ������
    uint16_t    weight[NUMBER_OF_PARAMETERS];               //����  ��λ����
    uint8_t     autoDrainangeFlag[NUMBER_OF_PARAMETERS];    //�Զ���ˮ��־
    uint8_t     collectFlag[NUMBER_OF_COLLECTIONS];         //�ղر�־
    uint8_t     currentNumber;                              //��ǰ���
}make_set_TypeDef;

typedef struct
{
    uint16_t time[WATERTAP_NUMBER_OF_PARAMETERS];          //ʱ��  ��λ��0.1��
    float pumpRatio;                                       //ˮ��100mL��ˮʱ�� Ԥ��
}waterTap_TypeDef;

//���ò����ṹ��
typedef struct 
{
    uint8_t version;                    // �汾��
    uint8_t isLock;                     //�Ƿ�����      0 ����  1 ����  
    uint8_t language;                   //��������      0 ����  1 Ӣ��
    uint8_t zeroLevel;                  //��λˮλ     ��λ 0.1��
    uint8_t coffeeCompensateLevel;      //���Ȳ���ˮλ ��λ 0.1��
    uint8_t teaCompensateLevel;         //�貹��ˮλ ��λ 0.1��
    uint16_t inletOverTime;             //��ˮ��ʱʱ�� ��λ ��
    uint16_t drainOverTime;             //��ˮ��ʱʱ��  ��λ ��
    uint16_t circulationValveSwicthTime;//ѭ��������ʱ�� ��λ ��
    uint16_t drainValveSwicthTime;      //��ˮ������ʱ�� ��λ ��
    uint8_t drainDelayTime;             //��ˮ����ӳ�ʱ�䣬�����ſչܵ�Һ�� ��λ ��
    uint8_t washDrainRepeat;            //��ϴ����ˮ���� ��2��ˮ����2��ˮ ��λ ��
    uint8_t washSingleTime;             //������ϴʱ��  ��λ ��
    uint8_t washPauseTime;              //��ϴ��ͣʱ��  ��λ ��
    uint8_t washLoopTimes;              //������ϴ��ͣѭ������ ��λ ��
    uint8_t washFirstVolume;            //��һ����ϴҺ�� ��λ0.1��
    uint8_t washSecondVolume;           //�ڶ�����ϴҺ�� ��λ0.1��
    uint16_t washTime;                  //��ϴ��ʱ�� ��λ ��
    uint8_t waterVolumeLow;             //����Һ����Сֵ
    uint8_t waterVolumeHigh;            //����Һ�����ֵ
    uint8_t waterChangeVal;             //����Һ���޸Ĳ���ֵ
    uint8_t makeChangeTime;             //����ʱ���޸Ĳ���ֵ ��λ ����
    uint16_t weightMin;                  //����������Сֵ
    uint16_t weightMax;                  //�����������ֵ
    uint8_t weightChangeVal;            //���������޸Ĳ���ֵ  
    uint8_t drainTime;                  //��ˮʱ�� ��λ ��
    make_set_TypeDef coffeeMake;        //������������
    make_set_TypeDef teaMake;           //����������
    waterTap_TypeDef waterTap;          //ˮ��ͷ����
}config_TypeDef;

typedef struct
{
    uint8_t isRegistered;               //�Ƿ�ע��
    char ETID[5];                       //�豸���ID    4λ     ע��ʹ��
    char ESN[14];                       //�豸���к�    13λ    ע��ʹ��
    char ClientID[29];                  //�ͻ�ID        28λ    ����������
    char UserName[14];                  //�û���        13λ    ����������
    char Password[65];                  //����          64λ    ����������
    char pubTopic[100];                 //��������
    char subTopic[100];                 //��������
    char registerAddress[100];          //ע���ַ
    char serverAddress[100];            //MQTT��������ַ
}MQTT_TypeDef;

//TODO
typedef struct { 
    uint8_t HardwareVersion;             //Ӳ���汾�� ����10��V1.0��20��V2.0
    uint8_t FirmwareVersion;             //����汾��   
    char DeviceID[14];                   //�豸ID��   13λ
    char DateOfManufacture[11];          //�������� ���10λ 2025/5/1
    MQTT_TypeDef MQTT;                   //MQTT���
} deviceInfo_TypeDef;

//��¼���ݽṹ��
typedef struct
{
    uint32_t coffeeMakeCnt;             //������������
    uint32_t teaMakeCnt;                //����������
    uint32_t washCnt;                   //��ϴ����
    uint32_t sanitCnt;                  //��������
    uint32_t circulationValveCnt;       //ѭ����ͨ�������д���     
    uint32_t drainValveCnt;             //��ˮ��ͨ�������д���
    uint32_t circulationPumpRunTime;    //ѭ��������ʱ�� ��λ������
    uint32_t waterInletValveCnt;        //��ˮ��ŷ����д���
    uint32_t washValveCnt;              //ˮ��ͷ��ϴ��ŷ����д���
    uint32_t waterTapwashCnt;           //ˮ��ͷ��ϴ����
    uint32_t DCPumpRunTime;             //ֱ��ˮ������ʱ��
    uint32_t coolingRunTime;            //��������ʱ�䣬�����������һ���ӿ� ��λ������
}record_TypeDef;




#pragma pack(pop)     // �ָ�Ĭ�϶��뷽ʽ


typedef union{
    config_TypeDef data;
    uint8_t bytes[sizeof(config_TypeDef)];
}config_Union;

typedef union{
    record_TypeDef data;
    uint8_t bytes[sizeof(record_TypeDef)];
}record_Union;

typedef union{
    deviceInfo_TypeDef data;
    uint8_t bytes[sizeof(deviceInfo_TypeDef)];
}deviceInfo_Union;


extern config_Union config;
extern record_Union record;
extern deviceInfo_Union deviceInfo;

// �����������ݲ��ֱ�ʶ��
typedef enum {
    CONFIG_VERSION,
    CONFIG_IS_LOCK,
    CONFIG_ZERO_LEVEL,
    CONFIG_COFFEE_COMPENSATE_LEVEL,
    CONFIG_TEA_COMPENSATE_LEVEL,
    CONFIG_INLET_OVER_TIME,
    CONFIG_DRAIN_OVER_TIME,
    CONFIG_CIRCULATION_VALVE_SWITCH_TIME,
    CONFIG_DRAIN_VALVE_SWITCH_TIME,
    CONFIG_DRAIN_DELAY_TIME,
    CONFIG_WASH_DRAIN_REPEAT,
    CONFIG_WASH_SINGLE_TIME,
    CONFIG_WASH_PAUSE_TIME,
    CONFIG_WASH_LOOP_TIMES,
    CONFIG_WASH_FIRST_VOLUME,
    CONFIG_WASH_SECOND_VOLUME,
    CONFIG_WASH_TIME,
    CONFIG_COFFEE_MAKE,
    CONFIG_TEA_MAKE,
    CONFIG_WATER_TAP,
    CONFIG_WATER_VOLUME_LOW,
    CONFIG_WATER_VOLUME_HIGH,
    CONFIG_WATER_CHANGE_VAL,
    CONFIG_WEIGHT_MIN,
    CONFIG_WEIGHT_MAX,
    CONFIG_WEIGHT_CHANGE_VAL,
    CONFIG_MAKE_CHANGE_TIME,
    CONFIG_ALL // ����������������
} config_DataPart;

// �����¼���ݲ��ֱ�ʶ��
typedef enum {
    RECORD_COFFEE_MAKE_CNT,
    RECORD_TEA_MAKE_CNT,
    RECORD_WASH_CNT,
    RECORD_SANIT_CNT,
    RECORD_CIRCULATION_VALVE_CNT,
    RECORD_DRAIN_VALVE_CNT,
    RECORD_CIRCULATION_PUMP_RUN_TIME,
    RECORD_WATER_INLET_VALVE_CNT,
    RECORD_WASH_VALVE_CNT,
    RECORD_WATER_TAP_WASH_CNT,
    RECORD_DC_PUMP_RUN_TIME,
    RECORD_COOLING_RUN_TIME,
    RECORD_ALL // �������м�¼����
} record_DataPart;

//�����豸��Ϣ���ݲ��ֱ�ʶ��
typedef enum {
    DEVICE_INFO_HARDWARE_VERSION,
    DEVICE_INFO_FIRMWARE_VERSION,
    DEVICE_INFO_DEVICE_ID,
    DEVICE_INFO_DATE_OF_MANUFACTURE,
    DEVICE_INFO_MQTT,
    DEVICE_INFO_ALL // ���������豸��Ϣ����
} deviceInfo_DataPart;




#define I2C_TIME_OUT           (uint16_t)(5000)
#define EEP_FIRST_PAGE         0x00

#define I2C_OK                 1
#define I2C_FAIL               0
#define I2C_END                1

/* I2C read and write functions */
uint8_t i2c_24c02_test(void);
/* initialize EEPROM address */
void i2c_eeprom_init(void);
/* write one byte to the EEPROM and use timeout function */
uint8_t eeprom_byte_write_timeout(uint8_t *p_buffer, uint16_t write_address);
/* write more than one byte to the EEPROM use timeout function */
uint8_t eeprom_page_write_timeout(uint8_t *p_buffer, uint16_t write_address, uint8_t number_of_byte);
/* write buffer of data to the EEPROM use timeout function */
void eeprom_buffer_write_timeout(uint8_t *p_buffer, uint16_t write_address, uint16_t number_of_byte);
/* read data from the EEPROM use timeout function */
uint8_t eeprom_buffer_read_timeout(uint8_t *p_buffer, uint16_t read_address, uint16_t number_of_byte);
/* wait for EEPROM standby state use timeout function */
uint8_t  eeprom_wait_standby_state_timeout(void);

void readSaveData(void);
void writeConfigData(void);
void writeRecordData(void);

void write_config_data(config_DataPart part);
void write_record_data(record_DataPart part);
void write_device_info_data(deviceInfo_DataPart part);
void read_device_info_data(deviceInfo_DataPart part);

#endif


