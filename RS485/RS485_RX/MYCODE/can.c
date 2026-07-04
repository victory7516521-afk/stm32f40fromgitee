#include "can.h"
#include "led.h"
#include "delay.h"
#include "usart.h"



/************************************
引脚说明

CAN1在APB1(42MHZ)
CAN_RX PD0 -- CAN1_RX
CAN_TX PD1 -- CAN1_TX

************************************/

//CAN初始化
//tsjw:重新同步跳跃时间单元.范围:CAN_SJW_1tq~ CAN_SJW_4tq
//tbs2:时间段2的时间单元.   范围:CAN_BS2_1tq~CAN_BS2_8tq;
//tbs1:时间段1的时间单元.   范围:CAN_BS1_1tq ~CAN_BS1_16tq
//brp :波特率分频器.范围:1~1024; tq=(brp)*tpclk1
//波特率=Fpclk1/((tbs1+1+tbs2+1+1)*brp);  Fpclk1 = 42MHZ
//mode:CAN_Mode_Normal,普通模式;CAN_Mode_LoopBack,回环模式;
//Fpclk1的时钟在初始化的时候设置为42M,如果设置CAN1_Mode_Init(CAN_SJW_1tq,CAN_BS2_6tq,CAN_BS1_7tq,6,CAN_Mode_LoopBack);
//则波特率为:42M/((6+7+1)*6)=500Kbps
//返回值:0,初始化OK;
//    其他,初始化失败; 


u8 CAN1_Mode_Init(u8 tsjw,u8 tbs2,u8 tbs1,u16 brp,u8 mode)
{

	GPIO_InitTypeDef GPIO_InitStructure; 
	CAN_InitTypeDef        CAN_InitStructure;
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
	
#if CAN1_RX0_INT_ENABLE 
   	NVIC_InitTypeDef  NVIC_InitStructure;
#endif
    //使能相关时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);//使能PORTD时钟	                   											 

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//使能CAN1时钟	
	
    //初始化GPIO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0| GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化PA11,PA12
	
	  //引脚复用映射配置
	  GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_CAN1); //GPIOA11复用为CAN1
	  GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_CAN1); //GPIOA12复用为CAN1
	  
  	//CAN单元设置
   	CAN_InitStructure.CAN_TTCM=DISABLE;	//非时间触发通信模式   
  	CAN_InitStructure.CAN_ABOM=DISABLE;	//软件自动离线管理	  
  	CAN_InitStructure.CAN_AWUM=DISABLE;//睡眠模式通过软件唤醒(清除CAN->MCR的SLEEP位)
  	CAN_InitStructure.CAN_NART=ENABLE;	//禁止报文自动传送 
  	CAN_InitStructure.CAN_RFLM=DISABLE;	//报文不锁定,新的覆盖旧的  
  	CAN_InitStructure.CAN_TXFP=DISABLE;	//优先级由报文标识符决定 
  	CAN_InitStructure.CAN_Mode= mode;	 //模式设置  禁止环回模式
  	CAN_InitStructure.CAN_SJW=tsjw;	//重新同步跳跃宽度(Tsjw)为tsjw+1个时间单位 CAN_SJW_1tq~CAN_SJW_4tq
  	CAN_InitStructure.CAN_BS1=tbs1; //Tbs1范围CAN_BS1_1tq ~CAN_BS1_16tq
  	CAN_InitStructure.CAN_BS2=tbs2;//Tbs2范围CAN_BS2_1tq ~	CAN_BS2_8tq
  	CAN_InitStructure.CAN_Prescaler=brp;  //分频系数(Fdiv)为brp+1	
  	CAN_Init(CAN1, &CAN_InitStructure);   // 初始化CAN1 
   


    //标准帧ID  11位
	//0x711是期望的报文ID
//    uint16_t CanFilterListSTID1 = 0x711; //标准ID，此值范围：0x000~0x7FF(0111 1111 1111)
//    uint8_t CanFilterListIDE1 = 0x00;//帧类型：0x00标准格式,0x01扩展格式
//    uint8_t CanFilterListRTR1 = 0x00;//帧类型：0x00数据帧,0x01远程帧
//	//配置过滤器--屏蔽位模式
//	CAN_FilterInitStructure.CAN_FilterNumber=0;	  //过滤器0
//	CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;   //标识符掩码
//	CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit; //32位 
//	
//	//32位筛选器分高16位与低16位，分两组寄存器ID与掩码  
//	//寄存器ID
//	//High占高16位 31~16位 
//	CAN_FilterInitStructure.CAN_FilterIdHigh=(CanFilterListSTID1<<5);   //0x711是期望的报文ID设置到ID寄存器
//	//Low占低16位  15~0位
//	CAN_FilterInitStructure.CAN_FilterIdLow=  0x0000; //不使用扩展帧，IDE=0，接收数据帧RTR也=0,
//	
//	//掩码寄存器
//	CAN_FilterInitStructure.CAN_FilterMaskIdHigh= 0x0000; //任何标准数据帧都能过 
//	CAN_FilterInitStructure.CAN_FilterMaskIdLow =0xFFFD;  //(1101)

//	
//	CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO0;//过滤器0关联到FIFO0
//	CAN_FilterInitStructure.CAN_FilterActivation=ENABLE; //激活过滤器0
//	
//	
//	CAN_FilterInit(&CAN_FilterInitStructure);//过滤器初始化





    //标准帧ID  11位
	//0x711是期望的报文ID
    uint16_t CanFilterListSTID1 = 0x711; //标准ID，此值范围：0x000~0x7FF(0111 1111 1111)
    uint8_t CanFilterListIDE1 = 0x00;//帧类型：0x00标准格式,0x01扩展格式
    uint8_t CanFilterListRTR1 = 0x00;//远程请求标志位：0x00数据帧,0x01远程帧
	//配置过滤器--屏蔽位模式
	CAN_FilterInitStructure.CAN_FilterNumber=0;	  //过滤器0
	CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;   //标识符掩码
	CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit; //32位 
	
	//32位筛选器分高16位与低16位，分两组寄存器ID与掩码  
	//寄存器ID
	//High占高16位 31~16位 
	//CanFilterListSTID1<<5后对应的标准ID才能存储到寄存器的标准ID位
	CAN_FilterInitStructure.CAN_FilterIdHigh=(CanFilterListSTID1<<5);   //0x711是期望的报文ID设置到ID寄存器
	//Low占低16位  15~0位
	CAN_FilterInitStructure.CAN_FilterIdLow=  0x0000; //不使用扩展帧，IDE=0，接收数据帧RTR也=0,
	
	//掩码寄存器
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh= 0xFFFF; //标准帧掩码位：1111 1111 111 匹配ID为你设置的ID 
	CAN_FilterInitStructure.CAN_FilterMaskIdLow =0xFFFD;  //D(1101) -- 标准帧可过  数据帧与遥控可过

	
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO0;//过滤器0关联到FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation=ENABLE; //激活过滤器0
	
	
	CAN_FilterInit(&CAN_FilterInitStructure);//过滤器初始化


#if CAN1_RX0_INT_ENABLE
	
	  CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);//FIFO0消息挂号中断允许.		    
  
  	NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;     // 主优先级为1
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // 次优先级为0
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
#endif
	return 0;
}   
 
#if CAN1_RX0_INT_ENABLE	//使能RX0中断
//中断服务函数			    
void CAN1_RX0_IRQHandler(void)
{
  	CanRxMsg RxMessage;
	int i=0;
    CAN_Receive(CAN1, 0, &RxMessage);
	for(i=0;i<8;i++)
	printf("rxbuf[%d]:%d\r\n",i,RxMessage.Data[i]);
}
#endif

//can发送一组数据(固定格式:ID为0X12,标准帧,数据帧)	
//len:数据长度(最大为8)				     
//msg:数据指针,最大为8个字节.
//返回值:0,成功;
//		 其他,失败;
u8 CAN1_Send_Msg(u8* msg,u8 len)
{	
  u8 mbox;
  u16 i=0;
  CanTxMsg TxMessage;
  TxMessage.StdId=0x18;	 // 标准标识符为0
  TxMessage.ExtId=0x12;	 // 设置扩展标示符（29位）
  TxMessage.IDE=0;		  // 使用标准帧
  TxMessage.RTR=0;		  // 消息类型为数据帧，一帧8位
  TxMessage.DLC=len;							 // 发送两帧信息
	
	
  for(i=0;i<len;i++)
  TxMessage.Data[i]=msg[i];				 // 第一帧信息


	
  mbox= CAN_Transmit(CAN1, &TxMessage);   
  i=0;
  while((CAN_TransmitStatus(CAN1, mbox)==CAN_TxStatus_Failed)&&(i<0XFFF))i++;	//等待发送结束
  if(i>=0XFFF)return 1;
  return 0;		

}
//can口接收数据查询
//buf:数据缓存区;	 
//返回值:0,无数据被收到;
//		 其他,接收的数据长度;
u8 CAN1_Receive_Msg(u8 *buf)
{		   		   
 	u32 i;
	CanRxMsg RxMessage;
    if( CAN_MessagePending(CAN1,CAN_FIFO0)==0)return 0;		//没有接收到数据,直接退出 
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);//读取数据	
    for(i=0;i<RxMessage.DLC;i++)
    buf[i]=RxMessage.Data[i];  
	return RxMessage.DLC;	
}














