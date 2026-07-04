#ifndef COMPRESS_NONE_HH_
#define COMPRESS_NONE_HH_

#include <memory>
#include "compress_base.hh"
#include "cm_maybe.hh"

class Compress_none : public CompressBase
{
    ByteBuffer result;

public:
    // When message_ended == true, the decoder can read the whole message.
    void add_data(View<uint8_t> data, CompressFlushMode)
    {
        result.append(data);
    }

    // Clean output buffer, reset state.
    void reset()
    {
        result.clear();
    }

    ByteBuffer &get() { return result; }
    ByteBuffer const &get() const { return result; }
};

#endif // COMPRESS_NONE_HH_
