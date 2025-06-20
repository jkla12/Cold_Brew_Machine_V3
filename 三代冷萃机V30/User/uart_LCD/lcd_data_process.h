#ifndef __LCD_DATA_PROCESS_H
#define __LCD_DATA_PROCESS_H

#include "uart_lcd.h"




typedef struct 
{
    uint8_t now_page;    //��ǰ����
    uint8_t language;    //����0:���� 1:Ӣ��
    uint8_t make_num;    //��ǰ��ȡ�������
    uint8_t coffe_set_par_change;   //�������ò����ı�
    uint8_t tea_set_par_change;     //�����ò����ı�
    uint8_t wine_set_par_change;    //�����ò����ı�
}LCD_data_TypeDef;

#define LOW_WATER       20//��ˮλ
#define vol_addr 	    7//ˮ����ַ
#define time_addr 	    8//ʱ���ַ
#define show_addr 	    13//��ǰ��ʾ��ַ
#define weight_addr 	25//���ص�ַ
#define collect1_addr   14//�ղ�1��ַ
#define collect2_addr   15//�ղ�2��ַ
#define collect3_addr   16//�ղ�3��ַ
#define collect4_addr   17//�ղ�4��ַ

#define collect1_button_addr   21//�ղ�1��ť��ַ
#define collect2_button_addr   22//�ղ�2��ť��ַ
#define collect3_button_addr   23//�ղ�3��ť��ַ
#define collect4_button_addr   24//�ղ�4��ť��ַ

#define NOTIFY_TOUCH_PRESS         0X01  //����������֪ͨ
#define NOTIFY_TOUCH_RELEASE       0X03  //�������ɿ�֪ͨ
#define NOTIFY_WRITE_FLASH_OK      0X0C  //дFLASH�ɹ�
#define NOTIFY_WRITE_FLASH_FAILD   0X0D  //дFLASHʧ��
#define NOTIFY_READ_FLASH_OK       0X0B  //��FLASH�ɹ�
#define NOTIFY_READ_FLASH_FAILD    0X0F  //��FLASHʧ��
#define NOTIFY_MENU                0X14  //�˵��¼�֪ͨ
#define NOTIFY_TIMER               0X43  //��ʱ����ʱ֪ͨ
#define NOTIFY_CONTROL             0XB1  //�ؼ�����֪ͨ
#define NOTIFY_READ_RTC            0XF7  //��ȡRTCʱ��
#define MSG_GET_CURRENT_SCREEN     0X01  //����ID�仯֪ͨ
#define MSG_GET_DATA               0X11  //�ؼ�����֪ͨ


//ˮ��ͷ���
#define WATERTAP_MAIN_PAGE  0
#define WATERTAP_SET_PAGE   1
#define MAIN_VOL1_ID    8   //��ҳ�ı�1��ַ
#define MAIN_VOL2_ID    9   //��ҳ�ı�2��ַ
#define MAIN_VOL3_ID    10  //��ҳ�ı�3��ַ
#define SET_VOL1_ID    3   //�����ı�1
#define SET_VOL2_ID    4   //�����ı�2
#define SET_VOL3_ID    5   //�����ı�3

#define PTR2U16(PTR) ((((uint8 *)(PTR))[0]<<8)|((uint8 *)(PTR))[1])  //�ӻ�����ȡ16λ����
#define PTR2U32(PTR) ((((uint8 *)(PTR))[0]<<24)|(((uint8 *)(PTR))[1]<<16)|(((uint8 *)(PTR))[2]<<8)|((uint8 *)(PTR))[3])  //�ӻ�����ȡ32λ����

#define vol_change_val  2   //ˮ�������޸�ֵ

//ҳ����Ϣ
enum pageType
{
    main_page=0x00,                   	//��ҳ��
    make_coffe_set_page,                //��ȡ��������ҳ��
	make_tea_set_page,				  	//��ȡ������ҳ��
	make_wine_set_page,				  	//��ȡ������ҳ��
	wash_page,                          //��ϴҳ��
	sanit_page,                         //����ҳ��
	setting_page,                       //����ҳ��
	drain_page,                         //��ˮҳ��
	production_page,                    //������Ϣҳ��
	language_page,                      //��������ҳ��
	wifi_setting_page,                  //WIFI����ҳ��
	set_time_page,                      //����ʱ��ҳ��
	device_info_page,                   //�豸��Ϣҳ��
	make_coffe_page,                    //��ȡ����ҳ��
	make_tea_page,					  	//��ȡ��ҳ��
	make_wine_page,					  	//��ȡ��ҳ��
    water_ingress_page,                 //��ˮ����ҳ��
    water_drain_page,                   //��ˮ����ҳ��
    water_level_page,                   //ˮλ����������ҳ��
    drain_choose_page,                  //��ˮѡ��ҳ��
};



enum CtrlType
{
    kCtrlUnknown=0x0,
    kCtrlButton=0x10,                     //��ť
    kCtrlText,                            //�ı�
    kCtrlProgress,                        //������
    kCtrlSlider,                          //������
    kCtrlMeter,                            //�Ǳ�
    kCtrlDropList,                        //�����б�
    kCtrlAnimation,                       //����
    kCtrlRTC,                             //ʱ����ʾ
    kCtrlGraph,                           //����ͼ�ؼ�
    kCtrlTable,                           //���ؼ�
    kCtrlMenu,                            //�˵��ؼ�
    kCtrlSelector,                        //ѡ��ؼ�
    kCtrlQRCode,                          //��ά��
};

#pragma pack(push)
#pragma pack(1)                           //���ֽڶ���

typedef struct
{
    uint8    cmd_head;                    //֡ͷ

    uint8    cmd_type;                    //��������(UPDATE_CONTROL)    
    uint8    ctrl_msg;                    //CtrlMsgType-ָʾ��Ϣ������
    uint16   screen_id;                   //������Ϣ�Ļ���ID
    uint16   control_id;                  //������Ϣ�Ŀؼ�ID
    uint8    control_type;                //�ؼ�����

    uint8    param[256];                  //�ɱ䳤�Ȳ��������256���ֽ�

    uint8  cmd_tail[4];                   //֡β
}CTRL_MSG,*PCTRL_MSG;

#pragma pack(pop)

void lcd_cmd_scan(void);
void lcd_cmd_scan2(void);
/*! 
*  \brief  ��Ϣ��������
*  \param msg ��������Ϣ
*  \param size ��Ϣ����
*/
void ProcessMessage( PCTRL_MSG msg, uint16 size );
void ProcessMessage2(PCTRL_MSG msg, uint16 size);

/*! 
*  \brief  �����л�֪ͨ
*  \details  ��ǰ����ı�ʱ(�����GetScreen)��ִ�д˺���
*  \param screen_id ��ǰ����ID
*/
void NotifyScreen(uint16 screen_id);
void NotifyScreen2(uint16 screen_id);
/*! 
*  \brief  ���������¼���Ӧ
*  \param press 1���´�������3�ɿ�������
*  \param x x����
*  \param y y����
*/
void NotifyTouchXY(uint8 press,uint16 x,uint16 y);


/*! 
*  \brief  ��ť�ؼ�֪ͨ
*  \details  ����ť״̬�ı�(�����GetControlValue)ʱ��ִ�д˺���
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*  \param state ��ť״̬��0����1����
*/
void NotifyButton(uint16 screen_id, uint16 control_id, uint8 state);
void NotifyButton2(uint16 screen_id, uint16 control_id, uint8  state);
/*! 
*  \brief  �ı��ؼ�֪ͨ
*  \details  ���ı�ͨ�����̸���(�����GetControlValue)ʱ��ִ�д˺���
*  \details  �ı��ؼ����������ַ�����ʽ�·���MCU������ı��ؼ������Ǹ���ֵ��
*  \details  ����Ҫ�ڴ˺����н��·��ַ�������ת�ظ���ֵ��
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*  \param str �ı��ؼ�����
*/
void NotifyText(uint16 screen_id, uint16 control_id, uint8 *str);
void NotifyText2(uint16 screen_id, uint16 control_id, uint8 *str);
/*!                                                                              
*  \brief  �������ؼ�֪ͨ                                                       
*  \details  ����GetControlValueʱ��ִ�д˺���                                  
*  \param screen_id ����ID                                                      
*  \param control_id �ؼ�ID                                                     
*  \param value ֵ                                                              
*/   
void NotifyProgress(uint16 screen_id, uint16 control_id, uint32 value);
void NotifyProgress2(uint16 screen_id, uint16 control_id, uint32 value);

/*!                                                                              
*  \brief  �������ؼ�֪ͨ                                                       
*  \details  ���������ı�(�����GetControlValue)ʱ��ִ�д˺���                  
*  \param screen_id ����ID                                                      
*  \param control_id �ؼ�ID                                                     
*  \param value ֵ
*/    
void NotifySlider(uint16 screen_id, uint16 control_id, uint32 value);
void NotifySlider2(uint16 screen_id, uint16 control_id, uint32 value);

/*! 
*  \brief  �Ǳ�ؼ�֪ͨ
*  \details  ����GetControlValueʱ��ִ�д˺���
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*  \param value ֵ
*/
void NotifyMeter(uint16 screen_id, uint16 control_id, uint32 value);
void NotifyMeter2(uint16 screen_id, uint16 control_id, uint32 value);

/*! 
*  \brief  �˵��ؼ�֪ͨ
*  \details  ���˵���»��ɿ�ʱ��ִ�д˺���
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*  \param item �˵�������
*  \param state ��ť״̬��0�ɿ���1����
*/
void NotifyMenu(uint16 screen_id, uint16 control_id, uint8  item, uint8  state);
void NotifyMenu2(uint16 screen_id, uint16 control_id, uint8 item, uint8 state);

/*! 
*  \brief  ѡ��ؼ�֪ͨ
*  \details  ��ѡ��ؼ��仯ʱ��ִ�д˺���
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*  \param item ��ǰѡ��
*/
void NotifySelector(uint16 screen_id, uint16 control_id, uint8  item);
void NotifySelector2(uint16 screen_id, uint16 control_id, uint8  item);

/*! 
*  \brief  ��ʱ����ʱ֪ͨ����
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*/
void NotifyTimer(uint16 screen_id, uint16 control_id);
void NotifyTimer2(uint16 screen_id, uint16 control_id);

/*! 
*  \brief  ��ȡ�û�FLASH״̬����
*  \param status 0ʧ�ܣ�1�ɹ�
*  \param _data ��������
*  \param length ���ݳ���
*/
void NotifyReadFlash(uint8 status,uint8 *_data,uint16 length);

/*! 
*  \brief  д�û�FLASH״̬����
*  \param status 0ʧ�ܣ�1�ɹ�
*/
void NotifyWriteFlash(uint8 status);
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
void NotifyReadRTC(uint8 year,uint8 month,uint8 week,uint8 day,uint8 hour,uint8 minute,uint8 second);

void UpdateUI(void);                                                                 //����UI����
// //ɨ�贮������
// void lcd_cmd_scan(void);
// void lcd_cmd_scan2(void);




#endif


