#include "cm_send_buffer_state.hh"
#include "cm_base_functs.hh"
#include "cm_debug.hh"
#include "compress_base.hh"
#include "compress_zstd.hh"
#include "compress_none.hh"
#include "cm_test.hh"
#include <fstream>
#include "cm_utils.hh"

#define PRINT(level, fmt, ...) cms_ns_if_print("send", level, fmt, __VA_ARGS__)

// #define PRINT(level, fmt, ...) ((void)0)

SendBufferState::SendBufferState(Maybe<uint8_t> delimiter, SendBufferSettings const &settings, bool use_header)
    : delimiter(delimiter),
      compression_enabled(settings.use_compression),
      use_header(use_header),
      write_dictionary_max_size(settings.write_dictionary_max_size)
{
    if (settings.use_compression)
    {
#ifdef USE_LIB_ZSTD
        Maybe<View<uint8_t>> dict;
        if (settings.dictionary)
            dict = view(*settings.dictionary);

        cms_ns_if_print("libcm", 3, "Using compression, level=%d, dictionary_size=%zu, use_header=%s",
                        settings.compression_level, dict ? dict->size() : -1,
                        use_header ? "yes" : "no");

        ZstdParams params;
        params.block_size_limit = 4096;
        compress.reset(new Compress_zstd(settings.compression_level, dict));
#else
        cms_ns_if_print("libcm", 1, "Cannot use compression, you must compile libCM with zstd");
        compression_enabled = false;
        compress.reset(new Compress_none());
#endif
    }
    else
    {
        compress.reset(new Compress_none());
    }

    if (settings.debug_fname)
    {
        libCM_CreateDirectoryFromPath(settings.debug_fname->c_str());
        debug_file = open_shared_file(*settings.debug_fname);
    }
    if (settings.write_dictionary_fname)
    {
        libCM_CreateDirectoryFromPath(settings.write_dictionary_fname->c_str());
        write_dictionary_file.reset(new std::ofstream(*settings.write_dictionary_fname));
    }
}

void SendBufferState::set_max_size(size_t size)
{
    max_size = size;
}

bool SendBufferState::can_add(size_t n_bytes)
{
    ByteBuffer const &buffer = compress->get();

    // Uz ted je zprava plna, nic se uz nevejde
    if (buffer.size() >= max_size)
        return false;

    // Prisla dlouha zprava, posleme ji samostatne priste
    if (input_size > 0 && buffer.size() + n_bytes + (delimiter ? 1 : 0) > max_size)
        return false;

    // Delku nasledujiciho bloku odhadneme podle toho predchoziho, a
    // uz se nam ten nasledujici blok nevejde.
    if (last_output_block_size && buffer.size() + *last_output_block_size > max_size)
        return false;

    return true;
}

size_t SendBufferState::get_optimal_message_size() const
{
    if (compression_enabled)
    {
        if (!last_output_block_size)
            return block_size;
        return *last_output_block_size;
    }
    else
    {
        return input_size <= max_size ? max_size - input_size : 0;
    }
}

// Nechavam tu nejakou rezervu pro pripad, kdyz by se to vubec
// nepodarilo zkomprimovat a zstd si jeste navic pridalo nejaky
// header. Snad to nebude zdroj chyb.
size_t SendBufferState::get_fragment_max_size() const
{
    size_t b_size = last_output_block_size ? *last_output_block_size : block_size;
    size_t min = std::min(max_size, b_size);
    if (min < 32)
        return 0;
    return min - 32;
}

bool SendBufferState::try_add_message(std::string const &msg)
{
    assert(msg.size() > 0);
    assert(msg.size() < 64*1024);

    bool is_good = can_add(msg.size());

    PRINT(5, "SendBufferState::try_add_message: %zu bytes, will_add=%s: %s:\n>>%s<<",
          msg.size(), is_good ? "yes" : "no", print_statistics().c_str(), msg.c_str());

    if (!is_good)
    {
        // assert(input_size != 0);
        // Tohle nevim, jestli delat flush, to chce vyzkouset.
        // compress->add_data({}, true);
        return false;
    }

    if (input_size == 0)
    {
        ByteBuffer &buffer = compress->get();
        assert(buffer.size() == 0);
        if (use_header)
        {
            if (compression_enabled)
            {
                if (frame_ended)
                    buffer.push_back('n');
                else
                    buffer.push_back('c');
            }
            else
            {
                buffer.push_back('t');
            }
        }
        frame_ended = false;
    }
    else
    {
        if (delimiter)
        {
            uint8_t c = *delimiter;
            compress->add_data(view_bytes(&c, &c+1), CompressFlushMode::None);
            input_size += 1;
        }
    }

    bool will_flush = input_size / block_size < (input_size + msg.size()) / block_size;

    PRINT(5, "SendBufferState:: will_flush=%d", will_flush);

    // Update input size
    ++input_message_count;
    input_size += msg.size();

    // Compress
    compress->add_data(view_bytes(msg), will_flush ? CompressFlushMode::Flush : CompressFlushMode::None);

    // Update statistics
    if (will_flush)
    {
        ByteBuffer const &buffer = compress->get();
        last_output_block_size = buffer.size() - output_block_begin;
        last_input_block_size = input_size - input_block_begin;
        output_block_begin = buffer.size();
        input_block_begin = input_size;
    }

    // Write dictionary
    if (write_dictionary_file || debug_file)
    {
        if (!current_input.empty() && delimiter)
            current_input.push_back(*delimiter);

        current_input.insert(current_input.end(), msg.begin(), msg.end());
        CM_ASSERT_EQ(current_input.size(), input_size);
    }

    return true;
}

void SendBufferState::reset_statistics()
{
    input_message_count = 0;
    input_size = 0;
    input_block_begin = 0;
    output_block_begin = 0;
    last_input_block_size = Nothing();
    last_output_block_size = Nothing();
}

std::string SendBufferState::print_statistics()
{
  return format("cm outsize=%6zu "
                "(max=%6zu) "
                "input_message_count=%4zu "
                "input_size=%6zu "
                "block_size=%6zu "
                "last_input_block_size=%6zu "
                "last_output_block_size=%6zu",
                compress->get().size(), max_size,
                input_message_count,
                input_size,
                block_size,
                last_output_block_size.get_or(0),
                last_output_block_size.get_or(0)
      );
}

bool SendBufferState::finish()
{
    assert(input_size > 0);
    compress->add_data({}, CompressFlushMode::Flush);
    bool is_good = compress->get().size() <= max_size;
    PRINT(5, "SendBufferState::finish ok=%s: %s",
          is_good ? "yes" : "no", print_statistics().c_str());

    if (is_good && debug_file)
    {
        SyncedWriter writer(*debug_file);
        writer << print_statistics() << "\n";
        writer.write((char const *)current_input.data(), current_input.size());
        writer << "\n";
        writer.flush();
    }

    reset_statistics();
    if (is_good)
    {
        if (last_interation_was_good && block_size < 4096)
            block_size *= 2;
        last_interation_was_good = true;
    }
    else
    {
        last_interation_was_good = false;
        if (block_size > 1)
            block_size /= 2;
    }

    if (is_good)
    {
        if (write_dictionary_file)
        {
            assert(write_dictionary_size < write_dictionary_max_size);
            size_t rem_size = write_dictionary_max_size - write_dictionary_size;
            if (current_input.size() > rem_size)
                current_input.resize(rem_size);
            write_dictionary_file->write((char const *)current_input.data(),
                                         current_input.size());
            write_dictionary_size += current_input.size();
            if (write_dictionary_size >= write_dictionary_max_size)
                write_dictionary_file = nullptr;
        }
    }

    current_input.clear();

    return is_good;
}

ByteBuffer &SendBufferState::get()
{
    return compress->get();
}

ByteBuffer const &SendBufferState::get() const
{
    return compress->get();
}

void SendBufferState::reset()
{
    frame_ended = true;
    compress->reset();
    current_input.clear();
    reset_statistics();
}

bool SendBufferState::empty() const
{
    return input_size == 0;
}
