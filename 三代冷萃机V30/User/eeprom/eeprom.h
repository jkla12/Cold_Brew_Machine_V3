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




#pragma pack(push, 1)  // 强制1字节对齐
typedef struct
{
    uint8_t     vol[NUMBER_OF_PARAMETERS];                  //水量 单位.0升
    uint8_t     time[NUMBER_OF_PARAMETERS];                 //时间  单位：分钟
    uint16_t    weight[NUMBER_OF_PARAMETERS];               //重量  单位：克
    uint8_t     autoDrainangeFlag[NUMBER_OF_PARAMETERS];    //自动排水标志
    uint8_t     collectFlag[NUMBER_OF_COLLECTIONS];         //收藏标志
    uint8_t     currentNumber;                              //当前编号
}make_set_TypeDef;

typedef struct
{
    uint16_t time[WATERTAP_NUMBER_OF_PARAMETERS];          //时间  单位：0.1秒
    float pumpRatio;                                       //水泵100mL出水时间 预留
}waterTap_TypeDef;

//设置参数结构体
typedef struct 
{
    uint8_t version;                    // 版本号
    uint8_t isLock;                     //是否锁定      0 解锁  1 锁定  
    uint8_t language;                   //语言设置      0 中文  1 英文
    uint8_t zeroLevel;                  //零位水位     单位 0.1升
    uint8_t coffeeCompensateLevel;      //咖啡补偿水位 单位 0.1升
    uint8_t teaCompensateLevel;         //茶补偿水位 单位 0.1升
    uint16_t inletOverTime;             //进水超时时间 单位 秒
    uint16_t drainOverTime;             //排水超时时间  单位 秒
    uint16_t circulationValveSwicthTime;//循环阀开关时间 单位 秒
    uint16_t drainValveSwicthTime;      //排水阀开关时间 单位 秒
    uint8_t drainDelayTime;             //排水完毕延迟时间，用以排空管道液体 单位 秒
    uint8_t washDrainRepeat;            //清洗总排水次数 上2次水，排2次水 单位 次
    uint8_t washSingleTime;             //单次清洗时间  单位 秒
    uint8_t washPauseTime;              //清洗暂停时间  单位 秒
    uint8_t washLoopTimes;              //单次清洗暂停循环次数 单位 次
    uint8_t washFirstVolume;            //第一次清洗液量 单位0.1升
    uint8_t washSecondVolume;           //第二次清洗液量 单位0.1升
    uint16_t washTime;                  //清洗总时间 单位 秒
    uint8_t waterVolumeLow;             //设置液量最小值
    uint8_t waterVolumeHigh;            //设置液量最大值
    uint8_t waterChangeVal;             //设置液量修改步进值
    uint8_t makeChangeTime;             //设置时间修改步进值 单位 分钟
    uint16_t weightMin;                  //设置重量最小值
    uint16_t weightMax;                  //设置重量最大值
    uint8_t weightChangeVal;            //设置重量修改步进值  
    uint8_t drainTime;                  //排水时间 单位 秒
    make_set_TypeDef coffeeMake;        //咖啡制作设置
    make_set_TypeDef teaMake;           //茶制作设置
    waterTap_TypeDef waterTap;          //水龙头设置
}config_TypeDef;

typedef struct
{
    uint8_t isRegistered;               //是否注册
    char ETID[5];                       //设备类别ID    4位     注册使用
    char ESN[14];                       //设备序列号    13位    注册使用
    char ClientID[29];                  //客户ID        28位    服务器反馈
    char UserName[14];                  //用户名        13位    服务器反馈
    char Password[65];                  //密码          64位    服务器反馈
    char pubTopic[100];                 //发布主题
    char subTopic[100];                 //订阅主题
    char registerAddress[100];          //注册地址
    char serverAddress[100];            //MQTT服务器地址
}MQTT_TypeDef;

//TODO
typedef struct { 
    uint8_t HardwareVersion;             //硬件版本号 例，10，V1.0；20，V2.0
    uint8_t FirmwareVersion;             //软件版本号   
    char DeviceID[14];                   //设备ID号   13位
    char DateOfManufacture[11];          //出厂日期 最多10位 2025/5/1
    MQTT_TypeDef MQTT;                   //MQTT相关
} deviceInfo_TypeDef;

//记录数据结构体
typedef struct
{
    uint32_t coffeeMakeCnt;             //咖啡制作次数
    uint32_t teaMakeCnt;                //茶制作次数
    uint32_t washCnt;                   //清洗次数
    uint32_t sanitCnt;                  //消毒次数
    uint32_t circulationValveCnt;       //循环三通阀门运行次数     
    uint32_t drainValveCnt;             //排水三通阀门运行次数
    uint32_t circulationPumpRunTime;    //循环泵运行时间 单位：分钟
    uint32_t waterInletValveCnt;        //进水电磁阀运行次数
    uint32_t washValveCnt;              //水龙头清洗电磁阀运行次数
    uint32_t waterTapwashCnt;           //水龙头清洗次数
    uint32_t DCPumpRunTime;             //直流水泵运行时间
    uint32_t coolingRunTime;            //制冷运行时间，制冷和消毒是一个接口 单位：分钟
}record_TypeDef;




#pragma pack(pop)     // 恢复默认对齐方式


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

// 定义配置数据部分标识符
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
    CONFIG_ALL // 保存所有配置数据
} config_DataPart;

// 定义记录数据部分标识符
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
    RECORD_ALL // 保存所有记录数据
} record_DataPart;

//定义设备信息数据部分标识符
typedef enum {
    DEVICE_INFO_HARDWARE_VERSION,
    DEVICE_INFO_FIRMWARE_VERSION,
    DEVICE_INFO_DEVICE_ID,
    DEVICE_INFO_DATE_OF_MANUFACTURE,
    DEVICE_INFO_MQTT,
    DEVICE_INFO_ALL // 保存所有设备信息数据
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


