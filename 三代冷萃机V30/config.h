#ifndef __CONFIG_H
#define __CONFIG_H

#define VERSION 17

#define WATER_COEFFICIENT 2.83f      //1cmˮ�283mL

//EEPROM��������
#define NUMBER_OF_PARAMETERS 10     //��������
#define NUMBER_OF_COLLECTIONS 4     //�ղ�����

#define WATERTAP_NUMBER_OF_PARAMETERS 3 //ˮ��ͷ��������

#define CONFIG_ADDRESS 0x0000   //�������ݱ����ַ 0��ʼ
#define RECORD_ADDRESS 0x400   //��¼���ݱ����ַ 1024��ʼ
#define DEVICE_INFO_ADDRESS 0x800   //�豸��Ϣ�����ַ 2048��ʼ

//Ĭ�ϲ���
#define IS_LOCK                         0       //�Ƿ�����
#define ZERO_LEVEL                      16      //��λˮλ
#define COFFEE_COMPENSATE_LEVEL         0       //����ģʽ����Һλ
#define TEA_COMPENSATE_LEVEL            0       //��ģʽ����Һλ
#define INLET_OVER_TIME                 300     //��ˮ��ʱ
#define DRAIN_OVER_TIME                 20      //��ˮ��ʱʱ��
#define CIRCULATION_VALVE_SWICTH_TIME   10      //ѭ�����л�ʱ��
#define DRAIN_VALVE_SWICTH_TIME         10      //��ˮ���л�ʱ��
#define DRAIN_DELAY_TIME                6       //��ˮ�ӳ�ʱ�䣬�ſ�ʣ��Һ��
#define WASH_DRAIN_REPEAT               2       //��ϴ�ظ�����
#define WASH_SINGLE_TIME                50      //������ϴʱ��
#define WASH_PAUSE_TIME                 10      //������ϴ��ͣʱ��
#define WASH_LOOP_TIMES                 2       //ѭ������
#define WASH_FIRST_VOLUME               35      //��һ����ϴ��Һ��
#define WASH_SECOND_VOLUME              55      //�ڶ�����ϴ��Һ��
#define WASH_TIME                       480     //����ϴʱ��
#define DRAIN_TIME                      60      //��ˮʱ��,��λ��

#define SANIT_TIME                      2700      //����ʱ�� ��λ ��
#define SANIT_VOLUME                    50      //����Һ�� ��λ 0.1��
#define SANIT_STEP1_NUM                 16       //��������1ѭ������ 
#define SANIT_STEP2_NUM                 4       //��������2ѭ������
#define SANIT_SINGLE_TIME               110     //������������ʱ��
#define SANIT_PAUSE_TIME                10      //����������ͣʱ��

#define COFFEE_MAKE_TIME                 30     //��������ʱ��
#define COFFEE_MAKE_VOLUME               40     //��������Һ��
#define COFFEE_MAKE_WEIGHT               300    //������������
#define COFFEE_MAKE_AUTO_DRAINANGE_FLAG  0      //�Ƿ��Զ���ˮ�� 0���Զ�
#define COFFEE_MAKE_COLLECT_FLAG         1      //�Ƿ��ղ�
#define COFFEE_MAKE_CURRENT_NUMBER       0      //��ǰ�������

#define TEA_MAKE_TIME                   30      //������ʱ��
#define TEA_MAKE_VOLUME                 40      //������Һ��
#define TEA_MAKE_WEIGHT                 100     //����������
#define TEA_MAKE_AUTO_DRAINANGE_FLAG    0       //�Ƿ��Զ���ˮ
#define TEA_MAKE_COLLECT_FLAG           1       //�Ƿ��ղ�
#define TEA_MAKE_CURRENT_NUMBER         0       //��ǰ�������

#define WATER_VOLUME_LOW                20      //Һ����С
#define WATER_VOLUME_HIGH               50      //Һ�����
#define WATER_CHANGE_VAL                1       //Һ���޸�ֵ

#define EXTRACTION_MIN_TIME             1       //��ȡ��Сʱ�� ��λ����
#define EXTRACTION_MAX_TIME             60      //��ȡ���ʱ�� ��λ����
#define TIME_CHANGE_VAL                 1       //��ȡʱ���޸�ֵ

#define WEIGHT_MIN                      0       //������С
#define WEIGHT_MAX                      500     //�������
#define WEIGHT_CHANGE_VAL               10      //�����޸�ֵ

#define WATERTAP_VOLUME_MIN             1.0f       //ˮ��ͷҺ����Сֵ
#define WATERTAP_VOLUME_MAX             99.0f       //ˮ��ͷҺ�����ֵ

//deviceInfo���Ĭ�ϲ���


#define HARDWARE_VERSION                20     //Ӳ���汾
#define SOFTWARE_VERSION                20     //����汾


//MQTT���
#define REGISTER_ADDR                   "http://www.sparkinger.com:30020/api/Equipment/AddDevice"   //�豸ע���ַ
#define UPDATE_ADDR                     "http://www.sparkinger.com:30020/api/Equipment/UEDV"         //�豸���°汾�ŵ�ַ
#define MQTT_PUBTOPIC                    "$oc/devices/%s/sys/properties/report"        //�豸�ϱ����Ե�ַ
#define MQTT_SUBTOPIC                    "$oc/devices/%s/sys/messages/down"        //�豸���������ַ
#define MQTT_ADDR                       "c867b1d505.st1.iotda-device.ap-southeast-1.myhuaweicloud.com"   //MQTT��������ַ
#define MQTT_PORT                       1883    //MQTT�˿�
#define MQTT_SUBSCRIBE_TOPIC            "/%s/thing/service/property/set"   //MQTT��������

#define ETID_                            "CB11"  //�豸���ID
#define DV_                              "1.0"   //�豸�汾��
#define ESN_                             "CB11_%s"   //�豸���к�

#endif

