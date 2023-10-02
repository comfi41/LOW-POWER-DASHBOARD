/*********************************************************************************************************
*
* File                : epd.c
* Hardware Environment: 
* Build Environment   : RealView MDK-ARM  Version: 4.74
* Version             : V1.0
* By                  : V
*
*                                  (c) Copyright 2005-2014, WaveShare
*                                       http://www.waveshare.net
*                                          All Rights Reserved
*
*********************************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/uart.h"
#include "epd.h"

/*
command define
*/
//static const unsigned char _cmd_handshake[8] = {0xA5, 0x00, 0x09, CMD_HANDSHAKE, 0xCC, 0x33, 0xC3, 0x3C};               //CMD_HANDSHAKE
//static const unsigned char _cmd_read_baud[8] = {0xA5, 0x00, 0x09, CMD_READ_BAUD, 0xCC, 0x33, 0xC3, 0x3C};               //CMD_READ_BAUD
//static const unsigned char _cmd_stopmode[8] = {0xA5, 0x00, 0x09, CMD_STOPMODE, 0xCC, 0x33, 0xC3, 0x3C};                 //CMD_STOPMODE
static const unsigned char _cmd_update[8] = {0xA5, 0x00, 0x09, CMD_UPDATE, 0xCC, 0x33, 0xC3, 0x3C};                     //CMD_UPDATE
//static const unsigned char _cmd_load_font[8] = {0xA5, 0x00, 0x09, CMD_LOAD_FONT, 0xCC, 0x33, 0xC3, 0x3C};               //CMD_LOAD_FONT
//static const unsigned char _cmd_load_pic[8] = {0xA5, 0x00, 0x09, CMD_LOAD_PIC, 0xCC, 0x33, 0xC3, 0x3C};                 //CMD_LOAD_PIC


static unsigned char _cmd_buff[CMD_SIZE];
#define BUF_SIZE 1024

/*
private function
*/
static void _putchars(const unsigned char * ptr, int n);
static unsigned char _verify(const void * ptr, int n);


void epd_init(void)
{
	uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);

	EINK_WAKE_OFF();
	EINK_RESET_OFF();
}

void epd_wakeup(void)
{
	EINK_WAKE_OFF();
	//EINK_RESET_OFF();
	vTaskDelay(10 / portTICK_PERIOD_MS);
	EINK_WAKE_ON();
	//EINK_RESET_ON();
	vTaskDelay(10 / portTICK_PERIOD_MS);
	EINK_WAKE_OFF();
	//EINK_RESET_OFF();
	vTaskDelay(10 / portTICK_PERIOD_MS);
}

void epd_set_memory(unsigned char mode)
{
	_cmd_buff[0] = FRAME_B;
	
	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0A;	
	
	_cmd_buff[3] = CMD_MEMORYMODE;
	
	_cmd_buff[4] = mode;
	
	_cmd_buff[5] = FRAME_E0;
	_cmd_buff[6] = FRAME_E1;
	_cmd_buff[7] = FRAME_E2;
	_cmd_buff[8] = FRAME_E3;	
	_cmd_buff[9] = _verify(_cmd_buff, 9);
	
	_putchars(_cmd_buff, 10);		
}

static void _putchars(const unsigned char * ptr, int n)
{
	uart_write_bytes(UART_NUM_0, (const char *) ptr, n);
	
}

static unsigned char _verify(const void * ptr, int n)
{
	int i;
	unsigned char * p = (unsigned char *)ptr;
	unsigned char result;
	
	for(i = 0, result = 0; i < n; i++)
	{
		result ^= p[i];
	}
	
	return result;
}

void epd_draw_pixel(int x0, int y0)
{
	_cmd_buff[0] = FRAME_B;
	
	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0D;	
	
	_cmd_buff[3] = CMD_DRAW_PIXEL;	
	
	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;
	
	_cmd_buff[8] = FRAME_E0;
	_cmd_buff[9] = FRAME_E1;
	_cmd_buff[10] = FRAME_E2;
	_cmd_buff[11] = FRAME_E3;	
	_cmd_buff[12] = _verify(_cmd_buff, 12);
	
	_putchars(_cmd_buff, 13);
}

void epd_udpate(void)
{
	memcpy(_cmd_buff, _cmd_update, 8);
	_cmd_buff[8] = _verify(_cmd_buff, 8);
	
	_putchars(_cmd_buff, 9);
}

void epd_clear(void)
{
	_cmd_buff[0] = FRAME_B;
	
	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x09;	
	
	_cmd_buff[3] = CMD_CLEAR;
	
	_cmd_buff[4] = FRAME_E0;
	_cmd_buff[5] = FRAME_E1;
	_cmd_buff[6] = FRAME_E2;
	_cmd_buff[7] = FRAME_E3;	
	_cmd_buff[8] = _verify(_cmd_buff, 8);
	
	_putchars(_cmd_buff, 9);	
}

void epd_draw_line(int x0, int y0, int x1, int y1)
{
	_cmd_buff[0] = FRAME_B;
	
	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x11;	
	
	_cmd_buff[3] = CMD_DRAW_LINE;	
	
	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;
	_cmd_buff[8] = (x1 >> 8) & 0xFF;
	_cmd_buff[9] = x1 & 0xFF;
	_cmd_buff[10] = (y1 >> 8) & 0xFF;
	_cmd_buff[11] = y1 & 0xFF;	
	
	_cmd_buff[12] = FRAME_E0;
	_cmd_buff[13] = FRAME_E1;
	_cmd_buff[14] = FRAME_E2;
	_cmd_buff[15] = FRAME_E3;	
	_cmd_buff[16] = _verify(_cmd_buff, 16);
	
	_putchars(_cmd_buff, 17);	
}

void epd_set_color(unsigned char color, unsigned char bkcolor)
{
	_cmd_buff[0] = FRAME_B;
	
	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0B;
	
	_cmd_buff[3] = CMD_SET_COLOR;
	
	_cmd_buff[4] = color;
	_cmd_buff[5] = bkcolor;
	
	_cmd_buff[6] = FRAME_E0;
	_cmd_buff[7] = FRAME_E1;
	_cmd_buff[8] = FRAME_E2;
	_cmd_buff[9] = FRAME_E3;
	_cmd_buff[10] = _verify(_cmd_buff, 10);
	
	_putchars(_cmd_buff, 11);
}

void epd_fill_rect(int x0, int y0, int x1, int y1)
{
	_cmd_buff[0] = FRAME_B;
	
	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x11;	
	
	_cmd_buff[3] = CMD_FILL_RECT;	
	
	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;
	_cmd_buff[8] = (x1 >> 8) & 0xFF;
	_cmd_buff[9] = x1 & 0xFF;
	_cmd_buff[10] = (y1 >> 8) & 0xFF;
	_cmd_buff[11] = y1 & 0xFF;	
	
	_cmd_buff[12] = FRAME_E0;
	_cmd_buff[13] = FRAME_E1;
	_cmd_buff[14] = FRAME_E2;
	_cmd_buff[15] = FRAME_E3;	
	_cmd_buff[16] = _verify(_cmd_buff, 16);
	
	_putchars(_cmd_buff, 17);		
}

void epd_draw_circle(int x0, int y0, int r)
{
	_cmd_buff[0] = FRAME_B;
	
	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0F;	
	
	_cmd_buff[3] = CMD_DRAW_CIRCLE;	
	
	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;
	_cmd_buff[8] = (r >> 8) & 0xFF;
	_cmd_buff[9] = r & 0xFF;
	
	
	_cmd_buff[10] = FRAME_E0;
	_cmd_buff[11] = FRAME_E1;
	_cmd_buff[12] = FRAME_E2;
	_cmd_buff[13] = FRAME_E3;	
	_cmd_buff[14] = _verify(_cmd_buff, 14);
	
	_putchars(_cmd_buff, 15);
}

void epd_fill_circle(int x0, int y0, int r)
{
	_cmd_buff[0] = FRAME_B;
	
	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0F;	
	
	_cmd_buff[3] = CMD_FILL_CIRCLE;	
	
	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;
	_cmd_buff[8] = (r >> 8) & 0xFF;
	_cmd_buff[9] = r & 0xFF;
	
	
	_cmd_buff[10] = FRAME_E0;
	_cmd_buff[11] = FRAME_E1;
	_cmd_buff[12] = FRAME_E2;
	_cmd_buff[13] = FRAME_E3;	
	_cmd_buff[14] = _verify(_cmd_buff, 14);
	
	_putchars(_cmd_buff, 15);	
}

void epd_draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2)
{
	_cmd_buff[0] = FRAME_B;
	
	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x15;	
	
	_cmd_buff[3] = CMD_DRAW_TRIANGLE;	
	
	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;
	_cmd_buff[8] = (x1 >> 8) & 0xFF;
	_cmd_buff[9] = x1 & 0xFF;
	_cmd_buff[10] = (y1 >> 8) & 0xFF;
	_cmd_buff[11] = y1 & 0xFF;	
	_cmd_buff[12] = (x2 >> 8) & 0xFF;
	_cmd_buff[13] = x2 & 0xFF;
	_cmd_buff[14] = (y2 >> 8) & 0xFF;
	_cmd_buff[15] = y2 & 0xFF;
	
	_cmd_buff[16] = FRAME_E0;
	_cmd_buff[17] = FRAME_E1;
	_cmd_buff[18] = FRAME_E2;
	_cmd_buff[19] = FRAME_E3;	
	_cmd_buff[20] = _verify(_cmd_buff, 20);
	
	_putchars(_cmd_buff, 21);		
}

void epd_set_en_font(unsigned char font)
{
	_cmd_buff[0] = FRAME_B;
	
	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0A;	
	
	_cmd_buff[3] = CMD_SET_EN_FONT;
	
	_cmd_buff[4] = font;
	
	_cmd_buff[5] = FRAME_E0;
	_cmd_buff[6] = FRAME_E1;
	_cmd_buff[7] = FRAME_E2;
	_cmd_buff[8] = FRAME_E3;	
	_cmd_buff[9] = _verify(_cmd_buff, 9);
	
	_putchars(_cmd_buff, 10);	
}

void epd_set_ch_font(unsigned char font)
{
	_cmd_buff[0] = FRAME_B;
	
	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0A;	
	
	_cmd_buff[3] = CMD_SET_CH_FONT;
	
	_cmd_buff[4] = font;
	
	_cmd_buff[5] = FRAME_E0;
	_cmd_buff[6] = FRAME_E1;
	_cmd_buff[7] = FRAME_E2;
	_cmd_buff[8] = FRAME_E3;	
	_cmd_buff[9] = _verify(_cmd_buff, 9);
	
	_putchars(_cmd_buff, 10);
}

void epd_disp_string(const void * p, int x0, int y0)
{
	int string_size;
	unsigned char * ptr = (unsigned char *)p;
	
	
	string_size = strlen((const char *)ptr);
	string_size += 14;
	
	_cmd_buff[0] = FRAME_B;
	
	_cmd_buff[1] = (string_size >> 8) & 0xFF;
	_cmd_buff[2] = string_size & 0xFF;
	
	_cmd_buff[3] = CMD_DRAW_STRING;
	
	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;
	
	strcpy((char *)(&_cmd_buff[8]), (const char *)ptr);
	
	string_size -= 5;
	
	_cmd_buff[string_size] = FRAME_E0;
	_cmd_buff[string_size + 1] = FRAME_E1;
	_cmd_buff[string_size + 2] = FRAME_E2;
	_cmd_buff[string_size + 3] = FRAME_E3;
	_cmd_buff[string_size + 4] = _verify(_cmd_buff, string_size + 4);
	
	_putchars(_cmd_buff, string_size + 5);
}