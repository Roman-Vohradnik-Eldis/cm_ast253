#include "cm_recv_buffer_state.hh"
#include "decompress_zstd.hh"
#include "decompress_none.hh"
#include "cm_utils.hh"

RecvBufferState::RecvBufferState(RecvBufferSettings const &settings)
{
    Maybe<View<uint8_t>> dict;
    if (settings.dictionary)
        dict = view(*settings.dictionary);

#ifdef USE_LIB_ZSTD
    decompress.reset(new Decompress_zstd(dict));
#else
    decompress.reset(new Decompress_none());
#endif
}

void RecvBufferState::reset()
{
    decompress->reset();
}

#ifdef USE_LIB_ZSTD
bool const is_zstd_supported = true;
#else
bool const is_zstd_supported = false;
#endif

bool is_compression_supported()
{
    return is_zstd_supported;
}

void RecvBufferState::add_data(View<uint8_t> msg)
{
    if (msg.empty())
    {
        cms_ns_if_print("libcm", 1, "Received empty message");
        return;
    }
    uint8_t header = msg[0];
    if (header == 'n')
    {
        if (!is_zstd_supported)
            throw std::runtime_error("compression not supported");
        decompress->reset();
    }
    else if (header == 'c')
    {
        if (!is_zstd_supported)
            throw std::runtime_error("compression not supported");
        // OK, continue frame
    }
    else if (header == 't')
    {
        // OK, uncompressed message
    }
    else
    {
        cms_ns_if_print("libcm", 1, "Invalid message header '%c' (ascii hex=%x)", header, header);
        throw std::runtime_error("RecvBufferState: received invalid message");
    }

    // decompress->get().clear();

    if (header == 't')
    {
        std::vector<uint8_t> &dest = decompress->get();
        dest.insert(dest.end(), msg.begin() + 1, msg.end());
    }
    else
    {
        decompress->add_data(view(msg.begin() + 1, msg.end()));
    }
}

std::vector<uint8_t> &RecvBufferState::get()
{
    return decompress->get();
}

std::vector<uint8_t> const &RecvBufferState::get() const
{
    return decompress->get();
}
