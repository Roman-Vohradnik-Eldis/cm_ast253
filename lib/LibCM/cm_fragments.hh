#ifndef CM_FRAGMENTS_HH_
#define CM_FRAGMENTS_HH_

#include "cm_string.hh"
#include "cm_maybe.hh"
#include <memory>

// Nyni je to udelane tak, ze pokud fragmentujeme, tak posilame aspon
// dvez pravy: FRG_NEW, a pak FRG_END.

class FragmentsState
{
    WeakString remaining_part;
    unsigned fragment_count = 0;
    bool was_inited = false;
    bool was_ended = false;
    friend class FragmentsToSend;

public:
    FragmentsState(WeakString r) : remaining_part(r) {}

    Maybe<std::string> generate_message(unsigned max_size);

    // For debugging
    std::string print() const;

    // For debugging
    unsigned get_fragment_count() const;
};

// Fragmentujeme jen zpravy typu DEF,UNDEF,SET,MOD, takze vzdycky
// jde o zmenu promenne. V jeden okamzik muze byt takto
// fragmentovana pouze jedna promenna. Bud moje, nebo partnerova.
class FragmentsToSend
{
    std::string variable_name;

    // Zatim neescapovana zprava
    std::shared_ptr<std::string> message;

    FragmentsState state;

    std::string debug_name;

public:
    // msg: cela zprava, kterou chceme odeslat
    FragmentsToSend(WeakString variable, std::string const &msg, std::string debug_name);
    
    std::string const &get_variable_name() const;

    FragmentsState get_state() const;

    void commit(FragmentsState const &state);

    // Jestli bylo vsechno odeslane (a commitnute). Kdyz ano, muzeme
    // tento objekt zaniknout.
    bool is_completed() const;

private:
    char const *msg_kind() const;
};

void test_fragments_to_send();

#endif // CM_FRAGMENTS_HH_
