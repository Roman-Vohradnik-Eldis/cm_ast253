#ifndef CM_IP_UDP_MESSAGE_HH_
#define CM_IP_UDP_MESSAGE_HH_

#include "cm_maybe.hh"
#include "cm_random.hh"
#include "weak_string.hh"
#include <cstdint>
#include "cm_utils.hh"

struct SessionId
{
    typedef uint16_t value_type;
    
    value_type value;

    static uint16_t max() { return 0xffff; };
    static SessionId parse(uint64_t value)
    {
        CHECK(value <= max(), "SessionId too big");
        return {(value_type)value};
    }
};


struct BootId
{
    typedef uint16_t value_type;
    
    value_type value;

    static value_type max() { return 0xff; };
    static BootId parse(uint64_t value)
    {
        CHECK(value <= max(), "BootId too big");
        return {(value_type)value};
    }
    static BootId random()
    {
        return {(value_type)(cm_get_random_value<value_type>() % max())};
    }
};

inline bool operator!=(BootId a, BootId b)
{
    return a.value != b.value;
}

// TODO:
struct SessionIdCounter
{
    uint64_t counter = cm_get_random_value<uint64_t>() % SessionId::max();

public:
    template <typename P>
    SessionId get_fresh(P const &is_ok)
    {
        for (;;)
        {
            SessionId id{(uint16_t)(counter % (uint64_t)SessionId::max())};
            ++counter;
            if (is_ok(id))
                return id;
        }
    }
};

inline bool operator==(SessionId a, SessionId b)
{
    return a.value == b.value;
}

inline bool operator<(SessionId a, SessionId b)
{
    return a.value < b.value;
}

// Client posle

struct ReceivedMessage
{
    char msg_kind; // 'i', 'a', 'm', 'p', or 'd'

    // Nasledujici polozky jsou nastavene, pokud jsou obsazeny v prislusnem typu zpravy
    Maybe<BootId> server_boot_id;
    Maybe<SessionId> session_id;

    // Zde jsou ulozeny msg_id, tak jak se posilaji, tj. modulo 2^16
    Maybe<int64_t> sender_msg_id;
    Maybe<int64_t> receiver_msg_id;

    Maybe<uint8_t> missing_count;

    // Data pro  typ 'i, 'a' nebo 'm': HASH nebo BODY
    WeakString data;

    ReceivedMessage(char kind) : msg_kind(kind) {}
};

std::string print_message(ReceivedMessage rm);

// Pokud by chybely nektere polozky, vyhodi std::logic:error
std::string write_message(ReceivedMessage rm);

// Vyhodi std::runtime_error, pokud je zprava ve spatnem formatu.
ReceivedMessage parse_message(WeakString buffer);

std::string gen_random_hash_str();

bool is_valid_hash_str(WeakString w);

std::string print_hex(uint64_t n, unsigned n_chars);
uint64_t read_hex(WeakString str);

// Message ID
//////////////////////////////////////////////////////////////////////

enum {
    msg_id_end = 0xffff,
    max_sent_messages = msg_id_end / 4,
};

int64_t convert_msg_id_to_abs(int64_t msg_id_rem, int64_t my_id_reference, char const *debug);

int64_t convert_msg_to_rem(int64_t msg_id);

#endif // CM_IP_UDP_MESSAGE_HH_
