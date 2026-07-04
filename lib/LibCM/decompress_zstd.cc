#ifdef USE_LIB_ZSTD

#include "decompress_zstd.hh"
#include "zstd.h"
#include "cm_utils.hh"

struct Decompress_zstd::Context
{
    ZSTD_DCtx *value = 0;
    ZSTD_DDict *dict = 0;
    
    ~Context()
    {
        if (value)
            ZSTD_freeDCtx(value);
        if (dict)
            ZSTD_freeDDict(dict);
    }
};

Decompress_zstd::Decompress_zstd(Maybe<View<uint8_t>> dict)
    : context(new Context())
{
    context->value = ZSTD_createDCtx();
    CHECK(context->value, "ZSTD_createCCtx failed");

    if (dict)
    {
        context->dict = ZSTD_createDDict(dict->begin(), dict->size());
        CHECK(context->dict, "ZSTD_createDDict failed");
        size_t rc = ZSTD_DCtx_refDDict(context->value, context->dict);
        CHECK(!ZSTD_isError(rc), "ZSTD_DCtx_refDDict failed: %s", ZSTD_getErrorName(rc));
    }
}

void Decompress_zstd::add_data(View<uint8_t> data)
{
    if (data.size() == 0)
        return;
    
    // ZSTD will advance pos
    ZSTD_inBuffer input = { data.begin(), data.size(), 0 };
    
    for (;;)
    {
        size_t prev_size = result.size();
        result.resize(prev_size + 4096);
        ZSTD_outBuffer output = { result.data(), result.size(), prev_size };

        // fprintf(stderr, "Decommpress before iteration: input.pos=%zu/%zu, output.pos=%zu/%zu, flush=%d\n",
        //         input.pos, data.size(), output.pos, result.size(), message_ended);
        
        size_t remaining = ZSTD_decompressStream(context->value, &output , &input);
        CHECK(!ZSTD_isError(remaining), "Decompression failed: %s", ZSTD_getErrorName(remaining));

        // fprintf(stderr, "Decompress after iteration: input.pos=%zu/%zu, output.pos=%zu/%zu, remaining=%zu\n",
        //         input.pos, data.size(), output.pos, result.size(), remaining);

        result.resize(output.pos);

        // If `output.pos < output.size`, decoder has flushed everything it could...
        if (output.pos != output.size)
        {
            // If `input.pos < input.size`, some input has not been consumed.
            if (input.pos == data.size())
            {
                break;
            }            
        }
    }
}

void Decompress_zstd::reset()
{
    result.clear();
    size_t rc = ZSTD_DCtx_reset(context->value, ZSTD_reset_session_only);

    // From doc: Resetting session never fails.
    assert(rc == 0);
}

Decompress_zstd::~Decompress_zstd()
{
}    

#endif // USE_LIB_ZSTD
