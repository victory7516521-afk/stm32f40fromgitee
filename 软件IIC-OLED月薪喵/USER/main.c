#include "stm32f4xx.h"
#include "OLED.h"
#include "delay.h"
#include "OLED_Data.h"

//程序基于江协科技OLED模版程序上编写

//OLED管脚连接:  显示屏GND-单片机GND   显示屏VCC-单片机3.3  显示屏SCL-单片机B8  显示屏SDA-单片机PB9


//程序有两个版本，一个是在中间原地跳舞的，还有一个是我改进过移动跳舞的版本。
//默认这个是移动版本，需要不动版本的在程序文件夹里面的复制之后替换主程序即可。


// 声明外部全局变量：OLED显示缓存数组
// 8页 × 128列，用于存储整屏要显示的点阵数据
//extern uint8_t OLED_DisplayBuf[8][128];

//int main(void)
//{
//	uint8_t frame;                // 定义动画帧变量，用于切换22帧动画
//	int16_t x_offset = -128;	     // 定义X轴偏移量，初始值-128，实现从左向右滚动效果
//	Delay_Init();		             // 初始化延时函数
//	OLED_Init();		             
//	while(1)		                 
//	{
//		// 循环播放22帧动画（frame从0 ~ 21循环）
//		for(frame=0; frame<22; frame++)
//		{
//			OLED_Clear();		     // 清空OLED显示缓存
//			
//			// 遍历OLED 8个页（屏幕纵向分为8页，每页8行像素）
//			for (uint8_t page = 0; page < 8; page++)
//			{
//				// 遍历OLED 128列（屏幕横向128列）
//				for (uint8_t col = 0; col < 128; col++)
//				{
//					// 计算当前列对应的图像原始列坐标
//					// 实现图像整体左右滚动
//					int16_t img_col = col - x_offset;
//					
//					if (img_col >= 0 && img_col < 128)
//					{
//						// 从动画数组中取出对应数据，写入OLED显示缓存
//						OLED_DisplayBuf[page][col] = YueXinMao[frame][page][img_col];
//					}
//				}
//			}
//			OLED_Update();		   
//			delay_ms(5);		//改变这个延时，就可以改变速度     
//			
//			x_offset += 1;		       // X轴偏移量+2，让图像向右移动2列
//			// 边界判断：当偏移量超过128时，重置为-128
//			// 实现循环滚动，无限播放
//			if (x_offset > 128)
//			{
//				x_offset = -128;
//			}
//		}
//	}
//}


//extern const uint8_t YueXinMao[22][8][128];

//int main(void)
//{
//	uint8_t frame = 0;
//	Delay_Init();		             
//	OLED_Init();     

//	while(1)
//	{
//		
//		OLED_ShowImage(0, 0, 128, 64, &YueXinMao[frame][0][0]);
//		OLED_Update();
//		delay_ms(5);   // 动画速度
//		
//		frame++;
//		if(frame >= 22) frame = 0;
//	}
//}


extern uint8_t OLED_DisplayBuf[8][128];
// 字模存Flash，节省RAM
extern const uint8_t YueXinMao[22][8][128];

int main(void)
{
    uint8_t frame;
    int16_t x_offset = -128;
    int8_t dir = 1;    // 1向右，-1向左
    uint8_t step = 2;  // 每次移动步长，越大越快
    
    Delay_Init();
    OLED_Init();
    
    while(1)
    {
        // 循环播放22帧动画
        for(frame = 0; frame < 22; frame++)
        {
            OLED_Clear();
            // 遍历8页
            for (uint8_t page = 0; page < 8; page++)
            {
                // 逐列填充缓存
                for (uint8_t col = 0; col < 128; col++)
                {
                    int16_t img_col = col - x_offset;
                    // 仅有效图像区域赋值，其余保持黑色
                    if (img_col >= 0 && img_col < 128)
                    {
                        OLED_DisplayBuf[page][col] = YueXinMao[frame][page][img_col];
                    }
                }
            }
            OLED_Update();
            delay_ms(5);
            
            // 根据方向更新偏移
            x_offset += dir * step;
            
            // 碰到左右边界，反向
            if(x_offset >= 128)
            {
                dir = -1;
            }
            else if(x_offset <= -128)
            {
                dir = 1;
            }
        }
    }
}