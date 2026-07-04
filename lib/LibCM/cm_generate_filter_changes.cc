#include "cm_generate_filter_changes.hh"
#include "weak_string.hh"
#include "cm_send_buffer_state.hh"
#include "cm_debug.hh"
#include <set>

inline bool has_prefix(std::string const &str, std::string const &prefix)
{
    if (str.size() < prefix.size())
    {
        // Pozn: cheme, jeste aby za prefixem neco bylo
        return false;
    }
    return WeakString(str.begin(), str.begin() + prefix.size()) == prefix;
}

bool need_send(CCmFilter const &filter, FilterParams const &params)
{
            
    std::string const &str_to_use = filter.toString();
    bool flagPattern = filter.getFlagPattern();
    if (params.myVariables.find(filter.getName()) != params.myVariables.end())
    {
        // Moje promenna
        return false;
    }

    if ((!flagPattern) && (params.onlyPrefix.size()>0) && !has_prefix(str_to_use, params.onlyPrefix))
    {
        // Nemuze matchovat zadnou jeho promennou
        return false;
    }
    return true;
}    

size_t count_variables(std::map<std::string, CCmFilter> const &map, FilterParams const &params)
{
    size_t count = 0;
    for (auto &p : map)
    {
        if (need_send(p.second, params))
            ++count;
    }
    return count;
}

struct SetToSend
{
    std::map<std::string, CCmFilter> set;
    std::string name;
    size_t n_variables_to_send;
};

enum class MessageStatus
{
    full,
    partial,
    cannot_fit,
    too_long,
};

bool is_good_filter(std::string const &str)
{
    for (char c : str)
        if (!(isalnum(c) || c == '_' || c == '?' || c == '*'))
            return false;
    return true;
}

MessageStatus try_add_changes(SendBufferState &buffer, CCmFiltersSet &dst, FilterParams const &params, SetToSend &set)
{
    std::set<std::string> prepared_filters;
    for (auto it = set.set.begin(); it != set.set.end(); ++it)
    {
        assert(is_good_filter(it->first));
        std::string str_to_use = set.name;
        str_to_use += ",";
        if (set.name == "FILT_UNDEF")
            str_to_use += it->second.getName();
        else
            str_to_use += it->second.toString();

        if (need_send(it->second, params))
        {
            bool ok = buffer.try_add_message(str_to_use);
            if (ok)
            {
                prepared_filters.insert(it->first);
            }
            else
            {
                // Tady samozrejme muzeme cyklus dojet do konce, ale
                // filtry jsou +- stejne dlouhy, a kdyz se nevesel
                // tenhle, tak se ve vetrisne pripadu nevejde ani
                // zadny jiny.
                break;
            }
            
        }
    }

    if (prepared_filters.empty())
    {
        // Mame neco k odeslani, ale nevejde se tam vubec nic...
        return MessageStatus::cannot_fit;
    }

    bool ok = buffer.finish();
    if (!ok)
    {
        // Tak jsem tam toho dali moc. Uzivatel to bude muset zavolat jeste jednou
        buffer.reset();
        return MessageStatus::too_long;
    }

    // Udelame commit
    if (set.name == "FILT_DEF")
        for (std::string const &v : prepared_filters)
            dst.add(set.set.at(v));
    else if (set.name == "FILT_UNDEF")
        for (std::string const &v : prepared_filters)
        {
            std::cout << "UNDEF: " << v.c_str() << std::endl;
            dst.erase(v.c_str());
        }
    else if (set.name == "FILT_CHG")
        for (std::string const &v : prepared_filters)
            dst.change(set.set.at(v));
    else
        assert(false);

    if (prepared_filters.size() < set.n_variables_to_send)
        return MessageStatus::partial;

    return MessageStatus::full;
}

// Mohly by byt const, ale bohuzel se v CCmFiltersSet cachuji nejake vysledky...
void init_sets_to_send(SetToSend (&all_sets)[3], CCmFiltersSet &src, CCmFiltersSet &dest,
                       FilterParams const &params)
{
    CCmFiltersSet CreatedSet;
    CCmFiltersSet DeletedSet;
    CCmFiltersSet ChangedSet;
    src.diff(dest, &CreatedSet, &DeletedSet, &ChangedSet, true, false);

    all_sets[0].set = CreatedSet.getMapAll();
    all_sets[1].set = DeletedSet.getMapAll();
    all_sets[2].set = ChangedSet.getMapAll();

    all_sets[0].name = "FILT_DEF";
    all_sets[1].name = "FILT_UNDEF";
    all_sets[2].name = "FILT_CHG";

    all_sets[0].n_variables_to_send = count_variables(all_sets[0].set, params);
    all_sets[1].n_variables_to_send = count_variables(all_sets[1].set, params);
    all_sets[2].n_variables_to_send = count_variables(all_sets[2].set, params);
}
    
// _src: Novy filtrset
// _dst: Puvodni filtrset
bool CompareFiltersAndGenerateChangeMessage(SendBufferState &buffer,
                                            CCmFiltersSet &src,
                                            CCmFiltersSet &dst,
                                            FilterParams const &params)
{
    SetToSend all_sets[3];
    init_sets_to_send(all_sets, src, dst, params);

    unsigned total_variables_to_send = 0;
    for (unsigned i = 0; i < 3; ++i)
        total_variables_to_send += all_sets[i].n_variables_to_send;

    if (total_variables_to_send == 0)
    {
        // Nothing to send
        return false;
    }

    // Vybere jeden set, ktery se bude posilat
    SetToSend *selected_set = nullptr;
    for (unsigned i = 0; i < 3; ++i)
    {
        if (all_sets[i].n_variables_to_send > 0)
        {
            selected_set = &all_sets[i];
            break;
        }
    }

    DebugTimer t;
    for (int tries = 1; ; ++tries)
    {
        assert(buffer.empty());
        MessageStatus status = try_add_changes(buffer, dst, params, *selected_set);
        assert(buffer.empty());

        double elapsed = t.elapsed_msec_double();
        if (elapsed > 1000)
            cms_ns_if_print("libcm", 4, "GenerateFilterChanges: %.3f msec, %d tries", elapsed, tries);
        
        switch (status)
        {
        case MessageStatus::full:
            return selected_set->n_variables_to_send < total_variables_to_send;
        case MessageStatus::partial:
            return true;
        case MessageStatus::cannot_fit:
            cms_ns_if_print("libcm", 1, "Cannot generate filter changes, buffer too small");
            return true;
        case MessageStatus::too_long:
            // Try again
            break;
        }
    }
}
