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
uint8_t cmd_buffer2[CMD_MAX_SIZE]; // ��Ļ2ָ������
// static uint16 current_screen_id = 0;                                                 //��ǰ����ID
// static uint16 current_screen_id2 = 0;                                                 //��ǰ��Ļ2����ID
// static int32 test_value = 0;                                                         //����ֵ
// static uint8 update_en = 0;                                                          //���±��

LCD_data_TypeDef LCD_data;  // LCD���ݽṹ��
LCD_data_TypeDef LCD_data2; // ��Ļ2LCD���ݽṹ��
                            // ����2���ݽṹ��

// ɨ�贮������
void lcd_cmd_scan(void)
{
    uint16_t size = queue_find_cmd(cmd_buffer, CMD_MAX_SIZE); // �Ӷ�����ȡ��һ��������ָ��
    if (size > 0 && cmd_buffer[1] != 0x07)                    // ����ָ��
    {
        ProcessMessage((PCTRL_MSG)cmd_buffer, size); // ������Ϣ
    }
    else if (size > 0 && cmd_buffer[1] == 0x07) // ��������,0x07Ϊ����ָ��
    {
    }
}

// ɨ����Ļ2��������
void lcd_cmd_scan2(void)
{
    uint16_t size = queue_find_cmd2(cmd_buffer2, CMD_MAX_SIZE); // �Ӷ�����ȡ��һ��������ָ��
    if (size > 0 && cmd_buffer[1] != 0x07)                      // ����ָ��
    {
        ProcessMessage2((PCTRL_MSG)cmd_buffer2, size); // ������Ϣ
    }
    else if (size > 0 && cmd_buffer2[1] == 0x07) // ��������,0x07Ϊ����ָ��
    {
    }
}

/*!
 *  \brief  ��Ϣ��������
 *  \param msg ��������Ϣ
 *  \param size ��Ϣ����
 */
void ProcessMessage(PCTRL_MSG msg, uint16 size)
{
    uint8 cmd_type = msg->cmd_type;                // ָ������
    uint8 ctrl_msg = msg->ctrl_msg;                // ��Ϣ������
    uint8 control_type = msg->control_type;        // �ؼ�����
    uint16 screen_id = PTR2U16(&msg->screen_id);   // ����ID
    uint16 control_id = PTR2U16(&msg->control_id); // �ؼ�ID
    uint32 value = PTR2U32(msg->param);            // ��ֵ

    switch (cmd_type)
    {
    case NOTIFY_TOUCH_PRESS:   // ����������
    case NOTIFY_TOUCH_RELEASE: // �������ɿ�
        NotifyTouchXY(cmd_buffer[1], PTR2U16(cmd_buffer + 2), PTR2U16(cmd_buffer + 4));
        break;
    case NOTIFY_WRITE_FLASH_OK: // дFLASH�ɹ�
        NotifyWriteFlash(1);
        break;
    case NOTIFY_WRITE_FLASH_FAILD: // дFLASHʧ��
        NotifyWriteFlash(0);
        break;
    case NOTIFY_READ_FLASH_OK:                        // ��ȡFLASH�ɹ�
        NotifyReadFlash(1, cmd_buffer + 2, size - 6); // ȥ��֡ͷ֡β
        break;
    case NOTIFY_READ_FLASH_FAILD: // ��ȡFLASHʧ��
        NotifyReadFlash(0, 0, 0);
        break;
    case NOTIFY_READ_RTC: // ��ȡRTCʱ��
        NotifyReadRTC(cmd_buffer[2], cmd_buffer[3], cmd_buffer[4], cmd_buffer[5], cmd_buffer[6], cmd_buffer[7], cmd_buffer[8]);
        break;
    case NOTIFY_CONTROL:
    {
        if (ctrl_msg == MSG_GET_CURRENT_SCREEN) // ����ID�仯֪ͨ
        {
            LCD_data.now_page = screen_id; // ���µ�ǰ����ID
            NotifyScreen(screen_id);       // �����л������ĺ���
        }
        else
        {
            switch (control_type)
            {
            case kCtrlButton: // ��ť�ؼ�
                NotifyButton(screen_id, control_id, msg->param[1]);
                break;
            case kCtrlText: // �ı��ؼ�
                NotifyText(screen_id, control_id, msg->param);
                break;
            case kCtrlProgress: // �������ؼ�
                NotifyProgress(screen_id, control_id, value);
                break;
            case kCtrlSlider: // �������ؼ�
                NotifySlider(screen_id, control_id, value);
                break;
            case kCtrlMeter: // �Ǳ�ؼ�
                NotifyMeter(screen_id, control_id, value);
                break;
            case kCtrlMenu: // �˵��ؼ�
                NotifyMenu(screen_id, control_id, msg->param[0], msg->param[1]);
                break;
            case kCtrlSelector: // ѡ��ؼ�
                NotifySelector(screen_id, control_id, msg->param[0]);
                break;
            case kCtrlRTC: // ����ʱ�ؼ�
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
 *  \brief  ��Ϣ��������
 *  \param msg ��������Ϣ
 *  \param size ��Ϣ����
 */

void ProcessMessage2(PCTRL_MSG msg, uint16 size)
{
    uint8 cmd_type = msg->cmd_type;                // ָ������
    uint8 ctrl_msg = msg->ctrl_msg;                // ��Ϣ������
    uint8 control_type = msg->control_type;        // �ؼ�����
    uint16 screen_id = PTR2U16(&msg->screen_id);   // ����ID
    uint16 control_id = PTR2U16(&msg->control_id); // �ؼ�ID
    uint32 value = PTR2U32(msg->param);            // ��ֵ
    switch (cmd_type)
    {
    case NOTIFY_CONTROL:
    {
        if (ctrl_msg == MSG_GET_CURRENT_SCREEN) // ��Ļ2����ID�仯֪ͨ
        {
            LCD_data2.now_page = screen_id; // ���µ�ǰ����ID
            NotifyScreen2(screen_id);       // �����л������ĺ���
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
 *  \brief  �����л�֪ͨ
 *  \details  ��ǰ����ı�ʱ(�����GetScreen)��ִ�д˺���
 *  \param screen_id ��ǰ����ID
 */
void NotifyScreen(uint16 screen_id)
{
    //    current_screen_id = screen_id;                                                   //�ڹ��������п��������л�֪ͨ����¼��ǰ����ID

    switch (screen_id)
    {
    case main_page: // ��ҳ��
        log_v("main_page");
        break;
    case make_coffe_set_page:   // ��������ҳ��
        UpdateCoffeeCollect(); // ���¿����ղ�״̬
        loadingCoffeeMakeSet(); // ���ؿ�������ҳ������
        log_v("make_coffe_set_page");
        break;
    case make_coffe_page:              // ��������ҳ��
        if (waterLevel.sensorErr == 1) // ����������
        {
            SetScreen(water_level_page, UART2_ID);
        }
        else
        {
            enterCoffeeExtractionPage(); // ���뿧����ȡҳ��
            log_v("make_coffe_page");
        }
        break;
    case make_tea_set_page:  // ��Ҷ����ҳ��
        updateTeaCollect(); // ���²�Ҷ�ղ�״̬
        loadingTeaMakeSet(); // ���ز�Ҷ����ҳ������
        log_v("make_tea_set_page");
        break;
    case make_tea_page:                // ��Ҷ����ҳ��
        if (waterLevel.sensorErr == 1) // ����������
        {
            SetScreen(water_level_page, UART2_ID);
        }
        else
        {
            enterTeaExtractionPage(); // �����Ҷ��ȡҳ��
            log_v("make_tea_page");
        }
        break;
    case wash_page: // ��ϴҳ��

        if (waterLevel.sensorErr == 1) // ����������
        {
            SetScreen(water_level_page, UART2_ID);
        }
        else
        {
            washShowTime(WASH_TIME, WASH_TIME); // ��ʾ��ϴʱ��
            log_v("wash_page");
        }
        break;
    case sanit_page:                   // ����ҳ��
        if (waterLevel.sensorErr == 1) // ����������
        {
            SetScreen(water_level_page, UART2_ID);
        }
        else
        {
            sanitShowTime(SANIT_TIME, SANIT_TIME); // ��ʾ����ʱ��
            log_v("sanit_page");
        }

        break;
    case setting_page: // ����ҳ��
        log_v("setting_page");
        break;
    case drain_page:                   // ��ˮҳ��
        if (waterLevel.sensorErr == 1) // ����������
        {
            SetScreen(water_level_page, UART2_ID);
        }
        else
        {
            AnimationPlayFrame(drain_page, 10, 0, UART2_ID); // �л���ˮͼ��
            drainShowTime(DRAIN_TIME, DRAIN_TIME);           // ��ʾ��ˮʱ��
            log_v("drain_page");
        }
        break;
    case production_page:      // ������Ϣҳ��
        enterProductionPage(); // ����������Ϣҳ��
        log_v("production_page");
        break;
    case language_page: // ��������ҳ��
        // log_v("language_page");
        break;
    case wifi_setting_page: // WIFI����ҳ��
        log_v("wifi_setting_page");
        break;
    case device_info_page: // �豸��Ϣҳ��
        log_v("device_info_page");
        break;
    case water_ingress_page: // ��ˮ����ҳ��
        log_v("water_ingress_page");
        break;
    case water_drain_page: // ��ˮ����ҳ��
        log_v("water_drain_page");
        break;
    case water_level_page: // ˮλ����������ҳ��
        log_v("water_level_page");
        break;
    default:
        break;
    }
}

/*!
 * \brief ��Ļ2�����л�֪ͨ
 * \details ��ǰ����ı�ʱ(�����GetScreen2)��ִ�д˺���
 * \param screen_id ��ǰ����ID
 */
void NotifyScreen2(uint16 screen_id)
{

    //    current_screen_id2 = screen_id;                                                   //�ڹ��������п��������л�֪ͨ����¼��ǰ����ID
    switch (screen_id)
    {

    default:
        break;
    }
}

/*!
 *  \brief  ���������¼���Ӧ
 *  \param press 1���´�������3�ɿ�������
 *  \param x x����
 *  \param y y����
 */
void NotifyTouchXY(uint8 press, uint16 x, uint16 y)
{
}

/*!
 *  \brief  ��������
 */
void UpdateUI()
{
}

/*!
 *  \brief  ��ť�ؼ�֪ͨ
 *  \details  ����ť״̬�ı�(�����GetControlValue)ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param state ��ť״̬��0����1����
 */
void NotifyButton(uint16 screen_id, uint16 control_id, uint8 state)
{
    switch (screen_id)
    {
    case main_page:

        break;
    case make_coffe_set_page:
        coffeeSetPageButton(control_id, state); // ��������ҳ�水ť��Ӧ
        break;
    case make_tea_set_page:
        teaSetPageButton(control_id, state); // ��Ҷ����ҳ�水ť��Ӧ
        break;
    case make_coffe_page:
        coffeeExtractionPageButton(control_id, state); // ������ȡҳ�水ť��Ӧ
        break;
    case make_tea_page:
        teaExtractionPageButton(control_id, state); // ��Ҷ��ȡҳ�水ť��Ӧ
        break;
    case wash_page:
        washPageButton(control_id, state); // ��ϴҳ�水ť��Ӧ
        break;
    case drain_page:
        drainPageButton(control_id, state); // ��ˮҳ�水ť��Ӧ
        break;
    case sanit_page:
        sanitPageButton(control_id, state); // ����ҳ�水ť��Ӧ
        break;
    case set_time_page:
        timeSetPageButton(control_id, state); // ����ʱ��ҳ�水ť��Ӧ
        break;
    case water_level_page:
        errPageButton(water_level_page, control_id, state); // ˮλ����������ҳ�水ť��Ӧ
        break;
    case water_ingress_page:
        errPageButton(water_ingress_page, control_id, state); // ��ˮ����ҳ�水ť��Ӧ
        break;
    case water_drain_page:
        errPageButton(water_drain_page, control_id, state); // ��ˮ����ҳ�水ť��Ӧ
        break;
    case drain_choose_page:
        drainChoosePageButton(control_id, state); // ��ˮѡ��ҳ�水ť��Ӧ
        break;
    case setting_page:
        settingPageButton(control_id, state); // ����ҳ�水ť��Ӧ
        break;
    case wifi_setting_page:
        wifiSetPageButton(control_id, state); // WIFI����ҳ�水ť��Ӧ
        break;
    default:

        break;
    }
}

/*!
 *  \brief  ��Ļ2��ť�ؼ�֪ͨ
 *  \details  ����ť״̬�ı�(�����GetControlValue)ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param state ��ť״̬��0����1����
 */
void NotifyButton2(uint16 screen_id, uint16 control_id, uint8 state)
{
    switch (screen_id)
    {
    case 0:
        waterTapMainPageButton(control_id, state); // ˮ��ͷ��ҳ�水ť��Ӧ
        break;
    case 1:
        waterTapSetPageButton(control_id, state); // ˮ��ͷ����ҳ�水ť��Ӧ
        break;
    default:
        break;
    }
}

/*!
 *  \brief  �ı��ؼ�֪ͨ
 *  \details  ���ı�ͨ�����̸���(�����GetControlValue)ʱ��ִ�д˺���
 *  \details  �ı��ؼ����������ַ�����ʽ�·���MCU������ı��ؼ������Ǹ���ֵ��
 *  \details  ����Ҫ�ڴ˺����н��·��ַ�������ת�ظ���ֵ��
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param str �ı��ؼ�����
 */
void NotifyText(uint16 screen_id, uint16 control_id, uint8 *str)
{
    //    uint8_t val[3];
    switch (screen_id)
    {
    case make_coffe_set_page:               // ��������ҳ��
        coffeeSetPageText(control_id, str); // ��������ҳ���ı���Ӧ����
        break;
    case make_tea_set_page:              // ��Ҷ����ҳ��
        teaSetPageText(control_id, str); // ��Ҷ����ҳ���ı���Ӧ����
        break;
    default:
        break;
    }
}

/*!
 *  \brief  ��Ļ2�ı��ؼ�֪ͨ
 *  \details  ���ı�ͨ�����̸���(�����GetControlValue)ʱ��ִ�д˺���
 *  \details  �ı��ؼ����������ַ�����ʽ�·���MCU������ı��ؼ������Ǹ���ֵ��
 *  \details  ����Ҫ�ڴ˺����н��·��ַ�������ת�ظ���ֵ��
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param str �ı��ؼ�����
 */
void NotifyText2(uint16 screen_id, uint16 control_id, uint8 *str)
{

    switch (screen_id)
    {
    case 1:
        waterTapSetPageText(control_id, str); // ˮ��ͷ����ҳ���ı���Ӧ����
        break;
    default:
        break;
    }
}

/*!
 *  \brief  �������ؼ�֪ͨ
 *  \details  ����GetControlValueʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param value ֵ
 */
void NotifyProgress(uint16 screen_id, uint16 control_id, uint32 value)
{
    // TODO: ����û�����
}

/*!
 *  \brief  ��Ļ2�������ؼ�֪ͨ
 *  \details  ����GetControlValueʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param value ֵ
 */
void NotifyProgress2(uint16 screen_id, uint16 control_id, uint32 value)
{
    // TODO: �����Ļ2�������ؼ�֪ͨ��Ӧ
}

/*!
 *  \brief  �������ؼ�֪ͨ
 *  \details  ���������ı�(�����GetControlValue)ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param value ֵ
 */
void NotifySlider(uint16 screen_id, uint16 control_id, uint32 value)
{
}

/*!
 *  \brief  ��Ļ2�������ؼ�֪ͨ
 *  \details  ���������ı�(�����GetControlValue)ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param value ֵ
 */
void NotifySlider2(uint16 screen_id, uint16 control_id, uint32 value)
{
    // TODO: �����Ļ2�������ؼ�֪ͨ��Ӧ
}

/*!
 *  \brief  �Ǳ�ؼ�֪ͨ
 *  \details  ����GetControlValueʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param value ֵ
 */
void NotifyMeter(uint16 screen_id, uint16 control_id, uint32 value)
{
    // TODO: ����û�����
}

/*!
 *  \brief  ��Ļ2�Ǳ�ؼ�֪ͨ
 *  \details  ����GetControlValueʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param value ֵ
 */
void NotifyMeter2(uint16 screen_id, uint16 control_id, uint32 value)
{
    // TODO: �����Ļ2�Ǳ�ؼ�֪ͨ��Ӧ
}

/*!
 *  \brief  �˵��ؼ�֪ͨ
 *  \details  ���˵���»��ɿ�ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param item �˵�������
 *  \param state ��ť״̬��0�ɿ���1����
 */
void NotifyMenu(uint16 screen_id, uint16 control_id, uint8 item, uint8 state)
{
    // TODO: ����û�����
}

/*!
 *  \brief  ��Ļ2�˵��ؼ�֪ͨ
 *  \details  ���˵���»��ɿ�ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param item �˵�������
 *  \param state ��ť״̬��0�ɿ���1����
 */
void NotifyMenu2(uint16 screen_id, uint16 control_id, uint8 item, uint8 state)
{
    // TODO: �����Ļ2�˵��ؼ�֪ͨ��Ӧ
}

/*!
 *  \brief  ѡ��ؼ�֪ͨ
 *  \details  ��ѡ��ؼ��仯ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param item ��ǰѡ��
 */
void NotifySelector(uint16 screen_id, uint16 control_id, uint8 item)
{
    switch (screen_id)
    {
    case set_time_page:                        // ����ʱ��ҳ��
        timeSetPageSelector(control_id, item); // ����ʱ��ҳ��ѡ����Ӧ����
        break;
    default:
        break;
    }
}

/*!
 *  \brief  ��Ļ2ѡ��ؼ�֪ͨ
 *  \details  ��ѡ��ؼ��仯ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param item ��ǰѡ��
 */
void NotifySelector2(uint16 screen_id, uint16 control_id, uint8 item)
{
    switch (screen_id)
    {
    // TODO: �����Ļ2ѡ��ؼ�֪ͨ��Ӧ
    default:
        break;
    }
}

/*!
 *  \brief  ��ʱ����ʱ֪ͨ����
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 */
void NotifyTimer(uint16 screen_id, uint16 control_id)
{
}

/*!
 *  \brief  ��Ļ2��ʱ����ʱ֪ͨ����
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 */
void NotifyTimer2(uint16 screen_id, uint16 control_id)
{
    // TODO  �����Ļ2��ʱ����ʱ֪ͨ����
}

/*!
 *  \brief  ��ȡ�û�FLASH״̬����
 *  \param status 0ʧ�ܣ�1�ɹ�
 *  \param _data ��������
 *  \param length ���ݳ���
 */
void NotifyReadFlash(uint8 status, uint8 *_data, uint16 length)
{
    // TODO: ����û�����
}

/*!
 *  \brief  д�û�FLASH״̬����
 *  \param status 0ʧ�ܣ�1�ɹ�
 */
void NotifyWriteFlash(uint8 status)
{
    // TODO: ����û�����
}

/*!
 *  \brief  ��ȡRTCʱ�䣬ע�ⷵ�ص���BCD��
 *  \param year �꣨BCD��
 *  \param month �£�BCD��
 *  \param week ���ڣ�BCD��
 *  \param day �գ�BCD��
 *  \param hour ʱ��BCD��
 *  \param minute �֣�BCD��
 *  \param second �루BCD��
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
    //    sec =(0xff & (second>>4))*10 +(0xf & second);                                    //BCD��תʮ����
    //    years =(0xff & (year>>4))*10 +(0xf & year);
    //    months =(0xff & (month>>4))*10 +(0xf & month);
    //    weeks =(0xff & (week>>4))*10 +(0xf & week);
    //    days =(0xff & (day>>4))*10 +(0xf & day);
    //    hours =(0xff & (hour>>4))*10 +(0xf & hour);
    //    minutes =(0xff & (minute>>4))*10 +(0xf & minute);
    //
}
