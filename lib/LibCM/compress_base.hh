#ifndef COMPRESS_BASE_HH_
#define COMPRESS_BASE_HH_

#include <stdint.h>
#include <stdexcept>
#include "cm_view.hh"
#include "cm_string_buffer.hh"

enum class CompressFlushMode
{
    // Do not flush. No output is generated, unless the input buffer
    // is very large.
    None,

    // Process all given input data so far, and put it to the output
    // buffer. The decompressor will be able to decompress the whole
    // chunk up to this flush point. Flushing too often will degrade
    // the compression.
    Flush,

    // Flush and clear the compression context.  The next
    // message will not depened on the previous one.
    Finish,
};

class CompressBase
{
public:
    virtual ~CompressBase() {}

    // Reset the internal state and clear output buffer. Usually, the
    // initialization of a compressor involves a lot of allocations
    // and initializations (e.g. loading dictionaries).  Thus it is
    // advisable to use `reset` to reuse an object.
    virtual void reset() = 0;

    // Add data to the input buffer. Depending on the flush mode, the
    // compressor may or may not generate output data.
    virtual void add_data(View<uint8_t> data, CompressFlushMode flush_mode) = 0;

    // Access output buffer. The output buffer can be cleared or
    // modified by the user at any time.
    virtual ByteBuffer &get() = 0;

    // Access output buffer.
    virtual ByteBuffer const &get() const = 0;
};


class DecompressBase
{
public:
    virtual ~DecompressBase() {}

    // Reset the internal state and clear output buffer. Usually, the
    // initialization of a decompressor involves a lot of allocations
    // and initializations (e.g. loading dictionaries).  Thus it is
    // advisable to use `reset` to reuse an object.
    virtual void reset() = 0;

    // Add data to the input buffer. If the there was a Flush point in
    // the message, an output is generated to the output buffer.
    virtual void add_data(View<uint8_t> data) = 0;

    // Access output buffer. The output buffer can be cleared or
    // modified by the user at any time.
    virtual std::vector<uint8_t> &get() = 0;

    // Access output buffer.
    virtual std::vector<uint8_t> const &get() const = 0;
}; 

#endif // COMPRESS_BASE_HH_
