#include "cm_base_partner.hh"
#include "cm_generate_changes.hh"
#include "cm_string.hh"
#include "cm_std_string.hh"
#include "cm_utils.hh"
#include "cm_var_utils.hh"
#include "weak_string.hh"
#include "cm_debug.hh"


// ==========================================================================================



void CCmPartner::init(double ActTime, unsigned connection_id)
{
    received_init_acc = false;
    received_init = false;
    sent_init_acc = false;
    sent_init = false;

    master_slave_control_variable_value = false;
    
  ID = connection_id;
  // init_flags = (1<<0) | (1<<1); // init bits : 0:send my init, 1:wait receive partner init
  init_wait_until = ActTime;

  ProtocolVersion = 0;
  UsedVersion = 0;
  ProtocolFlags = 0;

  RecACCpacket_no = 0;
  DisconnectMe = false;

  TimeLastSent = 0.0;
  SendBlockUntil = 0.0;
  TimeLastRecv = ActTime;
  // FiltersChanged = false;
  PartnerFiltersChanged = false;
  myFiltersChange = false;
  waitingPriorityMessage = false;
  PartnerVarPrefix = std::string("");
  MyVarPrefix = std::string("");

  PartnerFilters.set_smart(true);

}

bool CCmPartner::ClearFilters(void)
{
  if (!PartnerFilters.empty())
  {
    PartnerFilters.clear();
    // FiltersChanged = true;
    return true;
  }
  return false;
}

bool CCmPartner::UnDefineFilter(const char *name)
{
//  size_t origosize = PartnerFilters.size();
  bool changed = PartnerFilters.erase(name);
//  bool changed = (origosize != PartnerFilters.size());
  // if (changed) FiltersChanged = true;
  return changed;
}


bool CCmPartner::DefineFilter(const char *name)
{
  bool changed = PartnerFilters.add(name);
  // if (changed) FiltersChanged = true;
  return changed;
}

bool CCmPartner::ChangeFilter(const char *name)
{
  bool changed = PartnerFilters.change(name);
  // if (changed) FiltersChanged = true;
  return changed;
}


// Tohle volame, pokud si partner zmeni filtry:
//
// Pripravime mu nase promenne, ktere doposavad nechtel, a po zmene
// filtru je chce.
//
bool CCmPartner::MergeMyVariablesByChangedFilters(VarMap const *myWorkerVariables)
{
    ThreadTimer timer;
    bool new_data_to_send = false;
    // projedeme jen ty, co jsou definovany parametrem - ostatni byly zruseny jinak, nebo cekaji ve fronte
    for (VarMap::const_iterator it_worker=myWorkerVariables->begin(); it_worker!=myWorkerVariables->end(); it_worker++)
    {
        std::string name = MyVarPrefix + it_worker->first;
        
        CCmVariable const &var = it_worker->second;
        assert(var.EventFields.empty());
        if (var.haveFlagExternal())
        {
            if (PartnerFilters.test(name))
            {
                if (ChangeMyVariable(var))
                {
                    // erase all undef/change requests ,, change request to DEF or insert new DEF
                    new_data_to_send = true;
                }
            }
            else
            {
                // Promennou nechce, mozna ji zna, mozna ne.
                if (UndefineMyVariable(name))
                    new_data_to_send = true;
            }
        }
    }
    int64_t elapsed = timer.elapsed_usec();
    if (elapsed >= 1000)
    {
        cms_ns2_if_print("libcm", libCM_name, 3, "MergeMyVariablesByChangedFilters took %.3f msec", elapsed/1000.0);
    }
    return new_data_to_send;
}



bool CCmPartner::UndefineMyVariable(std::string const &name)
{
    bool new_data_to_send = false;

    // Zmaz ve fronte vsechny zaznamy teto promenne
    variables_state.myVariablesToSend.erase_variable(name);

    // Koukni, jestli jsme mu vubec posilali definici
    if (contains(variables_state.myVariables_State, name))
    {
        variables_state.myVariablesToSend.undefine(name);
        new_data_to_send = true;
    }
    return new_data_to_send;
}

template <typename Iter>
Iter cm_next(Iter it)
{
    ++it;
    return it;
}

// // Vrati true, pokud bude k dispozici neco noveho k odeslani.
// bool CCmPartner::ChangeMyVariableDoUpdate
// (std::string const &name, CCmVariable *var, bool full_define, bool dont_send, uint8_t newPriority)
// {
//     // Invariant inv(std::bind(&CCmPartner::debug, this), "ChangeMyVariableDoUpdate: " + print_var(*var) + " " + cm_tostring(dont_send));
//     CmEventWhat EventWhat = CmEventWhat::update;
//     if (full_define)
// 	EventWhat = CmEventWhat::define;
//     // int EventWhat = 2; // changed
//     // if (full_define)
//     // 	EventWhat = 1; // define
//     // var->EventWhat = 2; // changed
//     // if (full_define)
//     //     var->EventWhat = 1; // define

//     if (dont_send)
//     {
// 	cms_ns_if_print("libcm", 22, "ChangeMyVariableDoUpdate: dont_send: %s", print_var(*var).c_str());
// 	// Pokud zmenu provedl tento partner:
//         variables_state.myVariables_State[name] = *var;
// 	variables_state.myVariables_State[name].EventFields.clear();

//         VariableToSend.
        
//         std::list<VariableEvent>::iterator it = find_first_by_variable_name(variables_state.myVariablesToSend.begin(), variables_state.myVariablesToSend.end(), name);
//         if (it != variables_state.myVariablesToSend.end())
//         {
//             assert(find_first_by_variable_name(cm_next(it), variables_state.myVariablesToSend.end(), name) == variables_state.myVariablesToSend.end());

// 	    if (EventWhat == CmEventWhat::define) // Define
//             {
//                 variables_state.myVariablesToSend.erase(it);
//             }
//             else
//             {
//                 bool keep = erase_variable_changes_from(it->variable, *var);
//                 if (!keep)
//                 {
//                     debug_printf("Erasing fully");
//                     variables_state.myVariablesToSend.erase(it);
//                 }
//             }
//         }
//     }
//     else
//     {
// 	cms_ns_if_print("libcm", 22, "ChangeMyVariableDoUpdate: will send: %s", print_var(*var).c_str());
// 	// Zmenu jsme provedli my, takze:
// 	// - myVariables_State nechame tak, jak je.
// 	// - Zmeny pridame do fronty (var ma nastavene EventFields).
//         if (newPriority != CMLIB_VALUE_PRIORITY_UNDEF)
//         {
//             var->setPriority(newPriority);
//             var->setFlagPriorityChanged(true);
//         }
//         uint8_t Priority = var->getPriority();

// 	// Vymaz z fonty delete teto promenne, pokud tam je.


// 	// Vymaz z fronty vsechno ohledne teto promenne.
//         for (std::list <VariableEvent>::iterator it_w=variables_state.myVariablesToSend.begin(); it_w!=variables_state.myVariablesToSend.end(); )
//         {
//             std::list <VariableEvent>::iterator it_d = it_w++;
//             if (it_d->variable.getName() == name)
//             {
//                 // if (it_d->EventWhat == CmEventWhat::undefine) // delete
// 		// {
// 		    variables_state.myVariablesToSend.erase(it_d);
// 		// }
//             }
//         }
// 	var->EventFields.clear();
// 	// Najdeme ve fronte create/update teto promenne.  Pokud tato
// 	// zmena, ktera uz je ve fronte, ma moc vysoke prioritni
// 	// cislo, tak ji z fronty vyhodime a sloucime ji z tou nasi
// 	// vkladanou zmenou. Pokud ma prioritu dobrou, tak tam pridame
// 	// tu nasi zmenu a tim koncime.
//         bool found = false;
//         // for (std::list<VariableEvent>::iterator it_w=myVariablesToSend.begin(); it_w!=myVariablesToSend.end(); )
//         // {
//         //     std::list<VariableEvent>::iterator it_d = it_w++;
//         //     if (it_d->variable.getName() == name)
//         //     {
//         //         if ((!found) && ((it_d->variable.getPriority() == Priority)))
//         //         {
//         //             found = true;
// 	// 	    // Pokud ta nova zmena je redefinice, tak
// 	// 	    // promennou ve fronte celou nahradime.
//         //             if (EventWhat == CmEventWhat::update)
//         //             {
//         //                 if ((var->EventFields.size()>0)&&(it_d->variable.EventFields.size()>0))
//         //                 {
//         //                     var->addEventFields(&it_d->variable.EventFields);
//         //                 }
// 	// 		else
// 	// 		{
// 	// 		    var->EventFields.clear();
// 	// 		}
//         //             }
//         //             *(it_d) = VariableEvent(EventWhat, *var);
//         //         }
// 	// 	else
// 	// 	{
//         //             myVariablesToSend.erase(it_d);
//         //         }
//         //     }
//         // }
// 	// Tohle nastava, pokud ve fronte nebyl zadny create/update,
//         // nebo tam byl, a my jsme ho vyhodili podle predchoziho
//         // odstavce. Novou zmenu vlozime tak, aby promenne byly ve
//         // fronte serazeny od nejmensiho prioritniho cisla po
//         // nejvetsi.
//         if (!found)
//         {
//             // bool inserted = false;
//             // for (std::list<VariableEvent>::iterator it_w=myVariablesToSend.begin(); it_w!=myVariablesToSend.end(); it_w++)
//             // {
//             //     if (it_w->variable.getPriority() > Priority)
//             //     {
//             //         myVariablesToSend.insert(it_w, VariableEvent(EventWhat, *var));
//             //         inserted = true;
// 	    // 	    break;
//             //     }
//             // }
//             // if (!inserted)
//             // {
// 	    cms_ns_if_print("libcm", 22, "ChangeMyVariableDoUpdate: inserting: %s", print_var(*var).c_str());
// 	    variables_state.myVariablesToSend.push_back(VariableEvent(EventWhat, *var));
// 		// }
//         }
//         // Priority 0, 1
//         if (Priority == 0)
// 	    waitingPriorityMessage = true;
// 	return true;
//     }
//     return false;
// }

// TOHLE tam mel Roman rozdelane....
// //             if (var->diffFields(var_orig)) // rebuild_EventFields
// //        if (var_orig.diffFields(*var)) // rebuild_EventFields
// //        if (var->diffFields(var_orig)) // rebuild_EventFields
//             {
//                 bool useNewDiff = false;
//                 if (useNewDiff)
//                 {
//                     if ((var->isArray())&&(filters_index_array.size()>0)&&(var->EventFields.size()>0)) // && version==2
//                     {

//                         libCM_reduce_EventFields_by_IndexArray(var->getArrayY(), var->getArrayX(), &var->EventFields, &filters_index_array);
//                         if (var->EventFields.size()>0)
//                         {
//                             different = true;
// //               debug_printf("POTVRZEN FLAG SEND \"%s\" fields=%d", name.c_str(), (int)var->EventFields.size());
//                         } else {
//                             different = false;
// //               debug_printf("SHOZEN FLAG SEND \"%s\" - nikoho to nezajima", name.c_str());
//                         }
//                     } else {
//                         different = true;
//                     }
//                 }
//                 else
//                 {
//                     // oldDiff - send all without respect filter indexes
//                     different = true;
//                 }
//             } else {
//                 different = false;
// //          different = true; // PRACOVNE ... nutno porovnavat se stavem, jaky BUDE !!!!
//                 // same value...ss
//             }

// Mame nove zmeny. Ptame se, jestli partner, az dostane zmeny, ktere
// uz mame ve fronte, tyhle nase nove zmeny bude vubec potrebovat.
//
// v_new: Moje nova zmena (pouziva EventFields).
// orig: To, co ted ma partner.
// myVariablesToSend: To, co se mu bude posilat.
// enum DiffResult
// {
//     DiffNoNeed,
//     DiffDifferent,
//     DiffFullRedefine,
// };

// DiffResult doesModificationChangeVariable(CCmVariable const v_new,
//                                           CCmVariable const &orig,
//                                           MyVariablesToSend const &myVariablesToSend)
// {
//     CCmVariable var_orig = orig; // copy
//     var_orig.EventFields.clear();

//     assert(v_new.getName() == orig.getName());

//     if (VariableToSend const *vts = myVariablesToSend.find_variable(orig.getName()))
//     {
//         if (it_w->EventWhat != CmEventWhat::undefine) // not undefine
//         {
//             // Pozn: Pokud jsou EventFields prazdne, zkopiruje se vsechno.
//             var_orig.copy_indexes(it_w->variable, it_w->variable.EventFields);
//         }
//         else
//         {
//             // Ve fronte je oddefinice, takze musime udelat definici.
//             // Rovnou vratime
//             return DiffFullRedefine;
//         }        
//     }

//     CCmVariable v_new_copy = v_new;
//     bool different = v_new_copy.diffFields(var_orig); // diff vrati true, pokud jsou promenne ruzne.
//     return different ? DiffDifferent : DiffNoNeed;
// }

// std::string diff_result_string(DiffResult r)
// {
//     switch (r)
//     {
//     case DiffNoNeed: return "DiffNoNeed";
//     case DiffDifferent: return "DiffDifferent";
//     case DiffFullRedefine: return "DiffFullRedefine";
//     default: return "Error";
//     }
// }

bool check_variable(PartnerVariablesState const &variables_state, std::string name, CCmVariable const &var)
{
    MyVariableState const *state = map_find(variables_state.myVariables_State, name);
    return state && state->type.compareTypeAndLimits(var);
}

// Vrati true, bude k dispozici neco noveho k odeslani.
bool CCmPartner::AddMyVariableModification(CCmVariable const &var)
{
    // cms_ns2_if_print("libcm", libCM_name, 3, "AddMyVariableModification: %s", print_var(var).c_str());
    std::string const &name = var.getName();

    if (MyVariableToSend const *vts = variables_state.myVariablesToSend.find_variable(name))
    {
        // Mame ji ve fronte, partner ji bud zna nebo nezna
        if (vts->value->compareTypeAndLimits(var))
        {
            return variables_state.myVariablesToSend.update_existing_erase_event_fields(name, var);
        }
        else
        {
            // Tohle by se nemelo stat
            abort();
        }
    }
    else
    {
        // Neni nic potreba delat
        assert(check_variable(variables_state, name, var));
        return false;
    }
}

// Vrati true, bude k dispozici neco noveho k odeslani.
bool CCmPartner::ChangeMyVariable(CCmVariable const &var, Maybe<uint8_t> newPriority)
{
    // cms_ns2_if_print("libcm", libCM_name, 3, "ChangeMyVariable: %s", print_var(var).c_str());
    std::string const &name = var.getName();

    if (!PartnerFilters.test(name, MyVarPrefix))
    {
        // partner ji nechce
        return false;
    }

    if (MyVariableToSend const *vts = variables_state.myVariablesToSend.find_variable(name))
    {
        // Mame ji ve fronte, partner ji bud zna nebo nezna
        if (vts->value && vts->value->compareTypeAndLimits(var))
        {
            MyVariableState const *state = map_find(variables_state.myVariables_State, name);
            double SendBlockUntil = state ? state->SendBlockUntil : 0;
            return variables_state.myVariablesToSend.update_existing(name, var, newPriority, SendBlockUntil);
        }
        else
        {
            variables_state.myVariablesToSend.full_define_variable(name, var);
            return true;
        }
    }
    else
    {
        if (MyVariableState const *state = map_find(variables_state.myVariables_State, name))
        {
            // Partner promennou jiz zna
            if (state->type.compareTypeAndLimits(var))
            {
                return variables_state.myVariablesToSend.update_existing(name, var, newPriority, state->SendBlockUntil);
            }
            else
            {
                variables_state.myVariablesToSend.full_define_variable(name, var);
                return true;
            }
        }
        else
        {
            // Partner promennou nezna
            variables_state.myVariablesToSend.full_define_variable(name, var);
            return true;
        }
    }
}

std::string CCmPartner::debug()
{
    std::stringstream ss;
    ss << "hisVariables=" << print_map(variables_state.hisVariables) << ", ";
    ss << "his_variables_to_send=" << print_list(variables_state.his_variables_to_send);
    // ss << "myVariables_State=" << print_map(variables_state.myVariables_State);
    // ss << "myVariablesToSend=" << print_list(variables_state.myVariablesToSend);
    return ss.str();
}

// Ignoruji se EventFields
bool CCmPartner::ChangePartnerVariable(std::string name, CCmVariable *var, uint8_t newPriority)
{
   debug_printf("CCmPartner::ChangePartnerVariable: %s", print_var(*var).c_str());
   // Invariant inv(std::bind(&CCmPartner::debug, this), "ChangePartnerVariable " + print_var(*var));

    var->setFlagPriorityChanged(false);

    if (!var->haveFlagWritable())
    {
        cms_ns_if_print("libcm", 1, "cannot set variable named \"%s\", because variable is READ-ONLY!", name.c_str());
        return false;
    }

    //debug_printf("changePartnerVariable %s  .. writable=%s array=%s\n", name.c_str(), (var->haveFlagWritable() ? "Y" : "N"), (var->isArray() ? "Y" : "N"));

    VarMap::iterator it_his = variables_state.hisVariables.find(name);
    if (it_his == variables_state.hisVariables.end())
    {
        cms_ns_if_print("libcm", 1, "cannot set variable named \"%s\", internal error, data lost!", name.c_str());
        return false;
    }

    if (!var->diffAllFields(it_his->second))  // rebuild_EventFields
    {
        // variable unchanged
        return false;
    }

    // Updatni zaznam v hisVariables
    var->SendBlockUntil = it_his->second.SendBlockUntil; // nemelo by byt potreba, pro sichr, kdyby to bylo volano jinak
    it_his->second = *var;
    it_his->second.EventFields.clear();

    // Pridej zmenu do fronty k odeslani
    push_back_or_update(variables_state.his_variables_to_send, *var, newPriority);

    if (newPriority == 0 || var->getPriority() == 0)
        waitingPriorityMessage = true;

    return true;
}

// Pokud se posila hodne malych zprav, tak na pomalych spojenich to ma
// velkou rezii. Pro takova spojeni omezime, ze se muze posilat zprava
// o CM promennych treba maximalne jednou z 500 milisekund.
//
double CCmPartner::haveDataToSend(double ActTime, Maybe<double> max_variable_send_period)
{
    double next_time = ActTime + haveDataToSendPrecise(ActTime);
    double result = next_time;
    if (max_variable_send_period)
    {
        if (next_time < last_sent_time + *max_variable_send_period)
            result = last_sent_time + *max_variable_send_period;
    }
    // cms_ns_if_print("libcm", 1, "haveDataToSendPrecise: max_period=%.3f, precise_after=%.3f real_after=%.3f",
    //                 max_variable_send_period.get_or(-1),
    //                 next_time - ActTime,
    //                 result - ActTime);
   return result - ActTime;
}

double CCmPartner::haveDataToSendPrecise(double ActTime)
{
    if (variables_state.fragments_to_send)
        return 0.0;
    
    double RetVal = 10000.0;
    if ((variables_state.myVariablesToSend.size()>0) || (variables_state.his_variables_to_send.size()>0))
    {
        for (auto const &p : variables_state.myVariablesToSend)
	{
	    double myWait = 0.0;
	    if (p.second.SendBlockUntil > ActTime) myWait = p.second.SendBlockUntil - ActTime;
	    if (RetVal > myWait) RetVal = myWait;
	    if (p.second.newPriority < 2) RetVal = 0.0;
	    if (RetVal == 0.0) return RetVal;
	}
	for (std::list<CCmVariable>::iterator it = variables_state.his_variables_to_send.begin(); it != variables_state.his_variables_to_send.end(); ++it)
	{
	    double myWait = 0.0;
	    if (it->SendBlockUntil > ActTime) myWait = it->SendBlockUntil - ActTime;
	    if (RetVal > myWait) RetVal = myWait;
	    if (it->getPriority() < 2) RetVal = 0.0;
	    if (RetVal == 0.0) return RetVal;
	}
    }
    else
    {
	RetVal = 10000.0; // false
    }
    return RetVal;
}

// void CCmPartner::GenerateDataToSend(double ActTime, std::list<std::string> *ret_messages)
// {
//     assert(ret_messages);
//     DataToSendParams params = {
//         LengthIdeal,
//         LengthMax,
//         UseSetIds,
//         PartnerVarPrefix,
//     };
//     generate_data_to_send(*ret_messages, variables_state, params, libCM_GetActualTimeDouble());
// }
