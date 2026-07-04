#ifndef MUP_INTERFACE_HH_
#define MUP_INTERFACE_HH_

#if USE_LIB_MUPARSERX

#define CM_WORKER_EXT_FLAG_MUPARSER_UPDATE_ALL 0x8000

#include "cm_base_filter.hh"
#include <string>
#include <map>
#include <deque>

void CMUP__DefineFunctionGeneric(std::string FunName, std::string FunDef);
std::string CMUP_GlobalGetLastError(void);

class C_CM_MUP_Interface
{
    struct Impl;
    bool UseGlobalLock;
    Impl *impl;
    std::string LastErrorStr;
    bool EnableAutoCreateVar;
    virtual void LockMe(void);
    virtual void UnlockMe(void);
    
public :
    C_CM_MUP_Interface();
    virtual ~C_CM_MUP_Interface();
    virtual void SetUseGlobalLock(bool newVal);
    virtual std::string GetLastError(void);
    virtual void SetEnableAutoCreateVar(bool Enable = true);
    virtual void DefineFunctionGeneric(std::string FunName, std::string FunDef);
    virtual bool GetUsedVariables(std::string func, std::list<std::string> *UsedVariables_S, std::list<class CCmFilter> *UsedVariables_F, bool with_idx = true, bool output_vars = true, bool input_vars = true, bool with_debug = false);

    virtual bool GetUsedVariables(std::string func, std::list<std::string> *UsedVariables, bool with_idx = true, bool output_vars = true, bool input_vars = true, bool with_debug = false)
    { return GetUsedVariables(std::string(func), UsedVariables, NULL, with_idx, output_vars, input_vars, with_debug); }
    virtual bool GetUsedVariables(const char *func, std::list<std::string> *UsedVariables, bool with_idx = true, bool output_vars = true, bool input_vars = true, bool with_debug = false)
    { return GetUsedVariables(std::string(func), UsedVariables, with_idx, output_vars, input_vars, with_debug); }

    virtual bool GetUsedVariables(std::string func, std::list<class CCmFilter> *UsedVariables, bool with_idx = true, bool output_vars = true, bool input_vars = true, bool with_debug = false)
    { return GetUsedVariables(std::string(func), NULL, UsedVariables, with_idx, output_vars, input_vars, with_debug); }
    virtual bool GetUsedVariables(const char *func, std::list<class CCmFilter> *UsedVariables, bool with_idx = true, bool output_vars = true, bool input_vars = true, bool with_debug = false)
    { return GetUsedVariables(std::string(func), UsedVariables, with_idx, output_vars, input_vars, with_debug); }
    virtual bool IsVarDefined(const std::string &name);

    bool Evaluate(std::string func, char &RetType, std::string &RetValue, std::string &ErrorMessage, unsigned &ErrorPos, std::string *RPN);

    bool Evaluate(std::string func);
    bool Evaluate(const char * func)
    { return Evaluate(std::string(func)); }

    bool Evaluate(std::string func, bool &RetValue);
    bool Evaluate(const char * func, bool &RetValue)
    { return Evaluate(std::string(func), RetValue); }

    bool Evaluate(std::string func, double &RetValue);
    bool Evaluate(const char * func, double &RetValue)
    { return Evaluate(std::string(func), RetValue); }

    bool Evaluate(std::string func, long double &RetValue);
    bool Evaluate(const char * func, long double &RetValue)
    { return Evaluate(std::string(func), RetValue); }

    bool Evaluate(std::string func, int &RetValue);
    bool Evaluate(const char * func, int &RetValue)
    { return Evaluate(std::string(func), RetValue); }

    bool Evaluate(std::string func, uint64_t &RetValue);
    bool Evaluate(const char * func, uint64_t &RetValue)
    { return Evaluate(std::string(func), RetValue); }

    bool Evaluate(std::string func, int64_t &RetValue);
    bool Evaluate(const char * func, int64_t &RetValue)
    { return Evaluate(std::string(func), RetValue); }

    bool Evaluate(std::string func, std::string &RetValue);
    bool Evaluate(const char * func, std::string &RetValue)
    { return Evaluate(std::string(func), RetValue); }

private:
    virtual void SetVariableUnlocked(std::string name, bool                    value);

public:
    virtual void SetVariable(std::string name, bool                    value);
    virtual void SetVariable(std::string name, double                  value);
    virtual void SetVariable(std::string name, long double             value);
    virtual void SetVariable(std::string name, float                   value);
    virtual void SetVariable(std::string name, int                     value);
    virtual void SetVariable(std::string name, libcm_integer                value);
    virtual void SetVariable(std::string name, std::string             value);
    virtual void SetVariable(std::string name, bool                   *values, int dimensionY);
    virtual void SetVariable(std::string name, std::deque<bool>       &values, int dimensionY);
    virtual void SetVariable(std::string name, double                 *values, int dimensionY);
    virtual void SetVariable(std::string name, DoubleList const    &values, int dimensionY);
    virtual void SetVariable(std::string name, long double            *values, int dimensionY);
    virtual void SetVariable(std::string name, std::deque<long double>&values, int dimensionY);
    virtual void SetVariable(std::string name, float                  *values, int dimensionY);
    virtual void SetVariable(std::string name, std::deque<float>      &values, int dimensionY);
    virtual void SetVariable(std::string name, int                    *values, int dimensionY);
    virtual void SetVariable(std::string name, libcm_integer               *values, int dimensionY);
    virtual void SetVariable(std::string name, std::deque<int>        &values, int dimensionY);
    virtual void SetVariable(std::string name, UIntList const &values, int dimensionY);
    virtual void SetVariable(std::string name, std::string            *values, int dimensionY);
    virtual void SetVariable(std::string name, StringList const &values, int dimensionY);

    virtual void SetVariable(std::string name, bool                   *values, int dimensionY, int dimensionX);
    virtual void SetVariable(std::string name, std::deque<bool>       &values, int dimensionY, int dimensionX);
    virtual void SetVariable(std::string name, double                 *values, int dimensionY, int dimensionX);
    virtual void SetVariable(std::string name, DoubleList const    &values, int dimensionY, int dimensionX);
    virtual void SetVariable(std::string name, long double            *values, int dimensionY, int dimensionX);
    virtual void SetVariable(std::string name, std::deque<long double>&values, int dimensionY, int dimensionX);
    virtual void SetVariable(std::string name, float                  *values, int dimensionY, int dimensionX);
    virtual void SetVariable(std::string name, std::deque<float>      &values, int dimensionY, int dimensionX);
    virtual void SetVariable(std::string name, int                    *values, int dimensionY, int dimensionX);
    virtual void SetVariable(std::string name, libcm_integer               *values, int dimensionY, int dimensionX);
    virtual void SetVariable(std::string name, std::deque<int>        &values, int dimensionY, int dimensionX);
    virtual void SetVariable(std::string name, UIntList const  &values, int dimensionY, int dimensionX);
    virtual void SetVariable(std::string name, std::string            *values, int dimensionY, int dimensionX);
    virtual void SetVariable(std::string name, StringList const &values, int dimensionY, int dimensionX);
    virtual void SetVariable(class CCmVariable const *var);

    virtual bool GetVariable(std::string name, StringList &values, int &dimensionY, int &dimensionX);
    virtual bool GetVariable(std::string name, UIntList    &values, int &dimensionY, int &dimensionX);
    virtual bool GetVariable(std::string name, std::deque<int>         &values, int &dimensionY, int &dimensionX);
    virtual bool GetVariable(std::string name, DoubleList      &values, int &dimensionY, int &dimensionX);
    virtual bool GetVariable(std::string name, std::deque<long double> &values, int &dimensionY, int &dimensionX);
    virtual bool GetVariable(std::string name, std::deque<bool>        &values, int &dimensionY, int &dimensionX);

    virtual bool GetVariable(std::string name, bool        &val);
    virtual bool GetVariable(std::string name, libcm_integer   &val);
    virtual bool GetVariable(std::string name, int         &val);
    virtual bool GetVariable(std::string name, double      &val);
    virtual bool GetVariable(std::string name, long double &val);
    virtual bool GetVariable(std::string name, std::string &val);

    virtual bool GetVariable(class CCmVariable *var, bool EnableChangeDimension = false);
    virtual bool GetVariableFully(class CCmVariable *var, char preferType = ' ');

    virtual void RemoveVariable(std::string name);
    virtual void ClearVariables(void);
    virtual std::string ClearAllVoidVariables(void);

    virtual std::string mkUnusedVariableNameAndSet(std::string VariableNameBase, bool value);
private:
    virtual std::string mkUnusedVariableNameUnlocked(std::string VariableNameBase);
public:
    virtual std::string mkUnusedVariableName(std::string VariableNameBase);

    virtual std::string ReplaceVariableNameInExpression(std::string OldVar, std::string NewVar, std::string ExpressionForEdit);

    virtual std::string ListVariables(void);
    virtual std::string ListVariables(std::string pattern);
    virtual std::string ListConstants(void);
    virtual std::string ListFunctions(void);
    virtual std::string GetLastExprVarWithIDX(void);
    virtual std::string GetLastExprVarWithIDX_HumanReadable(void);
};

void mup_TESTER(void);

std::string libCM_ListAllKnownVariablesMUP(C_CM_MUP_Interface *_MUP, std::string pattern = "", bool printHeadFoot = true);

#endif // USE_LIB_MUPARSERX

#endif // MUP_INTERFACE_HH_
