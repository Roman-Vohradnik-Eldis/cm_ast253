#include "cm_queue.hh"
#include "cm_utils.hh"
#include "cm_var_utils.hh"
#include "cm_debug.hh"


// void CmQueue::erase(std::string const &name)
// {
//     find_and_erase_by_name(hisVariablesToSend, name);
//     hisVariablesToSend_map.erase(name);
// }

// void CmQueue::pop_front()
// {
//     hisVariablesToSend_map.erase(hisVariablesToSend.front().getName());
//     hisVariablesToSend.pop_front();
// }

// void CmQueue::push_back(CCmVariable &var)
// {
//     assert(var.getType() != '?');
//     assert(!contains(hisVariablesToSend_map, var.getName()));
//     hisVariablesToSend_map.insert(std::make_pair(var.getName(), var));
//     hisVariablesToSend.push_back(var);
//  }

void push_back_or_update(std::list<CCmVariable> &hisVariablesToSend, CCmVariable &var, uint8_t newPriority)
{
    assert(var.getType() != '?');    
    debug_printf("Queue: push_back_or_update: %s", print_var(var).c_str());
    // debug_printf("Insert %s to Queu", var.getName().c_str());
    // for (CCmVariable &v : hisVariablesToSend)
    //    debug_printf(" ", v.getName().c_str());
    //debug_printf("");

    std::string const &name = var.getName();

    // Pridej zaznam do hisVariablesToSend_map
    // if (var.isArray())
    // {
    //     var.EventFields.clear();
    //     VarMap::iterator it_his_to = hisVariablesToSend_map.find(name);
    //     if (it_his_to != hisVariablesToSend_map.end())
    //         var.addEventFields(&it_his_to->second.EventFields);
    // }
    var.check_consistency();

    // Vloz zaznam do hisVariablesToSend
    if (newPriority != CMLIB_VALUE_PRIORITY_UNDEF)
    {
        var.setPriority(newPriority);
        var.setFlagPriorityChanged(true);
    }
    // uint8_t Priority = var.getPriority();
    // if (Priority == 0 || Priority == 1)
    // {
    //     // Smaz promennou z fronty. Vloz novy zaznam co nejvic na zacatek
    //     // fronty, aby pred ni zustali pouze ty s <= prioritou.
    //     // TODO: sjednoceni EventFields
    //     abort();
    //     find_and_erase_by_name(hisVariablesToSend, name);
    //     assert(find_first_by_name(hisVariablesToSend.begin(), hisVariablesToSend.end(), name) == hisVariablesToSend.end());

    //     bool inserted = false;
    //     for (std::list<CCmVariable>::iterator it_s = hisVariablesToSend.begin(); it_s != hisVariablesToSend.end() && (!inserted); it_s++ )
    //     {
    //         if (it_s->getPriority() > Priority)
    //         {
    //             var.check_consistency();
    //             hisVariablesToSend.insert(it_s, var);
    //             inserted = true;
    //         }
    //     }
    //     if (!inserted)
    //     {
    //         var.check_consistency();
    //         hisVariablesToSend.push_back(var);
    //     }
    //     hisVariablesToSend_map[name] = var;
    // }
    // else
    // {
        // Pokud promenna je v seznamu, updatni ji. Pokud neexistuje, pridej ji
        // nakonec.
    bool found = false;
    for (std::list<CCmVariable>::iterator it_s = hisVariablesToSend.begin(); it_s != hisVariablesToSend.end(); it_s++)
    {
	if (it_s->getName() == name)
	{
	    debug_printf("Queue: push_back_or_update found: %s", print_var(*it_s).c_str());
	    found = true;
	    var.check_consistency();
	    CCmVariable new_var = var;
	    if (!new_var.EventFields.empty())
	    {
		// Kdyz se nemeni vsechno, tak sjednot EventFields s existujicim zaznamem
		if (it_s->EventFields.empty())
		    new_var.EventFields.clear();
		else
		    new_var.addEventFields(&it_s->EventFields);
	    }
	    *(it_s) = new_var;
	    // hisVariablesToSend_map[name] = new_var;
	    debug_printf("Queue: push_back_or_update new_value: %s", print_var(new_var).c_str());
	}
    }
    if (!found)
    {
	var.check_consistency();
	hisVariablesToSend.push_back(var); // ORDER BY TIME OF CHANGES
	// hisVariablesToSend_map[name] = var;
	debug_printf("Queue: not found, inserting: %s", print_var(var).c_str());
    }
    // }
    //debug_printf("Inserted %s, result Queu", var.getName().c_str());
    // for (CCmVariable &v : hisVariablesToSend)
    //    debug_printf(" ", v.getName().c_str());
    //debug_printf("");

    // for (CCmVariable &v : hisVariablesToSend)
    // {
    //     for (CCmVariable &w : hisVariablesToSend)
    //     {
    //         if (&v != &w)
    //             assert(v.getName() != w.getName());
    //     }
    // }
}

bool erase_variable_changes_from(CCmVariable &curr_val, CCmVariable &new_var)
{
    debug_printf("erase_variable_changes_from curr_val=%s, new_var=%s",
                 print_var(curr_val).c_str(),
                 print_var(new_var).c_str());
    assert(new_var.compareType(curr_val));
    if (curr_val.isArray())
    {
        if (new_var.EventFields.empty())
        {
            // Zmenila se cela hodnota, nic nemusime posilat
            debug_printf("erase_variable_changes_from: Completely erased");
            return false;
        }
        else
        {
            if (curr_val.EventFields.empty())
                fill_full_event_fields(curr_val);
            std::list<unsigned> new_event_fields;
            // new_event_fields = curr_val.EventFields setminus new_var.EventFields.
            CM_for (unsigned i, curr_val.EventFields)
            {
                if (std::find(new_var.EventFields.begin(), new_var.EventFields.end(), i) == new_var.EventFields.end())
                {
                    debug_printf("keeping event_field %u", i);
                    new_event_fields.push_back(i);
                }
                else
                {
                    debug_printf("erasing event_field %u", i);
                }
            }
            if (new_event_fields.empty())
            {
                // Zmenilo se uz vse, co jsme meli poslat.
		debug_printf("erase_variable_changes_from: all the event fields were updated");
                return false;
            }
            else
            {
                curr_val.EventFields = new_event_fields;
		debug_printf("erase_variable_changes_from: Assigned new EventFields");
                return true;
            }
        }
    }
    else
    {
        // Zmenila se cela hodnota
	debug_printf("erase_variable_changes_from: Completely erased");
        return false;
    }
}

template <typename Iter>
Iter cm_next(Iter it)
{
    ++it;
    return it;
}

void erase_variable_changes_from_queue(std::list<CCmVariable> &hisVariablesToSend, CCmVariable &v)
{
    assert(v.getType() != '?');
    // ScopedTimer sc("erase_variable_changes");
    debug_printf("Queue: erase_variable_changes: %s", print_var(v).c_str());
    std::string const &name = v.getName();
    DebugTimer t;
    std::list<CCmVariable>::iterator it = find_first_by_name(hisVariablesToSend.begin(), hisVariablesToSend.end(), name);

    if (it == hisVariablesToSend.end())
	return;
    
    // printf("find_first_by_name (size=%zu): %.3lf msec", hisVariablesToSend.size(), t.elapsed_msec_double());

    // assert(it != hisVariablesToSend.end());
    assert(find_first_by_name(cm_next(it), hisVariablesToSend.end(), name) == hisVariablesToSend.end());

    bool keep_var = erase_variable_changes_from(*it, v);
    if (!keep_var)
    {
	debug_printf("Queue: erase_variable_changes_from: erasing it: %s", print_var(v).c_str());
        hisVariablesToSend.erase(it);
    }
    else
    {
	debug_printf("Queue: erase_variable_changes_from: changed existing value to %s", print_var(*it).c_str());
        // hisVariablesToSend_map.at(name).EventFields = it->EventFields;
    }
}
