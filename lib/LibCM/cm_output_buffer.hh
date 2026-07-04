#ifndef CM_OUTPUT_BUFFER_HH_
#define CM_OUTPUT_BUFFER_HH_

#include <stdint.h>
#include <stdexcept>
#include "cm_view.hh"

class OutputBuffer
{
public:
    virtual ~OutputBuffer() {}

    // Clear the buffer, reset state
    virtual void reset() = 0;

    // Return the length of a message
    virtual size_t add_data(View<uint8_t> data, bool end_msg) = 0;
};

#endif // CM_OUTPUT_BUFFER_HH_
