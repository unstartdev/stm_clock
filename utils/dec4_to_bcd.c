#include "utils.h"
#include <stdint.h>

uint32_t dec_to_bcd(uint32_t num) {
    uint32_t bcd = 0;
    uint32_t shift = 0;
    while (num > 0) {
        bcd |= (num % 10) << shift;
        shift += 4;
        num /= 10;
    }
    return bcd;
}