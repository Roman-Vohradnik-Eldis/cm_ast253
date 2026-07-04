#ifndef CM_CONNECTION_HH_
#define CM_CONNECTION_HH_

#include "cm_base_variable.hh"
#include "cm_base_filters_set.hh"

#define CMLIB_VALUE_PRIORITY_UNDEF  255

// Tohle je rozhrani pro program, ktery pozaduje jednoduche rozhrani
// na praci s CM promennymi a filtry.  Takovy program funguje takto:
//
// - Definuje si nejake promenne
//
// - Meni svoje promenne
//
// - Meni cizi promenne nebo promenne z konfiguraku, o kterych predpoklada, ze existuji.
//
// - Dostava zmeny promennych.
//
// Ale kdyz meni promennou, tak nekontroluje, jestli se zmenila apod.

class CCmConnection
{
public:
    virtual ~CCmConnection() {}

    // Tohle pouziva muParser
    virtual void ChangeVariable      (CCmVariable const *var                                                     , uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF, bool _GenerateCB  = false) = 0;

    // Tohle pouziva cm_gui
    virtual void ChangeVariable      (std::string name, std::string StrParams                                    , uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF) = 0;

    
    virtual void ChangeVariableBool  (std::string name, bool          value, unsigned idx_y = 0, unsigned idx_x = 0, uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF) = 0;
    virtual void ChangeVariableInt   (std::string name, libcm_integer value, unsigned idx_y = 0, unsigned idx_x = 0, uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF) = 0;
    virtual void ChangeVariableFloat (std::string name, double        value, unsigned idx_y = 0, unsigned idx_x = 0, uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF) = 0;
    virtual void ChangeVariableString(std::string name, std::string   value, unsigned idx_y = 0, unsigned idx_x = 0, uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF) = 0;
    virtual void ChangeVariableEnum  (std::string name, int           value, unsigned idx_y = 0, unsigned idx_x = 0, uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF) = 0;
    virtual void ChangeVariableEnum  (std::string name, std::string   value, unsigned idx_y = 0, unsigned idx_x = 0, uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF) = 0;

    virtual CCmVariable GetVariable(std::string name, bool *found_and_filled) = 0;

    // Funkce pridaji jen ty promenne VAR, pro ktere plati ze
    //
    //   VAR.flags & FlagMaskTest == FlagMaskVal
    //
    virtual void GetAllVariables(VarMap &out, uint8_t FlagMaskTest=0, uint8_t FlagMaskVal=0) = 0;
    virtual void GetPartnersVariables(VarMap &var_list, uint8_t FlagMaskTest=0, uint8_t FlagMaskVal=0) = 0;
    virtual void AddMyVariables(VarMap &var_list, uint8_t FlagMaskTest=0, uint8_t FlagMaskVal=0) = 0;

    virtual void DefineMyVariable(CCmVariable const *var, bool _GenerateCB  = false) = 0;
    virtual void DefineMyVariable(CCmVariable const &v, bool _GenerateCB  = false) = 0;
    // str je cela definice promenne.
    virtual void DefineMyVariable(std::string str, bool _GenerateCB  = false) = 0;
    virtual bool UndefineVariable(std::string name, bool _GenerateCB  = false) = 0;

    // Filters
    virtual void AddFilter(CCmFilter *newFilter) = 0;
    virtual void AddFilters(std::list<CCmFilter> *filters) = 0;
    virtual void SetFilters(std::list<CCmFilter> *newFilter) = 0;
    virtual void SetFilters(std::map<std::string, CCmFilter> *newFilter) = 0;
    virtual void SetFilters(CCmFiltersSet *newFilter) = 0;
    virtual void ClearFilters() = 0;

    virtual std::string const &MyName() const = 0;
};

#endif
