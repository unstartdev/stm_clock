#include "music.h"
#include <stdint.h>
#include "main.h"
#include "tim.h"

TIM_HandleTypeDef unused;

#define PWM unused
#define CHANNEL TIM_CHANNEL_3

struct tone_state {
	uint32_t duration;
	uint32_t tick;
	uint32_t en;
};

static volatile struct tone_state state;

void tone(uint32_t freq)
{
	uint16_t period = 1.0f / freq * 10000 - 1;
	__HAL_TIM_SET_AUTORELOAD(&PWM, period);
	__HAL_TIM_SET_COMPARE(&PWM, CHANNEL, period / 2);
	HAL_TIM_PWM_Start(&PWM, CHANNEL);
}

void tone_clear()
{
	HAL_TIM_PWM_Stop(&PWM, CHANNEL);
	__HAL_TIM_SET_COUNTER(&PWM, 0);
}

void tone_irq()
{
	if (state.en != 1)
		return;
	if (state.tick < state.duration) {
		state.tick++;
	} else {
		state.tick = 0;
		state.en = 0;
		tone_clear();
	}
}

void tone_it(uint32_t duration, uint32_t freq)
{
	if (state.en)
		return;
	tone(freq);
	state.duration = duration;
	state.en = 1;
}	