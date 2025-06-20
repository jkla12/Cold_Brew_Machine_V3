#ifndef __UART_H
#define __UART_H
#include "gd32f30x.h"

#define UART0_ENABLE 1  //调试串口
#define UART1_ENABLE 1  //主串口屏
#define UART2_ENABLE 1  //水龙头串口屏
#define UART3_ENABLE 1  //485
#define UART4_ENABLE 1  //WIFI



#define DMA_MAX_TRANSFER_SIZE 256
typedef struct
{
	uint8_t rx_count;	    //接收数据长度
	uint8_t rx_flag;	    //接收标志位
	uint8_t rx_buf[200];
	uint8_t tx_count;
	uint8_t tx_flag;
	uint8_t tx_buf[256];
}uart_TypeDef;

#define QUEUE_SIZE 10
#define MAX_DATA_SIZE 256

typedef struct {
    uint8_t data[MAX_DATA_SIZE];
    uint32_t length;
} queue_item_t;

typedef struct {
    queue_item_t items[QUEUE_SIZE];
    uint32_t front;
    uint32_t rear;
    uint32_t count;
} queue_t;

#if UART0_ENABLE
    extern uart_TypeDef uart0_struct;
#endif

#if  UART1_ENABLE
    extern uart_TypeDef uart1_struct;
#endif

#if  UART2_ENABLE
    extern uart_TypeDef uart2_struct;
#endif

#if  UART3_ENABLE
    extern uart_TypeDef uart3_struct;
#endif

#if  UART4_ENABLE
    //extern uart_TypeDef uart4_struct;
#endif

void uart0_config(uint32_t baud);
void uart1_config(uint32_t baud);
void uart2_config(uint32_t baud);
void uart3_config(uint32_t baud);

void uart0_dma_init(void);
void uart0_dma_send(uint8_t *data,uint32_t size);
void uart0_receive_process(void);

void uart1_dma_init(void);
void uart1_dma_send(uint8_t *data,uint32_t size);
void uart1_receive_process(void);
void uart1_send_char(unsigned char t);

void uart2_dma_init(void);
void uart2_dma_send(uint8_t *data,uint32_t size);
void uart2_receive_process(void);

void uart3_dma_init(void);
void uart3_dma_send(uint8_t *data,uint32_t size);
void uart3_receive_process(void);

void uart1_send_str(const char *str);
void Uart3_send_buf(uint8_t *buf,uint8_t len);

void uart2_send_char(unsigned char t);

void uart4_config(uint32_t baud);
void uart4_send_buf(uint8_t *buf,uint8_t len);
void uart4_send_str(const char *str);

int enqueue(queue_t *queue, uint8_t *data, uint32_t length);
int dequeue(queue_t *queue, uint8_t **data, uint32_t *length);

#endif


