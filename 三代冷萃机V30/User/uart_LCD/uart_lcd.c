#include "uart_lcd.h"
#include "uart.h"
#include "systick.h"
#include <string.h>

// ͨ�÷��ͺ궨��
#define TX_8(P1, uart_id) SEND_DATA((P1)&0xFF, uart_id)                    // ���͵����ֽ�
#define TX_8N(P, N, uart_id) SendNU8((uint8 *)P, N, uart_id)               // ����N���ֽ�
#define TX_16(P1, uart_id) do { TX_8((P1)>>8, uart_id); TX_8(P1, uart_id); } while(0) // ����16λ����
#define TX_16N(P, N, uart_id) SendNU16((uint16 *)P, N, uart_id)            // ����N��16λ����
#define TX_32(P1, uart_id) do { TX_16((P1)>>16, uart_id); TX_16((P1)&0xFFFF, uart_id); } while(0) // ����32λ����

// UART ID����
#define UART1_ID 1  // ��Ļ2
#define UART2_ID 2  // ��Ļ1

#if(CRC16_ENABLE)

static uint16 _crc16 = 0xffff;
/*!
*  \brief ���CRC16У��
*  \param buffer ��У�������
*  \param n ���ݳ��ȣ�����CRC16
*  \param pcrc У����
*/
static void AddCRC16(uint8 *buffer, uint16 n, uint16 *pcrc)
{
    uint16 i, j, carry_flag, a;

    for (i = 0; i < n; i++)
    {
        *pcrc = *pcrc ^ buffer[i];
        for (j = 0; j < 8; j++)
        {
            a = *pcrc;
            carry_flag = a & 0x0001;
            *pcrc = *pcrc >> 1;
            if (carry_flag == 1)
                *pcrc = *pcrc ^ 0xa001;
        }
    }
}

/*!
*  \brief  ��������Ƿ����CRC16У��
*  \param buffer ��У������ݣ�ĩβ�洢CRC16
*  \param n ���ݳ��ȣ�����CRC16
*  \return У��ͨ������1�����򷵻�0
*/
uint16 CheckCRC16(uint8 *buffer, uint16 n)
{
    uint16 crc0 = 0x0;
    uint16 crc1 = 0xffff;

    if(n >= 2)
    {
        crc0 = ((buffer[n-2] << 8) | buffer[n-1]);
        AddCRC16(buffer, n-2, &crc1);
    }

    return (crc0 == crc1);
}

/*!
*  \brief  ����һ���ֽ�
*  \param  c �ֽ�����
*  \param  uart_id UART ID
*/
void SEND_DATA(uint8 c, uint8 uart_id)
{
    AddCRC16(&c, 1, &_crc16);
    if (uart_id == UART1_ID) {
        uart1_send_char(c);
    } else {
        uart2_send_char(c);
    }
}

/*!
*  \brief  ֡ͷ
*  \param  uart_id UART ID
*/
void BEGIN_CMD(uint8 uart_id)
{
    TX_8(0XEE, uart_id);
    _crc16 = 0XFFFF;                      // ��ʼ����CRC16
}

/*!
*  \brief  ֡β
*  \param  uart_id UART ID
*/
void END_CMD(uint8 uart_id)
{
    uint16 crc16 = _crc16;
    TX_16(crc16, uart_id);                // ����CRC16
    TX_32(0XFFFCFFFF, uart_id);
}

#else // NO CRC16

// ����һ���ֽ�
#define SEND_DATA(P, uart_id) ((uart_id) == UART1_ID ? uart1_send_char(P) : uart2_send_char(P))
// ֡ͷ
#define BEGIN_CMD(uart_id) TX_8(0XEE, uart_id)
// ֡β
#define END_CMD(uart_id) TX_32(0XFFFCFFFF, uart_id)

#endif

/*!
*  \brief  ��ʱ
*  \param  n ��ʱʱ��(���뵥λ)
*/
void DelayMS(unsigned int n)
{
    delay_1ms(n);
}

/*!
*  \brief  ���ڷ����ַ���
*  \param  str �ַ���
*  \param  uart_id UART ID
*/
void SendStrings(uchar *str, uint8 uart_id)
{
    while(*str)
    {
        TX_8(*str, uart_id);
        str++;
    }
}

/*!
*  \brief  ���ڷ���N���ֽ�
*  \param  pData ����ָ��
*  \param  nDataLen ���ݳ���
*  \param  uart_id UART ID
*/
void SendNU8(uint8 *pData, uint16 nDataLen, uint8 uart_id)
{
    for (uint16 i = 0; i < nDataLen; ++i)
    {
        TX_8(pData[i], uart_id);
    }
}

/*!
*  \brief  ���ڷ���N��16λ������
*  \param  pData ����ָ��
*  \param  nDataLen ���ݳ���
*  \param  uart_id UART ID
*/
void SendNU16(uint16 *pData, uint16 nDataLen, uint8 uart_id)
{
    for (uint16 i = 0; i < nDataLen; ++i)
    {
        TX_16(pData[i], uart_id);
    }
}

/*!
*  \brief  ������������
*  \param  uart_id UART ID
*/
void SetHandShake(uint8 uart_id)
{
    uint8_t send_buf[6] = {0xEE, 0x04, 0xFF, 0xFC, 0xFF, 0xFF};
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 6;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 6;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ����ǰ��ɫ
*  \param  color ǰ��ɫ
*  \param  uart_id UART ID
*/
void SetFcolor(uint16 color, uint8 uart_id)
{
    uint8_t send_buf[8] = {0xEE, 0x41, color>>8, color, 0xFF, 0xFC, 0xFF, 0xFF};
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 8;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 8;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ���ñ���ɫ
*  \param  color ����ɫ
*  \param  uart_id UART ID
*/
void SetBcolor(uint16 color, uint8 uart_id)
{
    uint8_t send_buf[8] = {0xEE, 0x42, color>>8, color, 0xFF, 0xFC, 0xFF, 0xFF};
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 8;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 8;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ��ȡ��ɫ
*  \param  mode ģʽ
*  \param  x X����
*  \param  y Y����
*  \param  uart_id UART ID
*/
void ColorPicker(uint8 mode, uint16 x, uint16 y, uint8 uart_id)
{
    uint8_t send_buf[11] = {0xEE, 0xA3, mode, x>>8, x, y>>8, y, 0xFF, 0xFC, 0xFF, 0xFF};
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 11;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 11;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  �������
*  \param  uart_id UART ID
*/
void GUI_CleanScreen(uint8 uart_id)
{
    uint8_t send_buf[6] = {0xEE, 0x01, 0xFF, 0xFC, 0xFF, 0xFF};
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 6;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 6;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  �������ּ��
*  \param  x_w ������
*  \param  y_w ������
*  \param  uart_id UART ID
*/
void SetTextSpace(uint8 x_w, uint8 y_w, uint8 uart_id)
{
    uint8_t send_buf[8] = {0xEE, 0x43, x_w, y_w, 0xFF, 0xFC, 0xFF, 0xFF};
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 8;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 8;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ����������ʾ����
*  \param  enable �Ƿ���������
*  \param  width ���
*  \param  height �߶�
*  \param  uart_id UART ID
*/
void SetFont_Region(uint8 enable, uint16 width, uint16 height, uint8 uart_id)
{
    uint8_t send_buf[11] = {0xEE, 0x45, enable, width>>8, width, height>>8, height, 0xFF, 0xFC, 0xFF, 0xFF};
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 11;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 11;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ���ù���ɫ
*  \param  fillcolor_dwon ��ɫ�½�
*  \param  fillcolor_up ��ɫ�Ͻ�
*  \param  uart_id UART ID
*/
void SetFilterColor(uint16 fillcolor_dwon, uint16 fillcolor_up, uint8 uart_id)
{
    uint8_t send_buf[10] = {0xEE, 0x44, fillcolor_dwon>>8, fillcolor_dwon, fillcolor_up>>8, fillcolor_up, 0xFF, 0xFC, 0xFF, 0xFF};
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 10;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 10;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ��ʾ�ı�
*  \param  x λ��X����
*  \param  y λ��Y����
*  \param  back ����ɫ
*  \param  font ����
*  \param  strings �ַ�������
*  \param  uart_id UART ID
*/
void DisText(uint16 x, uint16 y, uint8 back, uint8 font, uchar *strings, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x20, uart_id);
    TX_16(x, uart_id);
    TX_16(y, uart_id);
    TX_8(back, uart_id);
    TX_8(font, uart_id);
    SendStrings(strings, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ��ʾ���
*  \param  enable �Ƿ���ʾ
*  \param  x λ��X����
*  \param  y λ��Y����
*  \param  width ���
*  \param  height �߶�
*  \param  uart_id UART ID
*/
void DisCursor(uint8 enable, uint16 x, uint16 y, uint8 width, uint8 height, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x21, uart_id);
    TX_8(enable, uart_id);
    TX_16(x, uart_id);
    TX_16(y, uart_id);
    TX_8(width, uart_id);
    TX_8(height, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ��ʾȫ��ͼƬ
*  \param  image_id ͼƬ����
*  \param  masken �Ƿ�����͸������
*  \param  uart_id UART ID
*/
void DisFull_Image(uint16 image_id, uint8 masken, uint8 uart_id)
{
    uint8_t send_buf[9] = {0xEE, 0x31, image_id>>8, image_id, masken, 0xFF, 0xFC, 0xFF, 0xFF};
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 9;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 9;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ָ��λ����ʾͼƬ
*  \param  x λ��X����
*  \param  y λ��Y����
*  \param  image_id ͼƬ����
*  \param  masken �Ƿ�����͸������
*  \param  uart_id UART ID
*/
void DisArea_Image(uint16 x, uint16 y, uint16 image_id, uint8 masken, uint8 uart_id)
{
    uint8_t send_buf[13] = {0xEE, 0x32, x>>8, x, y>>8, y, image_id>>8, image_id, masken, 0xff, 0xFC, 0xFF, 0xFF};
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 13;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 13;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

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
                  uint16 image_l, uint16 image_w, uint8 masken, uint8 uart_id)
{
    uint8_t send_buf[21] = {
        0xEE, 0x33,
        x>>8, x,
        y>>8, y,
        image_id>>8, image_id,
        image_x>>8, image_x,
        image_y>>8, image_y,
        image_l>>8, image_l,
        image_w>>8, image_w,
        masken,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 21;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 21;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ��ʾGIF����
*  \param  x λ��X����
*  \param  y λ��Y����
*  \param  flashimage_id ͼƬ����
*  \param  enable �Ƿ���ʾ
*  \param  playnum ���Ŵ���
*  \param  uart_id UART ID
*/
void DisFlashImage(uint16 x, uint16 y, uint16 flashimage_id, uint8 enable, uint8 playnum, uint8 uart_id)
{
    uint8_t send_buf[14] = {
        0xEE, 0x80,
        x>>8, x,
        y>>8, y,
        flashimage_id>>8, flashimage_id,
        enable,
        playnum,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 14;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 14;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ����
*  \param  x λ��X����
*  \param  y λ��Y����
*  \param  uart_id UART ID
*/
void GUI_Dot(uint16 x, uint16 y, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x50, uart_id);
    TX_16(x, uart_id);
    TX_16(y, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ����
*  \param  x0 ��ʼλ��X����
*  \param  y0 ��ʼλ��Y����
*  \param  x1 ����λ��X����
*  \param  y1 ����λ��Y����
*  \param  uart_id UART ID
*/
void GUI_Line(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x51, uart_id);
    TX_16(x0, uart_id);
    TX_16(y0, uart_id);
    TX_16(x1, uart_id);
    TX_16(y1, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ������
*  \param  mode ģʽ
*  \param  dot ���ݵ�
*  \param  dot_cnt ����
*  \param  uart_id UART ID
*/
void GUI_ConDots(uint8 mode, uint16 *dot, uint16 dot_cnt, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x63, uart_id);
    TX_8(mode, uart_id);
    TX_16N(dot, dot_cnt*2, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  x����Ⱦ�ʹ��ǰ��ɫ����
*  \param  x ������
*  \param  x_space ����
*  \param  dot_y һ����������
*  \param  dot_cnt ���������
*  \param  uart_id UART ID
*/
void GUI_ConSpaceDots(uint16 x, uint16 x_space, uint16 *dot_y, uint16 dot_cnt, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x59, uart_id);
    TX_16(x, uart_id);
    TX_16(x_space, uart_id);
    TX_16N(dot_y, dot_cnt, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ��������ƫ������ǰ��ɫ����
*  \param  x ������
*  \param  y �ݾ���
*  \param  dot_offset ƫ����
*  \param  dot_cnt ƫ��������
*  \param  uart_id UART ID
*/
void GUI_FcolorConOffsetDots(uint16 x, uint16 y, uint16 *dot_offset, uint16 dot_cnt, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x75, uart_id);
    TX_16(x, uart_id);
    TX_16(y, uart_id);
    TX_16N(dot_offset, dot_cnt, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ��������ƫ�����ñ���ɫ����
*  \param  x ������
*  \param  y �ݾ���
*  \param  dot_offset ƫ����
*  \param  dot_cnt ƫ��������
*  \param  uart_id UART ID
*/
void GUI_BcolorConOffsetDots(uint16 x, uint16 y, uint8 *dot_offset, uint16 dot_cnt, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x76, uart_id);
    TX_16(x, uart_id);
    TX_16(y, uart_id);
    TX_16N((uint16*)dot_offset, dot_cnt, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  �Զ����ڱ�������
*  \param  enable ʹ��
*  \param  bl_off_level ��������
*  \param  bl_on_level ��������
*  \param  bl_on_time ƫ��������
*  \param  uart_id UART ID
*/
void SetPowerSaving(uint8 enable, uint8 bl_off_level, uint8 bl_on_level, uint8 bl_on_time, uint8 uart_id)
{
    uint8_t send_buf[10] = {
        0xEE, 0x77,
        enable,
        bl_off_level,
        bl_on_level,
        bl_on_time,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 10;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 10;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ��ָ���Ķ���������ǰ��ɫ��������
*  \param  dot �����
*  \param  dot_cnt ƫ��������
*  \param  uart_id UART ID
*/
void GUI_FcolorConDots(uint16 *dot, uint16 dot_cnt, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x68, uart_id);
    TX_16N(dot, dot_cnt*2, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ��ָ���Ķ��������ñ���ɫ��������
*  \param  dot �����
*  \param  dot_cnt ƫ��������
*  \param  uart_id UART ID
*/
void GUI_BcolorConDots(uint16 *dot, uint16 dot_cnt, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x69, uart_id);
    TX_16N(dot, dot_cnt*2, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ������Բ
*  \param  x Բ��λ��X����
*  \param  y Բ��λ��Y����
*  \param  r �뾶
*  \param  uart_id UART ID
*/
void GUI_Circle(uint16 x, uint16 y, uint16 r, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x52, uart_id);
    TX_16(x, uart_id);
    TX_16(y, uart_id);
    TX_16(r, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ��ʵ��Բ
*  \param  x Բ��λ��X����
*  \param  y Բ��λ��Y����
*  \param  r �뾶
*  \param  uart_id UART ID
*/
void GUI_CircleFill(uint16 x, uint16 y, uint16 r, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x53, uart_id);
    TX_16(x, uart_id);
    TX_16(y, uart_id);
    TX_16(r, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ������
*  \param  x Բ��λ��X����
*  \param  y Բ��λ��Y����
*  \param  r �뾶
*  \param  sa ��ʼ�Ƕ�
*  \param  ea ��ֹ�Ƕ�
*  \param  uart_id UART ID
*/
void GUI_Arc(uint16 x, uint16 y, uint16 r, uint16 sa, uint16 ea, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x67, uart_id);
    TX_16(x, uart_id);
    TX_16(y, uart_id);
    TX_16(r, uart_id);
    TX_16(sa, uart_id);
    TX_16(ea, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  �����ľ���
*  \param  x0 ��ʼλ��X����
*  \param  y0 ��ʼλ��Y����
*  \param  x1 ����λ��X����
*  \param  y1 ����λ��Y����
*  \param  uart_id UART ID
*/
void GUI_Rectangle(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x54, uart_id);
    TX_16(x0, uart_id);
    TX_16(y0, uart_id);
    TX_16(x1, uart_id);
    TX_16(y1, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ��ʵ�ľ���
*  \param  x0 ��ʼλ��X����
*  \param  y0 ��ʼλ��Y����
*  \param  x1 ����λ��X����
*  \param  y1 ����λ��Y����
*  \param  uart_id UART ID
*/
void GUI_RectangleFill(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x55, uart_id);
    TX_16(x0, uart_id);
    TX_16(y0, uart_id);
    TX_16(x1, uart_id);
    TX_16(y1, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ��������Բ
*  \param  x0 ��ʼλ��X����
*  \param  y0 ��ʼλ��Y����
*  \param  x1 ����λ��X����
*  \param  y1 ����λ��Y����
*  \param  uart_id UART ID
*/
void GUI_Ellipse(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x56, uart_id);
    TX_16(x0, uart_id);
    TX_16(y0, uart_id);
    TX_16(x1, uart_id);
    TX_16(y1, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ��ʵ����Բ
*  \param  x0 ��ʼλ��X����
*  \param  y0 ��ʼλ��Y����
*  \param  x1 ����λ��X����
*  \param  y1 ����λ��Y����
*  \param  uart_id UART ID
*/
void GUI_EllipseFill(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x57, uart_id);
    TX_16(x0, uart_id);
    TX_16(y0, uart_id);
    TX_16(x1, uart_id);
    TX_16(y1, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ���ñ�������
*  \param  light_level ���ȼ���
*  \param  uart_id UART ID
*/
void SetBackLight(uint8 light_level, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x60, uart_id);
    TX_8(light_level, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ����������
*  \param  time ����ʱ��(���뵥λ)
*  \param  uart_id UART ID
*/
void SetBuzzer(uint8 time, uint8 uart_id)
{
    uint8_t send_buf[7] = {0xEE, 0x61, time, 0xFF, 0xFC, 0xFF, 0xFF};
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 7;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 7;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ����ɫ
*  \param  x0 ��ʼλ��X����
*  \param  y0 ��ʼλ��Y����
*  \param  x1 ����λ��X����
*  \param  y1 ����λ��Y����
*  \param  uart_id UART ID
*/
void GUI_AreaInycolor(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x65, uart_id);
    TX_16(x0, uart_id);
    TX_16(y0, uart_id);
    TX_16(x1, uart_id);
    TX_16(y1, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ����������
*  \param  enable ����ʹ��
*  \param  beep_on ����������
*  \param  work_mode ��������ģʽ��0���¾��ϴ���1�ɿ����ϴ���2�����ϴ�����ֵ��3���º��ɿ����ϴ�����
*  \param  press_calibration �������������20��У׼��������0���ã�1����
*  \param  uart_id UART ID
*/
void SetTouchPaneOption(uint8 enable, uint8 beep_on, uint8 work_mode, uint8 press_calibration, uint8 uart_id)
{
    uint8 options = 0;

    if (enable)
        options |= 0x01;
    if (beep_on)
        options |= 0x02;
    if (work_mode)
        options |= (work_mode << 2);
    if (press_calibration)
        options |= (press_calibration << 5);

    BEGIN_CMD(uart_id);
    TX_8(0x70, uart_id);
    TX_8(options, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  У׼������
*  \param  uart_id UART ID
*/
void CalibrateTouchPane(uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x72, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ����������
*  \param  uart_id UART ID
*/
void TestTouchPane(uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x73, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  �����豸����
*  \param  uart_id UART ID
*/
void UnlockDeviceConfig(uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x09, uart_id);
    TX_8(0xDE, uart_id);
    TX_8(0xED, uart_id);
    TX_8(0x13, uart_id);
    TX_8(0x31, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  �����豸����
*  \param  uart_id UART ID
*/
void LockDeviceConfig(uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x08, uart_id);
    TX_8(0xA5, uart_id);
    TX_8(0x5A, uart_id);
    TX_8(0x5F, uart_id);
    TX_8(0xF5, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  �޸Ĵ������Ĳ�����
*  \details  ������ѡ�Χ[0~14]����Ӧʵ�ʲ�����
*  {1200,2400,4800,9600,19200,38400,57600,115200,1000000,2000000,218750,437500,875000,921800,2500000}
*  \param  option ������ѡ��
*  \param  uart_id UART ID
*/
void SetCommBps(uint8 option, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xA0, uart_id);
    TX_8(option, uart_id);
    END_CMD(uart_id);
}

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
void WriteLayer(uint8 layer, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xA1, uart_id);
    TX_8(layer, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ���õ�ǰ��ʾͼ��
*  \param  layer ͼ����
*  \param  uart_id UART ID
*/
void DisplyLayer(uint8 layer, uint8 uart_id)
{
    uint8_t send_buf[7] = {0xEE, 0xA2, layer, 0xFF, 0xFC, 0xFF, 0xFF};
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 7;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 7;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ����ͼ��
*  \param  src_layer ԭʼͼ��
*  \param  dest_layer Ŀ��ͼ��
*  \param  uart_id UART ID
*/
void CopyLayer(uint8 src_layer, uint8 dest_layer, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xA4, uart_id);
    TX_8(src_layer, uart_id);
    TX_8(dest_layer, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ���ͼ�㣬ʹͼ����͸��
*  \param  layer ͼ����
*  \param  uart_id UART ID
*/
void ClearLayer(uint8 layer, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x05, uart_id);
    TX_8(layer, uart_id);
    END_CMD(uart_id);
}

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
void GUI_DispRTC(uint8 enable, uint8 mode, uint8 font, uint16 color, uint16 x, uint16 y, uint8 uart_id)
{
    uint8_t send_buf[14] = {
        0xEE, 0x85,
        enable,
        mode,
        font,
        color,
        x>>8, x,
        y>>8, y,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 14;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 14;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  д���ݵ��������û��洢��
*  \param  startAddress ��ʼ��ַ
*  \param  length �ֽ���
*  \param  _data ��д�������
*  \param  uart_id UART ID
*/
void WriteUserFlash(uint32 startAddress, uint16 length, uint8 *_data, uint8 uart_id)
{
    if (length > 40) {  // �������ݳ��ȣ���ֹ���������
        length = 40;
    }
    
    uint8_t send_buf[50] = {0};
    send_buf[0] = 0xEE;
    send_buf[1] = 0x87;
    send_buf[2] = startAddress >> 24;
    send_buf[3] = startAddress >> 16;
    send_buf[4] = startAddress >> 8;
    send_buf[5] = startAddress;
    
    memcpy(&send_buf[6], _data, length);
    
    send_buf[6+length] = 0xFF;
    send_buf[7+length] = 0xFC;
    send_buf[8+length] = 0xFF;
    send_buf[9+length] = 0xFF;

    uint16 tx_count = 10 + length;
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = tx_count;
        memcpy(uart1_struct.tx_buf, send_buf, tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = tx_count;
        memcpy(uart2_struct.tx_buf, send_buf, tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  �Ӵ������û��洢����ȡ����
*  \param  startAddress ��ʼ��ַ
*  \param  length �ֽ���
*  \param  uart_id UART ID
*/
void ReadUserFlash(uint32 startAddress, uint16 length, uint8 uart_id)
{
    uint8_t send_buf[12] = {
        0xEE, 0x88,
        startAddress>>24, startAddress>>16, startAddress>>8, startAddress,
        length>>8, length,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 12;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 12;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ��ȡ��ǰ����
*  \param  uart_id UART ID
*/
void GetScreen(uint8 uart_id)
{
    uint8_t send_buf[7] = {0xEE, 0xB1, 0x01, 0xFF, 0xFC, 0xFF, 0xFF};
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 7;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 7;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ���õ�ǰ����
*  \param  screen_id ����ID
*  \param  uart_id UART ID
*/
void SetScreen(uint16 screen_id, uint8 uart_id)
{
    uint8_t send_buf[9] = {
        0xEE, 0xB1,
        0x00,
        screen_id>>8, screen_id,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 9;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 9;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

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
void SetScreenUpdateEnable(uint8 enable, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB3, uart_id);
    TX_8(enable, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ���ÿؼ����뽹��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  focus �Ƿ�������뽹��
*  \param  uart_id UART ID
*/
void SetControlFocus(uint16 screen_id, uint16 control_id, uint8 focus, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x02, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);
    TX_8(focus, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ��ʾ/���ؿؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  visible �Ƿ���ʾ
*  \param  uart_id UART ID
*/
void SetControlVisiable(uint16 screen_id, uint16 control_id, uint8 visible, uint8 uart_id)
{
    uint8_t send_buf[12] = {
        0xEE, 0xB1,
        0x03,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        visible,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 12;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 12;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ���ô����ؼ�ʹ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  enable �ؼ��Ƿ�ʹ��
*  \param  uart_id UART ID
*/
void SetControlEnable(uint16 screen_id, uint16 control_id, uint8 enable, uint8 uart_id)
{
    uint8_t send_buf[12] = {
        0xEE, 0xB1,
        0x04,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        enable,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 12;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 12;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ���ð�ť״̬
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  state ��ť״̬
*  \param  uart_id UART ID
*/
void SetButtonValue(uint16 screen_id, uint16 control_id, uchar state, uint8 uart_id)
{
    uint8_t send_buf[12] = {
        0xEE, 0xB1,
        0x10,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        state,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 12;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 12;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  �����ı�ֵ
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  str �ı�ֵ
*  \param  uart_id UART ID
*/
void SetTextValue(uint16 screen_id, uint16 control_id, uchar *str, uint8 uart_id)
{
    uint16_t len = strlen((char*)str);
    if (len > 180) {  // �����ַ������ȣ���ֹ���������
        len = 180;
    }
    
    uint8_t send_buf[200] = {0};
    send_buf[0] = 0xEE;
    send_buf[1] = 0xB1;
    send_buf[2] = 0x10;
    send_buf[3] = screen_id >> 8;
    send_buf[4] = screen_id;
    send_buf[5] = control_id >> 8;
    send_buf[6] = control_id;
    
    memcpy(&send_buf[7], str, len);
    
    send_buf[7+len] = 0xFF;
    send_buf[8+len] = 0xFC;
    send_buf[9+len] = 0xFF;
    send_buf[10+len] = 0xFF;
    
    uint16 tx_count = 11 + len;
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = tx_count;
        memcpy(uart1_struct.tx_buf, send_buf, tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = tx_count;
        memcpy(uart2_struct.tx_buf, send_buf, tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

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
void SetTextInt32(uint16 screen_id, uint16 control_id, uint32 value, uint8 sign, uint8 fill_zero, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x07, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);
    TX_8(sign ? 0X01 : 0X00, uart_id);
    TX_8((fill_zero & 0x0f) | 0x80, uart_id);
    TX_32(value, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  �����ı������ȸ���ֵ��Ҫ��FIRMWARE_VER>=908
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value �ı���ֵ
*  \param  precision С��λ��
*  \param  show_zeros Ϊ1ʱ����ʾĩβ0
*  \param  uart_id UART ID
*/
void SetTextFloat(uint16 screen_id, uint16 control_id, float value, uint8 precision, uint8 show_zeros, uint8 uart_id)
{
    uint8 i = 0;

    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x07, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);
    TX_8(0x02, uart_id);
    TX_8((precision & 0x0f) | (show_zeros ? 0x80 : 0x00), uart_id);

    for (i = 0; i < 4; ++i)
    {
        // ��Ҫ���ִ�С��
#if(0)
        TX_8(((uint8 *)&value)[i], uart_id);
#else
        TX_8(((uint8 *)&value)[3-i], uart_id);
#endif
    }
    END_CMD(uart_id);
}
#endif

/*!
*  \brief  ���ý���ֵ
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ��ֵ
*  \param  uart_id UART ID
*/
void SetProgressValue(uint16 screen_id, uint16 control_id, uint32 value, uint8 uart_id)
{
    uint8_t send_buf[15] = {
        0xEE, 0xB1,
        0x10,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        value>>24, value>>16, value>>8, value,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 15;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 15;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  �����Ǳ�ֵ
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ��ֵ
*  \param  uart_id UART ID
*/
void SetMeterValue(uint16 screen_id, uint16 control_id, uint32 value, uint8 uart_id)
{
    uint8_t send_buf[15] = {
        0xEE, 0xB1,
        0x10,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        value>>24, value>>16, value>>8, value,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 15;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 15;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  �����Ǳ�ֵ
*  \param  screen_id ����ID
*  \param  control_id ͼƬ�ؼ�ID
*  \param  value ��ֵ
*  \param  uart_id UART ID
*/
void Set_picMeterValue(uint16 screen_id, uint16 control_id, uint16 value, uint8 uart_id)
{
    uint8_t send_buf[13] = {
        0xEE, 0xB1,
        0x10,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        value>>8, value,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 13;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 13;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ���û�����
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ��ֵ
*  \param  uart_id UART ID
*/
void SetSliderValue(uint16 screen_id, uint16 control_id, uint32 value, uint8 uart_id)
{
    uint8_t send_buf[15] = {
        0xEE, 0xB1,
        0x10,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        value>>24, value>>16, value>>8, value,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 15;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 15;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ����ѡ��ؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  item ��ǰѡ��
*  \param  uart_id UART ID
*/
void SetSelectorValue(uint16 screen_id, uint16 control_id, uint8 item, uint8 uart_id)
{
    uint8_t send_buf[12] = {
        0xEE, 0xB1,
        0x10,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        item,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 12;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 12;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ��ȡ�ؼ�ֵ
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void GetControlValue(uint16 screen_id, uint16 control_id, uint8 uart_id)
{
    uint8_t send_buf[11] = {
        0xEE, 0xB1,
        0x11,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 11;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 11;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ��ʼ���Ŷ���
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void AnimationStart(uint16 screen_id, uint16 control_id, uint8 uart_id)
{
    uint8_t send_buf[11] = {
        0xEE, 0xB1,
        0x20,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 11;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 11;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ֹͣ���Ŷ���
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void AnimationStop(uint16 screen_id, uint16 control_id, uint8 uart_id)
{
    uint8_t send_buf[11] = {
        0xEE, 0xB1,
        0x21,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 11;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 11;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ��ͣ���Ŷ���
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void AnimationPause(uint16 screen_id, uint16 control_id, uint8 uart_id)
{
    uint8_t send_buf[11] = {
        0xEE, 0xB1,
        0x22,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 11;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 11;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ����ָ��֡
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  frame_id ֡ID
*  \param  uart_id UART ID
*/
void AnimationPlayFrame(uint16 screen_id, uint16 control_id, uint8 frame_id, uint8 uart_id)
{
    uint8_t send_buf[12] = {
        0xEE, 0xB1,
        0x23,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        frame_id,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 12;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 12;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ������һ֡
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void AnimationPlayPrev(uint16 screen_id, uint16 control_id, uint8 uart_id)
{
    uint8_t send_buf[11] = {
        0xEE, 0xB1,
        0x24,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 11;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 11;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ������һ֡
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void AnimationPlayNext(uint16 screen_id, uint16 control_id, uint8 uart_id)
{
    uint8_t send_buf[11] = {
        0xEE, 0xB1,
        0x25,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 11;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 11;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ���߿ؼ�-���ͨ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  channel ͨ����
*  \param  color ��ɫ
*  \param  uart_id UART ID
*/
void GraphChannelAdd(uint16 screen_id, uint16 control_id, uint8 channel, uint16 color, uint8 uart_id)
{
    uint8_t send_buf[14] = {
        0xEE, 0xB1,
        0x30,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        channel,
        color>>8, color,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 14;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 14;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ���߿ؼ�-ɾ��ͨ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  channel ͨ����
*  \param  uart_id UART ID
*/
void GraphChannelDel(uint16 screen_id, uint16 control_id, uint8 channel, uint8 uart_id)
{
    uint8_t send_buf[12] = {
        0xEE, 0xB1,
        0x31,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        channel,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 12;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 12;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ���߿ؼ�-�������
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  channel ͨ����
*  \param  pData ��������
*  \param  nDataLen ���ݸ���
*  \param  uart_id UART ID
*/
void GraphChannelDataAdd(uint16 screen_id, uint16 control_id, uint8 channel, uint8 *pData, uint16 nDataLen, uint8 uart_id)
{
    if (nDataLen > 480) {  // �������ݳ��ȣ���ֹ���������
        nDataLen = 480;
    }
    
    uint8_t send_buf[500] = {0};
    send_buf[0] = 0xEE;
    send_buf[1] = 0xB1;
    send_buf[2] = 0x32;
    send_buf[3] = screen_id >> 8;
    send_buf[4] = screen_id;
    send_buf[5] = control_id >> 8;
    send_buf[6] = control_id;
    send_buf[7] = channel;
    send_buf[8] = nDataLen >> 8;
    send_buf[9] = nDataLen;
    
    memcpy(&send_buf[10], pData, nDataLen);
    
    send_buf[10+nDataLen] = 0xFF;
    send_buf[11+nDataLen] = 0xFC;
    send_buf[12+nDataLen] = 0xFF;
    send_buf[13+nDataLen] = 0xFF;
    
    uint16 tx_count = 14 + nDataLen;
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = tx_count;
        memcpy(uart1_struct.tx_buf, send_buf, tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = tx_count;
        memcpy(uart2_struct.tx_buf, send_buf, tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ���߿ؼ�-�������
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  channel ͨ����
*  \param  uart_id UART ID
*/
void GraphChannelDataClear(uint16 screen_id, uint16 control_id, uint8 channel, uint8 uart_id)
{
    uint8_t send_buf[12] = {
        0xEE, 0xB1,
        0x33,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        channel,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 12;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 12;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

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
void GraphSetViewport(uint16 screen_id, uint16 control_id, int16 x_offset, uint16 x_mul, int16 y_offset, uint16 y_mul, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x34, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);
    TX_16(x_offset, uart_id);
    TX_16(x_mul, uart_id);
    TX_16(y_offset, uart_id);
    TX_16(y_mul, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ��ʼ��������
*  \param  screen_id ����ID
*  \param  uart_id UART ID
*/
void BatchBegin(uint16 screen_id, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x12, uart_id);
    TX_16(screen_id, uart_id);
}

/*!
*  \brief  �������°�ť�ؼ�
*  \param  control_id �ؼ�ID
*  \param  state ��ֵ
*  \param  uart_id UART ID
*/
void BatchSetButtonValue(uint16 control_id, uint8 state, uint8 uart_id)
{
    TX_16(control_id, uart_id);
    TX_16(1, uart_id);
    TX_8(state, uart_id);
}

/*!
*  \brief  �������½������ؼ�
*  \param  control_id �ؼ�ID
*  \param  value ��ֵ
*  \param  uart_id UART ID
*/
void BatchSetProgressValue(uint16 control_id, uint32 value, uint8 uart_id)
{
    TX_16(control_id, uart_id);
    TX_16(4, uart_id);
    TX_32(value, uart_id);
}

/*!
*  \brief  �������»������ؼ�
*  \param  control_id �ؼ�ID
*  \param  value ��ֵ
*  \param  uart_id UART ID
*/
void BatchSetSliderValue(uint16 control_id, uint32 value, uint8 uart_id)
{
    TX_16(control_id, uart_id);
    TX_16(4, uart_id);
    TX_32(value, uart_id);
}

/*!
*  \brief  ���������Ǳ�ؼ�
*  \param  control_id �ؼ�ID
*  \param  value ��ֵ
*  \param  uart_id UART ID
*/
void BatchSetMeterValue(uint16 control_id, uint32 value, uint8 uart_id)
{
    TX_16(control_id, uart_id);
    TX_16(4, uart_id);
    TX_32(value, uart_id);
}

/*!
*  \brief  �����ַ�������
*  \param  str �ַ���
*  \return �ַ�������
*/
uint32 GetStringLen(uchar *str)
{
    uchar *p = str;
    while (*str) {
        str++;
    }

    return (str - p);
}

/*!
*  \brief  ���������ı��ؼ�
*  \param  control_id �ؼ�ID
*  \param  strings �ַ���
*  \param  uart_id UART ID
*/
void BatchSetText(uint16 control_id, uchar *strings, uint8 uart_id)
{
    TX_16(control_id, uart_id);
    TX_16(GetStringLen(strings), uart_id);
    SendStrings(strings, uart_id);
}

/*!
*  \brief  �������¶���/ͼ��ؼ�
*  \param  control_id �ؼ�ID
*  \param  frame_id ֡ID
*  \param  uart_id UART ID
*/
void BatchSetFrame(uint16 control_id, uint16 frame_id, uint8 uart_id)
{
    TX_16(control_id, uart_id);
    TX_16(2, uart_id);
    TX_16(frame_id, uart_id);
}

#if FIRMWARE_VER>=908
/*!
*  \brief  �������ÿؼ��ɼ�
*  \param  control_id �ؼ�ID
*  \param  visible �Ƿ�ɼ�
*  \param  uart_id UART ID
*/
void BatchSetVisible(uint16 control_id, uint8 visible, uint8 uart_id)
{
    TX_16(control_id, uart_id);
    TX_8(1, uart_id);
    TX_8(visible, uart_id);
}

/*!
*  \brief  �������ÿؼ�ʹ��
*  \param  control_id �ؼ�ID
*  \param  enable �Ƿ�ʹ��
*  \param  uart_id UART ID
*/
void BatchSetEnable(uint16 control_id, uint8 enable, uint8 uart_id)
{
    TX_16(control_id, uart_id);
    TX_8(2, uart_id);
    TX_8(enable, uart_id);
}
#endif

/*!
*  \brief  ������������
*  \param  uart_id UART ID
*/
void BatchEnd(uint8 uart_id)
{
    END_CMD(uart_id);
}

/*!
*  \brief  ���õ���ʱ�ؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  timeout ����ʱ(��)
*  \param  uart_id UART ID
*/
void SetTimer(uint16 screen_id, uint16 control_id, uint32 timeout, uint8 uart_id)
{
    uint8_t send_buf[15] = {
        0xEE, 0xB1,
        0x40,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        timeout>>24, timeout>>16, timeout>>8, timeout,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 15;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 15;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ��������ʱ�ؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void StartTimer(uint16 screen_id, uint16 control_id, uint8 uart_id)
{
    uint8_t send_buf[11] = {
        0xEE, 0xB1,
        0x41,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 11;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 11;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ֹͣ����ʱ�ؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void StopTimer(uint16 screen_id, uint16 control_id, uint8 uart_id)
{
    uint8_t send_buf[11] = {
        0xEE, 0xB1,
        0x42,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 11;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 11;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ��ͣ����ʱ�ؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void PauseTimer(uint16 screen_id, uint16 control_id, uint8 uart_id)
{
    uint8_t send_buf[11] = {
        0xEE, 0xB1,
        0x44,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 11;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 11;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ���ÿؼ�����ɫ
*  \details  ֧�ֿؼ������������ı�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  color ����ɫ
*  \param  uart_id UART ID
*/
void SetControlBackColor(uint16 screen_id, uint16 control_id, uint16 color, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x18, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);
    TX_16(color, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ���ÿؼ�ǰ��ɫ
*  \details  ֧�ֿؼ���������
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  color ǰ��ɫ
*  \param  uart_id UART ID
*/
void SetControlForeColor(uint16 screen_id, uint16 control_id, uint16 color, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x19, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);
    TX_16(color, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ��ʾ/���ص����˵��ؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  show �Ƿ���ʾ��Ϊ0ʱfocus_control_id��Ч
*  \param  focus_control_id �������ı��ؼ�(�˵��ؼ�������������ı��ؼ�)
*  \param  uart_id UART ID
*/
void ShowPopupMenu(uint16 screen_id, uint16 control_id, uint8 show, uint16 focus_control_id, uint8 uart_id)
{
    uint8_t send_buf[14] = {
        0xEE, 0xB1,
        0x13,
        screen_id>>8, screen_id,
        control_id>>8, control_id,
        show,
        focus_control_id>>8, focus_control_id,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 14;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 14;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

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
void ShowKeyboard(uint8 show, uint16 x, uint16 y, uint8 type, uint8 option, uint8 max_len, uint8 uart_id)
{
    uint8_t send_buf[14] = {
        0xEE, 0x86,
        show,
        x>>8, x,
        y>>8, y,
        type,
        option,
        max_len,
        0xFF, 0xFC, 0xFF, 0xFF
    };
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 14;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 14;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

#if FIRMWARE_VER>=921
/*!
*  \brief  ����������
*  \param  ui_lang �û���������0~9
*  \param  sys_lang ϵͳ��������-0���ģ�1Ӣ��
*  \param  uart_id UART ID
*/
void SetLanguage(uint8 ui_lang, uint8 sys_lang, uint8 uart_id)
{
    uint8 lang = ui_lang;
    if (sys_lang) lang |= 0x80;

    BEGIN_CMD(uart_id);
    TX_8(0xC1, uart_id);
    TX_8(lang, uart_id);
    TX_8(0xC1+lang, uart_id); // У�飬��ֹ�����޸�����
    END_CMD(uart_id);
}
#endif

#if FIRMWARE_VER>=921
/*!
*  \brief  ��ʼ����ؼ���ֵ��FLASH
*  \param  version ���ݰ汾�ţ�������ָ������16λΪ���汾�ţ���16λΪ�ΰ汾��
*  \param  address �������û��洢���Ĵ�ŵ�ַ��ע���ֹ��ַ�ص�����ͻ
*  \param  uart_id UART ID
*/
void FlashBeginSaveControl(uint32 version, uint32 address, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0xAA, uart_id);
    TX_32(version, uart_id);
    TX_32(address, uart_id);
}

/*!
*  \brief  ����ĳ���ؼ�����ֵ��FLASH
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void FlashSaveControl(uint16 screen_id, uint16 control_id, uint8 uart_id)
{
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);
}

/*!
*  \brief  ��������ؼ���ֵ��FLASH
*  \param  uart_id UART ID
*/
void FlashEndSaveControl(uint8 uart_id)
{
    END_CMD(uart_id);
}

/*!
*  \brief  ��FLASH�лָ��ؼ�����
*  \param  version ���ݰ汾�ţ����汾�ű�����洢ʱһ�£���������ʧ��
*  \param  address �������û��洢���Ĵ�ŵ�ַ
*  \param  uart_id UART ID
*/
void FlashRestoreControl(uint32 version, uint32 address, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0xAB, uart_id);
    TX_32(version, uart_id);
    TX_32(address, uart_id);
    END_CMD(uart_id);
}
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
void HistoryGraph_SetValueInt8(uint16 screen_id, uint16 control_id, uint8 *value, uint8 channel, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x60, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);
    TX_8N(value, channel, uart_id);
    END_CMD(uart_id);
}


/*!
*  \brief  ������ʷ���߲�������ֵ(˫�ֽڣ�uint16��int16)
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ����������
*  \param  channel ͨ����
*  \param  uart_id UART ID
*/
void HistoryGraph_SetValueInt16(uint16 screen_id, uint16 control_id, uint16 *value, uint8 channel, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x60, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);
    TX_16N(value, channel, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ������ʷ���߲�������ֵ(���ֽڣ�uint32��int32)
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ����������
*  \param  channel ͨ����
*  \param  uart_id UART ID
*/
void HistoryGraph_SetValueInt32(uint16 screen_id, uint16 control_id, uint32 *value, uint8 channel, uint8 uart_id)
{
    uint8 i = 0;

    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x60, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);

    for (; i < channel; ++i)
    {
        TX_32(value[i], uart_id);
    }

    END_CMD(uart_id);
}

/*!
*  \brief  ������ʷ���߲�������ֵ(�����ȸ�����)
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ����������
*  \param  channel ͨ����
*  \param  uart_id UART ID
*/
void HistoryGraph_SetValueFloat(uint16 screen_id, uint16 control_id, float *value, uint8 channel, uint8 uart_id)
{
    uint8 i = 0;
    uint32 tmp = 0;

    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x60, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);

    for (; i < channel; ++i)
    {
        tmp = *(uint32 *)(value+i);
        TX_32(tmp, uart_id);
    }

    END_CMD(uart_id);
}

/*!
*  \brief  ������ֹ��ʷ���߲���
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  enable 0-��ֹ��1-����
*  \param  uart_id UART ID
*/
void HistoryGraph_EnableSampling(uint16 screen_id, uint16 control_id, uint8 enable, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x61, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);
    TX_8(enable, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ��ʾ��������ʷ����ͨ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  channel ͨ�����
*  \param  show 0-���أ�1-��ʾ
*  \param  uart_id UART ID
*/
void HistoryGraph_ShowChannel(uint16 screen_id, uint16 control_id, uint8 channel, uint8 show, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x62, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);
    TX_8(channel, uart_id);
    TX_8(show, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ������ʷ����ʱ�䳤��(����������)
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  sample_count һ����ʾ�Ĳ�������
*  \param  uart_id UART ID
*/
void HistoryGraph_SetTimeLength(uint16 screen_id, uint16 control_id, uint16 sample_count, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x63, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);
    TX_8(0x00, uart_id);
    TX_16(sample_count, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ��ʷ�������ŵ�ȫ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void HistoryGraph_SetTimeFullScreen(uint16 screen_id, uint16 control_id, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x63, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);
    TX_8(0x01, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ������ʷ�������ű���ϵ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  zoom ���Űٷֱ�(zoom>100%ʱˮƽ������С�������Ŵ�)
*  \param  max_zoom �������ƣ�һ�������ʾ��������
*  \param  min_zoom �������ƣ�һ��������ʾ��������
*  \param  uart_id UART ID
*/
void HistoryGraph_SetTimeZoom(uint16 screen_id, uint16 control_id, uint16 zoom, uint16 max_zoom, uint16 min_zoom, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x63, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);
    TX_8(0x02, uart_id);
    TX_16(zoom, uart_id);
    TX_16(max_zoom, uart_id);
    TX_16(min_zoom, uart_id);
    END_CMD(uart_id);
}
#endif

#if SD_FILE_EN
/*!
*  \brief  ���SD���Ƿ����
*  \param  uart_id UART ID
*/
void SD_IsInsert(uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x36, uart_id);
    TX_8(0x01, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  �򿪻򴴽��ļ�
*  \param  filename �ļ�����(��ASCII����)
*  \param  mode ģʽ����ѡ���ģʽ����FA_XXXX
*  \param  uart_id UART ID
*/
void SD_CreateFile(uint8 *filename, uint8 mode, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x36, uart_id);
    TX_8(0x05, uart_id);
    TX_8(mode, uart_id);
    SendStrings(filename, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  �Ե�ǰʱ�䴴���ļ�������:20161015083000.txt
*  \param  ext �ļ���׺������ txt
*  \param  uart_id UART ID
*/
void SD_CreateFileByTime(uint8 *ext, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x36, uart_id);
    TX_8(0x02, uart_id);
    SendStrings(ext, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  �ڵ�ǰ�ļ�ĩβд������
*  \param  buffer ����
*  \param  dlc ���ݳ���
*  \param  uart_id UART ID
*/
void SD_WriteFile(uint8 *buffer, uint16 dlc, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x36, uart_id);
    TX_8(0x03, uart_id);
    TX_16(dlc, uart_id);
    TX_8N(buffer, dlc, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ��ȡ��ǰ�ļ�
*  \param  offset �ļ�λ��ƫ��
*  \param  dlc ���ݳ���
*  \param  uart_id UART ID
*/
void SD_ReadFile(uint32 offset, uint16 dlc, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x36, uart_id);
    TX_8(0x07, uart_id);
    TX_32(offset, uart_id);
    TX_16(dlc, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ��ȡ��ǰ�ļ�����
*  \param  uart_id UART ID
*/
void SD_GetFileSize(uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x36, uart_id);
    TX_8(0x06, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  �رյ�ǰ�ļ�
*  \param  uart_id UART ID
*/
void SD_CloseFile(uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0x36, uart_id);
    TX_8(0x04, uart_id);
    END_CMD(uart_id);
}
#endif // SD_FILE_EN

/*!
*  \brief  ��¼�ؼ�-��������
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value �澯ֵ
*  \param  time �澯������ʱ�䣬Ϊ0ʱʹ����Ļ�ڲ�ʱ��
*  \param  uart_id UART ID
*/
void Record_SetEvent(uint16 screen_id, uint16 control_id, uint16 value, uint8 *time, uint8 uart_id)
{
    uint8 i = 0;

    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x50, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);
    TX_16(value, uart_id);

    if (time)
    {
        for (i = 0; i < 7; ++i)
            TX_8(time[i], uart_id);
    }

    END_CMD(uart_id);
}

/*!
*  \brief  ��¼�ؼ�-�������
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value �澯ֵ
*  \param  time �澯�����ʱ�䣬Ϊ0ʱʹ����Ļ�ڲ�ʱ��
*  \param  uart_id UART ID
*/
void Record_ResetEvent(uint16 screen_id, uint16 control_id, uint16 value, uint8 *time, uint8 uart_id)
{
    uint8 i = 0;

    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x51, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);
    TX_16(value, uart_id);

    if (time)
    {
        for (i = 0; i < 7; ++i)
            TX_8(time[i], uart_id);
    }

    END_CMD(uart_id);
}

/*!
*  \brief  ��¼�ؼ�- ��ӳ����¼
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  record һ����¼(�ַ���)������ͨ���ֺŸ��������磺��һ��;�ڶ���;������;
*  \param  uart_id UART ID
*/
void Record_Add(uint16 screen_id, uint16 control_id, uint8 *record, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x52, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);

    SendStrings(record, uart_id);

    END_CMD(uart_id);
}

/*!
*  \brief  ��¼�ؼ�-�����¼����
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void Record_Clear(uint16 screen_id, uint16 control_id, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x53, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ��¼�ؼ�-���ü�¼��ʾƫ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  offset ��ʾƫ�ƣ�������λ��
*  \param  uart_id UART ID
*/
void Record_SetOffset(uint16 screen_id, uint16 control_id, uint16 offset, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x54, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);
    TX_16(offset, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ��¼�ؼ�-��ȡ��ǰ��¼��Ŀ
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  uart_id UART ID
*/
void Record_GetCount(uint16 screen_id, uint16 control_id, uint8 uart_id)
{
    BEGIN_CMD(uart_id);
    TX_8(0xB1, uart_id);
    TX_8(0x55, uart_id);
    TX_16(screen_id, uart_id);
    TX_16(control_id, uart_id);
    END_CMD(uart_id);
}

/*!
*  \brief  ��ȡ��ĻRTCʱ��
*  \param  uart_id UART ID
*/
void ReadRTC(uint8 uart_id)
{
    uint8_t send_buf[6] = {0xEE, 0x82, 0xFF, 0xFC, 0xFF, 0xFF};
    
    if (uart_id == UART1_ID) {
        uart1_struct.tx_count = 6;
        memcpy(uart1_struct.tx_buf, send_buf, uart1_struct.tx_count);
        uart1_dma_send(uart1_struct.tx_buf, uart1_struct.tx_count);
    } else {
        uart2_struct.tx_count = 6;
        memcpy(uart2_struct.tx_buf, send_buf, uart2_struct.tx_count);
        uart2_dma_send(uart2_struct.tx_buf, uart2_struct.tx_count);
    }
}

/*!
*  \brief  ��������
*  \param  buffer ʮ�����Ƶ�����·��������
*  \param  uart_id UART ID
*/
void PlayMusic(uint8 *buffer, uint8 uart_id)
{
    uint8 i = 0;

    BEGIN_CMD(uart_id);
    if (buffer)
    {
        for (i = 0; i < 19; ++i)
            TX_8(buffer[i], uart_id);
    }
    END_CMD(uart_id);
}
