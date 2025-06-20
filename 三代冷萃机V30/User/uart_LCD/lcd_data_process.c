#include "lcd_data_process.h"
#include "lcd_cmd_queue.h"
#include <stdio.h>
#include "eeprom.h"
#include "string.h"
#include "uart.h"
#include "uart_lcd.h"
#include "coffeeSetPage.h"
#include "teaSetPage.h"
#include "waterLevel.h"
#include "coffeeExtractionPage.h"
#include "teaExtractionPage.h"
#include "washPage.h"
#include "drainPage.h"
#include "sanitPage.h"
#include "productionDataPage.h"
#include "timeSetPage.h"
#include "deviceInfoPage.h"
#include "errPage.h"
#include "waterTapPage.h"
#include "wifiSetPage.h"
#include "elog.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "lcd"
#include "timer.h"
uint8 cmd_buffer[CMD_MAX_SIZE];
uint8_t cmd_buffer2[CMD_MAX_SIZE]; // 屏幕2指令数组
// static uint16 current_screen_id = 0;                                                 //当前画面ID
// static uint16 current_screen_id2 = 0;                                                 //当前屏幕2画面ID
// static int32 test_value = 0;                                                         //测试值
// static uint8 update_en = 0;                                                          //更新标记

LCD_data_TypeDef LCD_data;  // LCD数据结构体
LCD_data_TypeDef LCD_data2; // 屏幕2LCD数据结构体
                            // 串口2数据结构体

// 扫描串口数据
void lcd_cmd_scan(void)
{
    uint16_t size = queue_find_cmd(cmd_buffer, CMD_MAX_SIZE); // 从队列中取出一条完整的指令
    if (size > 0 && cmd_buffer[1] != 0x07)                    // 接收指令
    {
        ProcessMessage((PCTRL_MSG)cmd_buffer, size); // 处理消息
    }
    else if (size > 0 && cmd_buffer[1] == 0x07) // 接收数据,0x07为开机指令
    {
    }
}

// 扫描屏幕2串口数据
void lcd_cmd_scan2(void)
{
    uint16_t size = queue_find_cmd2(cmd_buffer2, CMD_MAX_SIZE); // 从队列中取出一条完整的指令
    if (size > 0 && cmd_buffer[1] != 0x07)                      // 接收指令
    {
        ProcessMessage2((PCTRL_MSG)cmd_buffer2, size); // 处理消息
    }
    else if (size > 0 && cmd_buffer2[1] == 0x07) // 接收数据,0x07为开机指令
    {
    }
}

/*!
 *  \brief  消息处理流程
 *  \param msg 待处理消息
 *  \param size 消息长度
 */
void ProcessMessage(PCTRL_MSG msg, uint16 size)
{
    uint8 cmd_type = msg->cmd_type;                // 指令类型
    uint8 ctrl_msg = msg->ctrl_msg;                // 消息的类型
    uint8 control_type = msg->control_type;        // 控件类型
    uint16 screen_id = PTR2U16(&msg->screen_id);   // 画面ID
    uint16 control_id = PTR2U16(&msg->control_id); // 控件ID
    uint32 value = PTR2U32(msg->param);            // 数值

    switch (cmd_type)
    {
    case NOTIFY_TOUCH_PRESS:   // 触摸屏按下
    case NOTIFY_TOUCH_RELEASE: // 触摸屏松开
        NotifyTouchXY(cmd_buffer[1], PTR2U16(cmd_buffer + 2), PTR2U16(cmd_buffer + 4));
        break;
    case NOTIFY_WRITE_FLASH_OK: // 写FLASH成功
        NotifyWriteFlash(1);
        break;
    case NOTIFY_WRITE_FLASH_FAILD: // 写FLASH失败
        NotifyWriteFlash(0);
        break;
    case NOTIFY_READ_FLASH_OK:                        // 读取FLASH成功
        NotifyReadFlash(1, cmd_buffer + 2, size - 6); // 去除帧头帧尾
        break;
    case NOTIFY_READ_FLASH_FAILD: // 读取FLASH失败
        NotifyReadFlash(0, 0, 0);
        break;
    case NOTIFY_READ_RTC: // 读取RTC时间
        NotifyReadRTC(cmd_buffer[2], cmd_buffer[3], cmd_buffer[4], cmd_buffer[5], cmd_buffer[6], cmd_buffer[7], cmd_buffer[8]);
        break;
    case NOTIFY_CONTROL:
    {
        if (ctrl_msg == MSG_GET_CURRENT_SCREEN) // 画面ID变化通知
        {
            LCD_data.now_page = screen_id; // 更新当前画面ID
            NotifyScreen(screen_id);       // 画面切换调动的函数
        }
        else
        {
            switch (control_type)
            {
            case kCtrlButton: // 按钮控件
                NotifyButton(screen_id, control_id, msg->param[1]);
                break;
            case kCtrlText: // 文本控件
                NotifyText(screen_id, control_id, msg->param);
                break;
            case kCtrlProgress: // 进度条控件
                NotifyProgress(screen_id, control_id, value);
                break;
            case kCtrlSlider: // 滑动条控件
                NotifySlider(screen_id, control_id, value);
                break;
            case kCtrlMeter: // 仪表控件
                NotifyMeter(screen_id, control_id, value);
                break;
            case kCtrlMenu: // 菜单控件
                NotifyMenu(screen_id, control_id, msg->param[0], msg->param[1]);
                break;
            case kCtrlSelector: // 选择控件
                NotifySelector(screen_id, control_id, msg->param[0]);
                break;
            case kCtrlRTC: // 倒计时控件
                NotifyTimer(screen_id, control_id);
                break;
            default:
                break;
            }
        }
    }
    break;
    default:
        break;
    }
}

/*!
 *  \brief  消息处理流程
 *  \param msg 待处理消息
 *  \param size 消息长度
 */

void ProcessMessage2(PCTRL_MSG msg, uint16 size)
{
    uint8 cmd_type = msg->cmd_type;                // 指令类型
    uint8 ctrl_msg = msg->ctrl_msg;                // 消息的类型
    uint8 control_type = msg->control_type;        // 控件类型
    uint16 screen_id = PTR2U16(&msg->screen_id);   // 画面ID
    uint16 control_id = PTR2U16(&msg->control_id); // 控件ID
    uint32 value = PTR2U32(msg->param);            // 数值
    switch (cmd_type)
    {
    case NOTIFY_CONTROL:
    {
        if (ctrl_msg == MSG_GET_CURRENT_SCREEN) // 屏幕2画面ID变化通知
        {
            LCD_data2.now_page = screen_id; // 更新当前画面ID
            NotifyScreen2(screen_id);       // 画面切换调动的函数
        }
        else
        {
            switch (control_type)
            {
            case kCtrlButton:
                NotifyButton2(screen_id, control_id, msg->param[1]);
                break;
            case kCtrlText:
                NotifyText2(screen_id, control_id, msg->param);
                break;
            case kCtrlProgress:
                NotifyProgress2(screen_id, control_id, value);
                break;
            case kCtrlSlider:
                NotifySlider2(screen_id, control_id, value);
                break;
            case kCtrlMeter:
                NotifyMeter2(screen_id, control_id, value);
                break;
            case kCtrlMenu:
                NotifyMenu2(screen_id, control_id, msg->param[0], msg->param[1]);
                break;
            case kCtrlSelector:
                NotifySelector2(screen_id, control_id, msg->param[0]);
                break;
            case kCtrlRTC:
                NotifyTimer2(screen_id, control_id);
                break;
            default:
                break;
            }
        }
    }
    break;
    default:
        break;
    }
}

/*!
 *  \brief  画面切换通知
 *  \details  当前画面改变时(或调用GetScreen)，执行此函数
 *  \param screen_id 当前画面ID
 */
void NotifyScreen(uint16 screen_id)
{
    //    current_screen_id = screen_id;                                                   //在工程配置中开启画面切换通知，记录当前画面ID

    switch (screen_id)
    {
    case main_page: // 主页面
        log_v("main_page");
        break;
    case make_coffe_set_page:   // 咖啡设置页面
        UpdateCoffeeCollect(); // 更新咖啡收藏状态
        loadingCoffeeMakeSet(); // 加载咖啡设置页面数据
        log_v("make_coffe_set_page");
        break;
    case make_coffe_page:              // 咖啡制作页面
        if (waterLevel.sensorErr == 1) // 传感器故障
        {
            SetScreen(water_level_page, UART2_ID);
        }
        else
        {
            enterCoffeeExtractionPage(); // 进入咖啡萃取页面
            log_v("make_coffe_page");
        }
        break;
    case make_tea_set_page:  // 茶叶设置页面
        updateTeaCollect(); // 更新茶叶收藏状态
        loadingTeaMakeSet(); // 加载茶叶设置页面数据
        log_v("make_tea_set_page");
        break;
    case make_tea_page:                // 茶叶制作页面
        if (waterLevel.sensorErr == 1) // 传感器故障
        {
            SetScreen(water_level_page, UART2_ID);
        }
        else
        {
            enterTeaExtractionPage(); // 进入茶叶萃取页面
            log_v("make_tea_page");
        }
        break;
    case wash_page: // 清洗页面

        if (waterLevel.sensorErr == 1) // 传感器故障
        {
            SetScreen(water_level_page, UART2_ID);
        }
        else
        {
            washShowTime(WASH_TIME, WASH_TIME); // 显示清洗时间
            log_v("wash_page");
        }
        break;
    case sanit_page:                   // 消毒页面
        if (waterLevel.sensorErr == 1) // 传感器故障
        {
            SetScreen(water_level_page, UART2_ID);
        }
        else
        {
            sanitShowTime(SANIT_TIME, SANIT_TIME); // 显示消毒时间
            log_v("sanit_page");
        }

        break;
    case setting_page: // 设置页面
        log_v("setting_page");
        break;
    case drain_page:                   // 排水页面
        if (waterLevel.sensorErr == 1) // 传感器故障
        {
            SetScreen(water_level_page, UART2_ID);
        }
        else
        {
            AnimationPlayFrame(drain_page, 10, 0, UART2_ID); // 切换排水图标
            drainShowTime(DRAIN_TIME, DRAIN_TIME);           // 显示排水时间
            log_v("drain_page");
        }
        break;
    case production_page:      // 生产信息页面
        enterProductionPage(); // 进入生产信息页面
        log_v("production_page");
        break;
    case language_page: // 语言设置页面
        // log_v("language_page");
        break;
    case wifi_setting_page: // WIFI设置页面
        log_v("wifi_setting_page");
        break;
    case device_info_page: // 设备信息页面
        log_v("device_info_page");
        break;
    case water_ingress_page: // 进水故障页面
        log_v("water_ingress_page");
        break;
    case water_drain_page: // 排水故障页面
        log_v("water_drain_page");
        break;
    case water_level_page: // 水位传感器故障页面
        log_v("water_level_page");
        break;
    default:
        break;
    }
}

/*!
 * \brief 屏幕2画面切换通知
 * \details 当前画面改变时(或调用GetScreen2)，执行此函数
 * \param screen_id 当前画面ID
 */
void NotifyScreen2(uint16 screen_id)
{

    //    current_screen_id2 = screen_id;                                                   //在工程配置中开启画面切换通知，记录当前画面ID
    switch (screen_id)
    {

    default:
        break;
    }
}

/*!
 *  \brief  触摸坐标事件响应
 *  \param press 1按下触摸屏，3松开触摸屏
 *  \param x x坐标
 *  \param y y坐标
 */
void NotifyTouchXY(uint8 press, uint16 x, uint16 y)
{
}

/*!
 *  \brief  更新数据
 */
void UpdateUI()
{
}

/*!
 *  \brief  按钮控件通知
 *  \details  当按钮状态改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param state 按钮状态：0弹起，1按下
 */
void NotifyButton(uint16 screen_id, uint16 control_id, uint8 state)
{
    switch (screen_id)
    {
    case main_page:

        break;
    case make_coffe_set_page:
        coffeeSetPageButton(control_id, state); // 咖啡设置页面按钮响应
        break;
    case make_tea_set_page:
        teaSetPageButton(control_id, state); // 茶叶设置页面按钮响应
        break;
    case make_coffe_page:
        coffeeExtractionPageButton(control_id, state); // 咖啡萃取页面按钮响应
        break;
    case make_tea_page:
        teaExtractionPageButton(control_id, state); // 茶叶萃取页面按钮响应
        break;
    case wash_page:
        washPageButton(control_id, state); // 清洗页面按钮响应
        break;
    case drain_page:
        drainPageButton(control_id, state); // 排水页面按钮响应
        break;
    case sanit_page:
        sanitPageButton(control_id, state); // 消毒页面按钮响应
        break;
    case set_time_page:
        timeSetPageButton(control_id, state); // 设置时间页面按钮响应
        break;
    case water_level_page:
        errPageButton(water_level_page, control_id, state); // 水位传感器故障页面按钮响应
        break;
    case water_ingress_page:
        errPageButton(water_ingress_page, control_id, state); // 进水故障页面按钮响应
        break;
    case water_drain_page:
        errPageButton(water_drain_page, control_id, state); // 排水故障页面按钮响应
        break;
    case drain_choose_page:
        drainChoosePageButton(control_id, state); // 排水选择页面按钮响应
        break;
    case setting_page:
        settingPageButton(control_id, state); // 设置页面按钮响应
        break;
    case wifi_setting_page:
        wifiSetPageButton(control_id, state); // WIFI设置页面按钮响应
        break;
    default:

        break;
    }
}

/*!
 *  \brief  屏幕2按钮控件通知
 *  \details  当按钮状态改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param state 按钮状态：0弹起，1按下
 */
void NotifyButton2(uint16 screen_id, uint16 control_id, uint8 state)
{
    switch (screen_id)
    {
    case 0:
        waterTapMainPageButton(control_id, state); // 水龙头主页面按钮响应
        break;
    case 1:
        waterTapSetPageButton(control_id, state); // 水龙头设置页面按钮响应
        break;
    default:
        break;
    }
}

/*!
 *  \brief  文本控件通知
 *  \details  当文本通过键盘更新(或调用GetControlValue)时，执行此函数
 *  \details  文本控件的内容以字符串形式下发到MCU，如果文本控件内容是浮点值，
 *  \details  则需要在此函数中将下发字符串重新转回浮点值。
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param str 文本控件内容
 */
void NotifyText(uint16 screen_id, uint16 control_id, uint8 *str)
{
    //    uint8_t val[3];
    switch (screen_id)
    {
    case make_coffe_set_page:               // 咖啡设置页面
        coffeeSetPageText(control_id, str); // 咖啡设置页面文本响应函数
        break;
    case make_tea_set_page:              // 茶叶设置页面
        teaSetPageText(control_id, str); // 茶叶设置页面文本响应函数
        break;
    default:
        break;
    }
}

/*!
 *  \brief  屏幕2文本控件通知
 *  \details  当文本通过键盘更新(或调用GetControlValue)时，执行此函数
 *  \details  文本控件的内容以字符串形式下发到MCU，如果文本控件内容是浮点值，
 *  \details  则需要在此函数中将下发字符串重新转回浮点值。
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param str 文本控件内容
 */
void NotifyText2(uint16 screen_id, uint16 control_id, uint8 *str)
{

    switch (screen_id)
    {
    case 1:
        waterTapSetPageText(control_id, str); // 水龙头设置页面文本响应函数
        break;
    default:
        break;
    }
}

/*!
 *  \brief  进度条控件通知
 *  \details  调用GetControlValue时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifyProgress(uint16 screen_id, uint16 control_id, uint32 value)
{
    // TODO: 添加用户代码
}

/*!
 *  \brief  屏幕2进度条控件通知
 *  \details  调用GetControlValue时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifyProgress2(uint16 screen_id, uint16 control_id, uint32 value)
{
    // TODO: 添加屏幕2进度条控件通知响应
}

/*!
 *  \brief  滑动条控件通知
 *  \details  当滑动条改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifySlider(uint16 screen_id, uint16 control_id, uint32 value)
{
}

/*!
 *  \brief  屏幕2滑动条控件通知
 *  \details  当滑动条改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifySlider2(uint16 screen_id, uint16 control_id, uint32 value)
{
    // TODO: 添加屏幕2滑动条控件通知响应
}

/*!
 *  \brief  仪表控件通知
 *  \details  调用GetControlValue时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifyMeter(uint16 screen_id, uint16 control_id, uint32 value)
{
    // TODO: 添加用户代码
}

/*!
 *  \brief  屏幕2仪表控件通知
 *  \details  调用GetControlValue时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifyMeter2(uint16 screen_id, uint16 control_id, uint32 value)
{
    // TODO: 添加屏幕2仪表控件通知响应
}

/*!
 *  \brief  菜单控件通知
 *  \details  当菜单项按下或松开时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param item 菜单项索引
 *  \param state 按钮状态：0松开，1按下
 */
void NotifyMenu(uint16 screen_id, uint16 control_id, uint8 item, uint8 state)
{
    // TODO: 添加用户代码
}

/*!
 *  \brief  屏幕2菜单控件通知
 *  \details  当菜单项按下或松开时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param item 菜单项索引
 *  \param state 按钮状态：0松开，1按下
 */
void NotifyMenu2(uint16 screen_id, uint16 control_id, uint8 item, uint8 state)
{
    // TODO: 添加屏幕2菜单控件通知响应
}

/*!
 *  \brief  选择控件通知
 *  \details  当选择控件变化时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param item 当前选项
 */
void NotifySelector(uint16 screen_id, uint16 control_id, uint8 item)
{
    switch (screen_id)
    {
    case set_time_page:                        // 设置时间页面
        timeSetPageSelector(control_id, item); // 设置时间页面选择响应函数
        break;
    default:
        break;
    }
}

/*!
 *  \brief  屏幕2选择控件通知
 *  \details  当选择控件变化时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param item 当前选项
 */
void NotifySelector2(uint16 screen_id, uint16 control_id, uint8 item)
{
    switch (screen_id)
    {
    // TODO: 添加屏幕2选择控件通知响应
    default:
        break;
    }
}

/*!
 *  \brief  定时器超时通知处理
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 */
void NotifyTimer(uint16 screen_id, uint16 control_id)
{
}

/*!
 *  \brief  屏幕2定时器超时通知处理
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 */
void NotifyTimer2(uint16 screen_id, uint16 control_id)
{
    // TODO  添加屏幕2定时器超时通知处理
}

/*!
 *  \brief  读取用户FLASH状态返回
 *  \param status 0失败，1成功
 *  \param _data 返回数据
 *  \param length 数据长度
 */
void NotifyReadFlash(uint8 status, uint8 *_data, uint16 length)
{
    // TODO: 添加用户代码
}

/*!
 *  \brief  写用户FLASH状态返回
 *  \param status 0失败，1成功
 */
void NotifyWriteFlash(uint8 status)
{
    // TODO: 添加用户代码
}

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
void NotifyReadRTC(uint8 year, uint8 month, uint8 week, uint8 day, uint8 hour, uint8 minute, uint8 second)
{
    //    uint8 years;
    //    uint8 months;
    //    uint8 weeks;
    //    uint8 days;
    //    uint8 hours;
    //    uint8 minutes;
    //    uint8 sec;
    //
    //    sec =(0xff & (second>>4))*10 +(0xf & second);                                    //BCD码转十进制
    //    years =(0xff & (year>>4))*10 +(0xf & year);
    //    months =(0xff & (month>>4))*10 +(0xf & month);
    //    weeks =(0xff & (week>>4))*10 +(0xf & week);
    //    days =(0xff & (day>>4))*10 +(0xf & day);
    //    hours =(0xff & (hour>>4))*10 +(0xf & hour);
    //    minutes =(0xff & (minute>>4))*10 +(0xf & minute);
    //
}
