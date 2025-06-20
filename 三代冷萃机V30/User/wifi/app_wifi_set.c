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
 * @brief WIFI���������ô��룬���Կ����豸����
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
        case 0: //�ӳ�5����豸����
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
        case 1: //����AT����ȷ���豸������
            if(AT_SendCommand("AT", 500, ATCommandCallback))
            {
                step++;
            }
            break;
        case 2: //�ж��豸�Ƿ������ɹ�
            if (WIFI.isOK == 1)
            {
                WIFI.isInitialized = 1;
                step++;
                log_v("wifi init ok");
            }
            else if (WIFI.isOK == 2)    //��ʱ
            {
                step = 1;   //���·���AT����
                log_e("wifi init timeout");
            }
            break;
        case 3:
            if(AT_SendCommand("ATE0",500,ATCommandCallback))
            {
                step++;
            }
        case 4: //�ӳ�5��ȴ��豸����
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
        case 5: //��ѯ�豸�Ƿ�������
            if(AT_SendCommand("AT+WJAP?", 500, AT_WJAPCallback))
            {
                step++;
            }
            break;
        case 6:
            if (Network.connectionStatus == 0 || Network.connectionStatus == 3 ||Network.connectionStatus == 4)  //������WIFI
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
        case 7: //WIFI���ӿ���������
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
    { // ��ʱ
        WIFI.isOK = 2;
        log_e("AT command timeout");
    }
}

/**
 * ************************************************************************
 * @brief ��ѯ����AP״̬
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
    if (success)    //���յ���������
    {
        char *p = strstr(response, "+WJAP:");
        if (p)
        {
            // ����ʽ�Ƿ���ȷ
            if (p[5] == ':' && p[6] >= '0' && p[6] <= '9')
            {
                Network.connectionStatus = p[6] - '0';
                
                // ��������״̬���ñ�־
                if (Network.connectionStatus == 3)
                {
                    WIFI.isConnectionConfiguration = 1;  // ���ӳɹ�����ȡIP
                    Network.connectionStatus = 3;
                    WIFI.isInitialized = 1;
                    // ����δ����ͼ�꣬��ʾ������ͼ��
                    SetControlVisiable(main_page, 7, 0, UART2_ID);  // ����δ����ͼ��
                    SetControlVisiable(main_page, 8, 1, UART2_ID);  // ��ʾ������ͼ��
                    WIFI.isConnected = 1;
                    log_v("WiFi connected successfully with IP");
                }
                else if (Network.connectionStatus == 0)
                {
                    WIFI.isConnectionConfiguration = 0;  // δ����
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
                    WIFI.isConnectionConfiguration = 2;  // ����ʧ��
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
    { // ��ʱ
        log_e("WJAP query timeout");
        //WIFI.isConnectionConfiguration = 2;
    }
}


/**
 * ************************************************************************
 * @brief �ֻ�����
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
                if(cnt%10 == 0) //20���ѯһ������״̬
                {
                    AT_SendCommand("AT+WJAP?", 10000, AT_WJAPCallback);
                    log_v("WCONFIG success");
                }
                if(cnt>60)//120�볬ʱ
                {
                    delete_task(WIFIAutoConfigNetWork);
                    AnimationPlayFrame(wifi_setting_page,1,WIFI_SET_TIMEOUT,UART2_ID);
                    step = 0;
                }
                if(Network.connectionStatus == 3) //���ӳɹ�
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
 * @brief �����Զ��������غ���
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
    { // ��ʱ
        //Network.isAutoConnect = 2;
        log_e("set WAUTOCONN timeout");
    }
}
/**
 * ************************************************************************
 * @brief ����wifi����ģʽ���غ���
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
        // ��ʱ
        //Network.WMODE = 4;
        log_e("set WMODE timeout");
    }
}
/**
 * ************************************************************************
 * @brief �����Զ�������ʽ���غ���
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
        // ��ʱ
        //Network.isWCONFIG = 2;
        log_e("set WCONFIG timeout");
    }
}

/**
 * ************************************************************************
 * @brief ֹͣ�������غ���
 * 
 * @param[in] success  �ɹ���־
 * @param[in] response  ��������
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
        // ��ʱ
        //Network.isWCONFIG = 2;
        log_e("set WCONFIG timeout");
    }
}

