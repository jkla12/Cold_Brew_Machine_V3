#ifndef __CONFIG_H
#define __CONFIG_H

#define VERSION 17

#define WATER_COEFFICIENT 2.83f      //1cm水深，283mL

//EEPROM数据配置
#define NUMBER_OF_PARAMETERS 10     //参数个数
#define NUMBER_OF_COLLECTIONS 4     //收藏数量

#define WATERTAP_NUMBER_OF_PARAMETERS 3 //水龙头参数个数

#define CONFIG_ADDRESS 0x0000   //配置数据保存地址 0开始
#define RECORD_ADDRESS 0x400   //记录数据保存地址 1024开始
#define DEVICE_INFO_ADDRESS 0x800   //设备信息保存地址 2048开始

//默认参数
#define IS_LOCK                         0       //是否锁定
#define ZERO_LEVEL                      16      //零位水位
#define COFFEE_COMPENSATE_LEVEL         0       //咖啡模式补偿液位
#define TEA_COMPENSATE_LEVEL            0       //茶模式补偿液位
#define INLET_OVER_TIME                 300     //进水超时
#define DRAIN_OVER_TIME                 20      //排水超时时间
#define CIRCULATION_VALVE_SWICTH_TIME   10      //循环阀切换时间
#define DRAIN_VALVE_SWICTH_TIME         10      //排水阀切换时间
#define DRAIN_DELAY_TIME                6       //排水延迟时间，排空剩余液体
#define WASH_DRAIN_REPEAT               2       //清洗重复次数
#define WASH_SINGLE_TIME                50      //单次清洗时间
#define WASH_PAUSE_TIME                 10      //单次清洗暂停时间
#define WASH_LOOP_TIMES                 2       //循环次数
#define WASH_FIRST_VOLUME               35      //第一次清洗的液量
#define WASH_SECOND_VOLUME              55      //第二次清洗的液量
#define WASH_TIME                       480     //总清洗时间
#define DRAIN_TIME                      60      //排水时间,单位秒

#define SANIT_TIME                      2700      //消毒时间 单位 秒
#define SANIT_VOLUME                    50      //消毒液量 单位 0.1升
#define SANIT_STEP1_NUM                 16       //消毒步骤1循环次数 
#define SANIT_STEP2_NUM                 4       //消毒步骤2循环次数
#define SANIT_SINGLE_TIME               110     //单次消毒启动时间
#define SANIT_PAUSE_TIME                10      //单次消毒暂停时间

#define COFFEE_MAKE_TIME                 30     //咖啡制作时间
#define COFFEE_MAKE_VOLUME               40     //咖啡制作液量
#define COFFEE_MAKE_WEIGHT               300    //咖啡制作重量
#define COFFEE_MAKE_AUTO_DRAINANGE_FLAG  0      //是否自动排水， 0不自动
#define COFFEE_MAKE_COLLECT_FLAG         1      //是否收藏
#define COFFEE_MAKE_CURRENT_NUMBER       0      //当前制作编号

#define TEA_MAKE_TIME                   30      //茶制作时间
#define TEA_MAKE_VOLUME                 40      //茶制作液量
#define TEA_MAKE_WEIGHT                 100     //茶制作重量
#define TEA_MAKE_AUTO_DRAINANGE_FLAG    0       //是否自动排水
#define TEA_MAKE_COLLECT_FLAG           1       //是否收藏
#define TEA_MAKE_CURRENT_NUMBER         0       //当前制作编号

#define WATER_VOLUME_LOW                20      //液量最小
#define WATER_VOLUME_HIGH               50      //液量最大
#define WATER_CHANGE_VAL                1       //液量修改值

#define EXTRACTION_MIN_TIME             1       //萃取最小时间 单位分钟
#define EXTRACTION_MAX_TIME             60      //萃取最大时间 单位分钟
#define TIME_CHANGE_VAL                 1       //萃取时间修改值

#define WEIGHT_MIN                      0       //重量最小
#define WEIGHT_MAX                      500     //重量最大
#define WEIGHT_CHANGE_VAL               10      //重量修改值

#define WATERTAP_VOLUME_MIN             1.0f       //水龙头液量最小值
#define WATERTAP_VOLUME_MAX             99.0f       //水龙头液量最大值

//deviceInfo相关默认参数


#define HARDWARE_VERSION                20     //硬件版本
#define SOFTWARE_VERSION                20     //软件版本


//MQTT相关
#define REGISTER_ADDR                   "http://www.sparkinger.com:30020/api/Equipment/AddDevice"   //设备注册地址
#define UPDATE_ADDR                     "http://www.sparkinger.com:30020/api/Equipment/UEDV"         //设备更新版本号地址
#define MQTT_PUBTOPIC                    "$oc/devices/%s/sys/properties/report"        //设备上报属性地址
#define MQTT_SUBTOPIC                    "$oc/devices/%s/sys/messages/down"        //设备接收命令地址
#define MQTT_ADDR                       "c867b1d505.st1.iotda-device.ap-southeast-1.myhuaweicloud.com"   //MQTT服务器地址
#define MQTT_PORT                       1883    //MQTT端口
#define MQTT_SUBSCRIBE_TOPIC            "/%s/thing/service/property/set"   //MQTT订阅主题

#define ETID_                            "CB11"  //设备类别ID
#define DV_                              "1.0"   //设备版本号
#define ESN_                             "CB11_%s"   //设备序列号

#endif

