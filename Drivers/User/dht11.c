#include "dht11.h"
#include "main.h"
#include <stdint.h>
#include "gpio.h"

#define DQ_Port DHT11_DQ_GPIO_Port
#define DQ_Pin DHT11_DQ_Pin

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

static void dht11_dq_low()
{
	HAL_GPIO_WritePin(DQ_Port, DQ_Pin, GPIO_PIN_RESET);
}

static void dht11_dq_high()
{
	HAL_GPIO_WritePin(DQ_Port, DQ_Pin, GPIO_PIN_SET);
}

static uint8_t dht11_dq_read()
{
	return HAL_GPIO_ReadPin(DQ_Port, DQ_Pin) == GPIO_PIN_SET ? 1 : 0;
}

static uint8_t dht11_receive()
{
	volatile uint8_t t = 0;
	volatile uint8_t bit = 0;
	uint32_t start = 0;
	for (uint8_t i = 0; i < 8; i++) {
		while (dht11_dq_read() == 1) {
			delay_us(1);
		}
		while (dht11_dq_read() == 0) {
			delay_us(1);
		}
		delay_us(50);
		bit = dht11_dq_read();
		if (bit == 1) {
			while (dht11_dq_read() == 1) {
				delay_us(1);
			}
		}
		t <<= 1;
		t |= bit;
	}
	return t;
}

void dht11_get_data(volatile int8_t *temp, volatile uint8_t *humidity)
{
	dht11_dq_low();
	HAL_Delay(20);
	dht11_dq_high();
	uint32_t start = HAL_GetTick();
	while (dht11_dq_read() == 1 && HAL_GetTick() - start < 200) {
		delay_us(1);
	}
	if (HAL_GetTick() - start > 199)
		return;
	while (dht11_dq_read() == 0) {
		delay_us(1);
	}
	uint8_t verify = 0;
	uint8_t th = 0;
	uint8_t tl = 0;
	uint8_t hh = 0;
	uint8_t hl = 0;
	hh = dht11_receive();
	hl = dht11_receive();
	th = dht11_receive();
	tl = dht11_receive();
	verify = dht11_receive();
	if (verify != th + tl + hh + hl)
		return;
	*humidity = hh;
	if ((tl & 0x80) == 0x80) {
		*temp = -th;
	} else {
		*temp = th;
	}
}