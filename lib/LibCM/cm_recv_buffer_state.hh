#ifndef CM_RECV_BUFFER_STATE_HH_
#define CM_RECV_BUFFER_STATE_HH_

#include "cm_view.hh"
#include "cm_debug.hh"
#include "compress_base.hh"
#include "cm_maybe.hh"

#include <memory>

struct RecvBufferSettings
{
    Maybe<std::vector<uint8_t>> dictionary;
};

// Jesli jsme zkompilovany se zstd
//
bool is_compression_supported();

// Prijimame stream:
//
// Jeden blok dat muze prijit bud komprimovaný nebo nekomprimovaný.
//
// Objekt neresi, kde zprava konci.
class RecvBufferState : public DecompressBase
{
    bool expected_new_frame = true;
    std::unique_ptr<DecompressBase> decompress;

public:
    RecvBufferState(RecvBufferSettings const &settings);

    // On error, throw std::runtime_error
    void add_data(View<uint8_t> view);

    void reset();

    std::vector<uint8_t> &get();
    std::vector<uint8_t> const &get() const;
};

#endif // CM_RECV_BUFFER_STATE_HH_
