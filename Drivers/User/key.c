#include "key.h"
#include "system_status.h"
#include "main.h"
#include "gpio.h"
#include "led_digital_tube.h"
#include "beep.h"
#include "utils.h"

__ALIGNED(4) static volatile uint32_t is_key_triggered[4] = {0};

void key_irq(uint16_t pins)
{
	if ((pins & KEY_UP_Pin) == KEY_UP_Pin) {
		if (is_key_triggered[3] != 1)
			is_key_triggered[3] = HAL_GetTick();
	}
	if ((pins & KEY0_Pin) == KEY0_Pin) {
		if (is_key_triggered[0] != 1)
			is_key_triggered[0] = HAL_GetTick();
	}
	if ((pins & KEY1_Pin) == KEY1_Pin) {
		if (is_key_triggered[1] != 1)
			is_key_triggered[1] = HAL_GetTick();
	}
	if ((pins & KEY2_Pin) == KEY2_Pin) {
		if (is_key_triggered[2] != 1)
			is_key_triggered[2] = HAL_GetTick();
	}
}

void keyup_func()
{
	if (system_status.state != SYSTEM_ALARM)
		led_digital_next_row();
}

static void key0_time_setting()
{
	switch (time_setting.cur) {
	case TIME_CUR_HOUR:
		if (time_setting.tm.tm_hour == 0) {
			time_setting.tm.tm_hour = 23;
			break;
		}
		time_setting.tm.tm_hour--;
		break;
	case TIME_CUR_MIN:
		if (time_setting.tm.tm_min == 0) {
			time_setting.tm.tm_min = 59;
			break;
		}
		time_setting.tm.tm_min--;
		break;
	case TIME_CUR_SEC:
		if (time_setting.tm.tm_sec == 0) {
			time_setting.tm.tm_sec = 59;
			break;
		}
		time_setting.tm.tm_sec--;
	}
	save_setting((struct tm *)&time_setting.tm);
}

static const uint8_t mon[] = {0, // None
				0, //Jan
				0, // Feb
				1, // March
				0, // April
				1, // May
				0, // Jun
				1, // Jul
				1, // Aug,
				0, // Sept
				1, // Oct
				0, // Nov
				1 // Dec
};

static int is_sp_year(int year)
{
	if (year % 4 == 0 && year % 100 != 0)
		return 1;
	if (year % 400 == 0)
		return 1;
	return 0;
}

static void key0_date_setting()
{
	switch (date_setting.cur) {
	case DATE_CUR_YEAR:
		if (date_setting.tm.tm_year == 1970) {
			date_setting.tm.tm_year = 9999;
			break;
		}
		date_setting.tm.tm_year--;
		break;
	case DATE_CUR_MON:
		if (date_setting.tm.tm_mon == 1) {
			date_setting.tm.tm_mon = 12;
			break;
		}
		date_setting.tm.tm_mon--;
		break;
	case DATE_CUR_DAY:
		if (date_setting.tm.tm_mday == 1) {
			if (mon[date_setting.tm.tm_mon]) {
				date_setting.tm.tm_mday = 31;
			} else if (date_setting.tm.tm_mon == 2) {
				if (is_sp_year(date_setting.tm.tm_year)) {
					date_setting.tm.tm_mday = 29;
				} else {
					date_setting.tm.tm_mday = 28;
				}
			} else {
				date_setting.tm.tm_mday = 30;
			}
			break;
		}
		date_setting.tm.tm_mday--;
	}
	save_setting((struct tm *)&date_setting.tm);
}

static void key0_timezone_setting()
{
	if (system_status.tzoffset > -12) {
		system_status.tzoffset--;
		load_timezone_offset_setting();
	}
}

static void key0_alarm_setting()
{
	switch (alarm_setting.cur) {
	case ALARM_CUR_HOUR:
		if (system_status.alarm_h == 0) {
			system_status.alarm_h = 23;
			break;
		}
		system_status.alarm_h--;
		break;
	case ALARM_CUR_MIN:
		if (system_status.alarm_min == 0) {
			system_status.alarm_min = 59;
			break;
		}
		system_status.alarm_min--;
		break;
	case ALARM_CUR_EN:
		system_status.alarm_en = system_status.alarm_en ? 0 : 1;
		break;
	}
	load_alarm_setting();
}

void key0_func()
{
	switch (system_status.state) {
	case SYSTEM_STOP_WATCH:
		// STOP_WATCH RST
		stop_watch.en = 0;
		system_status.sport_tick = 0;
		led_digital_set_num_nopzero(STOP_WATCH_ROW, 0, 8, 0, 5);
		break;
	case SYSTEM_TIME_SETTING:
		// Decrease
		key0_time_setting();
		break;
	case SYSTEM_DATE_SETTING:
		key0_date_setting();
		break;
	case SYSTEM_TIMEZONE_SETTING:
		key0_timezone_setting();
		break;
	case SYSTEM_ALARM_SETTING:
		key0_alarm_setting();
		break;
	}
}

static void key1_time_setting()
{
	switch (time_setting.cur) {
	case TIME_CUR_HOUR:
		time_setting.cur = TIME_CUR_MIN;
		break;
	case TIME_CUR_MIN:
		time_setting.cur = TIME_CUR_SEC;
		break;
	case TIME_CUR_SEC:
		time_setting.cur = TIME_CUR_HOUR;
	}	
}

static void key1_date_setting()
{
	switch (date_setting.cur) {
	case DATE_CUR_YEAR:
		date_setting.cur = DATE_CUR_MON;
		break;
	case DATE_CUR_MON:
		date_setting.cur = DATE_CUR_DAY;
		break;
	case DATE_CUR_DAY:
		date_setting.cur = DATE_CUR_YEAR;
		break;
	}
}		

static void key1_alarm_setting()
{
	switch (alarm_setting.cur) {
	case ALARM_CUR_HOUR:
		alarm_setting.cur = ALARM_CUR_MIN;
		break;
	case ALARM_CUR_MIN:
		alarm_setting.cur = ALARM_CUR_EN;
		break;
	case ALARM_CUR_EN:
		alarm_setting.cur = ALARM_CUR_HOUR;
		break;
	}
	load_alarm_setting();
}

static void key1_se_setting()
{
	if (system_status.se_en) {
		system_status.se_en = 0;
	} else {
		system_status.se_en = 1;
	}
	load_se_setting();
}

static void key1_alarm()
{
	release_alarm();
}

void key1_func()
{
	switch (system_status.state) {
	case SYSTEM_STOP_WATCH:
		// STOP_WATCH EN / DISEN
		if (stop_watch.en) {
			stop_watch.en = 0;
		} else {
			stop_watch.en = 1;
		}
		break;
	case SYSTEM_TIME_SETTING:
		// cur move
		key1_time_setting();
		break;
	case SYSTEM_DATE_SETTING:
		key1_date_setting();
		break;
	case SYSTEM_ALARM_SETTING:
		key1_alarm_setting();
		break;
	case SYSTEM_SOUND_EFFECT_SETTING:
		key1_se_setting();
		break;
	case SYSTEM_ALARM:
		key1_alarm();
		break;
	}
}

static void key2_time_setting()
{
	switch (time_setting.cur) {
	case TIME_CUR_HOUR:
		if (time_setting.tm.tm_hour == 23) {
			time_setting.tm.tm_hour = 0;
			break;
		}
		time_setting.tm.tm_hour++;
		break;
	case TIME_CUR_MIN:
		if (time_setting.tm.tm_min == 59) {
			time_setting.tm.tm_min = 0;
			break;
		}
		time_setting.tm.tm_min++;
		break;
	case TIME_CUR_SEC:
		if (time_setting.tm.tm_sec == 59) {
			time_setting.tm.tm_sec = 0;
			break;
		}
		time_setting.tm.tm_sec++;
	}
	save_setting((struct tm *)&time_setting.tm);
}

static void key2_date_setting()
{
	uint8_t mday_max;
	switch (date_setting.cur) {
	case DATE_CUR_YEAR:
		if (date_setting.tm.tm_year == 9999) {
			date_setting.tm.tm_year = 1970;
			break;
		}
		date_setting.tm.tm_year++;
		break;
	case DATE_CUR_MON:
		if (date_setting.tm.tm_mon == 12) {
			date_setting.tm.tm_mon = 1;
			break;
		}
		date_setting.tm.tm_mon++;
		break;
	case DATE_CUR_DAY:
		if (mon[date_setting.tm.tm_mon]) {
			mday_max = 31;
		} else if (date_setting.tm.tm_mon == 2) {
			if (is_sp_year(date_setting.tm.tm_year)) {
				mday_max = 29;
			} else {
				mday_max = 28;
			}
		} else {
			mday_max = 30;
		}
		if (date_setting.tm.tm_mday == mday_max) {
			date_setting.tm.tm_mday = 1;
			break;
		}
		date_setting.tm.tm_mday++;
		break;
	}
	save_setting((struct tm *)&date_setting.tm);
}

static void key2_timezone_setting()
{
	if (system_status.tzoffset < 12) {
		system_status.tzoffset++;
		load_timezone_offset_setting();
	}
}

static void key2_alarm_setting()
{
	switch (alarm_setting.cur) {
	case ALARM_CUR_HOUR:
		if (system_status.alarm_h == 23) {
			system_status.alarm_h = 0;
			break;
		}
		system_status.alarm_h++;
		break;
	case ALARM_CUR_MIN:
		if (system_status.alarm_min == 59) {
			system_status.alarm_min = 0;
			break;
		}
		system_status.alarm_min++;
		break;
	}
	load_alarm_setting();
}

static void key2_alarm()
{
	suppress_alarm();
}

void key2_func()
{
	switch (system_status.state) {
	case SYSTEM_TIME_SETTING:
		// Increase
		key2_time_setting();
		break;
	case SYSTEM_DATE_SETTING:
		key2_date_setting();
		break;
	case SYSTEM_TIMEZONE_SETTING:
		key2_timezone_setting();
		break;
	case SYSTEM_ALARM_SETTING:
		key2_alarm_setting();
		break;
	case SYSTEM_ALARM:
		key2_alarm();
		break;
	}
}

#define TAU 100

void key_handler()
{
	if (is_key_triggered[0] && ts_duration(HAL_GetTick(), is_key_triggered[0]) > TAU) {
		if (HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin) == GPIO_PIN_SET) {
			is_key_triggered[0] = 0;
			return;
		}
		if (system_status.se_en)
			beep(100);
		key0_func();
		
		is_key_triggered[0] = 0;
		return;
	}
	if (is_key_triggered[1] && ts_duration(HAL_GetTick(), is_key_triggered[1]) > TAU) {
		if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_SET) {
			is_key_triggered[1] = 0;
			return;
		}
		if (system_status.se_en)
			beep(100);
		key1_func();
		
		is_key_triggered[1] = 0;
		return;
	}
	if (is_key_triggered[2] && ts_duration(HAL_GetTick(), is_key_triggered[2]) > TAU) {
		HAL_Delay(100);
		if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_SET) {
			is_key_triggered[2] = 0;
			return;
		}
		if (system_status.se_en)
			beep(100);
		key2_func();
		
		is_key_triggered[2] = 0;
		return;
	}
	if (is_key_triggered[3] && ts_duration(HAL_GetTick(), is_key_triggered[3]) > 100) {
		if (HAL_GPIO_ReadPin(KEY_UP_GPIO_Port, KEY_UP_Pin) == GPIO_PIN_RESET) {
			is_key_triggered[3] = 0;
			return;
		}
		if (system_status.se_en)
			beep(100);
		keyup_func();

		is_key_triggered[3] = 0;
		return;
	}
}