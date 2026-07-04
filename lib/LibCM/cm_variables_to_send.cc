#include "cm_variables_to_send.hh"
#include "cm_utils.hh"
#include "cm_var_utils.hh"

size_t MyVariablesToSend::size() const
{
    return variables.size();
}

MyVariableToSend const *MyVariablesToSend::find_variable(std::string const &name) const
{
    return map_find(variables, name);
}

void MyVariablesToSend::full_define_variable(std::string const &name, CCmVariable const &var)
{
    // cms_ns_if_print("libcm", 3, "full_define_variable: push_back=%s", print_var(var).c_str());
    variables.erase(name);
    CCmVariable copy = var;
    copy.EventFields.clear();
    variables.insert({name, {{copy}, CmEventWhat::define, var.getPriority(), 0}});
}

bool MyVariablesToSend::update_existing_erase_event_fields(std::string const &name, CCmVariable const &var)
{
    MyVariableToSend *vts = map_find(variables, name);
    if (!vts)
    {
        return false;
    }    
    assert(name == var.getName());
    assert(vts->value);
    assert(vts->EventWhat == CmEventWhat::define || vts->EventWhat == CmEventWhat::update);
    assert(vts->value->compareTypeAndLimits(var));

    // TODO: Lepsi by bylo erase event fields. Navic, pokud by se
    // uplne odecely, tak by se promenna mohla z fronty uplne vyhodit.
    CCmVariable copy = var;
    copy.EventFields = vts->value->EventFields;
    vts->value = copy;
    return true;
}

bool MyVariablesToSend::update_existing(std::string const &name,
                                        CCmVariable const &var,
                                        Maybe<uint8_t> newPriority,
                                        double SendBlockUntil)
{
    assert(name == var.getName());
    MyVariableToSend *vts = map_find(variables, name);

    if (!vts)
    {
        // cms_ns_if_print("libcm", 3, "update_existing: push_back=%s", print_var(var).c_str());
        variables.insert({name, {{var}, CmEventWhat::update, var.getPriority(), SendBlockUntil}});
        return true;
    }
    assert(vts->value);
    assert(vts->EventWhat == CmEventWhat::define || vts->EventWhat == CmEventWhat::update);
    assert(vts->value->compareTypeAndLimits(var));
    assert(vts->SendBlockUntil == 0 || vts->SendBlockUntil == SendBlockUntil);

    // Pripravime novou promennou
    CCmVariable copy = var;
    copy.EventFields.clear();

    if (var.EventFields.empty() || vts->value->EventFields.empty())
    {
        // Bude se menit cela promenna
    }
    else
    {
        // Add event fields where the value differ
        copy.diffAllFields(*vts->value);

        std::set<unsigned> ev(copy.EventFields.begin(), copy.EventFields.end());

        // Add original fields
        ev.insert(vts->value->EventFields.begin(), vts->value->EventFields.end());

        // Add new fields
        ev.insert(var.EventFields.begin(), var.EventFields.end());

        // Remove duplicates
        copy.EventFields.clear();
        copy.EventFields.insert(copy.EventFields.end(), ev.begin(), ev.end());
    }
    // cms_ns_if_print("libcm", 3, "update_existing: prev=%s, add=%s, result=%s", 
    //                 print_var(*vts->value).c_str(),
    //                 print_var(var).c_str(),
    //                 print_var(copy).c_str());
    vts->value = copy;
    return true;
}

bool MyVariablesToSend::undefine(std::string const &name)
{
    if (contains(variables, name))
        variables.erase(name);
    variables.insert({name, {{}, CmEventWhat::undefine, 0, 0}});
    return true;
}

void MyVariablesToSend::erase_variable(std::string const &name)
{
    variables.erase(name);
}

void MyVariablesToSend::erase_variables(std::vector<size_t> const &positions)
{
    auto it = variables.begin();
    size_t i = 0;
    for (size_t pos : positions)
    {
        assert(i <= pos);
        while (i < pos)
        {
            ++it;
            ++i;
        }
        it = variables.erase(it);
        ++i;
    }
}
