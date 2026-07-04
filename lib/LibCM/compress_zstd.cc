#ifdef USE_LIB_ZSTD

#include "compress_zstd.hh"
#include "zstd.h"
#include "cm_utils.hh"

struct Compress_zstd::Context
{
    ZSTD_CCtx *value = 0;
    ZSTD_CDict *dict = 0;

    ~Context()
    {
        if (value)
            ZSTD_freeCCtx(value);
        if (dict)
            ZSTD_freeCDict(dict);
    }
};

Compress_zstd::Compress_zstd(int level, Maybe<View<uint8_t>> dict, ZstdParams const &params)
    : params(params),
      context(new Context())
{
    context->value = ZSTD_createCCtx();
    CHECK(context->value, "ZSTD_createCCtx failed");

    {
        size_t rc = ZSTD_CCtx_setParameter(context->value, ZSTD_c_compressionLevel, level);
        CHECK(!ZSTD_isError(rc), "Failed to set compression level %d: %s", level, ZSTD_getErrorName(rc));
    }
    if (dict)
    {
        context->dict = ZSTD_createCDict(dict->begin(), dict->size(), level);
        CHECK(context->dict, "ZSTD_createCDict failed");
        size_t rc = ZSTD_CCtx_refCDict(context->value, context->dict);
        CHECK(!ZSTD_isError(rc), "ZSTD_CCtx_refCDict failed: %s", ZSTD_getErrorName(rc));
    }
}

void Compress_zstd::add_data_internal(View<uint8_t> data, bool message_ended, bool stream_ended)
{
    // ZSTD will advance pos:
    ZSTD_inBuffer input = { data.begin(), data.size(), 0 };

    compressed_in_current_frame += data.size();

    // Algoritmus zere vstup a postupne zapomina slovnik. My ho ale
    // zapomenout nechceme. Tak to delame tak, ze obcas ukoncime
    // frame, cimz algoritmus zapomene predchozi vstup a znovunacte
    // slovnik.
    if (message_ended && params.frame_size_limit && compressed_in_current_frame >= *params.frame_size_limit)
    {
        compressed_in_current_frame = 0;
        stream_ended = true;
    }

    for (;;)
    {
        size_t prev_size = result.size();
        result.resize(prev_size + 4096);
        ZSTD_outBuffer output = { result.data(), result.size(), prev_size };

        // fprintf(stderr, "Compress before iteration: input.pos=%zu/%zu, output.pos=%zu/%zu, flush=%d\n",
        //         input.pos, data.size(), output.pos, result.size(), message_ended);

        ZSTD_EndDirective const mode = stream_ended ? ZSTD_e_end : (message_ended ? ZSTD_e_flush : ZSTD_e_continue);
        size_t remaining = ZSTD_compressStream2(context->value, &output , &input, mode);
        CHECK(!ZSTD_isError(remaining), "Compression failed: %s", ZSTD_getErrorName(remaining));

        // fprintf(stderr, "Compress after iteration: input.pos=%zu/%zu, output.pos=%zu/%zu, flush=%d, remaining=%zu\n",
        //         input.pos, data.size(), output.pos, result.size(), message_ended, remaining);

        result.resize(output.pos);

        // Pozn: Pokud input.pos < size, tak asi nestacil output
        // buffer, a tak chceme iterace provest znovu.
        if (input.pos == data.size())
        {
            // My zstd pouzivame tak, ze jeden frame obsahuje vice
            // zprav. Takze muze byt remaining > 0, i kdyz uz tu
            // zpravu mame celou.
            if (!message_ended || remaining == 0)
                break;
        }
    }
}

void Compress_zstd::add_data(View<uint8_t> data, CompressFlushMode flush_mode)
{
    bool message_ended = flush_mode == CompressFlushMode::Flush;
    bool stream_ended = flush_mode == CompressFlushMode::Finish;
    if (data.size() == 0 && !message_ended)
        return;
    if (params.block_size_limit)
    {
        size_t limit = *params.block_size_limit;
        while (data.size() > limit)
        {
            add_data_internal(view(data.begin(), data.begin() + limit), true, false);
            data.advance(limit);
        }
    }
    add_data_internal(data, message_ended, false);
}

// void Compress_zstd::finish()
// {
//     add_data_internal({}, true, true);
// }

void Compress_zstd::reset()
{
    result.clear();
    compressed_in_current_frame = 0;

    // Preserve parameters and the dictionary. Never fails.
    ZSTD_CCtx_reset(context->value, ZSTD_reset_session_only); 
}

Compress_zstd::~Compress_zstd()
{
}    

#endif // USE_LIB_ZSTD
