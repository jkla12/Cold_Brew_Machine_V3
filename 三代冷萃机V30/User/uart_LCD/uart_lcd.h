#ifndef __UART_LCD_H
#define __UART_LCD_H

#include "gd32f30x.h"

#define FIRMWARE_VER 562   /*!< ���ִ˹̼��汾������ʵ��Ļһ�£�ȷ���������ܿ���*/
#define CRC16_ENABLE 0     /*!< �����ҪCRC16У�鹦�ܣ��޸Ĵ˺�Ϊ1(��ʱ��Ҫ��VisualTFT��������CRCУ��)*/
#define CMD_MAX_SIZE 64    /*!<����ָ���С��������Ҫ�������������ô�һЩ*/
#define QUEUE_MAX_SIZE 128 /*!< ָ����ջ�������С��������Ҫ�������������ô�һЩ*/

#define SD_FILE_EN 0
#define uint16   unsigned short int
#define uint8    unsigned char
#define uint32   unsigned long
#define uchar    unsigned char
#define int16    short int
#define int32    long

// UART ID����
#define UART1_ID 1  // ��Ļ2
#define UART2_ID 2  // ��Ļ1

/*!
*  \brief  ��������Ƿ����CRC16У��
*  \param buffer ��У������ݣ�ĩβ�洢CRC16
*  \param n ���ݳ��ȣ�����CRC16
*  \return У��ͨ������1�����򷵻�0
*/
uint16 CheckCRC16(uint8 *buffer, uint16 n);

/*!
*  \brief  ��ʱ
*  \param  n ��ʱʱ��(���뵥λ)
*/
void DelayMS(unsigned int n);

/*!
*  \brief  �����豸���ã�����֮����Ҫ�����������޸Ĳ����ʡ���������������������ʽ
*  \param  uart_id UART ID
*/
void LockDeviceConfig(uint8 uart_id);

/*!
*  \brief  �����豸����
*  \param  uart_id UART ID
*/
void UnlockDeviceConfig(uint8 uart_id);

/*!
*  \brief  �޸Ĵ������Ĳ�����
*  \details  ������ѡ�Χ[0~14]����Ӧʵ�ʲ�����
*  {1200,2400,4800,9600,19200,38400,57600,115200,1000000,2000000,218750,437500,875000,921800,2500000}
*  \param  option ������ѡ��
*  \param  uart_id UART ID
*/
void SetCommBps(uint8 option, uint8 uart_id);

/*!
*  \brief  ������������
*  \param  uart_id UART ID
*/
void SetHandShake(uint8 uart_id);

/*!
*  \brief  ����ǰ��ɫ
*  \param  color ǰ��ɫ
*  \param  uart_id UART ID
*/
void SetFcolor(uint16 color, uint8 uart_id);

/*!
*  \brief  ���ñ���ɫ
*  \param  color ����ɫ
*  \param  uart_id UART ID
*/
void SetBcolor(uint16 color, uint8 uart_id);

/*!
*  \brief  ��ȡ��ɫ
*  \param  mode ģʽ
*  \param  x X����
*  \param  y Y����
*  \param  uart_id UART ID
*/
void ColorPicker(uint8 mode, uint16 x, uint16 y, uint8 uart_id);

/*!
*  \brief  �������
*  \param  uart_id UART ID
*/
void GUI_CleanScreen(uint8 uart_id);

/*!
*  \brief  �������ּ��
*  \param  x_w ������
*  \param  y_w ������
*  \param  uart_id UART ID
*/
void SetTextSpace(uint8 x_w, uint8 y_w, uint8 uart_id);

/*!
*  \brief  ����������ʾ����
*  \param  enable �Ƿ���������
*  \param  width ���
*  \param  height �߶�
*  \param  uart_id UART ID
*/
void SetFont_Region(uint8 enable, uint16 width, uint16 height, uint8 uart_id);

/*!
*  \brief  ���ù���ɫ
*  \param  fillcolor_dwon ��ɫ�½�
*  \param  fillcolor_up ��ɫ�Ͻ�
*  \param  uart_id UART ID
*/
void SetFilterColor(uint16 fillcolor_dwon, uint16 fillcolor_up, uint8 uart_id);

/*!
*  \brief  ��ʾ�ı�
*  \param  x λ��X����
*  \param  y λ��Y����
*  \param  back ����ɫ
*  \param  font ����
*  \param  strings �ַ�������
*  \param  uart_id UART ID
*/
void DisText(uint16 x, uint16 y, uint8 back, uint8 font, uchar *strings, uint8 uart_id);

/*!
*  \brief  ��ʾ���
*  \param  enable �Ƿ���ʾ
*  \param  x λ��X����
*  \param  y λ��Y����
*  \param  width ���
*  \param  height �߶�
*  \param  uart_id UART ID
*/
void DisCursor(uint8 enable, uint16 x, uint16 y, uint8 width, uint8 height, uint8 uart_id);

/*!
*  \brief  ��ʾȫ��ͼƬ
*  \param  image_id ͼƬ����
*  \param  masken �Ƿ�����͸������
*  \param  uart_id UART ID
*/
void DisFull_Image(uint16 image_id, uint8 masken, uint8 uart_id);

/*!
*  \brief  ָ��λ����ʾͼƬ
*  \param  x λ��X����
*  \param  y λ��Y����
*  \param  image_id ͼƬ����
*  \param  masken �Ƿ�����͸������
*  \param  uart_id UART ID
*/
void DisArea_Image(uint16 x, uint16 y, uint16 image_id, uint8 masken, uint8 uart_id);

/*!
*  \brief  ��ʾ�ü�ͼƬ
*  \param  x λ��X����
*  \param  y λ��Y����
*  \param  image_id ͼƬ����
*  \param  image_x ͼƬ�ü�λ��X����
*  \param  image_y ͼƬ�ü�λ��Y����
*  \param  image_l ͼƬ�ü�����
*  \param  image_w ͼƬ�ü��߶�
*  \param  masken �Ƿ�����͸������
*  \param  uart_id UART ID
*/
void DisCut_Image(uint16 x, uint16 y, uint16 image_id, uint16 image_x, uint16 image_y,
                  uint16 image_l, uint16 image_w, uint8 masken, uint8 uart_id);

/*!
*  \brief  ��ʾGIF����
*  \param  x λ��X����
*  \param  y λ��Y����
*  \param  flashimage_id ͼƬ����
*  \param  enable �Ƿ���ʾ
*  \param  playnum ���Ŵ���
*  \param  uart_id UART ID
*/
void DisFlashImage(uint16 x, uint16 y, uint16 flashimage_id, uint8 enable, uint8 playnum, uint8 uart_id);

/*!
*  \brief  ����
*  \param  x λ��X����
*  \param  y λ��Y����
*  \param  uart_id UART ID
*/
void GUI_Dot(uint16 x, uint16 y, uint8 uart_id);

/*!
*  \brief  ����
*  \param  x0 ��ʼλ��X����
*  \param  y0 ��ʼλ��Y����
*  \param  x1 ����λ��X����
*  \param  y1 ����λ��Y����
*  \param  uart_id UART ID
*/
void GUI_Line(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 uart_id);

/*!
*  \brief  ������
*  \param  mode ģʽ
*  \param  dot ���ݵ�
*  \param  dot_cnt ����
*  \param  uart_id UART ID
*/
void GUI_ConDots(uint8 mode, uint16 *dot, uint16 dot_cnt, uint8 uart_id);

/*!
*  \brief  x����Ⱦ�ʹ��ǰ��ɫ����
*  \param  x ������
*  \param  x_space ����
*  \param  dot_y һ����������
*  \param  dot_cnt ���������
*  \param  uart_id UART ID
*/
void GUI_ConSpaceDots(uint16 x, uint16 x_space, uint16 *dot_y, uint16 dot_cnt, uint8 uart_id);

/*!
*  \brief  ��������ƫ������ǰ��ɫ����
*  \param  x ������
*  \param  y �ݾ���
*  \param  dot_offset ƫ����
*  \param  dot_cnt ƫ��������
*  \param  uart_id UART ID
*/
void GUI_FcolorConOffsetDots(uint16 x, uint16 y, uint16 *dot_offset, uint16 dot_cnt, uint8 uart_id);

/*!
*  \brief  ��������ƫ�����ñ���ɫ����
*  \param  x ������
*  \param  y �ݾ���
*  \param  dot_offset ƫ����
*  \param  dot_cnt ƫ��������
*  \param  uart_id UART ID
*/
void GUI_BcolorConOffsetDots(uint16 x, uint16 y, uint8 *dot_offset, uint16 dot_cnt, uint8 uart_id);

/*!
*  \brief  ��ָ���Ķ���������ǰ��ɫ��������
*  \param  dot �����
*  \param  dot_cnt ƫ��������
*  \param  uart_id UART ID
*/
void GUI_FcolorConDots(uint16 *dot, uint16 dot_cnt, uint8 uart_id);

/*!
*  \brief  ��ָ���Ķ��������ñ���ɫ��������
*  \param  dot �����
*  \param  dot_cnt ƫ��������
*  \param  uart_id UART ID
*/
void GUI_BcolorConDots(uint16 *dot, uint16 dot_cnt, uint8 uart_id);

/*!
*  \brief  ������Բ
*  \param  x Բ��λ��X����
*  \param  y Բ��λ��Y����
*  \param  r �뾶
*  \param  uart_id UART ID
*/
void GUI_Circle(uint16 x, uint16 y, uint16 r, uint8 uart_id);

/*!
*  \brief  ��ʵ��Բ
*  \param  x Բ��λ��X����
*  \param  y Բ��λ��Y����
*  \param  r �뾶
*  \param  uart_id UART ID
*/
void GUI_CircleFill(uint16 x, uint16 y, uint16 r, uint8 uart_id);

/*!
*  \brief  ������
*  \param  x Բ��λ��X����
*  \param  y Բ��λ��Y����
*  \param  r �뾶
*  \param  sa ��ʼ�Ƕ�
*  \param  ea ��ֹ�Ƕ�
*  \param  uart_id UART ID
*/
void GUI_Arc(uint16 x, uint16 y, uint16 r, uint16 sa, uint16 ea, uint8 uart_id);

/*!
*  \brief  �����ľ���
*  \param  x0 ��ʼλ��X����
*  \param  y0 ��ʼλ��Y����
*  \param  x1 ����λ��X����
*  \param  y1 ����λ��Y����
*  \param  uart_id UART ID
*/
void GUI_Rectangle(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 uart_id);

/*!
*  \brief  ��ʵ�ľ���
*  \param  x0 ��ʼλ��X����
*  \param  y0 ��ʼλ��Y����
*  \param  x1 ����λ��X����
*  \param  y1 ����λ��Y����
*  \param  uart_id UART ID
*/
void GUI_RectangleFill(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 uart_id);

/*!
*  \brief  ��������Բ
*  \param  x0 ��ʼλ��X����
*  \param  y0 ��ʼλ��Y����
*  \param  x1 ����λ��X����
*  \param  y1 ����λ��Y����
*  \param  uart_id UART ID
*/
void GUI_Ellipse(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 uart_id);

/*!
*  \brief  ��ʵ����Բ
*  \param  x0 ��ʼλ��X����
*  \param  y0 ��ʼλ��Y����
*  \param  x1 ����λ��X����
*  \param  y1 ����λ��Y����
*  \param  uart_id UART ID
*/
void GUI_EllipseFill(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 uart_id);

/*!
*  \brief  ���ñ�������
*  \param  light_level ���ȼ���
*  \param  uart_id UART ID
*/
void SetBackLight(uint8 light_level, uint8 uart_id);

/*!
*  \brief  ����������
*  \param  time ����ʱ��(���뵥λ)
*  \param  uart_id UART ID
*/
void SetBuzzer(uint8 time, uint8 uart_id);

/*!
*  \brief  ����ɫ
*  \param  x0 ��ʼλ��X����
*  \param  y0 ��ʼλ��Y����
*  \param  x1 ����λ��X����
*  \param  y1 ����λ��Y����
*  \param  uart_id UART ID
*/
void GUI_AreaInycolor(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 uart_id);

/*!
*  \brief  ����������
*  \param  enable ����ʹ��
*  \param  beep_on ����������
*  \param  work_mode ��������ģʽ��0���¾��ϴ���1�ɿ����ϴ���2�����ϴ�����ֵ��3���º��ɿ����ϴ�����
*  \param  press_calibration �������������20��У׼��������0���ã�1����
*  \param  uart_id UART ID
*/
void SetTouchPaneOption(uint8 enable, uint8 beep_on, uint8 work_mode, uint8 press_calibration, uint8 uart_id);

/*!
*  \brief  У׼������
*  \param  uart_id UART ID
*/
void CalibrateTouchPane(uint8 uart_id);

/*!
*  \brief  ����������
*  \param  uart_id UART ID
*/
void TestTouchPane(uint8 uart_id);

/*!
*  \brief  �Զ����ڱ�������
*  \param  enable ʹ��
*  \param  bl_off_level ��������
*  \param  bl_on_level ��������
*  \param  bl_on_time ƫ��������
*  \param  uart_id UART ID
*/
void SetPowerSaving(uint8 enable, uint8 bl_off_level, uint8 bl_on_level, uint8 bl_on_time, uint8 uart_id);

/*!
*  \brief  ���õ�ǰд��ͼ��
*  \details  һ������ʵ��˫����Ч��(��ͼʱ������˸)��
*  \details  uint8 layer = 0;
*  \details  WriteLayer(layer);   ����д���
*  \details  ClearLayer(layer);   ʹͼ���͸��
*  \details  ���һϵ�л�ͼָ��
*  \details  DisText(100,100,0,4,"hello hmi!!!");
*  \details  DisplyLayer(layer);  �л���ʾ��
*  \details  layer = (layer+1)%2; ˫�����л�
*  \see DisplyLayer
*  \see ClearLayer
*  \param  layer ͼ����
*  \param  uart_id UART ID
*/
void WriteLayer(uint8 layer, uint8 uart_id);

/*!
*  \brief  ���õ�ǰ��ʾͼ��
*  \param  layer ͼ����
*  \param  uart_id UART ID
*/
void DisplyLayer(uint8 layer, uint8 uart_id);

/*!
*  \brief  ����ͼ��
*  \param  src_layer ԭʼͼ��
*  \param  dest_layer Ŀ��ͼ��
*  \param  uart_id UART ID
*/
void CopyLayer(uint8 src_layer, uint8 dest_layer, uint8 uart_id);

/*!
*  \brief  ���ͼ�㣬ʹͼ����͸��
*  \param  layer ͼ����
*  \param  uart_id UART ID
*/
void ClearLayer(uint8 layer, uint8 uart_id);

/*!
*  \brief  ��ʾRTCʱ��
*  \param  enable �Ƿ�����
*  \param  mode ģʽ
*  \param  font ����
*  \param  color ��ɫ
*  \param  x X����
*  \param  y Y����
*  \param  uart_id UART ID
*/
void GUI_DispRTC(uint8 enable, uint8 mode, uint8 font, uint16 color, uint16 x, uint16 y, uint8 uart_id);

/*!
*  \brief  д���ݵ��������û��洢��
*  \param  startAddress ��ʼ��ַ
*  \param  length �ֽ���
*  \param  _data ��д�������
*  \param  uart_id UART ID
*/
void WriteUserFlash(uint32 startAddress, uint16 length, uint8 *_data, uint8 uart_id);

/*!
*  \brief  �Ӵ������û��洢����ȡ����
*  \param  startAddress ��ʼ��ַ
*  \param  length �ֽ���
*  \param  uart_id UART ID
*/
void ReadUserFlash(uint32 startAddress, uint16 length, uint8 uart_id);

/*!
*  \brief  ��ȡ��ǰ����
*  \param  uart_id UART ID
*/
void GetScreen(uint8 uart_id);

/*!
*  \brief  ���õ�ǰ����
*  \param  screen_id ����ID
*  \param  uart_id UART ID
*/
void SetScreen(uint16 screen_id, uint8 uart_id);

/*!
*  \brief  ����/���û������
*  \details ����/����һ��ɶ�ʹ�ã����ڱ�����˸�����ˢ���ٶ�
*  \details �÷���
*  \details SetScreenUpdateEnable(0);//��ֹ����
*  \details һϵ�и��»����ָ��
*  \details SetScreenUpdateEnable(1);//��������
*  \param  enable 0���ã�1����
*  \param  uart_id UART ID
*/
void SetScreenUpdateEnable(uint8 enable, uint8 uart_id);

/*!
*  \brief  ���ÿؼ����뽹��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  focus �Ƿ�������뽹��
*  \param  uart_id UART ID
*/
void SetControlFocus(uint16 screen_id, uint16 control_id, uint8 focus, uint8 uart_id);

/*!
*  \brief  ��ʾ/���ؿؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  visible �Ƿ���ʾ
*  \param  uart_id UART ID
*/
void SetControlVisiable(uint16 screen_id, uint16 control_id, uint8 visible, uint8 uart_id);

/*!
*  \brief  ���ô����ؼ�ʹ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  enable �ؼ��Ƿ�ʹ��
*  \param  uart_id UART ID
*/
void SetControlEnable(uint16 screen_id, uint16 control_id, uint8 enable, uint8 uart_id);

/*!
*  \brief  ��ȡ�ؼ�ֵ
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void GetControlValue(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  ���ð�ť״̬
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  state ��ť״̬
*  \param  uart_id UART ID
*/
void SetButtonValue(uint16 screen_id, uint16 control_id, uchar state, uint8 uart_id);

/*!
*  \brief  �����ı�ֵ
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  str �ı�ֵ
*  \param  uart_id UART ID
*/
void SetTextValue(uint16 screen_id, uint16 control_id, uchar *str, uint8 uart_id);

#if FIRMWARE_VER>=908
/*!
*  \brief  �����ı�Ϊ����ֵ��Ҫ��FIRMWARE_VER>=908
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value �ı���ֵ
*  \param  sign 0-�޷��ţ�1-�з���
*  \param  fill_zero ����λ��������ʱ��ಹ��
*  \param  uart_id UART ID
*/
void SetTextInt32(uint16 screen_id, uint16 control_id, uint32 value, uint8 sign, uint8 fill_zero, uint8 uart_id);

/*!
*  \brief  �����ı������ȸ���ֵ��Ҫ��FIRMWARE_VER>=908
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value �ı���ֵ
*  \param  precision С��λ��
*  \param  show_zeros Ϊ1ʱ����ʾĩβ0
*  \param  uart_id UART ID
*/
void SetTextFloat(uint16 screen_id, uint16 control_id, float value, uint8 precision, uint8 show_zeros, uint8 uart_id);
#endif

/*!
*  \brief  ���ý���ֵ
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ��ֵ
*  \param  uart_id UART ID
*/
void SetProgressValue(uint16 screen_id, uint16 control_id, uint32 value, uint8 uart_id);

/*!
*  \brief  �����Ǳ�ֵ
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ��ֵ
*  \param  uart_id UART ID
*/
void SetMeterValue(uint16 screen_id, uint16 control_id, uint32 value, uint8 uart_id);

/*!
*  \brief  �����Ǳ�ֵ
*  \param  screen_id ����ID
*  \param  control_id ͼƬ�ؼ�ID
*  \param  value ��ֵ
*  \param  uart_id UART ID
*/
void Set_picMeterValue(uint16 screen_id, uint16 control_id, uint16 value, uint8 uart_id);

/*!
*  \brief  ���û�����
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ��ֵ
*  \param  uart_id UART ID
*/
void SetSliderValue(uint16 screen_id, uint16 control_id, uint32 value, uint8 uart_id);

/*!
*  \brief  ����ѡ��ؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  item ��ǰѡ��
*  \param  uart_id UART ID
*/
void SetSelectorValue(uint16 screen_id, uint16 control_id, uint8 item, uint8 uart_id);

/*!
*  \brief  ��ʼ���Ŷ���
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void AnimationStart(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  ֹͣ���Ŷ���
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void AnimationStop(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  ��ͣ���Ŷ���
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void AnimationPause(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  ����ָ��֡
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  frame_id ֡ID
*  \param  uart_id UART ID
*/
void AnimationPlayFrame(uint16 screen_id, uint16 control_id, uint8 frame_id, uint8 uart_id);

/*!
*  \brief  ������һ֡
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void AnimationPlayPrev(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  ������һ֡
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void AnimationPlayNext(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  ���߿ؼ�-���ͨ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  channel ͨ����
*  \param  color ��ɫ
*  \param  uart_id UART ID
*/
void GraphChannelAdd(uint16 screen_id, uint16 control_id, uint8 channel, uint16 color, uint8 uart_id);

/*!
*  \brief  ���߿ؼ�-ɾ��ͨ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  channel ͨ����
*  \param  uart_id UART ID
*/
void GraphChannelDel(uint16 screen_id, uint16 control_id, uint8 channel, uint8 uart_id);

/*!
*  \brief  ���߿ؼ�-�������
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  channel ͨ����
*  \param  pData ��������
*  \param  nDataLen ���ݸ���
*  \param  uart_id UART ID
*/
void GraphChannelDataAdd(uint16 screen_id, uint16 control_id, uint8 channel, uint8 *pData, uint16 nDataLen, uint8 uart_id);

/*!
*  \brief  ���߿ؼ�-�������
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  channel ͨ����
*  \param  uart_id UART ID
*/
void GraphChannelDataClear(uint16 screen_id, uint16 control_id, uint8 channel, uint8 uart_id);

/*!
*  \brief  ���߿ؼ�-������ͼ����
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  x_offset ˮƽƫ��
*  \param  x_mul ˮƽ����ϵ��
*  \param  y_offset ��ֱƫ��
*  \param  y_mul ��ֱ����ϵ��
*  \param  uart_id UART ID
*/
void GraphSetViewport(uint16 screen_id, uint16 control_id, int16 x_offset, uint16 x_mul, int16 y_offset, uint16 y_mul, uint8 uart_id);

/*!
*  \brief  ��ʼ��������
*  \param  screen_id ����ID
*  \param  uart_id UART ID
*/
void BatchBegin(uint16 screen_id, uint8 uart_id);

/*!
*  \brief  �������°�ť�ؼ�
*  \param  control_id �ؼ�ID
*  \param  state ��ֵ
*  \param  uart_id UART ID
*/
void BatchSetButtonValue(uint16 control_id, uint8 state, uint8 uart_id);

/*!
*  \brief  �������½������ؼ�
*  \param  control_id �ؼ�ID
*  \param  value ��ֵ
*  \param  uart_id UART ID
*/
void BatchSetProgressValue(uint16 control_id, uint32 value, uint8 uart_id);

/*!
*  \brief  �������»������ؼ�
*  \param  control_id �ؼ�ID
*  \param  value ��ֵ
*  \param  uart_id UART ID
*/
void BatchSetSliderValue(uint16 control_id, uint32 value, uint8 uart_id);

/*!
*  \brief  ���������Ǳ�ؼ�
*  \param  control_id �ؼ�ID
*  \param  value ��ֵ
*  \param  uart_id UART ID
*/
void BatchSetMeterValue(uint16 control_id, uint32 value, uint8 uart_id);

/*!
*  \brief  �����ַ�������
*  \param  str �ַ���
*  \return �ַ�������
*/
uint32 GetStringLen(uchar *str);

/*!
*  \brief  ���������ı��ؼ�
*  \param  control_id �ؼ�ID
*  \param  strings �ַ���
*  \param  uart_id UART ID
*/
void BatchSetText(uint16 control_id, uchar *strings, uint8 uart_id);

/*!
*  \brief  �������¶���/ͼ��ؼ�
*  \param  control_id �ؼ�ID
*  \param  frame_id ֡ID
*  \param  uart_id UART ID
*/
void BatchSetFrame(uint16 control_id, uint16 frame_id, uint8 uart_id);

#if FIRMWARE_VER>=908
/*!
*  \brief  �������ÿؼ��ɼ�
*  \param  control_id �ؼ�ID
*  \param  visible �Ƿ�ɼ�
*  \param  uart_id UART ID
*/
void BatchSetVisible(uint16 control_id, uint8 visible, uint8 uart_id);

/*!
*  \brief  �������ÿؼ�ʹ��
*  \param  control_id �ؼ�ID
*  \param  enable �Ƿ�ʹ��
*  \param  uart_id UART ID
*/
void BatchSetEnable(uint16 control_id, uint8 enable, uint8 uart_id);
#endif

/*!
*  \brief  ������������
*  \param  uart_id UART ID
*/
void BatchEnd(uint8 uart_id);

/*!
*  \brief  ���õ���ʱ�ؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  timeout ����ʱ(��)
*  \param  uart_id UART ID
*/
void SetTimer(uint16 screen_id, uint16 control_id, uint32 timeout, uint8 uart_id);

/*!
*  \brief  ��������ʱ�ؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void StartTimer(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  ֹͣ����ʱ�ؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void StopTimer(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  ��ͣ����ʱ�ؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void PauseTimer(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  ���ÿؼ�����ɫ
*  \details  ֧�ֿؼ������������ı�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  color ����ɫ
*  \param  uart_id UART ID
*/
void SetControlBackColor(uint16 screen_id, uint16 control_id, uint16 color, uint8 uart_id);

/*!
*  \brief  ���ÿؼ�ǰ��ɫ
*  \details  ֧�ֿؼ���������
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  color ǰ��ɫ
*  \param  uart_id UART ID
*/
void SetControlForeColor(uint16 screen_id, uint16 control_id, uint16 color, uint8 uart_id);

/*!
*  \brief  ��ʾ/���ص����˵��ؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  show �Ƿ���ʾ��Ϊ0ʱfocus_control_id��Ч
*  \param  focus_control_id �������ı��ؼ�(�˵��ؼ�������������ı��ؼ�)
*  \param  uart_id UART ID
*/
void ShowPopupMenu(uint16 screen_id, uint16 control_id, uint8 show, uint16 focus_control_id, uint8 uart_id);

/*!
*  \brief  ��ʾ/����ϵͳ����
*  \param  show 0���أ�1��ʾ
*  \param  x ������ʾλ��X����
*  \param  y ������ʾλ��Y����
*  \param  type 0С���̣�1ȫ����
*  \param  option 0�����ַ���1���룬2ʱ������
*  \param  max_len ����¼���ַ���������
*  \param  uart_id UART ID
*/
void ShowKeyboard(uint8 show, uint16 x, uint16 y, uint8 type, uint8 option, uint8 max_len, uint8 uart_id);

#if FIRMWARE_VER>=921
/*!
*  \brief  ����������
*  \param  ui_lang �û���������0~9
*  \param  sys_lang ϵͳ��������-0���ģ�1Ӣ��
*  \param  uart_id UART ID
*/
void SetLanguage(uint8 ui_lang, uint8 sys_lang, uint8 uart_id);
#endif

#if FIRMWARE_VER>=921
/*!
*  \brief  ��ʼ����ؼ���ֵ��FLASH
*  \param  version ���ݰ汾�ţ�������ָ������16λΪ���汾�ţ���16λΪ�ΰ汾��
*  \param  address �������û��洢���Ĵ�ŵ�ַ��ע���ֹ��ַ�ص�����ͻ
*  \param  uart_id UART ID
*/
void FlashBeginSaveControl(uint32 version, uint32 address, uint8 uart_id);

/*!
*  \brief  ����ĳ���ؼ�����ֵ��FLASH
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void FlashSaveControl(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  ��������ؼ���ֵ��FLASH
*  \param  uart_id UART ID
*/
void FlashEndSaveControl(uint8 uart_id);

/*!
*  \brief  ��FLASH�лָ��ؼ�����
*  \param  version ���ݰ汾�ţ����汾�ű�����洢ʱһ�£���������ʧ��
*  \param  address �������û��洢���Ĵ�ŵ�ַ
*  \param  uart_id UART ID
*/
void FlashRestoreControl(uint32 version, uint32 address, uint8 uart_id);
#endif

#if FIRMWARE_VER>=921
/*!
*  \brief  ������ʷ���߲�������ֵ(���ֽڣ�uint8��int8)
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ����������
*  \param  channel ͨ����
*  \param  uart_id UART ID
*/
void HistoryGraph_SetValueInt8(uint16 screen_id, uint16 control_id, uint8 *value, uint8 channel, uint8 uart_id);

/*!
*  \brief  ������ʷ���߲�������ֵ(˫�ֽڣ�uint16��int16)
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ����������
*  \param  channel ͨ����
*  \param  uart_id UART ID
*/
void HistoryGraph_SetValueInt16(uint16 screen_id, uint16 control_id, uint16 *value, uint8 channel, uint8 uart_id);

/*!
*  \brief  ������ʷ���߲�������ֵ(���ֽڣ�uint32��int32)
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ����������
*  \param  channel ͨ����
*  \param  uart_id UART ID
*/
void HistoryGraph_SetValueInt32(uint16 screen_id, uint16 control_id, uint32 *value, uint8 channel, uint8 uart_id);

/*!
*  \brief  ������ʷ���߲�������ֵ(�����ȸ�����)
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ����������
*  \param  channel ͨ����
*  \param  uart_id UART ID
*/
void HistoryGraph_SetValueFloat(uint16 screen_id, uint16 control_id, float *value, uint8 channel, uint8 uart_id);

/*!
*  \brief  ������ֹ��ʷ���߲���
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  enable 0-��ֹ��1-����
*  \param  uart_id UART ID
*/
void HistoryGraph_EnableSampling(uint16 screen_id, uint16 control_id, uint8 enable, uint8 uart_id);

/*!
*  \brief  ��ʾ��������ʷ����ͨ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  channel ͨ�����
*  \param  show 0-���أ�1-��ʾ
*  \param  uart_id UART ID
*/
void HistoryGraph_ShowChannel(uint16 screen_id, uint16 control_id, uint8 channel, uint8 show, uint8 uart_id);

/*!
*  \brief  ������ʷ����ʱ�䳤��(����������)
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  sample_count һ����ʾ�Ĳ�������
*  \param  uart_id UART ID
*/
void HistoryGraph_SetTimeLength(uint16 screen_id, uint16 control_id, uint16 sample_count, uint8 uart_id);

/*!
*  \brief  ��ʷ�������ŵ�ȫ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void HistoryGraph_SetTimeFullScreen(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  ������ʷ�������ű���ϵ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  zoom ���Űٷֱ�(zoom>100%ʱˮƽ������С�������Ŵ�)
*  \param  max_zoom �������ƣ�һ�������ʾ��������
*  \param  min_zoom �������ƣ�һ��������ʾ��������
*  \param  uart_id UART ID
*/
void HistoryGraph_SetTimeZoom(uint16 screen_id, uint16 control_id, uint16 zoom, uint16 max_zoom, uint16 min_zoom, uint8 uart_id);
#endif

#if SD_FILE_EN
/*!
*  \brief  ���SD���Ƿ����
*  \param  uart_id UART ID
*/
void SD_IsInsert(uint8 uart_id);

/*!
*  \brief  �򿪻򴴽��ļ�
*  \param  filename �ļ�����(��ASCII����)
*  \param  mode ģʽ����ѡ���ģʽ����FA_XXXX
*  \param  uart_id UART ID
*/
void SD_CreateFile(uint8 *filename, uint8 mode, uint8 uart_id);

/*!
*  \brief  �Ե�ǰʱ�䴴���ļ�������:20161015083000.txt
*  \param  ext �ļ���׺������ txt
*  \param  uart_id UART ID
*/
void SD_CreateFileByTime(uint8 *ext, uint8 uart_id);

/*!
*  \brief  �ڵ�ǰ�ļ�ĩβд������
*  \param  buffer ����
*  \param  dlc ���ݳ���
*  \param  uart_id UART ID
*/
void SD_WriteFile(uint8 *buffer, uint16 dlc, uint8 uart_id);

/*!
*  \brief  ��ȡ��ǰ�ļ�
*  \param  offset �ļ�λ��ƫ��
*  \param  dlc ���ݳ���
*  \param  uart_id UART ID
*/
void SD_ReadFile(uint32 offset, uint16 dlc, uint8 uart_id);

/*!
*  \brief  ��ȡ��ǰ�ļ�����
*  \param  uart_id UART ID
*/
void SD_GetFileSize(uint8 uart_id);

/*!
*  \brief  �رյ�ǰ�ļ�
*  \param  uart_id UART ID
*/
void SD_CloseFile(uint8 uart_id);
#endif

/*!
*  \brief  ��¼�ؼ�-��������
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value �澯ֵ
*  \param  time �澯������ʱ�䣬Ϊ0ʱʹ����Ļ�ڲ�ʱ��
*  \param  uart_id UART ID
*/
void Record_SetEvent(uint16 screen_id, uint16 control_id, uint16 value, uint8 *time, uint8 uart_id);

/*!
*  \brief  ��¼�ؼ�-�������
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value �澯ֵ
*  \param  time �澯�����ʱ�䣬Ϊ0ʱʹ����Ļ�ڲ�ʱ��
*  \param  uart_id UART ID
*/
void Record_ResetEvent(uint16 screen_id, uint16 control_id, uint16 value, uint8 *time, uint8 uart_id);

/*!
*  \brief  ��¼�ؼ�- ��ӳ����¼
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  record һ����¼(�ַ���)������ͨ���ֺŸ��������磺��һ��;�ڶ���;������;
*  \param  uart_id UART ID
*/
void Record_Add(uint16 screen_id, uint16 control_id, uint8 *record, uint8 uart_id);

/*!
*  \brief  ��¼�ؼ�-�����¼����
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void Record_Clear(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  ��¼�ؼ�-���ü�¼��ʾƫ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  offset ��ʾƫ�ƣ�������λ��
*  \param  uart_id UART ID
*/
void Record_SetOffset(uint16 screen_id, uint16 control_id, uint16 offset, uint8 uart_id);

/*!
*  \brief  ��¼�ؼ�-��ȡ��ǰ��¼��Ŀ
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void Record_GetCount(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  ��ȡ��ĻRTCʱ��
*  \param  uart_id UART ID
*/
void ReadRTC(uint8 uart_id);

/*!
*  \brief  ��������
*  \param  buffer ʮ�����Ƶ�����·��������
*  \param  uart_id UART ID
*/
void PlayMusic(uint8 *buffer, uint8 uart_id);

/*!
*  \brief  ��¼�ؼ�-��ȡָ��λ�õļ�¼
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  index ����
*  \param  uart_id UART ID
*/
void Record_Read(uint16 screen_id, uint16 control_id, uint16 index, uint8 uart_id);

/*!
*  \brief  ��¼�ؼ�-�޸�ָ��λ�õļ�¼
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  index ����
*  \param  record һ����¼(�ַ���)������ͨ���ֺŸ��������磺��һ��;�ڶ���;������;
*  \param  uart_id UART ID
*/
void Record_Modify(uint16 screen_id, uint16 control_id, uint16 index, uint8 *record, uint8 uart_id);

/*!
*  \brief  ��¼�ؼ�-ɾ��ָ��λ�õļ�¼
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  index ����
*  \param  uart_id UART ID
*/
void Record_Delete(uint16 screen_id, uint16 control_id, uint16 index, uint8 uart_id);

/*!
*  \brief  ��¼�ؼ�-��ָ��λ�ò����¼
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  index ����
*  \param  record һ����¼(�ַ���)������ͨ���ֺŸ��������磺��һ��;�ڶ���;������;
*  \param  uart_id UART ID
*/
void Record_Insert(uint16 screen_id, uint16 control_id, uint16 index, uint8 *record, uint8 uart_id);

/*!
*  \brief  ��¼�ؼ�-ѡ��ָ����¼����
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  index ����
*  \param  uart_id UART ID
*/
void Record_Select(uint16 screen_id, uint16 control_id, uint16 index, uint8 uart_id);




#endif
