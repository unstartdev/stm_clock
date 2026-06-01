#ifndef __DHT11_H
#define __DHT11_H
#include <stdint.h>

void dht11_get_data(volatile int8_t *temp, volatile uint8_t *humidity);

#endif