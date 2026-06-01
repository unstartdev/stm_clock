#include "utils.h"

#define V_MAX 0xFFFFFFFFU

// return the duration for tick1 - tick2
uint32_t ts_duration(uint32_t tick1, uint32_t tick2)
{
	if (tick1 < tick2)
		return tick1 + V_MAX - tick2 + 1;
	return tick1 - tick2;
}