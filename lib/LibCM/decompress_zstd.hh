#ifndef DECOMPRESS_ZSTD_HH_
#define DECOMPRESS_ZSTD_HH_

#include <memory>
#include "compress_base.hh"
#include "cm_view.hh"
#include "cm_maybe.hh"

class Decompress_zstd : public DecompressBase
{
    // We store ZSTD context in an opaque pointer to prevent global
    // namespace polution.
    struct Context;

    std::vector<uint8_t> result;
    std::unique_ptr<Context> context;

public:
    // The compressor and the decompressor must use the same
    // dictionary.
    Decompress_zstd(Maybe<View<uint8_t>> dict = Nothing());
    ~Decompress_zstd();

    // On decompression error, throw std::runtime_error
    void add_data(View<uint8_t> data);

    void reset();

    std::vector<uint8_t> &get() { return result; }
    std::vector<uint8_t> const &get() const { return result; }
};

#endif // DECOMPRESS_ZSTD_HH_
