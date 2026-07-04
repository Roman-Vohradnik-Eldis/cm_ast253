#ifndef COMPRESS_ZSTD_HH_
#define COMPRESS_ZSTD_HH_

#include <memory>
#include "compress_base.hh"
#include "cm_maybe.hh"

struct ZstdParams
{
    // CompressFlushMode::Finish after `frame_size_limit` input bytes.
    Maybe<size_t> frame_size_limit = SIZE_MAX;

    // CompressFlushMode::Flush after `block_size_limit` input bytes.
    Maybe<size_t> block_size_limit;
}; 

class Compress_zstd : public CompressBase
{
    // We store ZSTD context in opaque pointer to prevent global
    // namespace polution.
    struct Context;

    ZstdParams params;
    size_t compressed_in_current_frame = 0;
    ByteBuffer result;
    std::unique_ptr<Context> context;

public:
    // The compressor and the decompressor must use the same
    // dictionary.
    Compress_zstd(int level, Maybe<View<uint8_t>> dict = Nothing(), ZstdParams const &params = {});
    ~Compress_zstd();

    void add_data(View<uint8_t> data, CompressFlushMode flush_mode);

    void reset();

    ByteBuffer &get() { return result; }
    ByteBuffer const &get() const { return result; }

private:
    void add_data_internal(View<uint8_t> data, bool message_ended, bool stream_endede);
}; 

#endif // COMPRESS_ZSTD_HH_
