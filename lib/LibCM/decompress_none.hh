#ifndef DECOMPRESS_NONE_HH_
#define DECOMPRESS_NONE_HH_

#include <memory>
#include "compress_base.hh"
#include "cm_maybe.hh"

class Decompress_none : public DecompressBase
{
    std::vector<uint8_t> result;

public:
    // When message_ended == true, the decoder can read the whole message.
    void add_data(View<uint8_t> data)
    {
        result.insert(result.end(), data.begin(), data.end());
    }

    // Clean output buffer, reset state.
    void reset()
    {
        result.clear();
    }

    std::vector<uint8_t> &get() { return result; }
    std::vector<uint8_t> const &get() const { return result; }
};

#endif // DECOMPRESS_NONE_HH_
