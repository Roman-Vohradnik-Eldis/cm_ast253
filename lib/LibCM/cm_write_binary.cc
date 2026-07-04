#include "cm_write_binary.hh"
#include "cm_utils.hh"
#include <limits>

static_assert(G_BYTE_ORDER == G_LITTLE_ENDIAN, "Expected Little Endian machine");

void write_unsigned(uint8_t *data, unsigned long long value, unsigned value_sizeof)
{
    if (value_sizeof == 1)
    {
        CHECK(value <= std::numeric_limits<uint8_t>::max(), "Value too big");
        memcpy(data, &value, value_sizeof);
    }
    else if (value_sizeof == 2)
    {
        CHECK(value <= std::numeric_limits<uint16_t>::max(), "Value too big");
        memcpy(data, &value, value_sizeof);
    }
    else if (value_sizeof == 4)
    {
        CHECK(value <= std::numeric_limits<uint32_t>::max(), "Value too big");
        memcpy(data, &value, value_sizeof);
    }
    else if (value_sizeof == 8)
    {
        CHECK(value <= std::numeric_limits<uint64_t>::max(), "Value too big");
        memcpy(data, &value, value_sizeof);
    }
    else
    {
        throw std::logic_error("Invalid value_sizeof");
    }
}

void write_signed(uint8_t *data, long long value, unsigned value_sizeof)
{
    if (value_sizeof == 1)
    {
        CHECK(value >= std::numeric_limits<int8_t>::min(), "Value too small");
        CHECK(value <= std::numeric_limits<int8_t>::max(), "Value too big");
        memcpy(data, &value, value_sizeof);
    }
    else if (value_sizeof == 2)
    {
        CHECK(value >= std::numeric_limits<int16_t>::min(), "Value too small");
        CHECK(value <= std::numeric_limits<int16_t>::max(), "Value too big");
        memcpy(data, &value, value_sizeof);
    }
    else if (value_sizeof == 4)
    {
        CHECK(value >= std::numeric_limits<int32_t>::min(), "Value too small");
        CHECK(value <= std::numeric_limits<int32_t>::max(), "Value too big");
        memcpy(data, &value, value_sizeof);
    }
    else if (value_sizeof == 8)
    {
        CHECK(value >= std::numeric_limits<int64_t>::min(), "Value too small");
        CHECK(value <= std::numeric_limits<int64_t>::max(), "Value too big");
        memcpy(data, &value, value_sizeof);
    }
    else
    {
        throw std::logic_error("Invalid value_sizeof");
    }
}

void write_float(uint8_t *buffer, double in, unsigned value_sizeof)
{
    if (value_sizeof == 4)
    {
        float f = in;
        memcpy(buffer, &f, sizeof(f));
    }
    else if (value_sizeof == 8)
    {
        double f = in;
        memcpy(buffer, &f, sizeof(f));
    }
    else
    {
        throw std::logic_error("Invalid value_sizeof");
    }
}

unsigned long long read_unsigned(uint8_t const *data, unsigned value_sizeof)
{
    if (value_sizeof == 1)
    {
        return data[0];
    }
    else if (value_sizeof == 2)
    {
        uint16_t x = 0;
        memcpy(&x, data, value_sizeof);
        return x;
    }
    else if (value_sizeof == 4)
    {
        uint32_t x = 0;
        memcpy(&x, data, value_sizeof);
        return x;
    }
    else if (value_sizeof == 8)
    {
        uint64_t x = 0;
        memcpy(&x, data, value_sizeof);
        return x;
    }
    else
    {
        throw std::logic_error("Invalid value sizeof");
    }
}

long long read_signed(uint8_t const *data, unsigned value_sizeof)
{
    if (value_sizeof == 1)
    {
        return data[0];
    }
    else if (value_sizeof == 2)
    {
        int16_t x = 0;
        memcpy(&x, data, value_sizeof);
        return x;
    }
    else if (value_sizeof == 4)
    {
        int32_t x = 0;
        memcpy(&x, data, value_sizeof);
        return x;
    }
    else if (value_sizeof == 8)
    {
        int64_t x = 0;
        memcpy(&x, data, value_sizeof);
        return x;
    }
    else
    {
        throw std::logic_error("Invalid value sizeof");
    }
}

double read_float(uint8_t const *buffer, unsigned value_sizeof)
{
    if (value_sizeof == 4)
    {
        float f = 0;
        memcpy(&f, buffer, sizeof(float));
        return f;
    }
    else if (value_sizeof == 8)
    {
        double f = 0;
        memcpy(&f, buffer, sizeof(double));
        return f;
    }
    else
    {
        throw std::logic_error("Invalid value sizeof");
    }
}

double read_double(uint8_t const *buffer)
{
    double f = 0;
    memcpy(&f, buffer, sizeof(double));
    return f;
}

// TODO: Slo by samozrejme rychleji
void push_unsigned(std::vector<uint8_t> &data, unsigned long long value, unsigned value_sizeof)
{
    std::vector<uint8_t> tmp(value_sizeof);
    write_unsigned(tmp.data(), value, value_sizeof);
    data.insert(data.end(), tmp.begin(), tmp.end());
}

// TODO: Slo by samozrejme rychleji
void push_signed(std::vector<uint8_t> &data, long long value, unsigned value_sizeof)
{
    std::vector<uint8_t> tmp(value_sizeof);
    write_signed(tmp.data(), value, value_sizeof);
    data.insert(data.end(), tmp.begin(), tmp.end());
}
