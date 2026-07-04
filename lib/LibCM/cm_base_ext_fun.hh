#ifndef CM_EXT_FUN_HH
#define CM_EXT_FUN_HH

#if USE_LIB_MUPARSERX

#include "cm_header_internal.hh"
#include "cm_base_varevent.hh"
#include "mup_interface.hh"
#include "cm_base_filters_set.hh"

#define CM_FUN_RETVAL_STATE_DONE 0
#define CM_FUN_RETVAL_STATE_ERROR 1
#define CM_FUN_RETVAL_STATE_LEAVE 2
#define CM_FUN_RETVAL_STATE_NOT_RUNNING 3


enum E_CM_EXT_FUNCTIONS_CMD_TYPE
{
      E_CM_EXT_FUNCTIONS_CMD_TYPE_SET,
      E_CM_EXT_FUNCTIONS_CMD_TYPE_DEF,
      E_CM_EXT_FUNCTIONS_CMD_TYPE_UNDEF,
      E_CM_EXT_FUNCTIONS_CMD_TYPE_IF,
      E_CM_EXT_FUNCTIONS_CMD_TYPE_CASE,
      E_CM_EXT_FUNCTIONS_CMD_TYPE_WHILE,
      E_CM_EXT_FUNCTIONS_CMD_TYPE_FOR,
      E_CM_EXT_FUNCTIONS_CMD_TYPE_WAIT,
      E_CM_EXT_FUNCTIONS_CMD_TYPE_CALL,
      E_CM_EXT_FUNCTIONS_CMD_TYPE_SYSEXEC,
      E_CM_EXT_FUNCTIONS_CMD_TYPE_FUNDEF,
      E_CM_EXT_FUNCTIONS_CMD_TYPE_KILL,
      E_CM_EXT_FUNCTIONS_CMD_TYPE_DEBUGPRINT
};

enum E_CM_EXT_FUNCTIONS_STATE
{
      E_CM_EXT_FUNCTIONS_STATE_ERROR,
      E_CM_EXT_FUNCTIONS_STATE_DONE,
      E_CM_EXT_FUNCTIONS_STATE_RUNNING,
      E_CM_EXT_FUNCTIONS_STATE_WAIT_INTERRUPT,
      E_CM_EXT_FUNCTIONS_STATE_WAIT_TIME,
      E_CM_EXT_FUNCTIONS_STATE_WAIT_VARS,
      E_CM_EXT_FUNCTIONS_STATE_WAIT_VARS_AND_TIME,
      E_CM_EXT_FUNCTIONS_STATE_CALL,
      E_CM_EXT_FUNCTIONS_STATE_WAIT_CALL
};


enum E_CM_EXT_FUNCTIONS_CALL_MODE1
{
  E_CM_EXT_FUNCTIONS_CALL_MODE1_WAIT, // WaitHereUntilChildDone (default)
  E_CM_EXT_FUNCTIONS_CALL_MODE1_THR   // ThreadedCall "T"
};

enum E_CM_EXT_FUNCTIONS_CALL_MODE2
{
  E_CM_EXT_FUNCTIONS_CALL_MODE2_IF,   // IfRunningDontCall (default)
  E_CM_EXT_FUNCTIONS_CALL_MODE2_KILL, // IfRunningKillOld "K"
  E_CM_EXT_FUNCTIONS_CALL_MODE2_INST  // CreateNewInstance "I"
};


class CCmExtFunAny
{
  public:
    enum E_CM_EXT_FUNCTIONS_CMD_TYPE cmdType;

    std::set<std::string> needVars; //used only at start for cumulateVars() and after is not updated
    std::set<std::string> modifyVars; //used only at start for cumulateVars() and after is not updated
     // set + undef , but not included procedures Then and Else

    std::string Expr; // set
    std::string TimeOut; // wait
    enum E_CM_EXT_FUNCTIONS_CALL_MODE1 CallMode1;
    enum E_CM_EXT_FUNCTIONS_CALL_MODE2 CallMode2;


//    std::string Undef; // undef variable
    std::vector<std::string> IfExpr; // for if only one item, for case more items
    std::vector<class CCmExtFunProc> Then; // for if only one item, for case more items

    CCmExtFunAny();
    bool prepare_expr(class C_CM_MUP_Interface *MUP, std::string &func, std::string &ErrorMessage, std::set<std::string> *_needVars = NULL, std::set<std::string> *_modifyVars = NULL);
    bool define_set(class C_CM_MUP_Interface *MUP, std::string func, std::string &ErrorMessage, bool &Empty);
    bool define_def(class C_CM_MUP_Interface *MUP, std::set<std::string> &_vars, std::string &ErrorMessage);
    bool define_undef(class C_CM_MUP_Interface *MUP, std::set<std::string> &_vars, std::string &ErrorMessage);
    bool define_debugprint(class C_CM_MUP_Interface *MUP, std::vector<std::string> &_params, std::string &ErrorMessage);
    bool define_if(class C_CM_MUP_Interface *MUP, std::string func, std::string &ErrorMessage, std::vector<class CCmExtFunProc> &_Then);
    bool define_case(class C_CM_MUP_Interface *MUP, std::vector<std::string> &_func, std::string &ErrorMessage, std::vector<class CCmExtFunProc> &_Then);
    bool define_for(class C_CM_MUP_Interface *MUP, std::string funcStart, std::string funcCompare, std::string funcIncrement, std::string &ErrorMessage, std::vector<class CCmExtFunProc> &_Then);
    bool define_while(class C_CM_MUP_Interface *MUP, std::string funcCompare, std::string &ErrorMessage, std::vector<class CCmExtFunProc> &_Then);
    bool define_wait(class C_CM_MUP_Interface *MUP, std::string _Expr, std::string _TimeOut, std::vector<std::string> &_IfExpr, std::string &ErrorMessage);
    bool define_call(class C_CM_MUP_Interface *MUP, enum E_CM_EXT_FUNCTIONS_CALL_MODE1 _CallMode1, enum E_CM_EXT_FUNCTIONS_CALL_MODE2 _CallMode2, std::string _Func, std::vector<std::string> &_IfExpr, std::string &ErrorMessage);
    bool define_sysexec(class C_CM_MUP_Interface *MUP, std::string &pidvar, std::vector<std::string> &_IfExpr, std::string &ErrorMessage);
    bool define_fundef(class C_CM_MUP_Interface *MUP, std::string _NameExpr, std::string _CodeExpr, std::string &ErrorMessage);
    bool define_kill(class C_CM_MUP_Interface *MUP, std::string _NamePatternExpr, std::string &ErrorMessage);
    std::string print_me(unsigned verbosity, std::string act_tab, std::string one_tab, std::string delimiter = "\n");
    void cumulateVars(std::set<std::string> &_needVars, std::set<std::string> &_modifyVars);

};


class CCmExtFunProc
{
   public:
    std::set<std::string> needVars;
    std::set<std::string> modifyVars; // set + undef
    std::vector<class CCmExtFunAny> AllCMDs;

    CCmExtFunProc();
    bool add_set(class C_CM_MUP_Interface *MUP, std::string func, std::string &ErrorMessage, bool &Empty);
    bool add_def(class C_CM_MUP_Interface *MUP, std::set<std::string> _vars, std::string &ErrorMessage);
    bool add_undef(class C_CM_MUP_Interface *MUP, std::set<std::string> _vars, std::string &ErrorMessage);
    bool add_debugprint(class C_CM_MUP_Interface *MUP, std::vector<std::string> _params, std::string &ErrorMessage);
    bool add_if(class C_CM_MUP_Interface *MUP, std::string func, std::string &ErrorMessage, std::vector<class CCmExtFunProc> &_Then);
    bool add_case(class C_CM_MUP_Interface *MUP, std::vector<std::string> &_func, std::string &ErrorMessage, std::vector<class CCmExtFunProc> &_Then);
    bool add_for(class C_CM_MUP_Interface *MUP, std::string funcStart, std::string funcCompare, std::string funcIncrement, std::string &ErrorMessage, std::vector<class CCmExtFunProc> &_Then);
    bool add_while(class C_CM_MUP_Interface *MUP, std::string funcCompare, std::string &ErrorMessage, std::vector<class CCmExtFunProc> &_Then);
    bool add_wait(class C_CM_MUP_Interface *MUP, std::string _Expr, std::string _TimeOut, std::vector<std::string> &_IfExpr, std::string &ErrorMessage);
    bool add_call(class C_CM_MUP_Interface *MUP, enum E_CM_EXT_FUNCTIONS_CALL_MODE1 _CallMode1, enum E_CM_EXT_FUNCTIONS_CALL_MODE2 _CallMode2, std::string _Expr, std::vector<std::string> &_IfExpr, std::string &ErrorMessage);
    bool add_sysexec(class C_CM_MUP_Interface *MUP, std::string &pidvar, std::vector<std::string> &_IfExpr, std::string &ErrorMessage);
    bool add_fundef(class C_CM_MUP_Interface *MUP, std::string _NameExpr, std::string _CodeExpr, std::string &ErrorMessage);
    bool add_kill(class C_CM_MUP_Interface *MUP, std::string _NamePatternExpr, std::string &ErrorMessage);
    void clear(void);
    std::string print_me(unsigned verbosity, std::string act_tab, std::string one_tab, std::string delimiter = "\n");
    void cumulateVars(std::set<std::string> &_needVars, std::set<std::string> &_modifyVars);
};




struct SCmExtFunRunParamStruct
{
    double ActTime;
    class C_CM_MUP_Interface *MUP;
    std::map<std::string, CmEventWhat> *UpdateSet;
    std::map<std::string, CmEventWhat> *UpdateSetAll;
//    VarMap *UpdateMap;
    enum E_CM_EXT_FUNCTIONS_STATE *state;
    double *wait_until;
    class CCmFiltersSet *wait_vars;
    std::list<unsigned> *run_point;

    std::string *now_call_function;
    enum E_CM_EXT_FUNCTIONS_CALL_MODE2 *now_call_function_mode2;
    std::vector<std::string> *now_call_function_params;
    std::string *now_fundef_name;
    std::string *now_fundef_code;
    std::string *now_kill_name;

    VarMap *ext_vars;
    VarMap *AllVariables;
    VarEventMap *local_vars;

};

class CCmExtFun
{

  private:
    std::string MyLibCM_name; // only for debug
    std::string name;
    bool instance;
    enum E_CM_EXT_FUNCTIONS_STATE state;
    class CCmExtFunProc main;

    double wait_until;
    class CCmFiltersSet wait_vars;
    std::list<unsigned> run_point;
    VarEventMap start_local_vars;
    VarEventMap local_vars;

    VarMap ext_vars;


    std::string wait_done_function;
    std::string now_call_function;
    enum E_CM_EXT_FUNCTIONS_CALL_MODE2 now_call_function_mode2;
    std::vector<std::string> now_call_function_params;
    std::string now_kill_name;
    std::string now_fundef_name;
    std::string now_fundef_code;


    std::set<std::string> needVars;
    std::set<std::string> modifyVars; // set + undef


//    VarMap FixLoadedGlobalVariables;
//    bool set_local_var(class C_CM_MUP_Interface *MUP, VarMap &local_vars, std::string str_var, std::string func_without_brackets);
    bool set_process_one_block(std::string func, class C_CM_MUP_Interface *MUP, class CCmExtFunProc &current_proc, VarEventMap &local_vars, VarMap &AvailExtVariables);

    bool tryLoadVariableInOut(class C_CM_MUP_Interface *MUP, VarMap *InOutVariables, 
                                    VarMap *GlobalVariables, std::set<std::string> &LoadedVariables, std::string varname);


    unsigned do_Procedure(class CCmExtFunProc &main, struct SCmExtFunRunParamStruct &Params, std::list<unsigned> &current_run_point);

    unsigned do_exec(struct SCmExtFunRunParamStruct &Params);
    unsigned modifyUpdateSet(struct SCmExtFunRunParamStruct &Params, std::string var, unsigned new_param);

    std::string LastError;
    void SetLastError(std::string Inp);

  public:
    CCmExtFun() {  } ;
      ~CCmExtFun() {} ;

    void init(void);
    bool set(std::string func, class C_CM_MUP_Interface *MUP, VarMap &AvailExtVariables, std::set<std::string> &FunUsedVars);
    std::string print_me(unsigned verbosity = 1);
    std::string GetLastError(void);
    unsigned exec(class C_CM_MUP_Interface *MUP, VarMap &MyAllVariables, VarEventMap &UpdateMap, std::map<std::string, CmEventWhat> &UpdateSetAll, class CCmFiltersSet &CmFunWaits, double &WaitTime, bool &NeedNextLoop, double ActTime);
    void set_name(std::string _name) { name = _name; }
    void set_instance(bool val) { instance = val; }
    bool get_instance(void) { return instance; }
    void setMyLibCM_name(std::string inp) { MyLibCM_name = inp; }
    void start_me(class C_CM_MUP_Interface *MUP, const StringList &params);
    bool changed_var(class C_CM_MUP_Interface *MUP, CCmVariable const *var);
    bool add_my_filters(class CCmFiltersSet &myFiltersFun);
    bool get_call_request(std::string &_now_call_function, enum E_CM_EXT_FUNCTIONS_CALL_MODE2 &_now_call_function_mode2, StringList &_now_call_function_params);
    bool get_fundef_request(std::string &_now_fundef_name, std::string &_now_fundef_code);
    bool get_kill_request(std::string &_now_kill_name);
    void set_wait_fun(std::string fun);
    void set_done(void) { state = E_CM_EXT_FUNCTIONS_STATE_DONE; }
    enum E_CM_EXT_FUNCTIONS_STATE get_state(void) { return state ; }
    void info_fun_done(std::string fun);
    bool is_running() { return ((run_point.size()>0) ? true : false); }
};

#endif //USE_LIB_MUPARSERX

#endif // CM_EXT_FUNCTIONS_HH
