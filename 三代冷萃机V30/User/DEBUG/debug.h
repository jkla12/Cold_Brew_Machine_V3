#ifndef __DEBUG_H
#define __DEBUG_H

#include "gd32f30x.h"


#define SN_MAX_LEN    		 16    // 最多存 15 字节字符串 + 结束符 
#define VER_LEN              4     // 用4字节存数字，比如 10、20（uint32）
#define DATE_LEN             10    // 格式 yyyy/mm/dd
/* —— 协议常量 —— */
#define FRAME_HEADER_WRITE   0xFA    // 上位机发来的写/读请求帧头
#define FRAME_END_WRITE      0xAF    // 上位机请求的帧尾
#define FRAME_HEADER_READ    0xFB    // MCU 发给上位机的响应帧头
#define FRAME_END_READ       0xBF    // MCU 响应的帧尾

#define DATA_TYPE_SN         0x01    // 数据类型：SN 码
#define DATA_TYPE_HW     	 0x02    // 硬件版本号
#define DATA_TYPE_SW     	 0x03    // 软件版本号
#define DATA_TYPE_DATE       0x04    // 出厂日期

#define OPERATION_READ       0x00    // 操作：读
#define OPERATION_WRITE      0x01    // 操作：写

void debugProcess(void);


static uint8_t calc_checksum(const uint8_t *buf, uint16_t len) ;

#endif
