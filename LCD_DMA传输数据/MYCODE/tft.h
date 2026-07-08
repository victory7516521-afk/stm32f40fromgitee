#ifndef __TFT_H__
#define __TFT_H__


#include "stm32f4xx.h"
#include "lcd_font.h"

#define LCD_WIDTH 240
#define LCD_HEIGHT 280

/* ����tft����Ҫƫ���� */
#define X_OFFSET	20
#define Y_OFFSET	20

#define LCD_SOFT_SPI_ENABLE  0

#define RED 	0XF800   	// ��ɫ
#define GREEN 	0X07E0 		// ��ɫ
#define BLUE 	0X001F  	// ��ɫ
#define WHITE 	0XFFFF 		// ��ɫ
#define BLACK 	0X0000 		// ��ɫ

#if LCD_SOFT_SPI_ENABLE

#define SPI_CS_0 PEout(13) = 0
#define SPI_CS_1 PEout(13) = 1

#define SPI_SCK_0 PEout(11) = 0
#define SPI_SCK_1 PEout(11) = 1

#define SPI_SDA_0 PEout(9) = 0
#define SPI_SDA_1 PEout(9) = 1

#define LCD_RST_0 PEout(7) = 0
#define LCD_RST_1 PEout(7) = 1

#define LCD_DC_0 PEout(15) = 0
#define LCD_DC_1 PEout(15) = 1

#define LCD_BLK_0 PDout(9) = 0
#define LCD_BLK_1 PDout(9) = 1

#else

//Ƭѡ
#define SPI_CS_0 PGout(6) = 0
#define SPI_CS_1 PGout(6) = 1

//ʱ��
//ʹ�ÿ⺯����ʱ����������Ӳ�����з���

#define SPI_SCK_0 PBout(3) = 0
#define SPI_SCK_1 PBout(3) = 1

//���ݷ��Ͷ�--MOSI
#define SPI_SDA_0 PBout(5) = 0
#define SPI_SDA_1 PBout(5) = 1

//��λ
#define LCD_RST_0 PGout(8) = 0
#define LCD_RST_1 PGout(8) = 1

//����������ѡ��
//LCD_DC_0:����
//LCD_DC_1:����
#define LCD_DC_0 PGout(7) = 0
#define LCD_DC_1 PGout(7) = 1

//����
#define LCD_BLK_0 PBout(4) = 0
#define LCD_BLK_1 PBout(4) = 1

#endif

extern uint32_t g_lcd_width ;
extern uint32_t g_lcd_height;
extern volatile u8 g_dma_tc_flag;  // DMA传输完成标志

extern void lcd_init(void);
extern void lcd_addr_set(uint32_t x_s, uint32_t y_s, uint32_t x_e, uint32_t y_e);
extern void lcd_fill(uint32_t x_s, uint32_t y_s, uint32_t x_len, uint32_t y_len,uint32_t color);
extern void lcd_clear(uint32_t color);
extern void lcd_send_cmd(uint8_t cmd);
extern void lcd_send_data(uint8_t dat);
extern void lcd_draw_point(uint32_t x, uint32_t y, uint32_t color);
extern void lcd_draw_picture(uint32_t x_s, uint32_t y_s, uint32_t width, uint32_t height, const uint8_t *pic);
extern void lcd_draw_picture_dma(uint32_t x_s, uint32_t y_s, uint32_t width, uint32_t height, const uint8_t *pic); // 使用DMA传输图片
extern void lcd_show_chn(uint32_t x, uint32_t y,uint8_t no, uint32_t fc, uint32_t bc,uint32_t font_size);
extern void lcd_show_string(u16 x,u16 y,const u8 *p,u16 fc,u16 bc,u8 font_size,u8 mode);
extern void lcd_show_integer(uint32_t x,uint32_t y,uint32_t num,uint32_t len,uint32_t fc,uint32_t bc,uint32_t font_size);
extern void lcd_show_float(uint32_t x,uint32_t y,float num,uint32_t len,uint32_t fc,uint32_t bc,uint32_t font_size);
extern void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color);
extern void lcd_draw_circle(uint16_t x0,uint16_t y0,uint8_t r,uint16_t color);
extern void lcd_draw_line(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color);
extern void lcd_set_direction(uint32_t dir);
extern uint32_t lcd_get_direction(void);
extern void spi1_tx_dma_init(void);
extern void spi1_tx_dma_start(void);
extern void spi1_tx_dma_stop(void);
extern void spi1_dma_send_buffer(const uint8_t *buf, uint32_t len);
#endif
