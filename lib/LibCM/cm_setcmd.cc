#include "cm_setcmd.hh"
#include "cm_debug.hh"

#include <algorithm>
#include "cm_utils.hh"

void fill_full_event_fields(std::list<unsigned> &EventFields, unsigned y_size, unsigned x_size)
{
    EventFields.clear();
    for (unsigned i = 0; i < x_size * y_size; ++i)
        EventFields.push_back(i);
}

bool CmSetCmdMap::is_safe_to_apply(std::string name, unsigned received_id)
{
    SetCmdStatus &st = m_map[name];
    unsigned min = st.get_min_value();
    return min <= received_id;
}

bool CmSetCmdMap::diff_event_fields(std::string name, unsigned y_size, unsigned x_size, std::list<unsigned> &EventFields, unsigned received_id)
{
    assert(contains(m_map, name));
    SetCmdStatus &st = m_map.at(name);

   // debug_printf("diff SetCmdStatus %", v.getName().c_str());
   //  for (auto &p : st.setcmd_ids)
   //     debug_printf(" [%u,%u]=", p.first.first, p.first.second, p.second);
   // debug_printf("");
   //  fflush(stdout);

    if (y_size > 1 || x_size > 1)
    {
        std::list<unsigned> new_event_fields;
        if (EventFields.empty())
            fill_full_event_fields(EventFields, y_size, x_size);
        CM_for (unsigned i, EventFields)
        {
            unsigned y = i % y_size;
            unsigned x = i / y_size;
            // assert(contains(st.setcmd_ids, std::make_pair(y, x)));
            // unsigned id = st.setcmd_ids.at(std::make_pair(y, x));
	    unsigned id = st.get_index(y, x);
            if (id <= received_id) // TODO: modulo n.
            {
                new_event_fields.push_back(i);
		cms_ns_if_print("libcm", 22, "keeping  index [%u,%u]: field_id=%u, received=%u", y, x, id, received_id);
            }
            else // (received_id < id <= n)
            {
		cms_ns_if_print("libcm", 22, "ignoring index [%u,%u]: field_id=%u, received=%u", y, x, id, received_id);
            }
        }
        EventFields = new_event_fields;
        return !EventFields.empty();
    }
    else
    {
        // assert(contains(st.setcmd_ids, std::make_pair(0, 0)));
        // return st.setcmd_ids.at(std::make_pair(0, 0)) == received_id;
	return st.get_index(0,0) == received_id;
    }
}

void CmSetCmdMap::update_fully(std::string name, unsigned id)
{
    SetCmdStatus &st = m_map[name];
    st.set_all(id);
}


void CmSetCmdMap::update_from_variable(std::string name, unsigned y_size, unsigned x_size, std::list<unsigned> const &EventFields, unsigned id)
{
    // Get or create Status
    SetCmdStatus &st = m_map[name];

    // st.last_setcmd_id = id;
    for (unsigned i = 0; i < x_size * y_size; ++i)
    {
        unsigned y = i % y_size;
        unsigned x = i / y_size;

        if (EventFields.empty() || std::find(EventFields.begin(), EventFields.end(), i) != EventFields.end())
        {
	    //setcmd_ids[std::make_pair(y, x)] = st.last_setcmd_id;
	    st.set_index(y, x, id);
        }
    }
   // debug_printf("updated SetCmdStatus %", v.getName().c_str());
   //  for (auto &p : st.setcmd_ids)
   //     debug_printf(" [%u,%u]=", p.first.first, p.first.second, p.second);
   // debug_printf("");
};

// unsigned CmSetCmdMap::get_last_id(std::string const &var_name) const
// {
//     assert(contains(m_map, var_name) && m_map.at(var_name).was_id_assigned);
//     return m_map.at(var_name).last_setcmd_id;
// }

unsigned CmSetCmdMap::get_next_id(std::string const &name) const
{
    if (SetCmdStatus const *st = map_find(m_map, name))
        return st->get_max_value() + 1;
    else
        return 0;
}

