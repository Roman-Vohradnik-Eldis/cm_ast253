#include "cm_db_query_state.hh"

bool can_query_be_erased(MyQueryState const &q)
{
    if (q.response_status == ResponseStatus::Running)
        return false;

    // TODO: Nema se tohle testovat jenom pro ty, ktere nejsou Completed?
    if (q.query_status == QueryStatus::NeedSendCancel)
        return false;

    if (q.is_forwarded_query)
    {
        switch (q.response_status)
        {
        case ResponseStatus::Running: return false; // Odpoved jeste neprisla cela
        case ResponseStatus::Completed: return q.response_buffer.empty();
        case ResponseStatus::Aborted: return false; // Uzivatel si ji musi vyzvednout
        case ResponseStatus::Cancelled: return true; // Uzivatel si uz vyzvedl chybovou zpravu
        }
        assert(false);
    }
    else
    {
        if (q.response_status == ResponseStatus::Cancelled)
            return true;  // Uzivatel uz nechce odpoved

        // Jinak ji chce:
        for (MyReceivedResponse const &r : q.received_responses)
        {
            if (!r.was_read)
                return false;
        }
        return true;
    }
}

