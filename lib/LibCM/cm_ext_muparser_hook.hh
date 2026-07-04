#ifndef CM_EXT_MUPARSER_HOOK_HH_
#define CM_EXT_MUPARSER_HOOK_HH_

#if USE_LIB_MUPARSERX

#include "mup_interface.hh"
#include "cm_ext_hook.hh"
#include "cm_base_varevent.hh"
#include "cm_scheduler.hh"

#define CM_BASE_EXT_KEEP_TMP_VARS 1

class CmMuParserHook : public CCmExtHook
{
  private:
    
class CCmWorkerExt_ExtDef_Action
{
public:
    class CCmFiltersSet MyUsedVarsRq;
    class CCmFiltersSet MyUsedVarsOut;
    class CCmFilter LastOut;
    bool LastOutDefined;

    CCmWorkerExt_ExtDef_Action(){ LastOutDefined = false; };
    virtual ~CCmWorkerExt_ExtDef_Action() {};
    virtual std::string PrintMe(void);
    virtual void ResetMe(double ActTime);
    virtual void GetUsedVariables(C_CM_MUP_Interface *MUP);
    virtual bool Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, double ActTime);
    virtual bool Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, bool &ReturnVal, double ActTime);
    virtual bool Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, std::string ExtraAssign, double ActTime);
    virtual bool SetMe(std::deque<std::string> &cmd_items, std::string line);
    virtual double Polling_GetNextTime(double ActTime);
    virtual bool Polling_CheckNow(double ActTime);
    void PrintLastErrorMUP(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, std::string &EvalString);
};

class CCmWorkerExt_ExtDef_Action_Eval : public CCmWorkerExt_ExtDef_Action
{
private:
    std::string EvalString;
public:
    CCmWorkerExt_ExtDef_Action_Eval(){};
    virtual ~CCmWorkerExt_ExtDef_Action_Eval() {};
    virtual std::string PrintMe(void);
    virtual void GetUsedVariables(C_CM_MUP_Interface *MUP);
    virtual bool Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, double ActTime);
    virtual bool Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, bool &ReturnVal, double ActTime);
    virtual bool Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, std::string ExtraAssign, double ActTime);
    virtual bool SetMe(std::deque<std::string> &cmd_items, std::string line);
};

class CCmWorkerExt_ExtDef_Action_Int2Str : public CCmWorkerExt_ExtDef_Action
{
private:
    std::string EvalString;
    std::string OutputVar;

    std::string PreviousStatus;
    std::string StatusUndefInVal;
    std::string StatusUnknown;

    std::list< std::pair<int,std::string> > StatusList;

    virtual std::string AddCode(const std::string &Inp, int code);
    virtual std::string Eval(C_CM_MUP_Interface *MUP, double ActTime);
public:
    CCmWorkerExt_ExtDef_Action_Int2Str(){};
    virtual ~CCmWorkerExt_ExtDef_Action_Int2Str() {};
    virtual std::string PrintMe(void);
    virtual void GetUsedVariables(C_CM_MUP_Interface *MUP);
    virtual bool Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, double ActTime);
    virtual bool Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, std::string ExtraAssign, double ActTime);
    virtual bool SetMe(std::deque<std::string> &cmd_items, std::string line);
};

class CCmWorkerExt_ExtDef_Action_WorstStatus : public CCmWorkerExt_ExtDef_Action
{
private:
    std::string EvalString;
    std::string OutputVar;

    std::string UndefinedUnavail;
    std::string Unknown;
    std::string CharsFromWorst;

    virtual std::string Eval(C_CM_MUP_Interface *MUP, double ActTime);
public:
    CCmWorkerExt_ExtDef_Action_WorstStatus(){};
    virtual ~CCmWorkerExt_ExtDef_Action_WorstStatus() {};
    virtual std::string PrintMe(void);
    virtual void GetUsedVariables(C_CM_MUP_Interface *MUP);
    virtual bool Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, double ActTime);
    virtual bool Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, std::string ExtraAssign, double ActTime);
    virtual bool SetMe(std::deque<std::string> &cmd_items, std::string line);
};

class CCmWorkerExt_ExtDef_Action_Average : public CCmWorkerExt_ExtDef_Action
{
private:
    std::string EvalString;
    std::string OutputVar;

    int AvgType; // 1=AVG, 2=MAX, 3=MIN
    double AvgMinPeriod;
    double AvgStandardPeriod;
    double RecalculationIdlePeriod;
    double ThresholdChange;
    double TimeUpdatePrecision;

    double LastCheckTime;
    double LastOutputSet;

    double FirstDataTime; // pokud je nenulovy a mensi aspon o 20 vterin, pak je OutputExist 
    //... pokud to je vice, nez 60, pak zakratime data v DataStorage a poposuneme cas
    double LastOutputValue; // pro porovnani, kdyz se zavola Polling_CheckNow();
    bool   LastOutputExist; // pro porovnani, kdyz se zavola Polling_CheckNow(); // check 20 sekund
 
 
    std::map<double, double> DataStorage; // first is time, second is value


    virtual void UpdateCumul(double &cumul_val, double &cumul_dur, double calc_val, double calc_dur, bool first_write);
    virtual void Eval(C_CM_MUP_Interface *MUP, double ActTime, bool &OutputExist, double &OutputValue);
public:
    CCmWorkerExt_ExtDef_Action_Average();
    virtual ~CCmWorkerExt_ExtDef_Action_Average() {};
    virtual std::string PrintMe(void);
    virtual void ResetMe(double ActTime);
    virtual void GetUsedVariables(C_CM_MUP_Interface *MUP);
    virtual bool Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, double ActTime);
    virtual bool Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, std::string ExtraAssign, double ActTime);
    virtual bool SetMe(std::deque<std::string> &cmd_items, std::string line);
    virtual double Polling_GetNextTime(double ActTime);
    virtual bool Polling_CheckNow(double ActTime);
};

class CCmWorkerExt_ExtDef
{
public:
    VarMap      LastUsedVariables;
    class CCmFiltersSet                           AllUsedVariables;   // filters .. All TMP names ... but with all other CmExt_Variables (except MyName)

    std::map<std::string, class CCmVariable *>    TmpVariables;   // for erase after Evaluate

    bool                                          ExistCondAutomatic; //  (origo/rewriten)
    bool                                          ExistCondHaveOnlyExistCheck; //  right now only , if cond is automatic
    class CCmFiltersSet                           ExistUsedVariables; // for refresh / for filters if filtered me
    std::list<class CCmWorkerExt_ExtDef_Action *> ExistExtEvaluators;

    class CCmFiltersSet                           EvalUsedVariables;  // (with indexes)// for refresh / for filters if filtered me
    std::list<class CCmWorkerExt_ExtDef_Action *> EvalExtEvaluators;

    class CCmFiltersSet                           ReverseUsedVariables; // for refresh / for filters if filtered me
    std::list<class CCmWorkerExt_ExtDef_Action *> ReverseExtEvaluators;

    uint16_t                                      EvaluatePending; //(1<<0)  = exist,  (1<<1) = eval,  (1<<2) = reverse,   (1<<4) ReReadKnownVars to LastUsedVariables
    bool                                          ExistActually;
    bool                                          CompleteActually;
    unsigned                                      EvalDepth; // actual depth of EvalExtEvaluators
    bool                                          havePollings;
    uint8_t                                       FiltersNeedMe; // (1<<0) myFilters ; (1<<1) partnerFilters; (1<<2) Ext (Self)

    std::string                                   VarName;
    CCmVariable                                   Var;

    std::string                                   DebugStringExist;
    std::string                                   DebugStringEval;
    std::string                                   DebugStringReverse;

    CCmWorkerExt_ExtDef();
    virtual bool CheckChangeOneVariable(CCmVariable const *var, std::string name, int changed);
    virtual void SetAllTmp(C_CM_MUP_Interface *MUP);
    virtual void GetAllTmp(C_CM_MUP_Interface *MUP);
    virtual void RemAllTmp(C_CM_MUP_Interface *MUP);
    virtual double getFirstPollingTime(double ActTime);
    virtual void SetEvalDepth(unsigned NewEvalDepth, double ActTime);
    virtual bool doCheckPollings(C_CM_MUP_Interface *MUP, std::list<class CCmWorkerExt_ExtDef_Action *> &Evaluators, double ActTime);
    virtual void doAllActionsExist(C_CM_MUP_Interface *MUP, std::list<class CCmWorkerExt_ExtDef_Action *> &Evaluators, bool &ReturnVal, double ActTime);
    virtual void doAllActionsEval(C_CM_MUP_Interface *MUP, std::list<class CCmWorkerExt_ExtDef_Action *> &Evaluators, bool &ReturnVal, double ActTime, bool WasNotExist = false);
    virtual void doAllActionsReverse(C_CM_MUP_Interface *MUP, std::list<class CCmWorkerExt_ExtDef_Action *> &Evaluators, VarEventMap &UpdateMap, double ActTime);
    virtual double doPendingActionsOrPoll(C_CM_MUP_Interface *MUP, VarMap &MyAllVariables, VarEventMap &UpdateMap, double ActTime);
    virtual void SetDebugStrings(std::string &MyLibCM_name);

};

  private:

    // Dostaneme pri vytvoreni
    CCmLiveDebug *LiveDebug;
    uint64_t LiveDebugFlags;

    // Dostaneme pri vytvoreni
    uint16_t CmWorkerFlags;

    // Dostaneme pri vytvoreni
    std::string MyLibCM_name;
    
    CMSMutex *EXT_Mutex;
    bool NeedAllExtVARS;

    uint64_t DEBUG_FLAG_PRINT_EXT_CREATE; // 1
    uint64_t DEBUG_FLAG_PRINT_EXT_DELETE; // 0
    uint64_t DEBUG_FLAG_PRINT_EXT_UPDATE; // 2

    CCmFiltersSet myFiltersExtInternal;

    std::map <std::string, class CCmWorkerExt_ExtDef> ExtDefMap;
    VarMap         MyAllVariables; // neni too zbytecne?
    VarMap         LastUsedVariables;
    CCmFiltersSet                               AllUsedVariables;
    CCmFiltersSet                               CmFunWaits;
    CCmFiltersSet                               myFiltersFun; // need vars

    std::map <std::string, class CCmExtFun>           CmFun;

    C_CM_MUP_Interface *CM_MUP_Interface;
    VarMap ExtVariables;

public:
    CmMuParserHook(uint16_t worker_flags, CCmLiveDebug *live_debug, uint64_t live_debug_flags, std::string my_libcm_name);
    ~CmMuParserHook();

    C_CM_MUP_Interface *  GetParserPtr(void);

    // Vrati jmena vsech Ext promennych z konfiguraku
    std::set<std::string> ReadConfig_Ext(CMSConfig *config, std::string Section_Var, std::string Section_Ext, std::string Section_MUP_Fun, std::string Section_CM_Fun, VarMap &);
    void SetNeedAllExtVARS(bool b);
    
    // Rozhrani CM HOOK:
    void Start();
    bool RefreshExtFilters(CCmFiltersSet &in, CCmFiltersSet &ext);
    bool internalChangePartnerVariable(CCmVariable const *var);
    bool internalChangeMyVariable(CCmVariable const *var, int EventWhat);

    bool VariableGoCB(VarMap *VariablesCreated,
			      VarMap *VariablesUpdated,
			      VarMap *VariablesDeleted);
    
    bool RegenaratePartnerFiltersExt(CCmFiltersSet &PartnerFiltersActive, CCmFiltersSet &myFiltersExt);

    void EvalPollExt(CCmConnection &conn, PollCond &, double ActTime);

private:
    //    virtual void        PartnerFiltersGoCB(class CCmFiltersSet *PartnerFiltersActive,
//                                           class CCmFiltersSet *PartnerFiltersCreated,
//                                           class CCmFiltersSet *PartnerFiltersDeleted);

    void internalCCmWorkerExtNew(void);
    bool internal_Ext_ChangeOneVariable(CCmVariable const *var, uint8_t what); // what: 0=undefine , 1=define , 2=update
    void MakeForCycleEval(std::deque<std::string> ForCycleParams, std::list<class CCmWorkerExt_ExtDef_Action *> *MyEvaluators, std::string TemplateEval, std::string line);
    std::string MakeForCycleMacro(std::deque<std::string> ForCycleParams, std::string TemplateEval);
    bool checkOtherFilters(CCmFiltersSet *pFilters, int pType, bool UnlimitedStars);
    void RegenerateMyFiltersExt(CCmFiltersSet &myFiltersExt);
    void Config_Ext_Line(std::string name, std::string line);
    std::string Config_Ext_Line_ExpandMacros(std::string line);
    
private:
    double GetActualTimeDouble();
    
    double CmFunExecOneLoop(CCmConnection &);
    bool CmFunAdd(const std::string name, const std::string fun, VarMap &UnUsedVariables, std::set<std::string> &FunUsedVars, bool UseLock = true);
    unsigned CmFunStart(const std::string name, const StringList &params, bool IfRunningDontStart = true);
    bool CmFunChangedVariable(CCmVariable const *var);
    bool AddCmFunFiltersAll(class CCmFiltersSet &_AllUsedVariables);
    bool AddCmFunFiltersMy(class CCmFiltersSet &_myFiltersExt);
    std::string CmFunMakeInstanceName(std::string inp);
    void CmFunSetDone(std::string fun);
    void CmFunErase(std::string fun);
    void CmFunKill(std::string kill_name_pattern, bool UseLock = true);
    void CmFunLoadFromFile_ForGenKeys(std::string FName, std::string LineStd, std::string ParamStr,  StringList &Olist);
    void CmFunLoadFromFile_OneLine(std::string FName, std::string LineStd, std::string &Output,  std::map<std::string, struct CmFunMacroType> &MacroList);
    std::string CmFunLoadFromFile(std::string FName, std::string FunName);

    void CmFunRunOneBlock(void) {}; // To "wait" , or to first modification or maximally (?5) instructions
    void CmFunStart(const std::string) {};
};

#endif // USE_LIB_MUPARSERX
 
#endif // CM_EXT_MUPARSER_HOOK_HH_
