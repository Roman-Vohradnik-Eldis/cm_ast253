#ifndef CM_SEND_BUFFER_SIMPLE_HH_
#define CM_SEND_BUFFER_SIMPLE_HH_

#include <limits>
#include "cm_string_buffer.hh"
#include "compress_base.hh"
#include <memory>
#include "cm_send_buffer_state.hh"

std::string change_nonprintable(View<uint8_t> in);

class OutputBufferSimple
{
public:
    virtual ~OutputBufferSimple() {}

    // Vrati pocet bytu, ktere se do zpravy zarucene vejdou,
    // tj. `try_add_message` vrati true.  Pokud se pouziva komprese,
    // tak se jich tam muze vejit vic.
    //
    virtual size_t get_max_size() const = 0;

    // Posle se zprava tvaru `| kind | msg_header | body ... |`
    //
    // `kind` a `msg_header` se posilaji nekomprimovane.
    //
    // `body` se muze posilat komprimovane. Muze se poslat jenom jeho
    // cast, podle toho, kolik se vejde do zpravy. Pokud se vejde cele
    // `body`, pak jako `kind` je pouzit `complete_kind`, jinak je
    // pouzit `partial_kind`.
    //
    // Vrati -1, pokud se nepodarilo poslat nic.  Jinak vrati pocet
    // poslanych znaku z `body`. Pokud bylo poslano vsechno, je to
    // `body.size()`.
    //
    virtual ssize_t try_add_message(char partial_kind,
                                    char complete_kind,
                                    View<uint8_t> msg_header,
                                    View<uint8_t> body,
                                    size_t send_at_least) = 0;
};

struct SendBufferSimpleSettings
{
    // Nothing() -> bez komprese
    // int -> parametr pro zstd
    Maybe<int> compression_level;

    bool use_control_character;

    Maybe<std::string> debug_file_name;
};

inline SendBufferSimpleSettings make_default_simple_buffer_settings()
{
    return {
        {},
        true,
        {},
    };
}

class SendBufferSimple : public OutputBufferSimple
{
    // std::vector<char> input;
    bool use_control_character;
    bool begin_new_frame = true;
    std::unique_ptr<CompressBase> compress;
    std::shared_ptr<SyncedFile> debug_file;
    size_t max_size = 256;
    bool use_compression;
    size_t last_size_to_add = 1024;

public:
    SendBufferSimple(SendBufferSimpleSettings const &settings);

    size_t get_max_size() const override;

    // Typicky voláme před začátkem sestavování zprávy.
    //
    void set_max_size(size_t size);

    // Před začátkem sestavování zprávy musíme zavolat `reset`, aby se
    // vyčistil výstupní buffer. Pak můžeme zavolat `try_add_message`,
    // kolikrát chceme.
    //
    // Pokud se zpráva nepodaří přidat, nic se nedeje.  Muze to
    // znevalidnit iteratory ve vystupnim bufferu.  Pokud se zprava
    // povedla pridat aktualizuje se výstupní buffer, uživatel ho hned
    // může použít.
    //
    ssize_t try_add_message(char partial_kind,
                            char complete_kind,
                            View<uint8_t> msg_header,
                            View<uint8_t> body,
                            size_t send_at_least) override;

    // Pro znovupouziti objektu.
    //
    void reset();

    void clear();

    // Kdyz skoncime pridavani vstupnich zprav, tak odtud si muzeme
    // zkopirovat vystup.
    //
    // Pokud jsme od posledniho resetu nepridali zadnou zpravu, je
    // vystup prazdny.
    //
    // Jinak je zde kompletni vystup obsahujici vsechny pridane zpravy
    // od posledniho resetu.
    //
    ByteBuffer const &get() const;
};

#endif // CM_SEND_BUFFER_SIMPLE_HH_
