#ifndef CM_RANDOM_HH_
#define CM_RANDOM_HH_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void cm_get_random(uint8_t *buffer, size_t size);

void cm_get_random_urandom(uint8_t *buffer, size_t size);

template <typename T>
T cm_get_random_value()
{
    T x;
    uint8_t buffer[sizeof(T)];
    cm_get_random(buffer, sizeof(T));
    memcpy(&x, buffer, sizeof(T));
    return x;
}

#endif // CM_RANDOM_HH_
