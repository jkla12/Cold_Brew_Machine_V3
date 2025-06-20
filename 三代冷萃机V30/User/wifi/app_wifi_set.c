#include "app_wifi_set.h"
#include "work.h"
#include "uart.h"
#include "string.h"
#include "stdio.h"
#include "timer.h"
#include "eeprom.h"
#include "lcd_data_process.h"
#include "wifiSetPage.h"
#include "app_mqtt.h"
#include "app_wifi.h"
#include "elog.h"

#ifdef 	LOG_TAG
#undef	LOG_TAG
#endif
#define LOG_TAG    "app_wifi_set"

NetworkTypeDef Network;
/**
 * ************************************************************************
 * @brief WIFI启动后配置代码，用以控制设备联网
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-23
 * 
 * ************************************************************************
 */
void WIFIConfiguration(void)
{
    static uint8_t step,delayTime = 0;
    
    switch (step)
    {
        case 0: //延迟5秒等设备启动
            if (delayTime < 5)  
            {
                delayTime++;
            }
            else
            {
                delayTime = 0;
                step++;
            }
            break;
        case 1: //发送AT命令确认设备已启动
            if(AT_SendCommand("AT", 500, ATCommandCallback))
            {
                step++;
            }
            break;
        case 2: //判断设备是否启动成功
            if (WIFI.isOK == 1)
            {
                WIFI.isInitialized = 1;
                step++;
                log_v("wifi init ok");
            }
            else if (WIFI.isOK == 2)    //超时
            {
                step = 1;   //重新发送AT命令
                log_e("wifi init timeout");
            }
            break;
        case 3:
            if(AT_SendCommand("ATE0",500,ATCommandCallback))
            {
                step++;
            }
        case 4: //延迟5秒等待设备联网
			if(delayTime < 5)	
            {
                delayTime++;
            }	
            else
            {
                delayTime = 0;
                step++;
            }
            break;
        case 5: //查询设备是否已联网
            if(AT_SendCommand("AT+WJAP?", 500, AT_WJAPCallback))
            {
                step++;
            }
            break;
        case 6:
            if (Network.connectionStatus == 0 || Network.connectionStatus == 3 ||Network.connectionStatus == 4)  //已连接WIFI
            {
                if(Network.connectionStatus == 3)
                {
                    WIFI.isConnectionConfiguration = 1;
                    log_v("wifi connected");

                }
                else if(Network.connectionStatus == 0)
                {
                    WIFI.isConnectionConfiguration = 0;
                    log_v("wifi no connection configured");
                }
                step++;
            }
            else
            {
                step = 5;
                log_v("wifi connecting");
            }
            break;
        case 7: //WIFI连接开机检测完成
            step = 0;
            delayTime = 0;
            delete_task(WIFIConfiguration);
            WIFI.runWIFIConfigEnd = 1;
            log_v("wifi Startup detection completed");
            break;
        default:
            break;
    }

    
}

/**
 * ************************************************************************
 * @brief
 *
 * @param[in] success  Comment
 * @param[in] response  Comment
 *
 *
 * @version 1.0
 * @author jiaokai
 * @date 2025-04-22
 *
 * ************************************************************************
 */
void ATCommandCallback(uint8_t success, const char *response)
{
    if (success)
    {
        WIFI.isOK = 1;
        log_v("AT command success");
    }
    else
    { // 超时
        WIFI.isOK = 2;
        log_e("AT command timeout");
    }
}

/**
 * ************************************************************************
 * @brief 查询连接AP状态
 * 
 * @param[in] success  Comment
 * @param[in] response  Comment
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-23
 * 
 * ************************************************************************
 */
void AT_WJAPCallback(uint8_t success, const char *response)
{
    if (success)    //接收到返回数据
    {
        char *p = strstr(response, "+WJAP:");
        if (p)
        {
            // 检查格式是否正确
            if (p[5] == ':' && p[6] >= '0' && p[6] <= '9')
            {
                Network.connectionStatus = p[6] - '0';
                
                // 根据连接状态设置标志
                if (Network.connectionStatus == 3)
                {
                    WIFI.isConnectionConfiguration = 1;  // 连接成功并获取IP
                    Network.connectionStatus = 3;
                    WIFI.isInitialized = 1;
                    // 隐藏未连接图标，显示已连接图标
                    SetControlVisiable(main_page, 7, 0, UART2_ID);  // 隐藏未连接图标
                    SetControlVisiable(main_page, 8, 1, UART2_ID);  // 显示已连接图标
                    WIFI.isConnected = 1;
                    log_v("WiFi connected successfully with IP");
                }
                else if (Network.connectionStatus == 0)
                {
                    WIFI.isConnectionConfiguration = 0;  // 未连接
                    log_v("WiFi not connected");
                    if (WIFI.isConnected == 1)
                    {
                        SetControlVisiable(main_page, 7, 1, UART2_ID);  
                        SetControlVisiable(main_page, 8, 0, UART2_ID); 
                    }
                    WIFI.isConnected = 0;
                }
                else if (Network.connectionStatus == 4)
                {
                    WIFI.isConnectionConfiguration = 2;  // 连接失败
                    log_e("WiFi connection failed");
                    if (WIFI.isConnected == 1)
                    {
                        SetControlVisiable(main_page, 7, 1, UART2_ID);  
                        SetControlVisiable(main_page, 8, 0, UART2_ID); 
                    }
                    WIFI.isConnected = 0;
                }
            }
            else
            {
                log_e("WJAP response format error");
            }
        }
    }
    else
    { // 超时
        log_e("WJAP query timeout");
        //WIFI.isConnectionConfiguration = 2;
    }
}


/**
 * ************************************************************************
 * @brief 手机配网
 * 
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-24
 * 
 * ************************************************************************
 */
void WIFIAutoConfigNetWork(void)
{
    static uint8_t step = 0;
    static uint8_t cnt = 0;
    switch (step)
    {
        case 0:
            if(WIFI.isConnected == 0)
            {
                delete_task(WIFIConfiguration);
                delete_task(mqttConfig);
                
            }
            step++;
            break;
        case 1:
        
            if(AT_SendCommand("AT", 1000, ATCommandCallback))
            {
                step++;
            }
            break;
        case 2:
            if(AT_SendCommand("AT+WMODE=1,1",1000,AT_WMODECallback))
            {
                log_v("set WMODE");
                //Network.WMODE = 0;
                step++;
            }
            break;
        case 3:
            if(Network.WMODE == 1)
            {
                Network.isAutoConnect = 0;
                step=4;
            }
            else if(Network.WMODE == 4)
            {
                step = 2;
            }
            break;
        case 4:
            if(AT_SendCommand("AT+WAUTOCONN=1",1000,AT_WAUTOCONNCallback))
            {
                log_v("set WAUTOCONN");
                step++;
            }
            break;
        case 5:
            if(Network.isAutoConnect == 1)
            {
                step++;
            }
            else if(Network.isAutoConnect == 2)
            {
                step = 3;
            }
            break;
        case 6:
            if(AT_SendCommand("AT+WCONFIG=9",10000,AT_WCONFIGCallback))
            {
                log_v("set WCONFIG");
                step++;
            }
            break;
        case 7:
            if(Network.isWCONFIG == 1)
            {
                cnt++;
                if(cnt%10 == 0) //20秒查询一次连接状态
                {
                    AT_SendCommand("AT+WJAP?", 10000, AT_WJAPCallback);
                    log_v("WCONFIG success");
                }
                if(cnt>60)//120秒超时
                {
                    delete_task(WIFIAutoConfigNetWork);
                    AnimationPlayFrame(wifi_setting_page,1,WIFI_SET_TIMEOUT,UART2_ID);
                    step = 0;
                }
                if(Network.connectionStatus == 3) //连接成功
                {
                    cnt = 0;
                    Network.isWCONFIG = 0;
                    Network.isAutoConnect = 0;
                    delete_task(WIFIAutoConfigNetWork);
                    add_task(mqttConfig,NULL,1000,true);
                    step = 0;
                    WIFI.runWIFIConfigEnd = 1;
                    AnimationPlayFrame(wifi_setting_page,1,WIFI_SET_COMPLETE,UART2_ID);
                    
                }
            }
            else if(Network.isWCONFIG == 2)
            {
                step = 6;
            }
            break;
        default:
            break;

    }
    
}

/**
 * ************************************************************************
 * @brief 设置自动联网返回函数
 * 
 * @param[in] success  Comment
 * @param[in] response  Comment
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-24
 * 
 * ************************************************************************
 */
void AT_WAUTOCONNCallback(uint8_t success, const char *response)
{
    if (success)
    {
        if (strstr(response, "OK"))
        {
            Network.isAutoConnect = 1;
            log_v("set WAUTOCONN success");
        }
    }
    else
    { // 超时
        //Network.isAutoConnect = 2;
        log_e("set WAUTOCONN timeout");
    }
}
/**
 * ************************************************************************
 * @brief 设置wifi工作模式返回函数
 * 
 * @param[in] success  Comment
 * @param[in] response  Comment
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-24
 * 
 * ************************************************************************
 */
void AT_WMODECallback(uint8_t success,const char *response)
{
    if (success)
    {
        if (strstr(response, "OK"))
        {
            Network.WMODE = 1;
            log_v("set WMODE success");
        }
    }
    else
    {
        // 超时
        //Network.WMODE = 4;
        log_e("set WMODE timeout");
    }
}
/**
 * ************************************************************************
 * @brief 设置自动配网方式返回函数
 * 
 * @param[in] success  
 * @param[in] response  Comment
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-04-24
 * 
 * ************************************************************************
 */
void AT_WCONFIGCallback(uint8_t success,const char *response)
{
    if (success)
    {
        if (strstr(response, "OK"))
        {
            Network.isWCONFIG = 1;
            log_v("set WCONFIG success");
        }
    }
    else
    {
        // 超时
        //Network.isWCONFIG = 2;
        log_e("set WCONFIG timeout");
    }
}

/**
 * ************************************************************************
 * @brief 停止配网返回函数
 * 
 * @param[in] success  成功标志
 * @param[in] response  返回数据
 * 
 * 
 * @version 1.0
 * @author jiaokai 
 * @date 2025-05-16
 * 
 * ************************************************************************
 */
void AT_WCONFIGSTOPCallback(uint8_t success,const char *response)
{
    if (success)
    {
        if (strstr(response, "OK"))
        {
            Network.isWCONFIG = 0;
            log_v("stop WCONFIG success");
        }
    }
    else
    {
        // 超时
        //Network.isWCONFIG = 2;
        log_e("set WCONFIG timeout");
    }
}

