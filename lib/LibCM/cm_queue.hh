#ifndef CM_QUEUE_HH_
#define CM_QUEUE_HH_

#include "cm_base_variable.hh"
#include "cm_connection.hh"
#include "cm_base_functs.hh"
#include <string>
#include <map>
#include <list>

// class CmQueue
// {
//     std::map<std::string, CCmVariable> hisVariablesToSend_map;
//     std::list<CCmVariable> hisVariablesToSend;

// public:
//     typedef std::list<CCmVariable>::const_iterator iterator;

//     iterator begin() const { return hisVariablesToSend.begin(); }
//     iterator end() const { return hisVariablesToSend.end(); }

void push_back_or_update(std::list<CCmVariable> &queue, CCmVariable &v, uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF);

    // void erase(std::string const &name);

    // void push_back(CCmVariable &v);

    // size_t size() const
    // { return hisVariablesToSend.size(); }

    // CCmVariable &front()
    // { return hisVariablesToSend.front(); }

    // void pop_front();

    // void clear()
    // {
    //     hisVariablesToSend.clear();
    //     hisVariablesToSend_map.clear();
    // }

    // Pokud ve fronte promenna neni, nedela nic. Pokud tam je a neni to pole,
    // tak ji vyhodi. Pokud tam je a je to pole, tak ji odebere ty EventFields,
    // ktere jsou ve var.
void erase_variable_changes_from_queue(std::list<CCmVariable> &, CCmVariable &var);

// Modifikuje orig_var. Vrati true, pokud tam jeste neco zbylo, vrati false,
// pokud je mozno ji smazat
bool erase_variable_changes_from(CCmVariable &orig_var, CCmVariable &new_var);

#endif // CM_QUEUE_HH_
