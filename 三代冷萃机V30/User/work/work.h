#ifndef __WORK_H
#define __WORK_H

#include "gd32f30x.h"

#define true 1
#define false 0

/* ϵͳ���� */
#define SYSTEM_TICK_MS 1 // ϵͳʱ�ӽ���(ms)

extern uint8_t deviceRunState; 

/* ���Ͷ��� */
typedef void (*task_func_t)(void);         // ������ָ������
typedef void (*task_func_param_t)(void *); // ��������������ָ������
typedef uint8_t (*task_condition_t)(void); // ������������ָ������

/* ����ڵ�ṹ�� */
typedef struct TaskNode {
    task_func_t     function;      // �޲���������
    task_func_param_t param_function; // ������������
    void*           param;         // ��������
    uint8_t            has_param;     // �Ƿ��в���
    task_condition_t condition;    // ִ������
    uint16_t        period;        // ����(ms)
    uint16_t        counter;       // ������
    uint8_t            ready;         // ������־
    uint8_t            enabled;       // ʹ�ܱ�־
    struct TaskNode* next;         // ��һ������ڵ�
} TaskNode_t;


/* ������� */
typedef struct {
    TaskNode_t* head;              // ����ͷ
    TaskNode_t* tail;              // ����β
    uint16_t    count;             // �������
} TaskQueue_t;



/* ״̬�����Ͷ��� */
typedef void (*state_func_t)(void);         // ״̬����ָ������
typedef uint8_t (*transition_func_t)(void); // ״̬ת����������ָ������

/* ״̬ת���ṹ�� */
typedef struct
{
    uint8_t next_state;          // ��һ��״̬
    transition_func_t condition; // ת������
} StateTransition_t;

/* ״̬�ṹ�� */
typedef struct
{
    state_func_t action;            // ״̬����
    StateTransition_t *transitions; // ״̬ת����
    uint8_t num_transitions;        // ת������
} State_t;

extern TaskQueue_t task_queue;
static uint8_t current_state;

typedef struct {
    uint8_t coffeeMake; //������ȡ
    uint8_t teaMake;    //����ȡ
    uint8_t inlet;      //��ˮ
    uint8_t drain;      //��ˮ
    uint8_t wash;       //��ϴ
    uint8_t sanit;      //����
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
