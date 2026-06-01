#ifndef __BEEP_H
#define __BEEP_H
#include <stdint.h>

void beep(uint32_t duration);
void beep_irq();
void beep_set();
void beep_clear();
void beep_alarm_irq();
void beep_alarm_set();
void beep_alarm_clear();

#endif