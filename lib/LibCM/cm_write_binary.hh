#ifndef CM_WRITE_BINARY_HH_
#define CM_WRITE_BINARY_HH_

#include <stdint.h>
#include <vector>

// value_sizeof: 1, 2, 4, 8
void write_unsigned(uint8_t *data, unsigned long long value, unsigned value_sizeof);

// value_sizeof: 1, 2, 4, 8
void write_signed(uint8_t *data, long long value, unsigned value_sizeof);

// value_sizeof: 4, 8
void write_float(uint8_t *buffer, double f, unsigned value_sizeof);

// value_sizeof: 1, 2, 4, 8
unsigned long long read_unsigned(uint8_t const *data, unsigned value_sizeof);

// value_sizeof: 1, 2, 4, 8
long long read_signed(uint8_t const *data, unsigned value_sizeof);

// value_sizeof: 4, 8
double read_float(uint8_t const *buffer, unsigned value_sizeof);

void push_unsigned(std::vector<uint8_t> &data, unsigned long long value, unsigned value_sizeof);

void push_signed(std::vector<uint8_t> &data, long long value, unsigned value_sizeof);

#endif // CM_WRITE_BINARY_HH_
