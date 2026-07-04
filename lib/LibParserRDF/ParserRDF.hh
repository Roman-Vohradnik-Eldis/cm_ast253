
#ifndef PARSER_RDF_HH
#define PARSER_RDF_HH

#define PARSER_RDF_VERSION "0.0.9"

#include "mup_interface.hh"
#include "RDFAsterix.hh"



enum E_RDF_PARSER_CMD_TYPE
{
      E_CMD_TYPE_SET,
      E_CMD_TYPE_UNDEF,
      E_CMD_TYPE_IF,
      E_CMD_TYPE_CASE,
      E_CMD_TYPE_FOR
};


class CParserRDFany
{
  public:
    enum E_RDF_PARSER_CMD_TYPE cmdType;

    std::set<std::string> needVars; //used only at start for cumulateVars() and after is not updated
    std::set<std::string> modifyVars; //used only at start for cumulateVars() and after is not updated
     // set + undef , but not included procedures Then and Else
    std::set<int>         modifyVars_i; // only UNDEF

    std::string Expr; // set
//    std::string Undef; // undef variable
    std::vector<std::string> IfExpr; // for if only one item, for case more items
    std::vector<class CParserRDFprocedure> Then; // for if only one item, for case more items
    std::vector<class CParserRDFprocedure> Else; // 0 or 1 item, 1 item, if is defined if-else, or case-default

    CParserRDFany();
    bool prepare_expr(class C_CM_MUP_Interface *MUP, std::string &func, std::string &ErrorMessage, std::set<std::string> *_needVars = NULL, std::set<std::string> *_modifyVars = NULL);
    bool define_set(class C_CM_MUP_Interface *MUP, std::string func, std::string &ErrorMessage, bool &Empty);
    bool define_undef(class C_CM_MUP_Interface *MUP, std::set<std::string> &_vars, std::string &ErrorMessage);
    bool define_if(class C_CM_MUP_Interface *MUP, std::string func, std::string &ErrorMessage, std::vector<class CParserRDFprocedure> &_Then, std::vector<class CParserRDFprocedure> &_Else);
    bool define_case(class C_CM_MUP_Interface *MUP, std::vector<std::string> &_func, std::string &ErrorMessage, std::vector<class CParserRDFprocedure> &_Then, std::vector<class CParserRDFprocedure> &_Else);
    bool define_for(class C_CM_MUP_Interface *MUP, std::string funcStart, std::string funcCompare, std::string funcIncrement, std::string &ErrorMessage, std::vector<class CParserRDFprocedure> &_Then);
    std::string print_me(unsigned verbosity, std::string act_tab, std::string one_tab);
    void cumulateVars(std::set<std::string> &_needVars, std::set<std::string> &_modifyVars);
    void optimizeVars(std::set<std::string> *parentVars, std::map<std::string, class CCmVariable> *GlobalVariables, std::map<std::string, class CCmVariable> *InOutVariables);

};

class CParserRDFprocedure
{
  public:

    std::set<std::string> needVars;
    std::set<std::string> needAddVars;
    std::set<int>         needAddVars_i;
    std::set<std::string> modifyVars; // set + undef
    std::set<int>         modifyVars_i; // set + undef
    std::vector<class CParserRDFany> AllCMDs;

    CParserRDFprocedure();
    bool add_set(class C_CM_MUP_Interface *MUP, std::string func, std::string &ErrorMessage, bool &Empty);
    bool add_undef(class C_CM_MUP_Interface *MUP, std::set<std::string> _vars, std::string &ErrorMessage);
    bool add_if(class C_CM_MUP_Interface *MUP, std::string func, std::string &ErrorMessage, std::vector<class CParserRDFprocedure> &_Then, std::vector<class CParserRDFprocedure> &_Else);
    bool add_case(class C_CM_MUP_Interface *MUP, std::vector<std::string> &_func, std::string &ErrorMessage, std::vector<class CParserRDFprocedure> &_Then, std::vector<class CParserRDFprocedure> &_Else);
    bool add_for(class C_CM_MUP_Interface *MUP, std::string funcStart, std::string funcCompare, std::string funcIncrement, std::string &ErrorMessage, std::vector<class CParserRDFprocedure> &_Then);
    void clear(void);
    std::string print_me(unsigned verbosity, std::string act_tab, std::string one_tab);
    void cumulateVars(void);
    void optimizeVars(std::set<std::string> *parentVars, std::map<std::string, class CCmVariable> *GlobalVariables, std::map<std::string, class CCmVariable> *InOutVariables);
};


class CParserRDF
{
  private:
    class CParserRDFprocedure main;

    bool FirstRunParse;
    bool SaveAndRestoreGlobals;

    bool BreakIfRemoveIsTrue;

    std::map<std::string, class CCmVariable> FixLoadedGlobalVariables;

    bool set_process_one_block(std::string func, class C_CM_MUP_Interface *MUP, class CParserRDFprocedure &current_proc);

    bool tryLoadVariableInOut(class C_CM_MUP_Interface *MUP, std::map<std::string, class CCmVariable> *InOutVariables, 
                                    std::map<std::string, class CCmVariable> *GlobalVariables, std::set<std::string> &LoadedVariables, std::string varname);


    template <typename T> bool do_Procedure(class CParserRDFprocedure &main, class C_CM_MUP_Interface *MUP, std::map<std::string, class CCmVariable> *InOutVariables,
                              std::map<std::string, class CCmVariable> *GlobalVariables,
                              std::set<std::string> &LoadedVariables, std::set<int> &LoadedVariables_i, std::set<std::string> &ChangedVariables,std::set<int> &ChangedVariables_i,
                              bool &DoBreak, T &RDFstruct);

    template <typename T> bool do_parse(T   &RDFstruct, class C_CM_MUP_Interface *MUP, std::map<std::string, class CCmVariable> *InOutVariables = NULL, std::map<std::string, class CCmVariable> *GlobalVariables = NULL);

    std::string LastError;
    void SetLastError(std::string Inp);


    bool tryLoadVariablesRDF(int var_i, class C_CM_MUP_Interface *MUP, std::set<int> &LoadedVariables_i,  RDFRadarService  *RDFstruct);
    bool tryLoadVariablesRDF(int var_i, class C_CM_MUP_Interface *MUP, std::set<int> &LoadedVariables_i,  RDFTargetPlot    *RDFstruct);
    bool tryLoadVariablesRDF(int var_i, class C_CM_MUP_Interface *MUP, std::set<int> &LoadedVariables_i,  RDFTargetTrack   *RDFstruct);

    bool tryStoreVariableRDF(int var_i, class C_CM_MUP_Interface *MUP, std::set<int> &ChangedVariables_i, RDFTargetPlot    *RDFstruct);
    bool tryStoreVariableRDF(int var_i, class C_CM_MUP_Interface *MUP, std::set<int> &ChangedVariables_i, RDFTargetTrack   *RDFstruct);
    bool tryStoreVariableRDF(int var_i, class C_CM_MUP_Interface *MUP, std::set<int> &ChangedVariables_i, RDFRadarService  *RDFstruct);


  public:
    CParserRDF() { SaveAndRestoreGlobals = true; FirstRunParse = true; BreakIfRemoveIsTrue=true; } ;
      ~CParserRDF() {} ;
    bool set(std::string func, class C_CM_MUP_Interface *MUP, std::map<std::string, class CCmVariable> *GlobalVariables = NULL, std::map<std::string, class CCmVariable> *InOutVariables = NULL, bool _SaveAndRestoreGlobals = false, bool _BreakIfRemoveIsTrue = true);
    //SaveAndRestoreGlobals : true .. before parse all GlobalVariables stored to MUP and after parse stored to GlobalVariables and cleared from MUP
    //                        false.. GlobalVariables stored to MUP at first call of parse and again keeped in MUP (faster, but for each ParserRDF need extra MUP)

    std::string print_me(unsigned verbosity = 1);
    void get_globals(class C_CM_MUP_Interface *MUP, std::map<std::string, class CCmVariable> *GlobalVariables);
    std::string GetLastError(void);

    bool parse(RDFRadarService  &RDFstruct, class C_CM_MUP_Interface *MUP, std::map<std::string, class CCmVariable> *InOutVariables = NULL, std::map<std::string, class CCmVariable> *GlobalVariables = NULL);
    bool parse(RDFTargetPlot    &RDFstruct, class C_CM_MUP_Interface *MUP, std::map<std::string, class CCmVariable> *InOutVariables = NULL, std::map<std::string, class CCmVariable> *GlobalVariables = NULL);
    bool parse(RDFTargetTrack   &RDFstruct, class C_CM_MUP_Interface *MUP, std::map<std::string, class CCmVariable> *InOutVariables = NULL, std::map<std::string, class CCmVariable> *GlobalVariables = NULL);
};

#endif // PARSER_RDF_HH
