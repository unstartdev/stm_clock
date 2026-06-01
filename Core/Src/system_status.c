#include "system_status.h"
#include "main.h"
#include "utils.h"
#include <time.h>
#include "led_digital_tube.h"
#include <string.h>
#include "beep.h"

__ALIGNED(4) volatile struct system_status system_status = {
	.state = SYSTEM_FREE,
	.time = 1628188200,
	.tms_tick = 0,
	.tzoffset = 8,
	.se_en = 1,
};

volatile struct stop_watch_status stop_watch = {0};

volatile struct time_setting_status time_setting = {0};

volatile struct date_setting_status date_setting = {0};

volatile struct alarm_setting_status alarm_setting = {0};

static void system_ts_to_tm(struct tm *tm)
{
	time_t time_copy = system_status.time;
	time_copy += 3600 * system_status.tzoffset;
	_localtime_r(&time_copy, tm);
	tm->tm_mon += 1;
	tm->tm_year += 1900;
}

void load_time_to_led_buffer(struct tm *tm, enum led_row_names row)
{	
	led_digital_set_num(row, 0, 2, tm->tm_hour);

	led_digital_set_div(row, 2);
	
	led_digital_set_num(row, 3, 2, tm->tm_min);
	
	led_digital_set_div(row, 5);

	led_digital_set_num(row, 6, 2, tm->tm_sec);
}

void load_date_to_led_buffer(struct tm *tm, enum led_row_names row)
{
	led_digital_set_num(row, 0, 4, tm->tm_year);
	
	led_digital_set_point(row, 3);
	
	led_digital_set_num(row, 4, 2, tm->tm_mon);
	
	led_digital_set_point(row, 5);
	
	led_digital_set_num(row, 6, 2, tm->tm_mday);	
}
	
void load_timedate_to_led_buffer()
{
	struct tm tm;
	system_ts_to_tm(&tm);
	
	load_time_to_led_buffer(&tm, TIME_ROW);
	if (system_status.alarm_en)
		led_digital_set_point(TIME_ROW, 0);
	load_date_to_led_buffer(&tm, DATE_ROW);
}

void load_time_setting()
{
	struct tm *tm = (struct tm *)&time_setting.tm;
	system_ts_to_tm(tm);
	if (system_status.alarm_en) {
		led_digital_set_point(ALARM_ROW, 0);
	} else {
		led_digital_clear_point(ALARM_ROW, 0);
	}
	load_time_to_led_buffer(tm, TIME_SETTING_ROW);
	led_digital_set_bp(time_setting.cur);
}

void load_date_setting()
{
	struct tm *tm = (struct tm *)&date_setting.tm;
	system_ts_to_tm(tm);
	
	load_date_to_led_buffer(tm, DATE_SETTING_ROW);
	led_digital_set_bp(date_setting.cur);
}

void load_timezone_offset_setting()
{
	led_digital_set_signed_num(TIMEZONE_SETTING_ROW, 5, 2, system_status.tzoffset);
}

void load_alarm_setting()
{
	if (system_status.alarm_en) {
		led_digital_set_point(ALARM_SETTING_ROW, 0);
	} else {
		led_digital_clear_point(ALARM_SETTING_ROW, 0);
	}
	led_digital_set_num(ALARM_SETTING_ROW, 3, 2, system_status.alarm_h);
	led_digital_set_num(ALARM_SETTING_ROW, 6, 2, system_status.alarm_min);
	led_digital_set_bp(alarm_setting.cur);
}

void save_setting(struct tm *tm_s)
{
	__disable_irq();
	struct tm tm;
	memcpy(&tm, tm_s, sizeof(struct tm));
	tm.tm_year -= 1900;
	tm.tm_mon -= 1;
	system_status.time = mktime(&tm) - 3600 * system_status.tzoffset;
	__enable_irq();
}

void load_sport_tick_to_led_buffer()
{
	led_digital_set_num_nopzero(STOP_WATCH_ROW, 0, 8, system_status.sport_tick, 5);
}

void load_se_setting()
{
	led_digital_set_num(SE_SETTING_ROW, 7, 1, system_status.se_en);
}

volatile struct alarm_status alarm_status = {0};

void load_alarm()
{
	alarm_status.last_trig = system_status.time;
	led_digital_set_num(ALARM_ROW, 3, 2, alarm_status.delayed ? alarm_status.delay_to_h : system_status.alarm_h);
	led_digital_set_div(ALARM_ROW, 5);
	led_digital_set_num(ALARM_ROW, 6, 2, alarm_status.delayed ? alarm_status.delay_to_m : system_status.alarm_min);
	system_status.state = SYSTEM_ALARM;
	led_digital_row_sel(ALARM_ROW);
	beep_alarm_set();
}

void release_alarm()
{
	beep_alarm_clear();
	alarm_status.delayed = 0;
	led_digital_row_sel(TIME_ROW);
}

void suppress_alarm()
{
	beep_alarm_clear();
	alarm_status.delay_to_h = system_status.alarm_h;
	alarm_status.delay_to_m = system_status.alarm_min;
	alarm_status.delay_to_m += 5;
	if (alarm_status.delay_to_m > 59) {
		alarm_status.delay_to_m -= 60;
		alarm_status.delay_to_h += 1;
	}
	if (alarm_status.delay_to_h > 23)
		alarm_status.delay_to_h -= 24;
	alarm_status.delayed = 1;
}

void alarm_trig()
{
	if (system_status.alarm_en == 0)
		return;
	if (system_status.time - alarm_status.last_trig < 60)
		return;
	system_ts_to_tm((struct tm *)&alarm_status.tm);
	if (alarm_status.tm.tm_hour == system_status.alarm_h && alarm_status.tm.tm_min == system_status.alarm_min) {
		load_alarm();
		return;
	}
	if (alarm_status.delayed == 0)
		return;
	if (alarm_status.tm.tm_hour == alarm_status.delay_to_h && alarm_status.tm.tm_min == alarm_status.delay_to_m) {
		load_alarm();
		return;
	}
}

void load_env()
{
	led_digital_set_signed_num(ENV_ROW, 0, 2, system_status.temp);
	led_digital_set_num(ENV_ROW, 6, 2, system_status.humidity);
}

void buffer_load()
{
	switch (system_status.state) {
	case SYSTEM_FREE:
		load_timedate_to_led_buffer();
		break;
	case SYSTEM_TIME_SETTING:
		load_time_setting();
		break;
	case SYSTEM_DATE_SETTING:
		load_date_setting();
		break;
	default:
		break;
	}
}