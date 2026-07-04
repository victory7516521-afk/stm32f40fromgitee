#include "hsr04.h"


void Hsr04_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStruct;
	
	//使能GPIOA组时钟//通用输出
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    //1、能定时器时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_2;	//引脚2
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_OUT; //输出模式
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_PP; //输出推挽
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_25MHz; //速度
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_3;	//引脚3
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_IN; //输入模式
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA, &GPIO_InitStruct);
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseInitStruct;
    

    
    TIM_TimeBaseInitStruct.TIM_Prescaler    = 84-1;  // 84分频 84MHZ/84 = 1MHZ 计一个数用1us
    TIM_TimeBaseInitStruct.TIM_Period       = 50000-1; //（其实计不到这么多数）                                TIM_TimeBaseInitStruct.TIM_CounterMode    = TIM_CounterMode_Up;    // 向上计数
    TIM_TimeBaseInitStruct.TIM_ClockDivision= TIM_CKD_DIV1;            // 分频因子
    //2、初始化定时器，配置ARR,PSC
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
            
    //5,不使能定时器
    TIM_Cmd(TIM3, DISABLE);		

}

int Get_Hsr04_distance(void)
{
	int t=0, dis, temp;
	/*触发信号*/
	//PA2输出低电平
	PAout(2) = 0;
	delay_us(8);
	//PA2输出高电平
	PAout(2) = 1;
	delay_us(20); //至少10us
	//PA2输出低电平
	PAout(2) = 0;
	
	
	
	

	//设置定时器的CNT为0  
	TIM3->CNT = 0;

	//PA3等待高电平到来，参考按键松开代码 
	while( PAin(3)  == 0 )
	{
		delay_us(10);
		t++;
		
		//等待5ms未有高电平，则返回-1
		if(t >= 500)  
			return -1;
	}

	//使能定时器开始计数
	TIM_Cmd(TIM3, ENABLE);		
	
	t = 0;
	//PA3等待低电平到来，
	while( PAin(3) == 1)
	{
		delay_us(10);
		t++;
		
		//等待24ms未有低电平，则返回-2
		if(t >= 2400)  
			return -2;	
	}
	
	//获取定时器CNT值，从而得到高电平持续时间    
	temp = TIM3->CNT;

	//关闭定时器
	TIM_Cmd(TIM3, DISABLE);	

	//通过公式计算出超声波测量距离
	dis = (temp*1)/58;

	return dis;
}


