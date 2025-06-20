#ifndef __WORK_H
#define __WORK_H

#include "gd32f30x.h"

#define true 1
#define false 0

/* 系统配置 */
#define SYSTEM_TICK_MS 1 // 系统时钟节拍(ms)

extern uint8_t deviceRunState; 

/* 类型定义 */
typedef void (*task_func_t)(void);         // 任务函数指针类型
typedef void (*task_func_param_t)(void *); // 带参数的任务函数指针类型
typedef uint8_t (*task_condition_t)(void); // 任务条件函数指针类型

/* 任务节点结构体 */
typedef struct TaskNode {
    task_func_t     function;      // 无参数任务函数
    task_func_param_t param_function; // 带参数任务函数
    void*           param;         // 函数参数
    uint8_t            has_param;     // 是否有参数
    task_condition_t condition;    // 执行条件
    uint16_t        period;        // 周期(ms)
    uint16_t        counter;       // 计数器
    uint8_t            ready;         // 就绪标志
    uint8_t            enabled;       // 使能标志
    struct TaskNode* next;         // 下一个任务节点
} TaskNode_t;


/* 任务队列 */
typedef struct {
    TaskNode_t* head;              // 队列头
    TaskNode_t* tail;              // 队列尾
    uint16_t    count;             // 任务计数
} TaskQueue_t;



/* 状态机类型定义 */
typedef void (*state_func_t)(void);         // 状态函数指针类型
typedef uint8_t (*transition_func_t)(void); // 状态转换条件函数指针类型

/* 状态转换结构体 */
typedef struct
{
    uint8_t next_state;          // 下一个状态
    transition_func_t condition; // 转换条件
} StateTransition_t;

/* 状态结构体 */
typedef struct
{
    state_func_t action;            // 状态动作
    StateTransition_t *transitions; // 状态转换表
    uint8_t num_transitions;        // 转换数量
} State_t;

extern TaskQueue_t task_queue;
static uint8_t current_state;

typedef struct {
    uint8_t coffeeMake; //咖啡萃取
    uint8_t teaMake;    //茶萃取
    uint8_t inlet;      //进水
    uint8_t drain;      //排水
    uint8_t wash;       //清洗
    uint8_t sanit;      //消毒
} State_TypeDef;



void memory_allocation_failed(void);
void init_task_queue(void);
void hw_init(void);
void task_init(void);
uint8_t add_task(task_func_t func, task_condition_t cond, uint16_t period, uint8_t enabled);
uint8_t add_task_with_param(task_func_param_t func, void *param, task_condition_t cond, uint16_t period, uint8_t enabled);
uint8_t delete_task(task_func_t func);
uint8_t delete_task_with_param(task_func_param_t func, void* param);

uint8_t enable_task(task_func_t func);
uint8_t enable_task_with_param(task_func_param_t func, void* param);
uint8_t disable_task(task_func_t func);
uint8_t disable_task_with_param(task_func_param_t func, void* param);
void set_state_machine(State_t *states, uint8_t count);
void execute_state_machine(void);
void scheduler(void);

void led_toggle_param(void *led_num);

uint8_t find_num(uint8_t *arr,uint8_t num);

void lockDevice(void);
void unlockDevice(void);
#endif
