#ifndef __LED_DIGITAL_COMM_H
#define __LED_DIGITAL_COMM_H
#include <stdint.h>

void led_digital_select(uint32_t pos);
void led_digital_set(uint32_t num);
void led_digital_latch_out();


#endif