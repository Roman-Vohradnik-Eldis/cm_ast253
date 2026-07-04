#ifndef CM_DB_RESPONSE_BUFFER_HH_
#define CM_DB_RESPONSE_BUFFER_HH_

#include "compress_base.hh"
#include <memory>

// DbResponseBuffer cte data od Partnera. Podle hlavicky sam pozna,
// jestli jsou data komprimovana, a podle toho se nastavi.
class DbResponseBuffer
{
    bool is_new_session = true;
    bool is_compressed = false;
    std::unique_ptr<DecompressBase> decompressor;

public:
    DbResponseBuffer();

    void reset();

    void add_data(View<uint8_t> data);

    std::vector<uint8_t> &get();
    std::vector<uint8_t> const &get() const;
};

#endif // CM_DB_RESPONSE_BUFFER_HH_
