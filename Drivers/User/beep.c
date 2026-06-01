#include "beep.h"
#include "main.h"
#include "gpio.h"
#include <stdint.h>

struct beep_state {
	uint32_t duration;
	uint32_t tick;
	uint32_t en;
};

static volatile struct beep_state state = {0};

void beep(uint32_t duration)
{
	if (state.en)
		return;
	state.en = 1;
	state.duration = duration;
	HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);
}

void beep_irq()
{
	if (state.en == 0)
		return;
	if (state.tick > state.duration) {
		HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);
		state.en = 0;
		state.tick = 0;
		return;
	}
	state.tick++;
}

void beep_set()
{
	HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);
}

void beep_clear()
{
	HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);
}

static volatile struct beep_state alarm_state = {
	.duration = 500,
};

void beep_alarm_set()
{
	alarm_state.en = 1;
}

void beep_alarm_clear()
{
	__disable_irq();
	alarm_state.en = 0;
	__enable_irq();
	beep_clear();
}

void beep_alarm_irq()
{
	if (alarm_state.en == 0)
		return;
	if (alarm_state.tick > alarm_state.duration) {
		HAL_GPIO_TogglePin(BEEP_GPIO_Port, BEEP_Pin);
		alarm_state.tick = 0;
		return;
	}
	alarm_state.tick++;
}