#include <stdint.h>

uint64_t prime_min(uint64_t a, uint64_t b) {
    return (a < b) ? a : b;
}