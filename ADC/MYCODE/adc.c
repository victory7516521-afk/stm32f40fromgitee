#include "adc.h"

/*
引脚说明

可调电阻连接在PA5
ADC12_IN5
PA5电压变化范围:0~3.3V

*/
void Adc_PA4_Init(void)
{
	GPIO_InitTypeDef		GPIOA_InitStruct;
	ADC_CommonInitTypeDef 	ADC_CommonInitStruct;
	ADC_InitTypeDef			ADC_InitStruct;
	
	//开启PA口时钟和ADC1时钟，设置PA5为模拟模式--改
	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);  
	

	GPIOA_InitStruct.GPIO_Pin	= GPIO_Pin_5;		//引脚0 --改
	GPIOA_InitStruct.GPIO_Mode	= GPIO_Mode_AN;		//模拟模式
	GPIOA_InitStruct.GPIO_PuPd	= GPIO_PuPd_NOPULL; //浮空
	GPIO_Init(GPIOA, &GPIOA_InitStruct);	
	

	ADC_CommonInitStruct.ADC_Mode		= ADC_Mode_Independent;//独立模式
	ADC_CommonInitStruct.ADC_Prescaler	= ADC_Prescaler_Div4; //84MHZ/4 = 21MHZ,这个频率不能下过36MHZ
	ADC_CommonInitStruct.ADC_DMAAccessMode	= ADC_DMAAccessMode_Disabled;//不使用DMA
	ADC_CommonInitStruct.ADC_TwoSamplingDelay= ADC_TwoSamplingDelay_16Cycles;//两个通道以上的ADC采样间隔
	//（3）、初始化ADC_CCR寄存器。
	ADC_CommonInit(&ADC_CommonInitStruct);
	
	
	ADC_InitStruct.ADC_Resolution 	= ADC_Resolution_12b; //12位分辨率 4096
	ADC_InitStruct.ADC_DataAlign  	= ADC_DataAlign_Right;//数据对齐 右对齐
	ADC_InitStruct.ADC_ScanConvMode = DISABLE; //不扫描,两个通道以上，才需要连续扫描
	ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;//配置为单次转换
	//这里不选定，下面的参数无意义--说明使用软件触发
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; //无边沿触发
	//ADC_InitStruct.ADC_ExternalTrigConv   = 
	ADC_InitStruct.ADC_NbrOfConversion = 1;  //转换通道数目

	//初始化ADC1参数，设置ADC1的工作模式以及规则序列的相关信息。--改
	ADC_Init(ADC1, &ADC_InitStruct);
	//使能ADC--改
	ADC_Cmd(ADC1, ENABLE);
	//配置规则通道参数-- 改  IN5--ADC_Channel_5 1:第一个转换 ADC_SampleTime_56Cycles:采样时间
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_56Cycles);

}

u16 Get_Adc_Value(void)
{
	u16 value;
	
	//开启软件转换：
	ADC_SoftwareStartConv(ADC1);
	//等待转换完成，读取ADC值--改
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	value = ADC_GetConversionValue(ADC1);

	return value;
}
