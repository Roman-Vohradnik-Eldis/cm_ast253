#ifndef CM_MSG_BUFFER_HH_
#define CM_MSG_BUFFER_HH_

#include "cm_view.hh"
#include "cm_debug.hh"
#include "cm_maybe.hh"
#include "compress_base.hh"
#include <assert.h>
#include <memory>

struct SendBufferSettings
{
    bool use_compression = false;
    bool legacy_mode = false;
    int compression_level = 9;
    Maybe<std::vector<uint8_t>> dictionary;
    Maybe<std::string> debug_fname;
    Maybe<std::string> write_dictionary_fname;
    size_t write_dictionary_max_size = 32*1024;
};

// Postupne plnime tim, co chceme odeslat. Z toho se udela jedna
// zprava. Pamatujeme si stav mezi jednotlivymi zpravami
class SendBufferState
{
    size_t block_size = 4096;

    // Statistics
    size_t input_message_count = 0;
    size_t input_size = 0;
    size_t input_block_begin = 0;
    size_t output_block_begin = 0;
    Maybe<size_t> last_input_block_size;
    Maybe<size_t> last_output_block_size;

    bool last_interation_was_good = true;
    bool frame_ended = true;
    size_t max_size = 1000;
    Maybe<uint8_t> delimiter;
    bool compression_enabled = false;
    bool use_header;
    std::unique_ptr<CompressBase> compress;
    std::shared_ptr<SyncedFile> debug_file;

    // Write dictionary:
    // Enabled if `write_dictionary_file` is not null.
    std::unique_ptr<std::ostream> write_dictionary_file;
    size_t write_dictionary_max_size = 4*1000*1000;
    size_t write_dictionary_size = 0;

    // Tohle se pouziva pri zapisovani slovniku nebo pri zapisu
    // debugovaciho souboru.
    std::vector<uint8_t> current_input;

public:
    SendBufferState(Maybe<uint8_t> delimiter, SendBufferSettings const &settings, bool use_header);

    void set_max_size(size_t max_size);

    void reset();

    // Return true if the message was added. Otherwise, the buffer is
    // full.
    bool try_add_message(std::string const &msg);

    // Tohle je odhad, kolik mame pridat dat z fragmentovane zpravy
    size_t get_optimal_message_size() const;

    // Tohle je odhad, jestli mame zpravu fragmentovat, nebo ne.
    size_t get_fragment_max_size() const;

    // Return true if the the was created successfully.  Otherwise,
    // the user must call reset and try to fill the buffer again.
    bool finish();

    bool empty() const;

    ByteBuffer &get();

    ByteBuffer const &get() const;

private:
    void reset_statistics();
    bool can_add(size_t n_bytes);
    std::string print_statistics();
};

#endif // CM_MSG_BUFFER_HH_
