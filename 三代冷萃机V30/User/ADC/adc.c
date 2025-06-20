#include "adc.h"
#include "systick.h"

uint16_t adc_val;

#define FILTER_WINDOW_SIZE 10

uint32_t filter_buffer[2][FILTER_WINDOW_SIZE];
uint32_t filter_index = 0;


void adc0_config(void)
{
	 /*����ADC��DMA�ṹ��*/
    dma_parameter_struct dma_data_parameter;
	 /*ʹ��GPIOʱ��*/
	 rcu_periph_clock_enable(ADC0_CLOCK);	
	 /*ʹ��ADCʱ��*/
	 rcu_periph_clock_enable(RCU_ADC0);
	 rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV8);
	 /*ʹ��DMAʱ��*/
	 rcu_periph_clock_enable(RCU_DMA0);
	
	 gpio_init(ADC0_PORT, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, ADC0_PIN);	//ADC0_CH0,PA0
	
			/*��λDMA*/
	dma_deinit(DMA0, DMA_CH0);
	
	/* ��ʼ��DMA���� */
	dma_data_parameter.periph_addr  = (uint32_t)(&ADC_RDATA(ADC0));
	dma_data_parameter.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
	
	dma_data_parameter.memory_addr  = (uint32_t)(&adc_val);
	dma_data_parameter.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
	
	dma_data_parameter.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
	dma_data_parameter.memory_width = DMA_MEMORY_WIDTH_16BIT;
	
	dma_data_parameter.direction    = DMA_PERIPHERAL_TO_MEMORY;
	dma_data_parameter.number       = 1;
	dma_data_parameter.priority     = DMA_PRIORITY_HIGH;
	dma_init(DMA0, DMA_CH0, &dma_data_parameter);
	dma_circulation_enable(DMA0, DMA_CH0);

	/* ʹ��DMA */
	dma_channel_enable(DMA0, DMA_CH0);
	
	 /* ADCɨ��ģʽ */
    adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);	//��ͨ���ر�ɨ��ģʽ
    /* ����ADC����ת��ģʽ */
    adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, ENABLE);	
   
	  /* ADC�������� */
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);	//�������
		
    /*ADC���뷽ʽ���Ҷ���*/
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
		
    adc_mode_config(ADC_MODE_FREE);	
		
    /* ADCͨ���鳤�ȣ��ɼ���·������Ϊ�� */
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 1);
		
		/* ʹ��ADC��DMA���� */
    adc_dma_mode_enable(ADC0);
		
    /* ����ADC������ */
    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_0, ADC_SAMPLETIME_239POINT5);	//239.5��ʱ�����ڲ�����



    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);	//ʹ��ת��
    
    /* ʹ��ADC */
    adc_enable(ADC0);
    delay_1ms(1);
    /* ʹ��ADCУ׼*/
    adc_calibration_enable(ADC0);


    /* ʹ�����ת��*/
    adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
	//filter_init();
}

///**********************************************************************
//**����ԭ��:	void readADCValue(void)
//**��������:	��ȡADCֵ
//**��ڲ���: *value:��ȡ��ֵ
//**          
//**���ڲ���:	��
//**��    ע:	1
//***********************************************************************/
//void readADCValue(uint16_t *value)
//{
//	moving_average_filter(adc_val[0],adc_val[1],value);
//}




//void filter_init(void) {
//    for (int i = 0; i < FILTER_WINDOW_SIZE; i++) {
//        filter_buffer[0][i] = 0;
//		filter_buffer[1][i] = 0;
//    }
//    filter_index = 0;
//}
///**********************************************************************
//**����ԭ��:	void moving_average_filter(uint16_t new_value1,uint16_t new_value2,uint16_t *filterValue)
//**��������:	�ƶ�ƽ���˲�
//**��ڲ���:	new_value1:��ֵ1
//**			new_value2:��ֵ2
//**			filterValue:�˲����ֵ
//**          
//**���ڲ���:	��
//**��    ע:	1
//***********************************************************************/
//void moving_average_filter(uint16_t new_value1,uint16_t new_value2,uint16_t *filterValue) 
//{
//	uint32_t sum[2] = 0;
//    filter_buffer[0][filter_index] = new_value1;
//	filter_buffer[1][filter_index] = new_value2;
//    filter_index = (filter_index + 1) % FILTER_WINDOW_SIZE;

//    
//    for (int i = 0; i < FILTER_WINDOW_SIZE; i++) {
//        sum[0] += filter_buffer[0][i];
//		sum[1] += filter_buffer[1][i];
//    }
//	filterValue[0] = sum[0] / FILTER_WINDOW_SIZE;
//	filterValue[1] = sum[1] / FILTER_WINDOW_SIZE;
//}


