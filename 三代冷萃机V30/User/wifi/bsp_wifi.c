#include "bsp_wifi.h"
#include "app_wifi.h"
#include "uart.h"
#include "string.h"





/**
 * ************************************************************************
 * @brief WIFI模组初始化
 * 
 * @param[in] baud  通讯波特率
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
    GPIO_BC(WIFI_EN_PORT) = WIFI_EN_PIN;	//电平置低，关闭WIFI
    uart4_config(baud);
    memset(&WIFI, 0, sizeof(WIFI_TypeDef));
}

/**
 * ************************************************************************
 * @brief 使能WIFI
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
    GPIO_BOP(WIFI_EN_PORT) = WIFI_EN_PIN;	//电平置高，打开WIFI
}

/**
 * ************************************************************************
 * @brief 关闭WIFI
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
    GPIO_BC(WIFI_EN_PORT) = WIFI_EN_PIN;	//电平置低，关闭WIFI
}




