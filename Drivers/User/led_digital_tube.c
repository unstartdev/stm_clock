#include "led_digital_tube.h"
#include "led_digital_comm.h"
#include "main.h"
#include "tim.h"
#include "utils.h"
#include "system_status.h"

static const uint8_t postive_led_digital_code_map[] = {0xC0U, 0xF9U, 0xA4U, 0xB0U, 0x99U, 0x92U, 0x82U, 0xF8U, 0x80U, 0x90U, 0x88U, 0x83U, 0xC6U, 0xA1U, 0x86U, 0x8EU,0x8CU, 0xC1U, 0x91U, 0x00U, 0xFFU, 0x48};
static const uint8_t negative_led_digital_code_map[] = 
	{0x3FU, 0x06U, 0x5BU, 0x4FU,
	0x66U, 0x6DU, 0x7DU, 0x07U,
	0x7FU, 0x6FU, 0x77U, 0x7CU,
	0x39U, 0x5EU, 0x79U, 0x71U,
	0x73U, 0x3EU, 0x31U, 0x6EU,
	0xFFU, 0x00U, 0x48};
	
#define DIV_CHAR 0x48
#define MINUS 0x40
#define OFF 0x00
#define POINT 0x80
	
#define LED_BIT 8
#define LED_ROW_HOLD_TIME 500

static __ALIGNED(4) uint8_t num[LAST_ROW][LED_BIT] = {
	{0},
	{0},
	{0x00, 0x3F, 0x3F, 0x39, 0x00, 0x3F, 0x3F, 0x3F},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x3F|0x80, 0x3F, 0x3F},
	{0x00, 0x00, 0x00, 0x3F, 0x3F, DIV_CHAR, 0x3F, 0x3F},
	{0},
	{0x00, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x3F, 0x7F},
	{0},
	{0x00, 0x00, 0x00, 0x3F, 0x3F, DIV_CHAR, 0x3F, 0x3F},
	{0},
};

__STATIC_INLINE uint8_t bcd_get_bit(unsigned int bcd, unsigned int loc)
{
	return (bcd >> (loc << 2)) & 0x000F; 
}

// n range from 0 to 9999
void led_digital_set_num(uint32_t row, uint32_t start, uint32_t len, uint32_t n)
{
	n = dec_to_bcd(n);
	__disable_irq();
	for (uint32_t i = 0; i < len; i++) {
		num[row][start + i] = negative_led_digital_code_map[bcd_get_bit(n, len - 1 - i)];
	}
	__enable_irq();
}

void led_digital_set_signed_num(uint32_t row, uint32_t start, uint32_t len, int32_t n)
{
	if (n < 0) {
		num[row][start] = MINUS;
		n = -n;
	} else {
		num[row][start] = 0;
	}
	led_digital_set_num(row, start + 1, len, n);
}

void led_digital_set_num_nopzero(uint32_t row, uint32_t start, uint32_t len, uint32_t n, uint32_t keep)
{
	n = dec_to_bcd(n);
	uint32_t flag_nz = 0;
	__disable_irq();
	for (uint32_t i = 0; i < len; i++) {
		uint8_t b = bcd_get_bit(n, len - 1 - i);
		if (b != 0)
			flag_nz = 1;
		num[row][start + i] = (flag_nz || i > keep - 1) ? negative_led_digital_code_map[b] : 0x00;
	}
	__enable_irq();
}

void led_digital_set_bit(uint32_t row, uint32_t pos, uint8_t n)
{
	__disable_irq();
	num[row][pos] = n;
	__enable_irq();
}

void led_digital_set_div(uint32_t row, uint32_t pos) {
	led_digital_set_bit(row, pos, DIV_CHAR);
}

struct led_refresh_state {
	uint32_t update_tick;
	uint32_t row;
	uint32_t row_hold_counter;
	uint32_t pos;
	uint32_t row_rolling_end;
	uint32_t row_blinking_pattern[LAST_ROW];
};

static __ALIGNED(4) const uint32_t fix_point_pos[LAST_ROW] = {0xFF, 0xFF, 5, 5, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static __ALIGNED(4) const uint32_t blinking_pattern[LAST_ROW][3][LED_BIT] = {
	{0},
	{0},
	{0},
	{0},
	{0},
	{
		{3, 3, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 3, 3, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 3, 3},
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0},
		{0},
	},
	{
		{3, 3, 3, 3, 0, 0, 0, 0},
		{0, 0, 0, 0, 3, 3, 0, 0},
		{0, 0, 0, 0, 0, 0, 3, 3},
	},
	{
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 1, 1},
		{0},
	},
	{0},
};

__ALIGNED(4) static volatile struct led_refresh_state state = {
	.update_tick = 0,
	.row = 0,
	.row_hold_counter = 0,
	.pos = 0,
	.row_rolling_end = ENV_ROW,
	.row_blinking_pattern = {0},
};

static void row_change_after_effect()
{
	state.pos = 0;
	state.row_hold_counter = 0;
	if (state.row == ALARM_ROW && system_status.state != SYSTEM_ALARM) {
		state.row++;
	}
	switch (state.row) {
	case STOP_WATCH_ROW:
		system_status.state = SYSTEM_STOP_WATCH;
		break;
	case ALARM_ROW:
		break;
	case TIME_SETTING_ROW:
		system_status.state = SYSTEM_TIME_SETTING;
		time_setting.cur = TIME_CUR_HOUR;
		break;
	case TIMEZONE_SETTING_ROW:
		system_status.state = SYSTEM_TIMEZONE_SETTING;
		break;
	case DATE_SETTING_ROW:
		system_status.state = SYSTEM_DATE_SETTING;
		date_setting.cur = DATE_CUR_YEAR;
		break;
	case ALARM_SETTING_ROW:
		system_status.state = SYSTEM_ALARM_SETTING;
		load_alarm_setting();
		break;
	case SE_SETTING_ROW:
		system_status.state = SYSTEM_SOUND_EFFECT_SETTING;
		load_se_setting();
		break;
	default:
		system_status.state = SYSTEM_FREE;
		break;
	}
}

void led_digital_next_row() {
	if (state.row < LAST_ROW - 1) {
		state.row++;
	} else {
		system_status.state = SYSTEM_FREE;
		state.row = 0;
	}
	row_change_after_effect();
}

void led_digital_row_sel(enum led_row_names row) {
	state.row = row;
	row_change_after_effect();
}

void led_digital_set_point(uint32_t row, uint32_t pos)
{
	num[row][pos] |= POINT;
}

void led_digital_set_bp(uint8_t bp)
{
	state.row_blinking_pattern[state.row] = bp;
}

void led_digital_clear_point(uint32_t row, uint32_t pos)
{
	num[row][pos] &= 0x7F;
}

#define UPDATE_TICK_MAX 20

void led_refresh()
{
	uint32_t bp = blinking_pattern[state.row][state.row_blinking_pattern[state.row]][state.pos];
	if (bp == 0 || state.update_tick == 0) {
		led_digital_set(state.pos == fix_point_pos[state.row] ? num[state.row][state.pos] | 0x80U : num[state.row][state.pos]);
		led_digital_select(state.pos);
		led_digital_latch_out();
	}
	state.pos++;
	if (state.pos == LED_BIT) {
		state.pos = 0x00;
		state.update_tick++;
		if (state.update_tick == UPDATE_TICK_MAX) {
			state.update_tick = 0;
	}
		if (state.row < state.row_rolling_end + 1)
			state.row_hold_counter++;
	}
	if (state.row_hold_counter == LED_ROW_HOLD_TIME) {
		state.row_hold_counter = 0;
		state.row++;
		if (state.row == state.row_rolling_end + 1)
			state.row = 0;
	}
}

void led_refresh_init()
{
	HAL_TIM_Base_Start_IT(&refreshClock);
}