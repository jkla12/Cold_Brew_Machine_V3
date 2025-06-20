#ifndef __LCD_DATA_PROCESS_H
#define __LCD_DATA_PROCESS_H

#include "uart_lcd.h"




typedef struct 
{
    uint8_t now_page;    //当前画面
    uint8_t language;    //语言0:中文 1:英文
    uint8_t make_num;    //当前萃取制作编号
    uint8_t coffe_set_par_change;   //咖啡设置参数改变
    uint8_t tea_set_par_change;     //茶设置参数改变
    uint8_t wine_set_par_change;    //酒设置参数改变
}LCD_data_TypeDef;

#define LOW_WATER       20//低水位
#define vol_addr 	    7//水量地址
#define time_addr 	    8//时间地址
#define show_addr 	    13//当前显示地址
#define weight_addr 	25//粉重地址
#define collect1_addr   14//收藏1地址
#define collect2_addr   15//收藏2地址
#define collect3_addr   16//收藏3地址
#define collect4_addr   17//收藏4地址

#define collect1_button_addr   21//收藏1按钮地址
#define collect2_button_addr   22//收藏2按钮地址
#define collect3_button_addr   23//收藏3按钮地址
#define collect4_button_addr   24//收藏4按钮地址

#define NOTIFY_TOUCH_PRESS         0X01  //触摸屏按下通知
#define NOTIFY_TOUCH_RELEASE       0X03  //触摸屏松开通知
#define NOTIFY_WRITE_FLASH_OK      0X0C  //写FLASH成功
#define NOTIFY_WRITE_FLASH_FAILD   0X0D  //写FLASH失败
#define NOTIFY_READ_FLASH_OK       0X0B  //读FLASH成功
#define NOTIFY_READ_FLASH_FAILD    0X0F  //读FLASH失败
#define NOTIFY_MENU                0X14  //菜单事件通知
#define NOTIFY_TIMER               0X43  //定时器超时通知
#define NOTIFY_CONTROL             0XB1  //控件更新通知
#define NOTIFY_READ_RTC            0XF7  //读取RTC时间
#define MSG_GET_CURRENT_SCREEN     0X01  //画面ID变化通知
#define MSG_GET_DATA               0X11  //控件数据通知


//水龙头相关
#define WATERTAP_MAIN_PAGE  0
#define WATERTAP_SET_PAGE   1
#define MAIN_VOL1_ID    8   //主页文本1地址
#define MAIN_VOL2_ID    9   //主页文本2地址
#define MAIN_VOL3_ID    10  //主页文本3地址
#define SET_VOL1_ID    3   //设置文本1
#define SET_VOL2_ID    4   //设置文本2
#define SET_VOL3_ID    5   //设置文本3

#define PTR2U16(PTR) ((((uint8 *)(PTR))[0]<<8)|((uint8 *)(PTR))[1])  //从缓冲区取16位数据
#define PTR2U32(PTR) ((((uint8 *)(PTR))[0]<<24)|(((uint8 *)(PTR))[1]<<16)|(((uint8 *)(PTR))[2]<<8)|((uint8 *)(PTR))[3])  //从缓冲区取32位数据

#define vol_change_val  2   //水量单次修改值

//页面信息
enum pageType
{
    main_page=0x00,                   	//主页面
    make_coffe_set_page,                //萃取咖啡设置页面
	make_tea_set_page,				  	//萃取茶设置页面
	make_wine_set_page,				  	//萃取酒设置页面
	wash_page,                          //清洗页面
	sanit_page,                         //消毒页面
	setting_page,                       //设置页面
	drain_page,                         //排水页面
	production_page,                    //生产信息页面
	language_page,                      //语言设置页面
	wifi_setting_page,                  //WIFI设置页面
	set_time_page,                      //设置时间页面
	device_info_page,                   //设备信息页面
	make_coffe_page,                    //萃取咖啡页面
	make_tea_page,					  	//萃取茶页面
	make_wine_page,					  	//萃取酒页面
    water_ingress_page,                 //进水故障页面
    water_drain_page,                   //排水故障页面
    water_level_page,                   //水位传感器故障页面
    drain_choose_page,                  //排水选择页面
};



enum CtrlType
{
    kCtrlUnknown=0x0,
    kCtrlButton=0x10,                     //按钮
    kCtrlText,                            //文本
    kCtrlProgress,                        //进度条
    kCtrlSlider,                          //滑动条
    kCtrlMeter,                            //仪表
    kCtrlDropList,                        //下拉列表
    kCtrlAnimation,                       //动画
    kCtrlRTC,                             //时间显示
    kCtrlGraph,                           //曲线图控件
    kCtrlTable,                           //表格控件
    kCtrlMenu,                            //菜单控件
    kCtrlSelector,                        //选择控件
    kCtrlQRCode,                          //二维码
};

#pragma pack(push)
#pragma pack(1)                           //按字节对齐

typedef struct
{
    uint8    cmd_head;                    //帧头

    uint8    cmd_type;                    //命令类型(UPDATE_CONTROL)    
    uint8    ctrl_msg;                    //CtrlMsgType-指示消息的类型
    uint16   screen_id;                   //产生消息的画面ID
    uint16   control_id;                  //产生消息的控件ID
    uint8    control_type;                //控件类型

    uint8    param[256];                  //可变长度参数，最多256个字节

    uint8  cmd_tail[4];                   //帧尾
}CTRL_MSG,*PCTRL_MSG;

#pragma pack(pop)

void lcd_cmd_scan(void);
void lcd_cmd_scan2(void);
/*! 
*  \brief  消息处理流程
*  \param msg 待处理消息
*  \param size 消息长度
*/
void ProcessMessage( PCTRL_MSG msg, uint16 size );
void ProcessMessage2(PCTRL_MSG msg, uint16 size);

/*! 
*  \brief  画面切换通知
*  \details  当前画面改变时(或调用GetScreen)，执行此函数
*  \param screen_id 当前画面ID
*/
void NotifyScreen(uint16 screen_id);
void NotifyScreen2(uint16 screen_id);
/*! 
*  \brief  触摸坐标事件响应
*  \param press 1按下触摸屏，3松开触摸屏
*  \param x x坐标
*  \param y y坐标
*/
void NotifyTouchXY(uint8 press,uint16 x,uint16 y);


/*! 
*  \brief  按钮控件通知
*  \details  当按钮状态改变(或调用GetControlValue)时，执行此函数
*  \param screen_id 画面ID
*  \param control_id 控件ID
*  \param state 按钮状态：0弹起，1按下
*/
void NotifyButton(uint16 screen_id, uint16 control_id, uint8 state);
void NotifyButton2(uint16 screen_id, uint16 control_id, uint8  state);
/*! 
*  \brief  文本控件通知
*  \details  当文本通过键盘更新(或调用GetControlValue)时，执行此函数
*  \details  文本控件的内容以字符串形式下发到MCU，如果文本控件内容是浮点值，
*  \details  则需要在此函数中将下发字符串重新转回浮点值。
*  \param screen_id 画面ID
*  \param control_id 控件ID
*  \param str 文本控件内容
*/
void NotifyText(uint16 screen_id, uint16 control_id, uint8 *str);
void NotifyText2(uint16 screen_id, uint16 control_id, uint8 *str);
/*!                                                                              
*  \brief  进度条控件通知                                                       
*  \details  调用GetControlValue时，执行此函数                                  
*  \param screen_id 画面ID                                                      
*  \param control_id 控件ID                                                     
*  \param value 值                                                              
*/   
void NotifyProgress(uint16 screen_id, uint16 control_id, uint32 value);
void NotifyProgress2(uint16 screen_id, uint16 control_id, uint32 value);

/*!                                                                              
*  \brief  滑动条控件通知                                                       
*  \details  当滑动条改变(或调用GetControlValue)时，执行此函数                  
*  \param screen_id 画面ID                                                      
*  \param control_id 控件ID                                                     
*  \param value 值
*/    
void NotifySlider(uint16 screen_id, uint16 control_id, uint32 value);
void NotifySlider2(uint16 screen_id, uint16 control_id, uint32 value);

/*! 
*  \brief  仪表控件通知
*  \details  调用GetControlValue时，执行此函数
*  \param screen_id 画面ID
*  \param control_id 控件ID
*  \param value 值
*/
void NotifyMeter(uint16 screen_id, uint16 control_id, uint32 value);
void NotifyMeter2(uint16 screen_id, uint16 control_id, uint32 value);

/*! 
*  \brief  菜单控件通知
*  \details  当菜单项按下或松开时，执行此函数
*  \param screen_id 画面ID
*  \param control_id 控件ID
*  \param item 菜单项索引
*  \param state 按钮状态：0松开，1按下
*/
void NotifyMenu(uint16 screen_id, uint16 control_id, uint8  item, uint8  state);
void NotifyMenu2(uint16 screen_id, uint16 control_id, uint8 item, uint8 state);

/*! 
*  \brief  选择控件通知
*  \details  当选择控件变化时，执行此函数
*  \param screen_id 画面ID
*  \param control_id 控件ID
*  \param item 当前选项
*/
void NotifySelector(uint16 screen_id, uint16 control_id, uint8  item);
void NotifySelector2(uint16 screen_id, uint16 control_id, uint8  item);

/*! 
*  \brief  定时器超时通知处理
*  \param screen_id 画面ID
*  \param control_id 控件ID
*/
void NotifyTimer(uint16 screen_id, uint16 control_id);
void NotifyTimer2(uint16 screen_id, uint16 control_id);

/*! 
*  \brief  读取用户FLASH状态返回
*  \param status 0失败，1成功
*  \param _data 返回数据
*  \param length 数据长度
*/
void NotifyReadFlash(uint8 status,uint8 *_data,uint16 length);

/*! 
*  \brief  写用户FLASH状态返回
*  \param status 0失败，1成功
*/
void NotifyWriteFlash(uint8 status);
/*! 
*  \brief  读取RTC时间，注意返回的是BCD码
*  \param year 年（BCD）
*  \param month 月（BCD）
*  \param week 星期（BCD）
*  \param day 日（BCD）
*  \param hour 时（BCD）
*  \param minute 分（BCD）
*  \param second 秒（BCD）
*/
void NotifyReadRTC(uint8 year,uint8 month,uint8 week,uint8 day,uint8 hour,uint8 minute,uint8 second);

void UpdateUI(void);                                                                 //更新UI数据
// //扫描串口数据
// void lcd_cmd_scan(void);
// void lcd_cmd_scan2(void);




#endif


