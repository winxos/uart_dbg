/*
 * oled.c
 *
 *  Created on: Nov 17, 2021
 *      Author: wvv
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <main.h>
#include <i2c.h>
#include <usart.h>
extern const uint8_t f5x8[5];
void oled_write_cmd(uint8_t cmd)
{
	HAL_I2C_Mem_Write(&hi2c2, 0x78, 0x00, I2C_MEMADD_SIZE_8BIT, &cmd, 1, 0x100);
}
#define WIDTH 128
#define HEIGHT 4
#define LINE_MAX_CHARS 21
#define PIXEL_MODE_PAINT 	0
#define PIXEL_MODE_CLEAR	1
uint8_t _buf[HEIGHT * WIDTH] =
{ 0 };
uint8_t loc_x, loc_y;
void oled_clear(void)
{
	memset(_buf, 0x00, sizeof(_buf));
	loc_x = 0;
	loc_y = 0;
}
void oled_pixel(uint8_t x, uint8_t y, uint8_t mode)
{
	if (x >= WIDTH || y >= HEIGHT * 8)
	{
		return;
	}
	if (mode == PIXEL_MODE_PAINT)
	{
		_buf[(y >> 3) * WIDTH + x] |= 1 << (y & 0x07);
	}
	else
	{
		_buf[(y >> 3) * WIDTH + x] &= ~(1 << (y & 0x07));
	}

}
void oled_putchar(char c, uint8_t x, uint8_t y)
{
	if (x >= WIDTH || y >= HEIGHT)
	{
		return;
	}
	const uint8_t *p = &f5x8[(c - ' ') * 5];
	for (int i = x; (i < x + 5) && i < WIDTH; i++)
	{
		_buf[y * WIDTH + i] = *p;
		p++;
	}
}
void oled_str(char *str, uint8_t x, uint8_t y)
{
	int offset = 0;
	while (*str)
	{
		oled_putchar(*str, x + offset, y);
		offset += 6;
		str++;
	}
}
void oled_line_shift()
{
	for (int i = 0; i < HEIGHT - 1; i++)
	{
		memcpy(&_buf[i * WIDTH], &_buf[(i + 1) * WIDTH], WIDTH);
	}
	memset(&_buf[(HEIGHT - 1) * WIDTH], 0, WIDTH);
}
void oled_input_ch(char ch)
{
	if (loc_x >= LINE_MAX_CHARS)
	{
		loc_y++;
		loc_x = 0;
	}
	if (loc_y >= HEIGHT)
	{
		oled_line_shift();
		loc_y = HEIGHT - 1;
	}
	if (ch >= ' ' && ch <= '~')
	{
		oled_putchar(ch, loc_x * 6, loc_y);
		loc_x++;
	}
	else if (ch == '\n')
	{
		loc_x = 0;
		loc_y++;
	}
	else
	{
		oled_putchar(127, loc_x * 6, loc_y);
		loc_x++;
	}
	ch++;
}
void oled_input(char *ch)
{
	while (*ch)
	{
		oled_input_ch(*ch);
		ch++;
	}
}
void oled_refresh(void)
{
	for (uint8_t i = 0; i < HEIGHT; i++)
	{
		oled_write_cmd(0xb0 + i);
		oled_write_cmd(0x00);
		oled_write_cmd(0x10);
		HAL_StatusTypeDef ret = HAL_I2C_Mem_Write(&hi2c2, 0x78, 0x40, 1,
				(uint8_t*) &_buf[i * WIDTH], WIDTH, 100);
		if (ret != HAL_OK)
		{
			HAL_I2C_DeInit(&hi2c2);
			HAL_I2C_Init(&hi2c2);
		}
	}
}
void oled_init(void)
{
	oled_write_cmd(0xAE);
	oled_write_cmd(0x20);
	oled_write_cmd(0x10);

	oled_write_cmd(0xB0);
	oled_write_cmd(0xC0); //com scan dir, c8 rotate 180
	oled_write_cmd(0x00);
	oled_write_cmd(0x10);
	oled_write_cmd(0x40);

	oled_write_cmd(0x81);
	oled_write_cmd(0xff);

	oled_write_cmd(0xa0); //remap, a1 rotate 180
	oled_write_cmd(0xa6);

	oled_write_cmd(0xa8);
	oled_write_cmd(0x1f);

	oled_write_cmd(0xd3);
	oled_write_cmd(0x00);

	oled_write_cmd(0xd5);
	oled_write_cmd(0xf0);

	oled_write_cmd(0xd9);
	oled_write_cmd(0x22);

	oled_write_cmd(0xda);
	oled_write_cmd(0x02);

	oled_write_cmd(0xdb);
	oled_write_cmd(0x20);

	oled_write_cmd(0x8d);
	oled_write_cmd(0x14);

	oled_write_cmd(0xaf);
	HAL_Delay(100);
	oled_clear();
	oled_refresh();
}
void logo()
{
	for (int i = 0; i < 200; i++)
	{
		oled_putchar((rand() % 60 + '0'), rand() % 21 * 6, rand() % 4);
		oled_refresh();
	}
	for (int i = 0; i < 21; i++)
	{
		oled_putchar(128, i * 6, 0);
		oled_putchar(128, i * 6, 1);
		oled_putchar(128, i * 6, 2);
		oled_putchar(128, i * 6, 3);
		oled_refresh();
		HAL_Delay(50);
	}
	oled_clear();
}
void drv_uart_init();
void system_run()
{
	HAL_Delay(1000); //wait for oled power on
	drv_uart_init();
	oled_init();
	logo();
	oled_input("SERIAL DEBUGGER\n");
	oled_input("        wvv 20211125\n");
	oled_input("SUPPORT TTL/RS485\n");
	oled_input("PIN   +5v - tx rx a b");
	while (1)
	{
		oled_refresh();
		HAL_Delay(50);
	}
}

