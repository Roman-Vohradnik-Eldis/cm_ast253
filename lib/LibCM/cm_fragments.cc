#include "cm_fragments.hh"
#include "cm_debug.hh"
#include "cm_test.hh"
#include "assert.h"

std::string FragmentsState::print() const
{
    return format("{%zu bytes, frags=%u, inited=%d, ended=%d}",
                  remaining_part.size(),
                  fragment_count,
                  was_inited,
                  was_ended);
}

unsigned FragmentsState::get_fragment_count() const
{
    return fragment_count;
}

FragmentsToSend::FragmentsToSend(WeakString variable, std::string const &msg, std::string debug_name)
    : variable_name(variable),
      message(new std::string(msg)),
      state(WeakString(message->begin(), message->end())),
      debug_name(debug_name)
{
}

FragmentsState FragmentsToSend::get_state() const
{
    return state;
}

std::string const &FragmentsToSend::get_variable_name() const
{
    return variable_name;
}

void FragmentsToSend::commit(FragmentsState const &uncommited_state)
{
    assert(uncommited_state.remaining_part.end() == state.remaining_part.end());
    size_t sent_size = state.remaining_part.size() - uncommited_state.remaining_part.size();
    // cms_ns2_if_print("cm_frag", debug_name, 3, "Commit %u fragments of variable %s: %zu bytes",
    //                  uncommited_state.fragment_count,
    //                  variable_name.c_str(),
    //                  sent_size);
    state = uncommited_state;
}

bool FragmentsToSend::is_completed() const
{
    return state.was_ended;
}

Maybe<std::string> FragmentsState::generate_message(unsigned max_size)
{
    if (was_ended)
    {
        // Vse uz jsme poslali
        assert(remaining_part.empty());
        return Nothing();
    }

    char empty_msg[] = "FRG_END,\"\"";
    if (max_size <= strlen(empty_msg))
    {
        // Do zpravy se nevejde ani hlavicka
        return Nothing();
    }

    // Vyhrad si misto na hlavicku
    max_size -= strlen(empty_msg);
    assert(max_size > 0);

    // Odrizni kus zpravy
    std::string body;
    if (!remaining_part.empty())
        body = escape_special_chars_and_advance(remaining_part, max_size);

    // Pokud je max_size mala, remaining_part je mala, je mozne, ze se
    // do ni kvuli escapovanym znakum se tam nic nevejde.
    if (body.empty() && !remaining_part.empty())
        return Nothing();

    char const *kind = was_inited ? "FRG_ADD" : "FRG_NEW";
    if (was_inited && remaining_part.empty())
    {
        was_ended = true;
        kind = "FRG_END";
    }
    was_inited = true;
    ++fragment_count;
        
    // Create message
    std::string msg;
    msg += kind;
    msg += ",\"";
    msg += body;
    msg += "\"";
    return msg;
}

void test_fragments_to_send()
{
    {
        printf("----------------------------\n");
        FragmentsToSend f("TEST", "0123456789012345678901234567890123456789", "main");
        FragmentsState fts = f.get_state();
        CM_TEST_EQ(fts.generate_message(21).get_or("error"), "FRG_NEW,\"0123456789\"");
        CM_TEST_EQ(fts.generate_message(21).get_or("error"), "FRG_ADD,\"0123456789\"");
        f.commit(fts);
        CM_TEST_EQ(fts.generate_message(21).get_or("error"), "FRG_ADD,\"0123456789\"");
        fts = f.get_state();
        CM_TEST_EQ(fts.generate_message(100).get_or("error"), "FRG_END,\"01234567890123456789\"");
        f.commit(fts);
        CM_TEST_EQ(f.is_completed(), true);
    }
    {
        printf("----------------------------\n");
        // Tady jsem zkousel escapovani
        FragmentsToSend f("TEST", "v=\"xx\"\nneco neco\nend", "main");
        FragmentsState fts = f.get_state();
        CM_TEST_EQ(f.is_completed(), false);
        CM_TEST_EQ(fts.generate_message(17).get_or("error"), "FRG_NEW,\"v=\\\"xx\"");
        f.commit(fts);
        CM_TEST_EQ(f.is_completed(), false);
        CM_TEST_EQ(fts.generate_message(17).get_or("error"), "FRG_ADD,\"\\\"\\nne\"");
        f.commit(fts);
        CM_TEST_EQ(fts.generate_message(30).get_or("error"), "FRG_END,\"co neco\\nend\"");
        CM_TEST_EQ(f.is_completed(), false);
        f.commit(fts);
        CM_TEST_EQ(f.is_completed(), true);
    }
    {
        printf("----------------------------\n");
        // Poslani zpravy najednou
        FragmentsToSend f("TEST", "neco", "main");
        FragmentsState fts = f.get_state();
        CM_TEST_EQ(fts.generate_message(50).get_or("error"), "FRG_NEW,\"neco\"");
        f.commit(fts);
        CM_TEST_EQ(fts.generate_message(50).get_or("error"), "FRG_END,\"\"");
        CM_TEST_EQ(f.is_completed(), false);
        f.commit(fts);
        CM_TEST_EQ(f.is_completed(), true);
    }
}
