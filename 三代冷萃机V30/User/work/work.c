#include "work.h"
#include "systick.h"
#include <stdio.h>
#include <stdlib.h>
#include "led.h"
#include "math.h"
#include "timer.h"
#include "uart.h"

#include "button.h"
#include "dr_button_api.h"
#include "led.h"
#include "arm_math.h"
#include "arm_const_structs.h"
#include "math.h"
#include "lcd_data_process.h"
#include "iic.h"
#include "eeprom.h"
#include "lcd_cmd_queue.h"
#include "uart_lcd.h"
#include "relay.h"
#include "dial.h"
#include "app_wifi.h"
#include "bsp_wifi.h"
#include "app_wifi_set.h"
#include "adc.h"
#include "waterLevel.h"
#include "waterTap.h"
#include "wash.h"
#include "sanit.h"
#include "coffeeMake.h"
#include "teaMake.h"
#include "app_mqtt.h"
#include "debug.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "work"

static led_typedef_enum led1 = LED1;

uint8_t deviceRunState = 0 ; // 设备状态,0空闲，1运行，2异常



/* 全局变量 */
/* 全局变量 */
TaskQueue_t task_queue = {NULL, NULL, 0};  // 任务队列
uint8_t current_state = 0;                 // 当前状态

/* 系统状态机 */
State_t* state_machine = NULL;          // 状态机
uint8_t state_count = 0;                // 状态数量


/* 内存分配失败处理函数 */
void memory_allocation_failed(void) {
    // 处理内存分配失败，可以点亮错误LED，进入安全模式等
    while(1) {
        // 安全循环
        LED_ON(LED2);
    }
}

/* 初始化任务队列 */
void init_task_queue(void) {
    task_queue.head = NULL;
    task_queue.tail = NULL;
    task_queue.count = 0;
}

/* 硬件初始化函数 */
void hw_init(void)
{
    //初始化时钟系统
    systick_config();
    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);
    //初始化GPIO
    led_init();
    Button_gpio_init();
    DR_Key_Init();
    //初始化串口
    uart0_config(115200);
    uart0_dma_init();
    uart1_config(115200);
	uart1_dma_init();
    uart2_config(115200);
	uart2_dma_init();
	uart3_config(115200);
    uart3_dma_init();
    wifiInit(115200);
    //初始化ADC
    adc0_config();
    //初始化定时器，配置系统滴答
    timer1_config();
    // 其他外设初始化
    dial_init();
    relay_init();		//MOS输出初始化
	i2c0_config();
	i2c_eeprom_init();
    WIFI_ENABLE();
    
    elog_init();
	elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
	elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
	elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
	elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    //除了时间，进程线程信息之外，其余全部输出
	elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_T_INFO | ELOG_FMT_P_INFO));
	elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_T_INFO | ELOG_FMT_P_INFO));
    // 设置日志过滤级别为 DEBUG，只输出 DEBUG 及以上等级的日志
   //elog_set_filter_lvl(ELOG_LVL_ERROR);
	//开启log输出
	elog_start();
    readSaveData();
    
    waterLevel_init();
    washInit();
    sanitInit();
    coffeeMakeInit();
    teaMakeInit();
    log_i("Version %d HW %d.%d SW %d,%d",config.data.version,
        deviceInfo.data.HardwareVersion/10,deviceInfo.data.HardwareVersion%10,
        deviceInfo.data.FirmwareVersion/10,deviceInfo.data.FirmwareVersion%10);
    log_i("username %s",deviceInfo.data.MQTT.UserName);
    log_i("password %s",deviceInfo.data.MQTT.Password);
    log_i("ClientID %s",deviceInfo.data.MQTT.ClientID);
    delay_1ms(2000);    // 延时2秒，等待串口屏初始化完成
    waterTapInit();
    if(config.data.isLock)  //判断是否锁定设备
    {
        log_i("lock");
        lockDevice();
    }
    else
    {
        log_i("unlock");
        unlockDevice();
    }
    if(gpio_input_bit_get(ADDR0_PORT,ADDR0_PIN) == 1)   //判断是否开启输出
    {
        elog_set_output_enabled(true);
    }
    else
    {
        elog_set_output_enabled(false);
    }
}

/**
 * ************************************************************************
 * @brief  初始化任务
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-06-20
 * 
 * ************************************************************************
 */
void task_init(void)
{
    add_task_with_param(led_toggle_param,&led1,NULL, 100, true);    //LED运行指示灯
    add_task(lcd_cmd_scan,NULL,1,true);                             //冷萃机串口屏数据解析
    add_task(lcd_cmd_scan2,NULL,1,true);                            //水龙头串口屏数据解析
    add_task(waterLevel_update,NULL,100,true);                      //水位检测
    add_task(waterTapProcessControl,NULL,100,true);                 //水龙头数据处理
    add_task(debugProcess,NULL,10,true);                            //上位机数据接收
}

/* 添加无参数任务 */
uint8_t add_task(task_func_t func, task_condition_t cond, uint16_t period, uint8_t enabled)
{
    // 分配新任务节点内存
    TaskNode_t* new_task = (TaskNode_t*)malloc(sizeof(TaskNode_t));
    if (new_task == NULL) {
        memory_allocation_failed();
        return false;
    }
    
    // 初始化任务节点
    new_task->function = func;
    new_task->param_function = NULL;
    new_task->param = NULL;
    new_task->has_param = false;
    new_task->condition = cond;
    new_task->period = period;
    new_task->counter = 0;
    new_task->ready = false;
    new_task->enabled = enabled;
    new_task->next = NULL;
    
    // 添加到队列
    if (task_queue.head == NULL) {
        // 空队列
        task_queue.head = new_task;
        task_queue.tail = new_task;
    } else {
        // 添加到队列尾部
        task_queue.tail->next = new_task;
        task_queue.tail = new_task;
    }
    
    task_queue.count++;
    return true;
}


/* 添加带参数的任务 */
uint8_t add_task_with_param(task_func_param_t func, void* param, task_condition_t cond, uint16_t period, uint8_t enabled)
{
    // 分配新任务节点内存
    TaskNode_t* new_task = (TaskNode_t*)malloc(sizeof(TaskNode_t));
    if (new_task == NULL) {
        memory_allocation_failed();
        return false;
    }
    
    // 初始化任务节点
    new_task->function = NULL;
    new_task->param_function = func;
    new_task->param = param;
    new_task->has_param = true;
    new_task->condition = cond;
    new_task->period = period;
    new_task->counter = 0;
    new_task->ready = false;
    new_task->enabled = enabled;
    new_task->next = NULL;
    
    // 添加到队列
    if (task_queue.head == NULL) {
        // 空队列
        task_queue.head = new_task;
        task_queue.tail = new_task;
    } else {
        // 添加到队列尾部
        task_queue.tail->next = new_task;
        task_queue.tail = new_task;
    }
    
    task_queue.count++;
    return true;
}


/* 删除任务 */
uint8_t delete_task(task_func_t func) {
    TaskNode_t* current = task_queue.head;
    TaskNode_t* prev = NULL;
    
    while (current != NULL) {
        if (!current->has_param && current->function == func) {
            // 找到要删除的任务
            if (prev == NULL) {
                // 删除头节点
                task_queue.head = current->next;
                if (task_queue.head == NULL) {
                    task_queue.tail = NULL;
                }
            } else {
                prev->next = current->next;
                if (current == task_queue.tail) {
                    task_queue.tail = prev;
                }
            }
            
            free(current);
            task_queue.count--;
            return true;
        }
        
        prev = current;
        current = current->next;
    }
    
    return false;
}

/* 删除带参数的任务 */
uint8_t delete_task_with_param(task_func_param_t func, void* param) {
    TaskNode_t* current = task_queue.head;
    TaskNode_t* prev = NULL;
    
    while (current != NULL) {
        if (current->has_param && current->param_function == func && current->param == param) {
            // 找到要删除的任务
            if (prev == NULL) {
                // 删除头节点
                task_queue.head = current->next;
                if (task_queue.head == NULL) {
                    task_queue.tail = NULL;
                }
            } else {
                prev->next = current->next;
                if (current == task_queue.tail) {
                    task_queue.tail = prev;
                }
            }
            
            free(current);
            task_queue.count--;
            return true;
        }
        
        prev = current;
        current = current->next;
    }
    
    return false;
}

/* 启用任务 */
uint8_t enable_task(task_func_t func) {
    TaskNode_t* current = task_queue.head;
    
    while (current != NULL) {
        if (!current->has_param && current->function == func) {
            current->enabled = true;
            return true;
        }
        current = current->next;
    }
    
    return false;
}

/* 启用带参数的任务 */
uint8_t enable_task_with_param(task_func_param_t func, void* param) {
    TaskNode_t* current = task_queue.head;
    
    while (current != NULL) {
        if (current->has_param && current->param_function == func && current->param == param) {
            current->enabled = true;
            return true;
        }
        current = current->next;
    }
    
    return false;
}

/* 禁用任务 */
uint8_t disable_task(task_func_t func) {
    TaskNode_t* current = task_queue.head;
    
    while (current != NULL) {
        if (!current->has_param && current->function == func) {
            current->enabled = false;
            return true;
        }
        current = current->next;
    }
    
    return false;
}

/* 禁用带参数的任务 */
uint8_t disable_task_with_param(task_func_param_t func, void* param) {
    TaskNode_t* current = task_queue.head;
    
    while (current != NULL) {
        if (current->has_param && current->param_function == func && current->param == param) {
            current->enabled = false;
            return true;
        }
        current = current->next;
    }
    
    return false;
}


/* 设置状态机 */
void set_state_machine(State_t* states, uint8_t count)
{
    state_machine = states;
    state_count = count;
    current_state = 0;
}


/* 状态机执行 */
void execute_state_machine(void)
{
    if (state_machine == NULL || current_state >= state_count) {
        return;
    }
    
    // 执行当前状态动作
    if (state_machine[current_state].action != NULL) {
        state_machine[current_state].action();
    }
    
    // 检查状态转换
    uint8_t i;
    for (i = 0; i < state_machine[current_state].num_transitions; i++) {
        StateTransition_t* trans = &state_machine[current_state].transitions[i];
        if (trans->condition != NULL && trans->condition()) {
            current_state = trans->next_state;
            break;
        }
    }
}


/* 任务调度器 */
void scheduler(void)
{
    TaskNode_t* current = task_queue.head;
    
    while (current != NULL) {
        if (current->ready && current->enabled) {
            // 检查执行条件
            if (current->condition == NULL || current->condition()) {
                if (current->has_param) {
                    current->param_function(current->param);
                } else {
                    current->function();
                }
            }
            current->ready = false;
        }
        current = current->next;
    }
}



/**
 * ************************************************************************
 * @brief  LED闪烁函数
 * 
 * @param[in] led_num  LED编号
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-06-20
 * 
 * ************************************************************************
 */
void led_toggle_param(void* led_num) {
    gd_eval_led_toggle(*(led_typedef_enum*)led_num);
}


//寻找数字是否在数组中
/**
 * ************************************************************************
 * @brief 寻找数字是否在数组中
 * 
 * @param[in] arr  数组
 * @param[in] num  需要寻找的数字
 * 
 * @return 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-12
 * 
 * ************************************************************************
 */
uint8_t find_num(uint8_t *arr,uint8_t num)
{
    for(uint8_t i = 0;i<4;i++)
    {
        if(arr[i] == num)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * ************************************************************************
 * @brief 锁定设备
 * 
 * @return 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-12
 * 
 * ************************************************************************
 */
void lockDevice(void)
{
    SetControlEnable(make_coffe_set_page,5,0,UART2_ID);   //禁用咖啡萃取开始按钮
    SetControlEnable(make_tea_set_page,5,0,UART2_ID);   //禁用咖啡萃取设置按钮
    SetControlEnable(main_page,4,0,UART2_ID);   //禁用消毒按钮
    SetControlEnable(main_page,3,0,UART2_ID);   //禁用清洗按钮
}

/**
 * ************************************************************************
 * @brief 解锁设备
 * 
 * @return 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-12
 * 
 * ************************************************************************
 */
void unlockDevice(void)
{
    SetControlEnable(make_coffe_set_page,5,1,UART2_ID);   //解锁咖啡萃取开始按钮
    SetControlEnable(make_tea_set_page,5,1,UART2_ID);   //解锁咖啡萃取设置按钮
    SetControlEnable(main_page,4,1,UART2_ID);   //解锁消毒按钮
    SetControlEnable(main_page,3,1,UART2_ID);   //解锁清洗按钮
}

