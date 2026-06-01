#ifndef __LED_DIGITAL_TUBE_H
#define __LED_DIGITAL_TUBE_H
#include <stdint.h>

enum led_row_names {
	TIME_ROW = 0,
	DATE_ROW,
	ENV_ROW,
	STOP_WATCH_ROW,
	ALARM_ROW,
	TIME_SETTING_ROW,
	TIMEZONE_SETTING_ROW,
	DATE_SETTING_ROW,
	ALARM_SETTING_ROW,
	SE_SETTING_ROW,
	LAST_ROW,
};

void led_digital_set_num(uint32_t row, uint32_t start, uint32_t len, uint32_t n);
void led_digital_set_signed_num(uint32_t row, uint32_t start, uint32_t len, int32_t n);
void led_digital_set_bit(uint32_t row, uint32_t pos, uint8_t n);
void led_digital_set_div(uint32_t row, uint32_t pos);
void led_digital_next_row();
void led_digital_set_point(uint32_t row, uint32_t pos);
void led_digital_clear_point(uint32_t row, uint32_t pos);
void led_digital_set_bp(uint8_t bp);
void led_digital_row_sel(enum led_row_names row);
void led_digital_set_num_nopzero(uint32_t row, uint32_t start, uint32_t len, uint32_t n, uint32_t keep);
void led_refresh();
void led_refresh_init();
#endif