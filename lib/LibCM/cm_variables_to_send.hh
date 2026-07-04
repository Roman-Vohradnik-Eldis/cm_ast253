#ifndef CM_VARIABLES_TO_SEND_HH_
#define CM_VARIABLES_TO_SEND_HH_

#include "cm_base_variable.hh"
#include "cm_base_varevent.hh"
#include "cm_maybe.hh"
#include <set>

// Za kazdou nasi promennou, jejiz definici jsme poslali partnerovi,
// mame jednu tuto strukturu.
//
// Pri odeslani definice promenne ji vytvorime. Pri odeslani
// undefinice ji zase smazeme.
struct MyVariableState
{
    CCmVariable type;

    // Tohle je cas posledniho odeslani zmeny + jeji MaxUpdateTime.
    // Kdyz promennou definujeme/predefinujeme nebo posleme update,
    // tak to tady updatneme.
    double SendBlockUntil;
};

// Ve `value` si drzime vzdy aktualni hodnotu promenne: Tj. tu samou,
// ktera je ve `myVariables`.  V EventFields jsou indexy, ktere mu
// budeme posilat.
//
// Nyni algoritmus funguje tak, ze nas nezajima, jakou hodnotu jsme mu
// odeslali. Pokud tedy napriklad zmenim promennou, upravim podle toho
// VariableToSend, a pak bych ji hned zmenil zpet na puvodni hodnotu,
// tak ve VariablesToSend ta zmena zustane. Partner pak dostane zmenu,
// ktera s jeho hodnotu nic neudela.
struct MyVariableToSend
{
    // Nothing() -> undefine
    // CCmVariable -> define or set
    // Ma nastavene EventFields, co se bude posílat.
    Maybe<CCmVariable> value;

    CmEventWhat EventWhat;

    // Tohle se zkopiruje z typu promenne, popr to uzivatel muze
    // nastavit, kdyz chce promennou odeslat prioritne. Fronta pak
    // muze promenne podle toho tridit.
    uint8_t newPriority;

    // Tohle se sem zkopiruje z MyVariableState, kdyz tuto strukturu
    // davame do fronty. Kdyz se jedna o definici/undefinici, tak sem
    // dame 0, ze to chceme poslat co nejdrive.
    double SendBlockUntil;
};

class MyVariablesToSend
{
    std::map<std::string, MyVariableToSend> variables;

public:
    typedef std::map<std::string, MyVariableToSend>::const_iterator const_iterator;

    // Read only access
    const_iterator begin() const { return variables.begin(); }
    const_iterator end() const { return variables.end(); }

    // Number of variables to send
    size_t size() const;

    // If variable is not found, NULL is returned.
    MyVariableToSend const *find_variable(std::string const &name) const;

    // Nehlede na to, co je ve fronte a co jsme poslali, se cela
    // promenna definuje/redefinuje.
    void full_define_variable(std::string const &name, CCmVariable const &var);

    // Musi byt zavolano na existujici promennou stejneho typu:
    // Tj. Bud jiz nadefinovou, nebo s definici ve fronte.  Tohle
    // volame, kdyz jsme my nebo jiny partner zmenili promennou.
    // `var` je aktualni hodnota promenne, EventFields obsahujou to,
    // co jsme zmenili (to se prida k odeslani).
    bool update_existing(std::string const &name,
                         CCmVariable const &var,
                         Maybe<uint8_t> newPriority,
                         double sendBlockUntil);

    // Musi byt zavolano na existujici promennou stejneho
    // typu. Tj. Bud jiz nadefinovou, nebo s definici ve fronte. Tohle
    // volame kdyz tento partner zmenil tuto promennou.  `var` je
    // aktualni hodnota promenne, EventFields obsahujou to, co zmenil
    // (to se odecte ze zmen k odeslani).
    bool update_existing_erase_event_fields(std::string const &name,
                                            CCmVariable const &var);

    // Nehlede na to, co je ve fronte a co jsme poslali, se cela
    // promenna definuje/redefinuje. Tohle volame pouze v pripade, ze
    // jsme mu uz poslali definici teto promenne.
    bool undefine(std::string const &name);

    void erase_variables(std::vector<size_t> const &positions);

    // Vyhod z fronty, pokud tam je.
    void erase_variable(std::string const &name);
}; 

#endif // CM_VARIABLES_TO_SEND_HH_
