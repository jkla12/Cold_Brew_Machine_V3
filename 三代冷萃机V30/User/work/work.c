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

uint8_t deviceRunState = 0 ; // �豸״̬,0���У�1���У�2�쳣



/* ȫ�ֱ��� */
/* ȫ�ֱ��� */
TaskQueue_t task_queue = {NULL, NULL, 0};  // �������
uint8_t current_state = 0;                 // ��ǰ״̬

/* ϵͳ״̬�� */
State_t* state_machine = NULL;          // ״̬��
uint8_t state_count = 0;                // ״̬����


/* �ڴ����ʧ�ܴ����� */
void memory_allocation_failed(void) {
    // �����ڴ����ʧ�ܣ����Ե�������LED�����밲ȫģʽ��
    while(1) {
        // ��ȫѭ��
        LED_ON(LED2);
    }
}

/* ��ʼ��������� */
void init_task_queue(void) {
    task_queue.head = NULL;
    task_queue.tail = NULL;
    task_queue.count = 0;
}

/* Ӳ����ʼ������ */
void hw_init(void)
{
    //��ʼ��ʱ��ϵͳ
    systick_config();
    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);
    //��ʼ��GPIO
    led_init();
    Button_gpio_init();
    DR_Key_Init();
    //��ʼ������
    uart0_config(115200);
    uart0_dma_init();
    uart1_config(115200);
	uart1_dma_init();
    uart2_config(115200);
	uart2_dma_init();
	uart3_config(115200);
    uart3_dma_init();
    wifiInit(115200);
    //��ʼ��ADC
    adc0_config();
    //��ʼ����ʱ��������ϵͳ�δ�
    timer1_config();
    // ���������ʼ��
    dial_init();
    relay_init();		//MOS�����ʼ��
	i2c0_config();
	i2c_eeprom_init();
    WIFI_ENABLE();
    
    elog_init();
	elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
	elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
	elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
	elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    //����ʱ�䣬�����߳���Ϣ֮�⣬����ȫ�����
	elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_T_INFO | ELOG_FMT_P_INFO));
	elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_T_INFO | ELOG_FMT_P_INFO));
    // ������־���˼���Ϊ DEBUG��ֻ��� DEBUG �����ϵȼ�����־
   //elog_set_filter_lvl(ELOG_LVL_ERROR);
	//����log���
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
    delay_1ms(2000);    // ��ʱ2�룬�ȴ���������ʼ�����
    waterTapInit();
    if(config.data.isLock)  //�ж��Ƿ������豸
    {
        log_i("lock");
        lockDevice();
    }
    else
    {
        log_i("unlock");
        unlockDevice();
    }
    if(gpio_input_bit_get(ADDR0_PORT,ADDR0_PIN) == 1)   //�ж��Ƿ������
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
 * @brief  ��ʼ������
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
    add_task_with_param(led_toggle_param,&led1,NULL, 100, true);    //LED����ָʾ��
    add_task(lcd_cmd_scan,NULL,1,true);                             //���ͻ����������ݽ���
    add_task(lcd_cmd_scan2,NULL,1,true);                            //ˮ��ͷ���������ݽ���
    add_task(waterLevel_update,NULL,100,true);                      //ˮλ���
    add_task(waterTapProcessControl,NULL,100,true);                 //ˮ��ͷ���ݴ���
    add_task(debugProcess,NULL,10,true);                            //��λ�����ݽ���
}

/* ����޲������� */
uint8_t add_task(task_func_t func, task_condition_t cond, uint16_t period, uint8_t enabled)
{
    // ����������ڵ��ڴ�
    TaskNode_t* new_task = (TaskNode_t*)malloc(sizeof(TaskNode_t));
    if (new_task == NULL) {
        memory_allocation_failed();
        return false;
    }
    
    // ��ʼ������ڵ�
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
    
    // ��ӵ�����
    if (task_queue.head == NULL) {
        // �ն���
        task_queue.head = new_task;
        task_queue.tail = new_task;
    } else {
        // ��ӵ�����β��
        task_queue.tail->next = new_task;
        task_queue.tail = new_task;
    }
    
    task_queue.count++;
    return true;
}


/* ��Ӵ����������� */
uint8_t add_task_with_param(task_func_param_t func, void* param, task_condition_t cond, uint16_t period, uint8_t enabled)
{
    // ����������ڵ��ڴ�
    TaskNode_t* new_task = (TaskNode_t*)malloc(sizeof(TaskNode_t));
    if (new_task == NULL) {
        memory_allocation_failed();
        return false;
    }
    
    // ��ʼ������ڵ�
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
    
    // ��ӵ�����
    if (task_queue.head == NULL) {
        // �ն���
        task_queue.head = new_task;
        task_queue.tail = new_task;
    } else {
        // ��ӵ�����β��
        task_queue.tail->next = new_task;
        task_queue.tail = new_task;
    }
    
    task_queue.count++;
    return true;
}


/* ɾ������ */
uint8_t delete_task(task_func_t func) {
    TaskNode_t* current = task_queue.head;
    TaskNode_t* prev = NULL;
    
    while (current != NULL) {
        if (!current->has_param && current->function == func) {
            // �ҵ�Ҫɾ��������
            if (prev == NULL) {
                // ɾ��ͷ�ڵ�
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

/* ɾ�������������� */
uint8_t delete_task_with_param(task_func_param_t func, void* param) {
    TaskNode_t* current = task_queue.head;
    TaskNode_t* prev = NULL;
    
    while (current != NULL) {
        if (current->has_param && current->param_function == func && current->param == param) {
            // �ҵ�Ҫɾ��������
            if (prev == NULL) {
                // ɾ��ͷ�ڵ�
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

/* �������� */
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

/* ���ô����������� */
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

/* �������� */
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

/* ���ô����������� */
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


/* ����״̬�� */
void set_state_machine(State_t* states, uint8_t count)
{
    state_machine = states;
    state_count = count;
    current_state = 0;
}


/* ״̬��ִ�� */
void execute_state_machine(void)
{
    if (state_machine == NULL || current_state >= state_count) {
        return;
    }
    
    // ִ�е�ǰ״̬����
    if (state_machine[current_state].action != NULL) {
        state_machine[current_state].action();
    }
    
    // ���״̬ת��
    uint8_t i;
    for (i = 0; i < state_machine[current_state].num_transitions; i++) {
        StateTransition_t* trans = &state_machine[current_state].transitions[i];
        if (trans->condition != NULL && trans->condition()) {
            current_state = trans->next_state;
            break;
        }
    }
}


/* ��������� */
void scheduler(void)
{
    TaskNode_t* current = task_queue.head;
    
    while (current != NULL) {
        if (current->ready && current->enabled) {
            // ���ִ������
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
 * @brief  LED��˸����
 * 
 * @param[in] led_num  LED���
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


//Ѱ�������Ƿ���������
/**
 * ************************************************************************
 * @brief Ѱ�������Ƿ���������
 * 
 * @param[in] arr  ����
 * @param[in] num  ��ҪѰ�ҵ�����
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
 * @brief �����豸
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
    SetControlEnable(make_coffe_set_page,5,0,UART2_ID);   //���ÿ�����ȡ��ʼ��ť
    SetControlEnable(make_tea_set_page,5,0,UART2_ID);   //���ÿ�����ȡ���ð�ť
    SetControlEnable(main_page,4,0,UART2_ID);   //����������ť
    SetControlEnable(main_page,3,0,UART2_ID);   //������ϴ��ť
}

/**
 * ************************************************************************
 * @brief �����豸
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
    SetControlEnable(make_coffe_set_page,5,1,UART2_ID);   //����������ȡ��ʼ��ť
    SetControlEnable(make_tea_set_page,5,1,UART2_ID);   //����������ȡ���ð�ť
    SetControlEnable(main_page,4,1,UART2_ID);   //����������ť
    SetControlEnable(main_page,3,1,UART2_ID);   //������ϴ��ť
}

