#ifndef __LCD_CMD_QUEUE_H
#define __LCD_CMD_QUEUE_H

#include "uart_lcd.h"

typedef unsigned char qdata;
typedef unsigned short qsize;

/*! 
 *  \brief  ���ָ������
 */
extern void queue_reset(void);

extern void queue_reset2(void);

/*! 
 * \brief  ���ָ������
 * \detial ���ڽ��յ����ݣ�ͨ���˺�������ָ����� 
 *  \param  _data ָ������
 */
extern void queue_push(qdata _data);

extern void queue_push2(qdata _data);

/*! 
 *  \brief  ��ָ�������ȡ��һ��������ָ��
 *  \param  cmd ָ����ջ�����
 *  \param  buf_len ָ����ջ�������С
 *  \return  ָ��ȣ�0��ʾ������������ָ��
 */
extern qsize queue_find_cmd(qdata *cmd,qsize buf_len);

extern qsize queue_find_cmd2(qdata *cmd,qsize buf_len);

#endif

