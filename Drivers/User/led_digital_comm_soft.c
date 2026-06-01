#include "led_digital_comm.h"
#include "main.h"

//#define HC138_A0_GPIO_Port HC138_A0_GPIO_Port
//#define HC138_A1_GPIO_Port HC138_A0_GPIO_Port
//#define HC138_A2_GPIO_Port HC138_A2_GPIO_Port
//#define HC138_A0_Pin HC138_A0_Pin
//#define HC138_A1_Pin HC138_A1_Pin
//#define HC138_A2_Pin HC138_A2_Pin
//#define HC595_SCLK_GPIO_Port HC595_SCLK_GPIO_Port
//#define HC595_SCLK_Pin HC595_SCLK_Pin
//#define HC595_LCLK_GPIO_Port HC595_LCLK_GPIO_Port
//#define HC595_LCLK_Pin HC595_LCLK_Pin
//#define HC595_DATA_GPIO_Port HC595_DATA_GPIO_Port
//#define HC595_DATA_Pin HC595_DATA_Pin

static void delay_us(uint32_t us)
{
	uint32_t us_cnt = (SysTick->LOAD + 1) / 1000;
	int32_t total_cnt = us_cnt * us;
	uint32_t tick_prev = SysTick->VAL;
	uint32_t tick_curr = SysTick->VAL;
	uint16_t tick_diff = 0;
	
	while (total_cnt > 0) {
		tick_curr = SysTick->VAL;
		if (tick_prev > tick_curr) {
			tick_diff = tick_prev - tick_curr;
		} else {
			tick_diff = tick_prev + (SysTick->LOAD + 1) - tick_curr;
		}
		total_cnt -= tick_diff;
		tick_prev = tick_curr;
	}
}

/**
74HC138
A0 PC10
A1 PC11
A2 PC12
**/

void led_digital_select(uint32_t pos)
{
	pos &= 0x07U;
	HAL_GPIO_WritePin(HC138_A0_GPIO_Port, HC138_A0_Pin, pos & 0x01U);
	HAL_GPIO_WritePin(HC138_A1_GPIO_Port, HC138_A1_Pin, (pos >> 0x01U) & 0x01U);
	HAL_GPIO_WritePin(HC138_A2_GPIO_Port, HC138_A2_Pin, (pos >> 0x02U) & 0x01U);
	return;
}

void led_digital_set(uint32_t num)
{
	HAL_GPIO_WritePin(HC595_SCLK_GPIO_Port, HC595_SCLK_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(HC595_LCLK_GPIO_Port, HC595_LCLK_Pin, GPIO_PIN_RESET);
	for (uint32_t i = 0; i < 8; i++) {
		HAL_GPIO_WritePin(HC595_SCLK_GPIO_Port, HC595_SCLK_Pin, GPIO_PIN_RESET);
		delay_us(5);
		HAL_GPIO_WritePin(HC595_DATA_GPIO_Port, HC595_DATA_Pin, (num & 0x80U) >> 7);
		num <<= 1;
		HAL_GPIO_WritePin(HC595_SCLK_GPIO_Port, HC595_SCLK_Pin, GPIO_PIN_SET);
		delay_us(5);
	}
	return;
}

void led_digital_latch_out()
{
	HAL_GPIO_WritePin(HC595_LCLK_GPIO_Port, HC595_LCLK_Pin, GPIO_PIN_SET);
	delay_us(5);
	HAL_GPIO_WritePin(HC595_LCLK_GPIO_Port, HC595_LCLK_Pin, GPIO_PIN_RESET);
	return;
}