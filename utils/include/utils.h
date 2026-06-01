#ifndef __UTILS_H
#define __UTILS_H
#include <stdint.h>

uint32_t dec_to_bcd(uint32_t n);
uint32_t ts_duration(uint32_t tick1, uint32_t tick2);
#endif