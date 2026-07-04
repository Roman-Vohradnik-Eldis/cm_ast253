#include "cm_db_response_buffer.hh"
#include "decompress_zstd.hh"
#include "decompress_none.hh"

#ifdef USE_LIB_ZSTD
static bool const is_zstd_supported = true;

DbResponseBuffer::DbResponseBuffer()
    : decompressor(new Decompress_zstd())
{
}
#else
static bool const is_zstd_supported = false;

DbResponseBuffer::DbResponseBuffer()
    : decompressor(new Decompress_none())
{
}
#endif

void DbResponseBuffer::reset()
{
    is_new_session = true;
    decompressor.reset();
}

void DbResponseBuffer::add_data(View<uint8_t> data)
{
    if (data.empty())
        return;

    if (is_new_session)
    {
        char kind = data[0];
        if (kind == 't')
        {
            is_compressed = false;
        }
        else if (kind == 'n')
        {
            if (is_zstd_supported)
                throw std::runtime_error("Compressed messages not supported");
            is_compressed = true;
        }
        else
        {
            throw std::runtime_error("Unknown data header");
        }
        is_new_session = false;
        data.advance(1);
    }

    if (is_compressed)
    {
        decompressor->add_data(data);
    }
    else
    {
        std::vector<uint8_t> &buffer = decompressor->get();
        buffer.insert(buffer.end(), data.begin(), data.end());
    }
}

std::vector<uint8_t> &DbResponseBuffer::get()
{
    return decompressor->get();
}

std::vector<uint8_t> const &DbResponseBuffer::get() const
{
    return decompressor->get();
}
