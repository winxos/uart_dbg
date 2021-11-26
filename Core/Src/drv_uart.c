/*
 * drv_uart.c
 *
 *  Created on: Nov 21, 2021
 *      Author: wvv
 */
#include <usart.h>
#include <tim.h>

#define UART_MAX_COUNT 2
#define RX_BUF_SIZE 80
enum
{
	S_UART_IDLE, S_UART_BUSY, S_UART_READY,
};
struct UartInfo
{
	UART_HandleTypeDef *huart;
	uint32_t timeout;
	uint8_t buf;
	uint8_t state;
	uint8_t len;
	uint8_t rxbuf[RX_BUF_SIZE];
};

static struct UartInfo _uarts[] =
{
{ .huart = &huart1 },
{ .huart = &huart2 } };

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	for (uint8_t id = 0; id < UART_MAX_COUNT; id++)
	{
		if (huart == _uarts[id].huart)
		{
			_uarts[id].rxbuf[_uarts[id].len++] = _uarts[id].buf;
			_uarts[id].state = S_UART_BUSY;
			_uarts[id].timeout = 0;
			HAL_UART_Receive_IT(huart, &_uarts[id].buf, 1);
		}
	}
}
void oled_input_ch(char ch);
void oled_input(char *ch);
void drv_uart_idle()
{
	if (_uarts[0].state == S_UART_BUSY)
	{
		_uarts[0].timeout++;
		if (_uarts[0].timeout >= 3)
		{
			for (uint8_t i = 0; i < _uarts[0].len; i++)
			{
				oled_input_ch(_uarts[0].rxbuf[i]);
			}
			HAL_UART_Transmit(&huart2, _uarts[0].rxbuf, _uarts[0].len, 0xffff);  //transparent to uart2
			_uarts[0].state = S_UART_IDLE;
			_uarts[0].timeout = 0;
			_uarts[0].len = 0;
		}
	}
	if (_uarts[1].state == S_UART_BUSY)
	{
		_uarts[1].timeout++;
		if (_uarts[1].timeout >= 3)
		{
			HAL_UART_Transmit(&huart1, _uarts[1].rxbuf, _uarts[1].len, 0xffff);  //transparent to uart1
			_uarts[1].state = S_UART_IDLE;
			_uarts[1].timeout = 0;
			_uarts[1].len = 0;
		}
	}
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	drv_uart_idle();
}
void drv_uart_init()
{
	for (uint8_t id = 0; id < UART_MAX_COUNT; id++)
	{
		HAL_UART_Receive_IT(_uarts[id].huart, &_uarts[id].buf, 1);
	}
	HAL_TIM_Base_Start_IT(&htim14);
}

