#ifndef __MUSIC_H
#define __MUSIC_H
#include <stdint.h>

void tone(uint32_t freq);
void tone_clear();
void tone_it(uint32_t duration, uint32_t freq);
void tone_irq();

#endif