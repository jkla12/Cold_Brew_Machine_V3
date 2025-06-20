#include "bsp_wifi.h"
#include "app_wifi.h"
#include "uart.h"
#include "string.h"





/**
 * ************************************************************************
 * @brief WIFIģ���ʼ��
 * 
 * @param[in] baud  ͨѶ������
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-22
 * 
 * ************************************************************************
 */
void wifiInit(uint32_t baud)
{
    rcu_periph_clock_enable(WIFI_EN_CLOCK);
	gpio_init(WIFI_EN_PORT,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,WIFI_EN_PIN);
    GPIO_BC(WIFI_EN_PORT) = WIFI_EN_PIN;	//��ƽ�õͣ��ر�WIFI
    uart4_config(baud);
    memset(&WIFI, 0, sizeof(WIFI_TypeDef));
}

/**
 * ************************************************************************
 * @brief ʹ��WIFI
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-22
 * 
 * ************************************************************************
 */
void WIFI_ENABLE(void)
{
    GPIO_BOP(WIFI_EN_PORT) = WIFI_EN_PIN;	//��ƽ�øߣ���WIFI
}

/**
 * ************************************************************************
 * @brief �ر�WIFI
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-22
 * 
 * ************************************************************************
 */
void WIFI_DISABLE(void)
{
    GPIO_BC(WIFI_EN_PORT) = WIFI_EN_PIN;	//��ƽ�õͣ��ر�WIFI
}




