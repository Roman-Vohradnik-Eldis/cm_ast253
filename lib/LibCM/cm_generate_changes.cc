#include "cm_generate_changes.hh"
#include "cm_debug.hh"
#include "cm_fragments.hh"
#include "cm_partner_variables.hh"
#include "cm_send_buffer_state.hh"
#include "cm_utils.hh"
#include "cm_maybe.hh"
#include "cm_var_utils.hh"

struct PreparedVariables
{
    std::vector<size_t> my_prepared_variables;
    std::vector<size_t> his_prepared_variables;
    Maybe<FragmentsToSend> new_fragments_to_send;
    Maybe<FragmentsState> fragments_state;
};

bool has_changes(PreparedVariables const &prepared_variables)
{
    return 
        !prepared_variables.my_prepared_variables.empty() || 
        !prepared_variables.his_prepared_variables.empty() ||
        prepared_variables.new_fragments_to_send || 
        prepared_variables.fragments_state;
}

// Vrati true, pokud jsme uspesne vygenerovali zpravu, a muze se pak
// tato promennou vyhodit z fronty. Uspesne vygenerovali = bud jsme ji
// uspesne pridali do vystupniho bufferu, nebo ji zacali zpracovavat
// jako fragmentovanou promennou.
bool add_msg_to_ret_messages(std::string var_name,
                             std::string const &msg,
                             PartnerVariablesState const &variables_state,
                             SendBufferState &ret_messages,
                             PreparedVariables &prepared_variables,
                             DataToSendParams const &params)
{
    if (variables_state.fragments_to_send && variables_state.fragments_to_send->get_variable_name() == var_name)
    {
        // Tohle osetrujeme pred generovani zpravy
        assert(false);
        return false;
    }

    assert(!prepared_variables.new_fragments_to_send ||
           prepared_variables.new_fragments_to_send->get_variable_name() != var_name);

    if (msg.size() > ret_messages.get_fragment_max_size())
    {
        // Jsme konzervativni, radsi budeme v tomto pripade vzdy
        // fragmentovat, i kdyz by se to do zpravy po kompresi mohlo
        // vejit, a nebo zrovna tato zprava musi byt hodne kratka.
        if (variables_state.fragments_to_send || prepared_variables.new_fragments_to_send)
        {
            // Zrovna uz posilame jinou promennou, tak to zkusime
            // priste
            return false;
        }
        else
        {
            FragmentsToSend new_fragments_to_send = FragmentsToSend(var_name, msg, params.my_libcm_name);
            size_t max_size = ret_messages.get_optimal_message_size();
            if (max_size > 0)
            {
                FragmentsState state = new_fragments_to_send.get_state();
                if (Maybe<std::string> frag = state.generate_message(max_size))
                {
                    cms_ns_if_print("cm_gen", 4, "try adding message of %s", var_name.c_str());
                    if (ret_messages.try_add_message(*frag))
                    {
                        prepared_variables.new_fragments_to_send = std::move(new_fragments_to_send);
                        prepared_variables.fragments_state = state;
                        return true;
                    }
                }
            }
            return false;
        }
    }
    else
    {
        cms_ns_if_print("cm_gen", 4, "try adding message of %s", var_name.c_str());
        return ret_messages.try_add_message(msg);
    }
}

std::string GenerateChangePartnerVariable(PartnerVariablesState const &variables_state,
                                          DataToSendParams const &params,
                                          std::string const &reduced_name,
                                          CCmVariable const &var,
                                          uint8_t Priority)
{
    std::string msg;
    msg += "MOD";
    if (params.UseSetIds)
    {
        assert(contains(variables_state.his_vars_setcmd_id, var.getName()));
        msg += ",";
        msg += cm_tostring(variables_state.his_vars_setcmd_id.at(var.getName()));
    }
    msg += ",";
    msg += reduced_name;
    if (var.haveFlagPriorityChanged())
    {
        char WStr[100];
        sprintf(WStr,"/%d",Priority);
        msg += std::string(WStr);
    }
    msg += ",";
    msg += var.getVALUES(true);
    // return std::string("MOD,") + name_full + std::string(",") + var.getVALUES(true);
    return msg;
}

std::string make_reduced_name(std::string const &orig_name, std::string const &PartnerVarPrefix)
{
    std::string reduced_name = orig_name;
    if (PartnerVarPrefix.size() != 0)
    {
	if (reduced_name.size() > PartnerVarPrefix.size())
	{
	    if (PartnerVarPrefix == reduced_name.substr(0,PartnerVarPrefix.size())) 
	    {
		reduced_name = reduced_name.substr(PartnerVarPrefix.size(),
                                                   reduced_name.size() - PartnerVarPrefix.size());
            }
	}
    }
    return reduced_name;
}

void update_his_variable_state_after_send(PartnerVariablesState &variables_state,
                                          double ActTime,
                                          std::string name,
                                          CCmVariable const &var)
{
    // find_and_erase_by_name(his_variables_to_send, name);
    double SendBlockUntil = ActTime + var.MaxUpdateFreq;
    
    // his_variables_to_send.erase(name);
    // VarMap::iterator it_st_m = hisVariablesToSend_map.find(name);
    // if (it_st_m != hisVariablesToSend_map.end())
    // {
    //     hisVariablesToSend_map.erase(it_st_m);
    // }
    VarMap::iterator it_st_bl = variables_state.hisVariables.find(name);
    if (it_st_bl != variables_state.hisVariables.end())
	it_st_bl->second.SendBlockUntil = SendBlockUntil;


    VarMap::iterator it_st = variables_state.hisVariables.find(name);
    if (it_st != variables_state.hisVariables.end())
    {
	it_st->second.copy_indexes((var), var.EventFields);
//          it_st->second.SendBlockUntil = var.SendBlockUntil;
    }
}

// AddHisVariablesToSend
//
// @ret_messages: where to put messages
// @length: sum of all the lengths of strings in ret_messages
//
// V hisVariablesToSend zbyde to, co zatim nejde odeslat.
//
void add_his_variables_to_send(PartnerVariablesState const &variables_state,
                               DataToSendParams const &params,
                               double ActTime,
                               SendBufferState &ret_messages,
                               PreparedVariables &prepared_variables)
{
    cms_ns2_if_print("cm_gen", params.my_libcm_name, 4, "add_his_variables_to_send");
    size_t i = 0;
    for (CCmVariable const &var : variables_state.his_variables_to_send)
    {
        std::string name = var.getName();
        uint8_t Priority = var.getPriority();

	std::string reduced_name = make_reduced_name(name, params.PartnerVarPrefix);

        if ((var.SendBlockUntil <= ActTime || Priority < 2) &&
            (!variables_state.fragments_to_send ||
             variables_state.fragments_to_send->get_variable_name() != name))
        {
    	    // Slozime zpravu s novym set_id, bud ho pouzijeme, nebo ne.
	    std::string msg = GenerateChangePartnerVariable(variables_state, params, reduced_name, var, Priority);
    	    bool confirmed = add_msg_to_ret_messages(name, msg, variables_state, ret_messages, prepared_variables, params);
    	    if (confirmed)
            {
                prepared_variables.his_prepared_variables.push_back(i);
            }
            else
            {
                // break;
            }
        }
        ++i;
    }
}

std::string GenerateChangeMyVariable(DataToSendParams const &params,
                                     std::string const &name,
                                     MyVariableToSend const &var,
                                     uint8_t Priority,
                                     unsigned set_id) 
{
    // cms_ns_if_print("libcm", 3, "GenerateChangeMyVariable: %s", var.value ? print_var(*var.value).c_str() : "undef");
    // ThreadTimer t;
    std::string msg;
    if (var.EventWhat == CmEventWhat::undefine)
    {
        msg = std::string("UNDEF");
        if (params.UseSetIds)
        {
            msg += ",";
            msg += cm_tostring(set_id);
        }
        msg += ",";
        msg += name;
    }
    else if (var.EventWhat == CmEventWhat::define)
    {
        msg = std::string("DEF");
        if (params.UseSetIds)
        {
            msg += ",";
            msg += cm_tostring(set_id);
        }
        msg += ",";
        msg += var.value->toStringDefine();
    }
    else if (var.EventWhat == CmEventWhat::update)
    {
        msg = "SET";
        if (params.UseSetIds)
        {
            msg += ",";
            msg += cm_tostring(set_id);
        }
        msg += ",";
        msg += name;
        if (var.value->haveFlagPriorityChanged())
        {
            char WStr[100];
            sprintf(WStr,"/%d",Priority);
            msg += std::string(WStr);
        }
        msg += ",";
        msg += var.value->getVALUES(true);
    }
    else
    {
        // TODO:
        abort();
    }
    // double msec = t.elapsed_msec_double();
    // cms_ns2_if_print("libcm", "Test0", 3, "Generate change in %.3f msec: %s", msec, msg.c_str());
    return msg;
}

void update_my_variable_state_after_send(PartnerVariablesState &variables_state,
                                         double ActTime,
                                         std::string name,
                                         MyVariableToSend const &var_event)
{
    unsigned set_id = variables_state.my_vars_setcmd_id.get_next_id(name);
    CCmVariable const *var = var_event.value ? &*var_event.value : nullptr;
    // CCmVariable const &var = var_event.variable;
    if (var_event.EventWhat == CmEventWhat::define)
	assert(var->EventFields.empty());

    // Zprava se bude odesilat, takze nastavime, ze ziskane set_id je pouzito.
    if (var_event.EventWhat == CmEventWhat::undefine || var_event.EventWhat == CmEventWhat::define)
	variables_state.my_vars_setcmd_id.update_fully(name, set_id);
    else
	variables_state.my_vars_setcmd_id.update_from_variable(name,
                                                               var->getArrayY(), var->getArrayX(),
                                                               var->EventFields,
                                                               set_id);

    if (var_event.EventWhat == CmEventWhat::undefine)
    {
        assert(contains(variables_state.myVariables_State, name));
        variables_state.myVariables_State.erase(name);
    }
    if (var_event.EventWhat == CmEventWhat::define)
    {
        // Tady by klidne mohlo byt taky ActTime + var->MaxUpdateFreq
	variables_state.myVariables_State[name] = {*var, ActTime + var->MaxUpdateFreq};
    }
    if (var_event.EventWhat == CmEventWhat::update)
    {
        assert(contains(variables_state.myVariables_State, name));
        assert(variables_state.myVariables_State.at(name).type.compareTypeAndLimits(*var));
        variables_state.myVariables_State.at(name) = {*var, ActTime + var->MaxUpdateFreq};
    }
}

// AddMyVariablesToSend
// ret_messages: where to put messages
// length: sum of all the lengths of strings in ret_messages
//
// Pro kazdou promenou v hisVariablesToSend, kterou jde odeslat:
//
// (1) Vygenerujeme zpravu do ret_messages
// (2) Vyhodime ji z hisVariablesToSend_map
// (3) Updatneme my_vars_setcmd_id
//
// V hisVariablesToSend zbyde to, co zatim nejde odeslat.
//
void add_my_variables_to_send(PartnerVariablesState const &variables_state,
                              DataToSendParams const &params,
                              double ActTime,
                              SendBufferState &ret_messages,
                              PreparedVariables &prepared_variables)
{
    cms_ns2_if_print("cm_gen", params.my_libcm_name, 4, "add_my_variables_to_send");
    size_t i = 0;
    for (auto &p : variables_state.myVariablesToSend)
    {
        MyVariableToSend const &var_event = p.second;
	// CCmVariable const &var = var_event.variable;
        std::string const &name = p.first;
        unsigned set_id = variables_state.my_vars_setcmd_id.get_next_id(name);
	
        if ((var_event.SendBlockUntil <= ActTime || var_event.newPriority < 2) &&
            (!variables_state.fragments_to_send ||
             variables_state.fragments_to_send->get_variable_name() != name))
        {
	    // Slozime zpravu s novym set_id, bud ho pouzijeme, nebo ne.
	    std::string msg = GenerateChangeMyVariable(params, name, var_event, var_event.newPriority, set_id);
	    bool confirmed = add_msg_to_ret_messages(name, msg, variables_state, ret_messages, prepared_variables, params);
            if (confirmed)
            {
                prepared_variables.my_prepared_variables.push_back(i);
            }
            else
            {
                // break;
            }
        }
        ++i;
    }
}

void commit_my_changes(PartnerVariablesState &variables_state,
                       std::vector<size_t> const &prepared_vars,
                       double ActTime)
{
    auto it = variables_state.myVariablesToSend.begin();
    size_t i = 0;
    for (size_t pos : prepared_vars)
    {
        assert(i <= pos);
        while (i < pos)
        {
            ++it;
            ++i;
        }
        std::string const &name = it->first;
        // VariableEvent const &e = *it;
        // CCmVariable const &var = e.variable.;
        update_my_variable_state_after_send(variables_state, ActTime, name, it->second);
        ++it;
        ++i;
    }

    variables_state.myVariablesToSend.erase_variables(prepared_vars);
}

void commit_his_changes(PartnerVariablesState &variables_state,
                        std::vector<size_t> const &prepared_vars,
                        double ActTime)
{
    auto it = variables_state.his_variables_to_send.begin();
    size_t i = 0;
    for (size_t pos : prepared_vars)
    {
        assert(i <= pos);
        while (i < pos)
        {
            ++it;
            ++i;
        }
        CCmVariable &var = *it;
        update_his_variable_state_after_send(variables_state, ActTime, var.getName(), var);
        it = variables_state.his_variables_to_send.erase(it);
        ++i;
    }
}

void commit_fragments(PartnerVariablesState &variables_state, PreparedVariables const &prepared_variables)
{
    if (prepared_variables.new_fragments_to_send)
    {
        assert(!variables_state.fragments_to_send);
        variables_state.fragments_to_send = prepared_variables.new_fragments_to_send;
    }
    if (prepared_variables.fragments_state)
    {
        assert(variables_state.fragments_to_send);
        variables_state.fragments_to_send->commit(*prepared_variables.fragments_state);

        if (variables_state.fragments_to_send->is_completed())
            variables_state.fragments_to_send = Nothing();
    }
}

Maybe<FragmentsState> get_current_fragment_state(PartnerVariablesState const &variables_state,
                                                 PreparedVariables const &prepared_variables)
{
    if (prepared_variables.fragments_state)
    {
        return prepared_variables.fragments_state;
    }
    else
    {
        assert(!prepared_variables.new_fragments_to_send);
        if (variables_state.fragments_to_send)
            return variables_state.fragments_to_send->get_state();
    }
    return Nothing();
}

// Doplnime zpravu fragmentovanou promennou, pokud nejakou aktualne
// fragmentujeme.
//
// Pozor: Z nejakeho duvodu to nefunguje, kdyz neomezime iterations,
// SendBufferState zpravu prijme, ale vzdy pretece.
void generate_fragments(PartnerVariablesState const &variables_state,
                        PreparedVariables &prepared_variables,
                        SendBufferState &ret_messages)
{
    Maybe<FragmentsState> state = get_current_fragment_state(variables_state, prepared_variables);
    if (!state)
        return;

    int iterations = 0;
    for (; iterations <= 1; ++iterations)
    {
        size_t max_size = ret_messages.get_optimal_message_size();
        if (max_size > 0)
        {
            FragmentsState new_state = *state;
            if (Maybe<std::string> msg = new_state.generate_message(max_size))
            {
                if (ret_messages.try_add_message(*msg))
                {
                    state = new_state;
                    continue;
                }
            }
        }
        break;
    }

    if (iterations > 0)
        prepared_variables.fragments_state = state;
}

void commit_changes(PartnerVariablesState &variables_state,
                    PreparedVariables const &prepared_variables,
                    double ActTime)
{
    commit_my_changes(variables_state, prepared_variables.my_prepared_variables, ActTime);
    commit_his_changes(variables_state, prepared_variables.his_prepared_variables, ActTime);
    commit_fragments(variables_state, prepared_variables);
}

void print_debug_message(DataToSendParams const &params, PreparedVariables const &prepared_variables, ThreadTimer &t)
{
    int64_t elapsed_usec = t.elapsed_usec();
    // if (elapsed_usec >= 1000)
    // {
    unsigned frag_count = prepared_variables.fragments_state ?
        prepared_variables.fragments_state->get_fragment_count() : 0;

    cms_ns2_if_print("cm_gen", params.my_libcm_name, 4,
                     "Generated data to send in %.3f msec (my_vars=%zu his_vars=%zu, frag=%d)",
                     elapsed_usec/1000.0,
                     prepared_variables.my_prepared_variables.size(),
                     prepared_variables.his_prepared_variables.size(),
                     frag_count);
    // }    
}

void print_partner_debug_message(std::string const &my_libcm_name, PartnerVariablesState const &state, int iteration)
{
    std::string fragmented = state.fragments_to_send ? state.fragments_to_send->get_variable_name() : "no";
    cms_ns2_if_print("cm_gen", my_libcm_name, 4, "generate_data_to_send iteration %d: my_vars=%zu, his_vars=%zu, fragmented=%s",
                     iteration,
                     state.myVariablesToSend.size(),
                     state.his_variables_to_send.size(),
                     fragmented.c_str());    
}

bool generate_data_to_send(SendBufferState &ret_messages,
                           PartnerVariablesState &variables_state,
                           DataToSendParams const &params,
                           double ActTime)
{
    ThreadTimer t;
    int iterations = 0;
    for (;; ++iterations)
    {
        print_partner_debug_message(params.my_libcm_name, variables_state, iterations);
        PreparedVariables prepared_variables;
        
        add_his_variables_to_send(variables_state, params, ActTime, ret_messages, prepared_variables);
        add_my_variables_to_send(variables_state, params, ActTime, ret_messages, prepared_variables);
        generate_fragments(variables_state, prepared_variables, ret_messages); // Tohle musi jit naposled
                                                                                                       
        if (has_changes(prepared_variables))
        {
            if (ret_messages.finish())
            {
                commit_changes(variables_state, prepared_variables, ActTime);

                print_debug_message(params, prepared_variables, t);
                return true;
            }
            else
            {
                cms_ns2_if_print("cm_gen", params.my_libcm_name, 4, "Failed to create message, trying again");
                ret_messages.reset();
            }
        }
        else
        {
            // No data to send
            cms_ns2_if_print("cm_gen", params.my_libcm_name, 4, "No data to send, returning false");
            assert(ret_messages.empty());
            return false;
        }
    }
}
