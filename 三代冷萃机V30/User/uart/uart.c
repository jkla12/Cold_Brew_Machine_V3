#include "uart.h"
#include "lcd_cmd_queue.h"
#include <stdio.h>
#include "string.h"


#if UART0_ENABLE
uart_TypeDef uart0_struct;
queue_t tx0_queue = {{{{0}, 0}}, 0, 0, 0};
#endif

#if UART1_ENABLE
uart_TypeDef uart1_struct;
queue_t tx1_queue = {{{{0}, 0}}, 0, 0, 0};
#endif

#if UART2_ENABLE
uart_TypeDef uart2_struct;
queue_t tx2_queue = {{{{0}, 0}}, 0, 0, 0};
#endif

#if UART3_ENABLE
uart_TypeDef uart3_struct;
#endif

#if UART4_ENABLE
//uart_TypeDef uart4_struct;
#endif





/**********************************************************************
**函数原型:	void uart0_config(uint32_t baud)
**函数作用:	串口0初始化
**入口参数:	uint32_t baud 波特率
**          
**出口参数:	无
**备    注:	无
************************************************************************/
void uart0_config(uint32_t baud)
{
    /* 使能串口时钟 */
    rcu_periph_clock_enable(RCU_USART0);
    /* 使能GPIO时钟 */
    rcu_periph_clock_enable(RCU_GPIOA);
    
    /* 配置TX引脚 */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    /* 配置RX引脚 */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* 串口配置 */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, baud);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
	//开启DMA传输
	usart_dma_receive_config(USART0, USART_RECEIVE_DMA_ENABLE);
    usart_dma_transmit_config(USART0, USART_TRANSMIT_DMA_ENABLE);
		
    nvic_irq_enable(USART0_IRQn,1,2);
    usart_interrupt_enable(USART0,USART_INT_IDLE);  //开启空闲中断
    usart_enable(USART0);
}
/**********************************************************************
**函数原型:	void uart0_dma_init(void)
**函数作用:	串口0 DMA 配置
**入口参数:	无
**          
**出口参数:	无
**备    注:	无
************************************************************************/
void uart0_dma_init(void)
{
    dma_parameter_struct dma_init_struct;
    rcu_periph_clock_enable(RCU_DMA0);

    /*配置串口0发送DMA通道3 */
    dma_deinit(DMA0, DMA_CH3);
    dma_struct_para_init(&dma_init_struct);

    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_addr = (uint32_t)NULL;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = 0;
    dma_init_struct.periph_addr = (uint32_t)(&USART_DATA(USART0));
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH3, &dma_init_struct);

	dma_circulation_disable(DMA0, DMA_CH3);
    dma_memory_to_memory_disable(DMA0, DMA_CH3);
		
	dma_interrupt_enable(DMA0, DMA_CH3, DMA_INT_FTF);
	dma_channel_disable(DMA0, DMA_CH3);
	dma_flag_clear(DMA0, DMA_CH3, DMA_FLAG_G);
	nvic_irq_enable(DMA0_Channel3_IRQn, 0, 4);
		
		
    /*配置串口0接收DMA通道4 */
    dma_deinit(DMA0, DMA_CH4);
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)uart0_struct.rx_buf;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = 256;
    dma_init_struct.periph_addr = (uint32_t)(&USART_DATA(USART0));
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH4, &dma_init_struct);


    /* 禁用DMA循环模式 */
    dma_circulation_disable(DMA0, DMA_CH4);
    /* 开启DMA通道4 */
    dma_channel_enable(DMA0, DMA_CH4);


    
}
/**********************************************************************
**函数原型:	void USART0_IRQHandler(void)
**函数作用:	串口0中断函数
**入口参数:	无
**          
**出口参数:	无
**备    注:	无
************************************************************************/
void USART0_IRQHandler(void)
{
    if(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_IDLE)) {
        /* clear IDLE flag */
        usart_data_receive(USART0);

        /* number of data received */
        uart0_struct.rx_count = 256 - (dma_transfer_number_get(DMA0, DMA_CH4));
        uart0_struct.rx_flag = 1;

        /* disable DMA and reconfigure */
        dma_channel_disable(DMA0, DMA_CH4);
        dma_transfer_number_config(DMA0, DMA_CH4, 256);
        dma_channel_enable(DMA0, DMA_CH4);
    }
}

/**********************************************************************
**函数原型:	void DMA0_Channel3_IRQHandler(void)
**函数作用:	DMA0通道3中断函数 
**入口参数:	无
**          
**出口参数:	无
**备    注:	无
************************************************************************/
void DMA0_Channel3_IRQHandler(void)
{
     if(dma_interrupt_flag_get(DMA0,DMA_CH3, DMA_INT_FTF)){
        /* 处理DMA传输完成事件 */
        dma_interrupt_flag_clear(DMA0,DMA_CH3, DMA_INT_FTF); // 清除传输完成标志
        // ... 可以在这里添加代码来处理数据发送完毕的情况 ...
        uart0_struct.tx_flag = 0;
         // 检查队列中是否有待发送的数据
         uint8_t *data;
         uint32_t size;
         if (dequeue(&tx0_queue, &data, &size) == 0) {
             dma_channel_disable(DMA0, DMA_CH3); // 关闭DMA
             dma_memory_address_config(DMA0, DMA_CH3, (uint32_t)data); // 设置DMA内存地址
             dma_transfer_number_config(DMA0, DMA_CH3, size); // 设置DMA传输数量
             usart_dma_transmit_config(USART0, USART_TRANSMIT_DMA_ENABLE);
             dma_channel_enable(DMA0, DMA_CH3); // 使能DMA通道
             uart0_struct.tx_flag = 1;
         }
    }
}

/**********************************************************************
**函数原型:	void uart0_dma_send(uint8_t *data,uint32_t size)
**函数作用:	串口0使用DMA发送函数
**入口参数:	uint8_t *data   发送数据
**          uint32_t size   数组长度
**出口参数:	无
**备    注:	无
************************************************************************/
void uart0_dma_send(uint8_t *data,uint32_t size)
{
	// dma_channel_disable(DMA0, DMA_CH3);//关闭DMA
    // dma_memory_address_config(DMA0,DMA_CH3, (uint32_t)data); // 设置DMA内存地址
    // dma_transfer_number_config(DMA0,DMA_CH3, size); // 设置DMA传输数量
	// usart_dma_transmit_config(USART0, USART_TRANSMIT_DMA_ENABLE);
    // dma_channel_enable(DMA0,DMA_CH3); // 使能DMA通道

    enqueue(&tx0_queue, data, size);
    
    if (uart0_struct.tx_flag == 0) {
        uint8_t *next_data;
        uint32_t next_size;
        if (dequeue(&tx0_queue, &next_data, &next_size) == 0) {
            dma_channel_disable(DMA0, DMA_CH3); // 关闭DMA
            dma_memory_address_config(DMA0, DMA_CH3, (uint32_t)next_data); // 设置DMA内存地址
            dma_transfer_number_config(DMA0, DMA_CH3, next_size); // 设置DMA传输数量
            usart_dma_transmit_config(USART0, USART_TRANSMIT_DMA_ENABLE);
            dma_channel_enable(DMA0, DMA_CH3); // 使能DMA通道
            uart0_struct.tx_flag = 1;
        }
    }
}


/**********************************************************************
**函数原型:	void uart0_receive_process(void)
**函数作用:	UART0接收数据处理
**入口参数:	无
**出口参数:	无
**备    注:	无
************************************************************************/
void uart0_receive_process(void)
{
//    uint8_t tx_cnt;
    if(uart0_struct.rx_flag == 1)
    {
        //将收到数据发送回去
        // for(tx_cnt = 0;tx_cnt<uart0_struct.rx_count;tx_cnt++)
        // {
        //     while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
        //         usart_data_transmit(USART0, uart0_struct.rx_buf[tx_cnt]);
        // }

		uart0_struct.rx_flag = 0;
    }
}



/**********************************************************************
**函数原型:	void uart1_config(uint32_t baud)
**函数作用:	串口1初始化
**入口参数:	uint32_t baud 波特率
**          
**出口参数:	无
**备    注:	无
************************************************************************/
void uart1_config(uint32_t baud)
{
    /* 使能串口时钟 */
    rcu_periph_clock_enable(RCU_USART1);
    /* 使能GPIO时钟 */
    rcu_periph_clock_enable(RCU_GPIOA);
    
    /* 配置TX引脚 */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
    /* 配置RX引脚 */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_3);

    /* 串口配置 */
    usart_deinit(USART1);
    usart_baudrate_set(USART1, baud);
    usart_word_length_set(USART1, USART_WL_8BIT);
    usart_stop_bit_set(USART1, USART_STB_1BIT);
    usart_parity_config(USART1, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART1, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART1, USART_CTS_DISABLE);
    usart_receive_config(USART1, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);

    //开启DMA传输
	usart_dma_receive_config(USART1, USART_RECEIVE_DMA_ENABLE);
    usart_dma_transmit_config(USART1, USART_TRANSMIT_DMA_ENABLE);
		
    nvic_irq_enable(USART1_IRQn,1,1);
    usart_interrupt_enable(USART1,USART_INT_IDLE);  //开启空闲中断
    usart_enable(USART1);
}


/**********************************************************************
**函数原型:	void uart1_dma_init(void)
**函数作用:	串口1 DMA 配置
**入口参数:	无
**          
**出口参数:	无
**备    注:	无
************************************************************************/
void uart1_dma_init(void)
{
    dma_parameter_struct dma_init_struct;
    rcu_periph_clock_enable(RCU_DMA0);

    /*配置串口1发送DMA通道6 */
    dma_deinit(DMA0, DMA_CH6);
    dma_struct_para_init(&dma_init_struct);

    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_addr = (uint32_t)NULL;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = 0;
    dma_init_struct.periph_addr = (uint32_t)(&USART_DATA(USART1));
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH6, &dma_init_struct);

	dma_circulation_disable(DMA0, DMA_CH6);
    dma_memory_to_memory_disable(DMA0, DMA_CH6);
		
	dma_interrupt_enable(DMA0, DMA_CH6, DMA_INT_FTF);
	dma_channel_disable(DMA0, DMA_CH6);
	dma_flag_clear(DMA0, DMA_CH6, DMA_FLAG_G);
	nvic_irq_enable(DMA0_Channel6_IRQn, 0, 3);
		
		
    /*配置串口1接收DMA通道5 */
    dma_deinit(DMA0, DMA_CH5);
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)uart1_struct.rx_buf;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = 256;
    dma_init_struct.periph_addr = (uint32_t)(&USART_DATA(USART1));
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH5, &dma_init_struct);


    /* 禁用DMA循环模式 */
    dma_circulation_disable(DMA0, DMA_CH5);
    /* 开启DMA通道5 */
    dma_channel_enable(DMA0, DMA_CH5);
}
/**********************************************************************
**函数原型:	void USART1_IRQHandler(void)
**函数作用:	串口1中断函数
**入口参数:	无
**          
**出口参数:	无
**备    注:	无
************************************************************************/
void USART1_IRQHandler(void)
{
    if(RESET != usart_interrupt_flag_get(USART1, USART_INT_FLAG_IDLE)) {
        /* 清除中断标志位 */
        usart_data_receive(USART1);

        /* 计数 */
        uart1_struct.rx_count = 256 - (dma_transfer_number_get(DMA0, DMA_CH5));
        for(uint8_t i = 0;i<uart1_struct.rx_count;i++)
        {
            queue_push2(uart1_struct.rx_buf[i]);
        }
        uart1_struct.rx_flag = 1;

        /* disable DMA and reconfigure */
        dma_channel_disable(DMA0, DMA_CH5);
        dma_transfer_number_config(DMA0, DMA_CH5, 256);
        dma_channel_enable(DMA0, DMA_CH5);
    }
}

/**********************************************************************
**函数原型:	void DMA0_Channel6_IRQHandler(void)
**函数作用:	DMA0通道6中断函数
**入口参数:	无
**          
**出口参数:	无
**备    注:	无
************************************************************************/
void DMA0_Channel6_IRQHandler(void)
{
     if(dma_interrupt_flag_get(DMA0,DMA_CH6, DMA_INT_FTF)){
        /* 处理DMA传输完成事件 */
        dma_interrupt_flag_clear(DMA0,DMA_CH6, DMA_INT_FTF); // 清除传输完成标志
        // ... 可以在这里添加代码来处理数据发送完毕的情况 ...
		uart1_struct.tx_flag = 0;

         // 检查队列中是否有待发送的数据
        uint8_t *data;
        uint32_t size;
        if (dequeue(&tx1_queue, &data, &size) == 0) {
            dma_channel_disable(DMA0, DMA_CH6); // 关闭DMA
            dma_memory_address_config(DMA0, DMA_CH6, (uint32_t)data); // 设置DMA内存地址
            dma_transfer_number_config(DMA0, DMA_CH6, size); // 设置DMA传输数量
            usart_dma_transmit_config(USART1, USART_TRANSMIT_DMA_ENABLE);
            dma_channel_enable(DMA0, DMA_CH6); // 使能DMA通道
            uart1_struct.tx_flag = 1;
        }
    }
}

/**********************************************************************
**函数原型:	void uart1_dma_send(uint8_t *data,uint32_t size)
**函数作用:	串口1的DMA发送
**入口参数:	uint8_t *data 发送数组
**          uint32_t size   发送数据长度
**出口参数:	无
**备    注:	无
************************************************************************/
void uart1_dma_send(uint8_t *data,uint32_t size)
{
     enqueue(&tx1_queue, data, size);
    
    if (uart1_struct.tx_flag == 0) {
        uint8_t *next_data;
        uint32_t next_size;
        if (dequeue(&tx1_queue, &next_data, &next_size) == 0) {
            dma_channel_disable(DMA0, DMA_CH6); // 关闭DMA
            dma_memory_address_config(DMA0, DMA_CH6, (uint32_t)next_data); // 设置DMA内存地址
            dma_transfer_number_config(DMA0, DMA_CH6, next_size); // 设置DMA传输数量
            usart_dma_transmit_config(USART1, USART_TRANSMIT_DMA_ENABLE);
            dma_channel_enable(DMA0, DMA_CH6); // 使能DMA通道
            uart1_struct.tx_flag = 1;
        }
    }
}

/**********************************************************************
**函数原型:	void uart1_receive_process(void)
**函数作用:	UART1接收数据处理
**入口参数:	无
**出口参数:	无
**备    注:	无
************************************************************************/
void uart1_receive_process(void)
{
    // uint8_t tx_cnt;
    if(uart1_struct.rx_flag == 1)
    {

		uart1_struct.rx_flag = 0;
    }
}


void uart1_send_char(unsigned char t)
{
    usart_data_transmit(USART1, t);
    while(RESET == usart_flag_get(USART1, USART_FLAG_TBE));
}

/**********************************************************************
**函数原型:	void uart2_config(uint32_t baud)
**函数作用:	串口2初始化
**入口参数:	uint32_t baud 波特率
**          
**出口参数:	无
**备    注:	无
************************************************************************/
void uart2_config(uint32_t baud)
{
    /*使能串口时钟*/
    rcu_periph_clock_enable(RCU_USART2);
    /*使能GPIO时钟*/
    rcu_periph_clock_enable(RCU_GPIOB);
    
    /* 配置TX引脚 */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    /* 配置RX引脚 */
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_11);

    /* 串口配置 */
    usart_deinit(USART2);
    usart_baudrate_set(USART2, baud);
    usart_word_length_set(USART2, USART_WL_8BIT);
    usart_stop_bit_set(USART2, USART_STB_1BIT);
    usart_parity_config(USART2, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART2, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART2, USART_CTS_DISABLE);
    usart_receive_config(USART2, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART2, USART_TRANSMIT_ENABLE);

    //开启DMA传输
	usart_dma_receive_config(USART2, USART_RECEIVE_DMA_ENABLE);
    usart_dma_transmit_config(USART2, USART_TRANSMIT_DMA_ENABLE);
		
    nvic_irq_enable(USART2_IRQn,1,0);
    usart_interrupt_enable(USART2,USART_INT_IDLE);  //开启空闲中断
    usart_enable(USART2);
}

/**********************************************************************
**函数原型:	void uart2_dma_init(void)
**函数作用:	串口2 DMA 配置
**入口参数:	无
**          
**出口参数:	无
**备    注:	无
************************************************************************/
void uart2_dma_init(void)
{
    dma_parameter_struct dma_init_struct;
    rcu_periph_clock_enable(RCU_DMA0);

    /*配置串口2发送DMA通道1 */
    dma_deinit(DMA0, DMA_CH1);
    dma_struct_para_init(&dma_init_struct);

    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_addr = (uint32_t)NULL;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = 0;
    dma_init_struct.periph_addr = (uint32_t)(&USART_DATA(USART2));
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH1, &dma_init_struct);

	dma_circulation_disable(DMA0, DMA_CH1);
    dma_memory_to_memory_disable(DMA0, DMA_CH1);
		
	dma_interrupt_enable(DMA0, DMA_CH1, DMA_INT_FTF);
	dma_channel_disable(DMA0, DMA_CH1);
	dma_flag_clear(DMA0, DMA_CH1, DMA_FLAG_G);
	nvic_irq_enable(DMA0_Channel1_IRQn, 0, 2);
		
		
    /*配置串口2接收DMA通道2 */
    dma_deinit(DMA0, DMA_CH2);
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)uart2_struct.rx_buf;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = 256;
    dma_init_struct.periph_addr = (uint32_t)(&USART_DATA(USART2));
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH2, &dma_init_struct);


    /* 禁用DMA循环模式 */
    dma_circulation_disable(DMA0, DMA_CH2);
    /* 开启DMA通道5 */
    dma_channel_enable(DMA0, DMA_CH2);
}

/**********************************************************************
**函数原型:	void USART2_IRQHandler(void)
**函数作用:	串口2中断函数
**入口参数:	无
**          
**出口参数:	无
**备    注:	无
************************************************************************/
void USART2_IRQHandler(void)
{
    if(RESET != usart_interrupt_flag_get(USART2, USART_INT_FLAG_IDLE)) {
        /* clear IDLE flag */
        usart_data_receive(USART2);

        /* number of data received */
        uart2_struct.rx_count = 256 - (dma_transfer_number_get(DMA0, DMA_CH2));
        for(uint8_t i = 0;i<uart2_struct.rx_count;i++)
        {
            queue_push(uart2_struct.rx_buf[i]);
        }
        uart2_struct.rx_flag = 1;
				
        /* disable DMA and reconfigure */
        dma_channel_disable(DMA0, DMA_CH2);
        dma_transfer_number_config(DMA0, DMA_CH2, 256);
        dma_channel_enable(DMA0, DMA_CH2);

    }
}

/**********************************************************************
**函数原型:	void DMA0_Channel1_IRQHandler(void)
**函数作用:	DMA0的通道1中断函数
**入口参数:	无
**          
**出口参数:	无
**备    注:	无
************************************************************************/
void DMA0_Channel1_IRQHandler(void)
{
     if(dma_interrupt_flag_get(DMA0,DMA_CH1, DMA_INT_FTF)){
        /* 处理DMA传输完成事件 */
        dma_interrupt_flag_clear(DMA0,DMA_CH1, DMA_INT_FLAG_G); // 清除传输完成标志
        // ... 可以在这里添加代码来处理数据发送完毕的情况 ...
        uart2_struct.tx_flag = 0;

         // 检查队列中是否有待发送的数据
        uint8_t *data;
        uint32_t size;
        if (dequeue(&tx2_queue, &data, &size) == 0) {
            dma_channel_disable(DMA0, DMA_CH1); // 关闭DMA
            dma_memory_address_config(DMA0, DMA_CH1, (uint32_t)data); // 设置DMA内存地址
            dma_transfer_number_config(DMA0, DMA_CH1, size); // 设置DMA传输数量
            usart_dma_transmit_config(USART2, USART_TRANSMIT_DMA_ENABLE);
            dma_channel_enable(DMA0, DMA_CH1); // 使能DMA通道
            uart2_struct.tx_flag = 1;
        }
    }
}

/**********************************************************************
**函数原型:	void uart2_dma_send(uint8_t *data,uint32_t size)
**函数作用:	串口2的DMA发送
**入口参数:	uint8_t *data 发送数组
**          uint32_t size 数据长度
**出口参数:	无
**备    注:	无
************************************************************************/
void uart2_dma_send(uint8_t *data,uint32_t size)
{
    enqueue(&tx2_queue, data, size);
    
    if (uart2_struct.tx_flag == 0) {
        uint8_t *next_data;
        uint32_t next_size;
        if (dequeue(&tx2_queue, &next_data, &next_size) == 0) {
            dma_channel_disable(DMA0, DMA_CH1); // 关闭DMA
            dma_memory_address_config(DMA0, DMA_CH1, (uint32_t)next_data); // 设置DMA内存地址
            dma_transfer_number_config(DMA0, DMA_CH1, next_size); // 设置DMA传输数量
            usart_dma_transmit_config(USART2, USART_TRANSMIT_DMA_ENABLE);
            dma_channel_enable(DMA0, DMA_CH1); // 使能DMA通道
            uart2_struct.tx_flag = 1;
        }
    }
}

/**********************************************************************
**函数原型:	void uart2_receive_process(void)
**函数作用:	UART2接收数据处理
**入口参数:	无
**出口参数:	无
**备    注:	无
************************************************************************/
void uart2_receive_process(void)
{
//    uint8_t tx_cnt;
    if(uart2_struct.rx_flag == 1)
    {

		uart2_struct.rx_flag = 0;
    }
}

void uart2_send_char(unsigned char t)
{
    usart_data_transmit(USART2, t);
    while(RESET == usart_flag_get(USART1, USART_FLAG_TBE));
}



/**********************************************************************
**函数原型:	void uart3_config(uint32_t baud)
**函数作用:	串口3初始化
**入口参数:	uint32_t baud 波特率
**          
**出口参数:	无
**备    注:	无
************************************************************************/
void uart3_config(uint32_t baud)
{
    /* 使能串口时钟 */
    rcu_periph_clock_enable(RCU_UART3);
    /* 使能GPIO时钟 */
    rcu_periph_clock_enable(RCU_GPIOC);
    /* 配置TX引脚 */
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    /* 配置RX引脚 */
    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_11);

    /* 串口配置 */
    usart_deinit(UART3);
    usart_baudrate_set(UART3, baud);
    usart_word_length_set(UART3, USART_WL_8BIT);
    usart_stop_bit_set(UART3, USART_STB_1BIT);
    usart_parity_config(UART3, USART_PM_NONE);
    usart_hardware_flow_rts_config(UART3, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(UART3, USART_CTS_DISABLE);
    usart_receive_config(UART3, USART_RECEIVE_ENABLE);
    usart_transmit_config(UART3, USART_TRANSMIT_ENABLE);
    
   //开启DMA传输
	usart_dma_receive_config(UART3, USART_RECEIVE_DMA_ENABLE);
    usart_dma_transmit_config(UART3, USART_TRANSMIT_DMA_ENABLE);
		
    nvic_irq_enable(UART3_IRQn,1,3);
    usart_interrupt_enable(UART3,USART_INT_IDLE);  //开启空闲中断
    usart_enable(UART3);
}

/**********************************************************************
**函数原型:	void uart3_dma_init(void)
**函数作用:	串口3 DMA 配置
**入口参数:	无
**          
**出口参数:	无
**备    注:	无
************************************************************************/
void uart3_dma_init(void)
{
    dma_parameter_struct dma_init_struct;
    rcu_periph_clock_enable(RCU_DMA1);

    /*配置串口3发送DMA通道4 */
    dma_deinit(DMA1, DMA_CH4);
    dma_struct_para_init(&dma_init_struct);

    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_addr = (uint32_t)NULL;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = 0;
    dma_init_struct.periph_addr = (uint32_t)(&USART_DATA(UART3));
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA1, DMA_CH4, &dma_init_struct);

	dma_circulation_disable(DMA1, DMA_CH4);
    dma_memory_to_memory_disable(DMA1, DMA_CH4);
		
	dma_interrupt_enable(DMA1, DMA_CH4, DMA_INT_FTF);
	dma_channel_disable(DMA1, DMA_CH4);
	dma_flag_clear(DMA1, DMA_CH4, DMA_FLAG_G);
	nvic_irq_enable(DMA1_Channel3_Channel4_IRQn, 0, 5);
		
		
    /*配置串口2接收DMA通道2 */
    dma_deinit(DMA1, DMA_CH2);
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)uart3_struct.rx_buf;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = 256;
    dma_init_struct.periph_addr = (uint32_t)(&USART_DATA(UART3));
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA1, DMA_CH2, &dma_init_struct);


    /* 禁用DMA循环模式 */
    dma_circulation_disable(DMA1, DMA_CH2);
    /* 开启DMA通道5 */
    dma_channel_enable(DMA1, DMA_CH2);
}

/**********************************************************************
**函数原型:	void UART3_IRQHandler(void)
**函数作用:	串口3中断函数
**入口参数:	无
**          
**出口参数:	无
**备    注:	无
************************************************************************/
void UART3_IRQHandler(void)
{
    if(RESET != usart_interrupt_flag_get(UART3, USART_INT_FLAG_IDLE)) {
        /* clear IDLE flag */
        usart_data_receive(UART3);

        /* number of data received */
        uart3_struct.rx_count = 256 - (dma_transfer_number_get(DMA1, DMA_CH2));
        uart3_struct.rx_flag = 1;

        /* disable DMA and reconfigure */
        dma_channel_disable(DMA1, DMA_CH2);
        dma_transfer_number_config(DMA1, DMA_CH2, 256);
        dma_channel_enable(DMA1, DMA_CH2);
    }
}

/**********************************************************************
**函数原型:	void DMA1_Channel3_4_IRQHandler(void)
**函数作用:	DMA1的通道3和4中断函数
**入口参数:	无
**          
**出口参数:	无
**备    注:	无
************************************************************************/
void DMA1_Channel3_4_IRQHandler(void)
{
     if(dma_interrupt_flag_get(DMA1,DMA_CH4, DMA_INT_FTF)){
        /* 处理DMA传输完成事件 */
        dma_interrupt_flag_clear(DMA1,DMA_CH4, DMA_INT_FTF); // 清除传输完成标志
        // ... 可以在这里添加代码来处理数据发送完毕的情况 ...
			 
    }
}
/**********************************************************************
**函数原型:	void uart3_dma_send(uint8_t *data,uint32_t size)
**函数作用:	DMA1的通道3和4中断函数
**入口参数:	uint8_t *data   发送数组
**          uint32_t size   数据长度
**出口参数:	无
**备    注:	无
************************************************************************/
void uart3_dma_send(uint8_t *data,uint32_t size)
{
    dma_channel_disable(DMA1, DMA_CH4);//关闭DMA
    dma_memory_address_config(DMA1,DMA_CH4, (uint32_t)data); // 设置DMA内存地址
    dma_transfer_number_config(DMA1,DMA_CH4, size); // 设置DMA传输数量
	usart_dma_transmit_config(UART3, USART_TRANSMIT_DMA_ENABLE);
    dma_channel_enable(DMA1,DMA_CH4); // 使能DMA通道
}

/**********************************************************************
**函数原型:	void uart3_receive_process(void)
**函数作用:	UART3接收数据处理
**入口参数:	无
**出口参数:	无
**备    注:	无
************************************************************************/
void uart3_receive_process(void)
{
    if(uart3_struct.rx_flag == 1)
    {

		uart3_struct.rx_flag = 0;
    }
}

/**********************************************************************
**函数原型:	void uart4_config(uint32_t baud)
**函数作用:	串口4初始化
**入口参数:	uint32_t baud 波特率
**          
**出口参数:	无
**备    注:	无
************************************************************************/
void uart4_config(uint32_t baud)
{
    /* 使能串口时钟 */
    rcu_periph_clock_enable(RCU_UART4);
    /* 使能GPIO时钟 */
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);
    /* 配置TX引脚 */
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    /* 配置RX引脚 */
    gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

    /* 串口配置 */
    usart_deinit(UART4);
    usart_baudrate_set(UART4, baud);
    usart_word_length_set(UART4, USART_WL_8BIT);
    usart_stop_bit_set(UART4, USART_STB_1BIT);
    usart_parity_config(UART4, USART_PM_NONE);
    usart_hardware_flow_rts_config(UART4, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(UART4, USART_CTS_DISABLE);
    usart_receive_config(UART4, USART_RECEIVE_ENABLE);
    usart_transmit_config(UART4, USART_TRANSMIT_ENABLE);
		
    nvic_irq_enable(UART4_IRQn,1,4);
    usart_interrupt_enable(UART4,USART_INT_RBNE);  //开启空闲中断,
//	usart_interrupt_enable(UART4,USART_INT_IDLE);
    usart_enable(UART4);
}


/**********************************************************************
**函数原型:	void Uart4_send_buf(uint8_t *buf,uint8_t len)
**函数作用:	串口4发送字符串
**入口参数:	uint8_t *buf 发送数组
            uint8_t len 数据长度
**出口参数:	无
**备    注:	无
************************************************************************/
void uart4_send_buf(uint8_t *buf,uint8_t len)
{
	while(len>0)
	{
		usart_data_transmit(UART4, *buf);
        while(RESET == usart_flag_get(UART4, USART_FLAG_TBE));
        buf++;
            len--;
	}
}
/**********************************************************************
**函数原型:	void uart4_send_str(const char *str)
**函数作用:	串口4发送字符串
**入口参数:	const char *str 发送字符串
**出口参数:	无
**备    注:	无
************************************************************************/
void uart4_send_str(const char *str)
{
    while (*str != '\0') {
        usart_data_transmit(UART4, *str);
        while(RESET == usart_flag_get(UART4, USART_FLAG_TBE));
        str++;
    }
}


/**********************************************************************
**函数原型:	void uart3_receive_process(void)
**函数作用:	UART3接收数据处理
**入口参数:	无
**出口参数:	无
**备    注:	无
************************************************************************/
void uart4_receive_process(void)
{
    // uint8_t tx_cnt;
    // if(uart4_struct.rx_flag == 1)
    // {
       
	// 	uart4_struct.rx_flag = 0;
    // }
}




/**********************************************************************
**函数原型:	void uart1_send_str(const char *str)
**函数作用:	串口1发送字符串
**入口参数:	const char *str 发送字符串
**出口参数:	无
**备    注:	无
************************************************************************/
void uart1_send_str(const char *str)
{
    while (*str != '\0') {
        usart_data_transmit(USART1, *str);
        while(RESET == usart_flag_get(USART1, USART_FLAG_TBE));
        str++;
    }
}

/**********************************************************************
**函数原型:	void Uart3_send_buf(uint8_t *buf,uint8_t len)
**函数作用:	串口3发送字符串
**入口参数:	uint8_t *buf 发送数组
            uint8_t len 数据长度
**出口参数:	无
**备    注:	无
************************************************************************/
void Uart3_send_buf(uint8_t *buf,uint8_t len)
{
	while(len>0)
	{
		usart_data_transmit(UART3, *buf);
    while(RESET == usart_flag_get(UART3, USART_FLAG_TBE));
    buf++;
		len--;
	}
}


int enqueue(queue_t *queue, uint8_t *data, uint32_t length) {
    if (length > MAX_DATA_SIZE) {
        return -1; // 数据太大
    }

    if (queue->count == QUEUE_SIZE) {
        // 队列满，覆盖掉队列头的数据
        queue->front = (queue->front + 1) % QUEUE_SIZE;
        queue->count--;
    }

    memcpy(queue->items[queue->rear].data, data, length);
    queue->items[queue->rear].length = length;
    queue->rear = (queue->rear + 1) % QUEUE_SIZE;
    queue->count++;
    return 0;
}

int dequeue(queue_t *queue, uint8_t **data, uint32_t *length) {
    if (queue->count == 0) {
        return -1; // 队列空
    }
    *data = queue->items[queue->front].data;
    *length = queue->items[queue->front].length;
    queue->front = (queue->front + 1) % QUEUE_SIZE;
    queue->count--;
    return 0;
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(USART0, (uint8_t)ch);
    while (RESET == usart_flag_get(USART0, USART_FLAG_TBE))
        ;

    return ch;
}

