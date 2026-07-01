#include "adc.h"

/************************************
引脚说明：

可调电阻连接在PA5
ADC12_IN5
PA5电压变化范围:0~3.3V
*************************************/

void Adc_PA4_init(void)
{
	GPIO_InitTypeDef 		GPIO_InitStructure;
	ADC_CommonInitTypeDef	ADC_CommonInitStruct;
	ADC_InitTypeDef			ADC_InitStruct;
	
	//（2）、开启PA口时钟和ADC1时钟，设置PA5为模拟模式。
	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);  
	
	//配置为GPIOF9
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_5;		//引脚5
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AN;		//模拟功能
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_NOPULL ;//浮空
	GPIO_Init(GPIOA, &GPIO_InitStructure);  

	
	ADC_CommonInitStruct.ADC_Prescaler		= ADC_Prescaler_Div4; //84MHZ/4 = 21MHZ ADC转换频率不超过36MHZ
	ADC_CommonInitStruct.ADC_Mode			= ADC_Mode_Independent;//独立模式
	ADC_CommonInitStruct.ADC_DMAAccessMode  = ADC_DMAAccessMode_Disabled; //不使能DMA
	ADC_CommonInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_15Cycles;//ADC之间采样间隔
	//（3）、初始化ADC_CCR寄存器。
	ADC_CommonInit(&ADC_CommonInitStruct);
	
	
	ADC_InitStruct.ADC_Resolution			= ADC_Resolution_12b; //1位分辨率
	ADC_InitStruct.ADC_ScanConvMode			= DISABLE; //不连续转换
	ADC_InitStruct.ADC_ContinuousConvMode	= DISABLE; //单次转换
	ADC_InitStruct.ADC_ExternalTrigConvEdge	= ADC_ExternalTrigConvEdge_None; //无极性，选定软件触发
	//选择软件触发，此参数ADC_InitStruct.ADC_ExternalTrigConv	可忽略
	ADC_InitStruct.ADC_ExternalTrigConv		= ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStruct.ADC_DataAlign			= ADC_DataAlign_Right; //数据右对齐
	ADC_InitStruct.ADC_NbrOfConversion		= 1; //AD转换通道数目

	
	//（4）、初始化ADC1参数，设置ADC1的工作模式以及规则序列的相关信息。
	ADC_Init(ADC1, &ADC_InitStruct);
	
	
	//（5）、使能ADC。
	ADC_Cmd(ADC1, ENABLE);
	//（6）、配置规则通道参数：IN5 == ADC_Channel_5
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_28Cycles);


}


u16 Get_Adc1_value(void)
{
	u16 value;
	
	//开启软件转换：
	ADC_SoftwareStartConv(ADC1);
	//等待转换完成，读取ADC值。
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	
	value = ADC_GetConversionValue(ADC1);	

}
