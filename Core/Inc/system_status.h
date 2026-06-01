#ifndef __SYSTEM_STATUS_H
#define __SYSTEM_STATUS_H
#include <stdint.h>
#include <time.h>

enum system_state {
	SYSTEM_STATE_HEAD,
	SYSTEM_FREE,
	SYSTEM_STOP_WATCH,
	SYSTEM_ALARM,
	SYSTEM_TIME_SETTING,
	SYSTEM_TIMEZONE_SETTING,
	SYSTEM_DATE_SETTING,
	SYSTEM_ALARM_SETTING,
	SYSTEM_SOUND_EFFECT_SETTING,
	SYSTEM_STATE_TAIL,
};

struct system_status {
	time_t time;
	time_t err;
	uint32_t sport_tick;
	uint8_t tms_tick;
	int8_t tzoffset;
	enum system_state state;
	uint32_t alarm_en;
	uint32_t alarm_h;
	uint32_t alarm_min;
	uint32_t se_en;
	int8_t temp;
	uint8_t humidity;
	uint8_t dht_update_flag;
	uint8_t dht_update_tick;
	uint8_t led_update_flag;
	uint8_t led_buffer_load_flag;
};

extern volatile struct system_status system_status;

struct stop_watch_status {
	uint8_t en;
};

extern volatile struct stop_watch_status stop_watch;

enum time_setting_cur {
	TIME_CUR_HOUR,
	TIME_CUR_MIN,
	TIME_CUR_SEC,
};

struct time_setting_status {
	struct tm tm;
	enum time_setting_cur cur;
};

extern volatile struct time_setting_status time_setting;

enum date_setting_cur {
	DATE_CUR_YEAR,
	DATE_CUR_MON,
	DATE_CUR_DAY,
};

struct date_setting_status {
	struct tm tm;
	enum date_setting_cur cur;
};

extern volatile struct date_setting_status date_setting;

enum alarm_setting_cur {
	ALARM_CUR_HOUR,
	ALARM_CUR_MIN,
	ALARM_CUR_EN
};

struct alarm_setting_status {
	enum alarm_setting_cur cur;
};

extern volatile struct alarm_setting_status alarm_setting;

struct alarm_status {
	struct tm tm;
	time_t last_trig;
	uint32_t delayed;
	uint32_t delay_to_h;
	uint32_t delay_to_m;
};

extern volatile struct alarm_status alarm_status;

void load_timedate_to_led_buffer();
void load_time_setting();
void load_date_setting();
void load_timezone_offset_setting();
void load_alarm_setting();
void load_env();
void save_setting(struct tm *tm);
void load_sport_tick_to_led_buffer();
void load_se_setting();
void release_alarm();
void suppress_alarm();
void alarm_trig();
void buffer_load();

#endif