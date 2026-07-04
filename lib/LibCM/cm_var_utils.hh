#ifndef CM_VAR_UTILS_HH_
#define CM_VAR_UTILS_HH_

#include "cm_base_variable.hh"
#include "cm_base_partner.hh"

std::string print_var(CCmVariable const &v);
std::string print_var(VariableEvent const &v);
std::string print_var(CCmFilter const &f);
std::string print_var(CCmFiltersSet const &set);

template <typename List>
std::string print_list(List const &m)
{
    std::stringstream ss;
    bool first = true;
    ss << "[";
    CM_for (typename List::value_type const &v, m)
    {
        if (first)
            first = false;
        else
            ss << " ";
        ss << print_var(v);
    }
    ss << "]";
    return ss.str();
}

template <typename Map>
std::string print_map(Map const &m)
{
    std::stringstream ss;
    bool first = true;
    ss << "{";
    CM_for (typename Map::value_type const &p, m)
    {
        if (first)
            first = false;
        else
            ss << " ";
        ss << print_var(p.second);
    }
    ss << "}";
    return ss.str();
}

#endif // CM_VAR_UTILS_HH_
