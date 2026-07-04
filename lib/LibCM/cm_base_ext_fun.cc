#if USE_LIB_MUPARSERX

#include "cm_base_ext_fun.hh"
#include "cm_debug.hh"
#include "cm_string.hh"
#include "cm_std_string.hh"
#include "mup_interface.hh"
#include "cm_utils.hh"
#include "cm_base_varevent.hh"

bool LibCM_AutoCreateVar(class C_CM_MUP_Interface *MUP, VarEventMap &local_vars, std::string str_var, std::string func_without_brackets, std::string &ErrorMessage)
{
  if (str_var.find("=", 0) != std::string::npos)
  {
    char prefferedType = ' ';
    std::string str_varN = str_var; // replace \n,\r\,t ' '
    gchar *no_returns = cm_string_strdup_replace_outside_string(str_var.c_str(), "\t\n\r\f\v", ' ');
    if (no_returns)
    {
      if ((strlen(no_returns)>3)&&(no_returns[0]=='(')&&(no_returns[2]==')'))
      {
        prefferedType = no_returns[1];
        str_varN = std::string(no_returns+3);
      } else {
        str_varN = std::string(no_returns);
      }
      g_free(no_returns);
    }
    std::list<std::string> OutList;
    if (!str_varN.empty())
    {
      std::string var_name;
      if (!MUP->GetUsedVariables(str_varN, &OutList, false, true, false, false)) // output
      {
        ErrorMessage = "MUP::ERROR:GetUsedVariables for Local Var \"" + str_varN + "\" -> \'" + MUP->GetLastError() + "\'";
        return false;
      }
      if (OutList.size()!=1)
      {
        ErrorMessage = "Local Var \"" + str_varN + "\" have bad number of modiffied variables ... need 1 , but "+ cm_std_string_print_short_float((float)OutList.size())+" ";
        return false;
      }
      if (!MUP->Evaluate(str_varN))
      {
        ErrorMessage = "MUP::Error (SET Local Var)" + str_varN + " :: " + MUP->GetLastError();
        return false;
      }
      var_name = *OutList.begin();
      class CCmVariable var;
      var.setName(var_name);
      MUP->GetVariableFully(&var, prefferedType);
      MUP->RemoveVariable(var_name);
//     debug_printf("local var is \"%s\"",var.fullPrint().c_str());
      insert_or_replace(local_vars, var_name, VariableEvent(CmEventWhat::define, var)); // TODO: Co ma byt EventWhat???
    }
    return true;
  } else {
    ErrorMessage = "WRONG LOCAL VAR DEFINITION (param nums) VAR:\"" + str_var + "\" have not assigment of forst value : \"" + func_without_brackets + "\"";
    return false;
  }
}



CCmExtFunAny::CCmExtFunAny()
{
}


bool CCmExtFunAny::prepare_expr(class C_CM_MUP_Interface *MUP, std::string &func, std::string &ErrorMessage, std::set<std::string> *_needVars, std::set<std::string> *_modifyVars)
{
  std::string RetVal = func; // replace \n,\r\,t ' '
  gchar *no_returns = cm_string_strdup_replace_outside_string(func.c_str(), "\t\n\r\f\v", ' ');
  if (no_returns)
  {
    RetVal = std::string(no_returns);
    g_free(no_returns);
  }
  func = RetVal;
  std::list<std::string> OutList;
  std::list<std::string> RqList;
  if (!RetVal.empty())
  {
    if (!MUP->GetUsedVariables(RetVal, &OutList, false, true, false, false)) // output
    {
      ErrorMessage = "MUP::ERROR:GetUsedVariables \"" + RetVal + "\" -> \'" + MUP->GetLastError() + "\'";
      return false;
    }
    if (!MUP->GetUsedVariables(RetVal, &RqList, false, false, true)) // input
//    if (!MUP->GetUsedVariables(RetVal, &RqList, false, (bool)false, (bool)true)) // input
    {
      ErrorMessage = "MUP::ERROR:GetUsedVariables \"" + RetVal + "\" -> \'" + MUP->GetLastError() + "\'";
      return false;
    }
  }

//     debug_printf("Prepare(%d):\"%s\"",cmdType,RetVal.c_str());

  for (std::list<std::string>::iterator it = OutList.begin(); it != OutList.end(); it++)
  {
    needVars.insert(*it);
    modifyVars.insert(*it);
//       debug_printf("   ***OUT:%s",it->c_str());
  }
  for (std::list<std::string>::iterator it = RqList.begin(); it != RqList.end(); it++)
  {
    needVars.insert(*it);
//       debug_printf("   ***IN:%s",it->c_str());
  }

  return true;
}

bool CCmExtFunAny::define_set(class C_CM_MUP_Interface *MUP, std::string func, std::string &ErrorMessage, bool &Empty)
{
  cmdType=E_CM_EXT_FUNCTIONS_CMD_TYPE_SET;
  Expr = func;
  bool RetVal = prepare_expr(MUP, Expr, ErrorMessage, &needVars, &modifyVars);
  if (Expr.empty()) Empty = true; else Empty = false;
  return ( Empty ? true : RetVal);
}

bool CCmExtFunAny::define_def(class C_CM_MUP_Interface *MUP, std::set<std::string> &_vars, std::string &ErrorMessage)
{
  cmdType=E_CM_EXT_FUNCTIONS_CMD_TYPE_DEF;
  needVars = _vars;
  modifyVars = _vars;
  return true;
}

bool CCmExtFunAny::define_undef(class C_CM_MUP_Interface *MUP, std::set<std::string> &_vars, std::string &ErrorMessage)
{
  cmdType=E_CM_EXT_FUNCTIONS_CMD_TYPE_UNDEF;
  needVars = _vars;
  modifyVars = _vars;
  return true;
}

bool CCmExtFunAny::define_debugprint(class C_CM_MUP_Interface *MUP, std::vector<std::string> &_params, std::string &ErrorMessage)
{
  cmdType=E_CM_EXT_FUNCTIONS_CMD_TYPE_DEBUGPRINT;
  for (unsigned i=0;i<_params.size();i++)
  {
    if (!prepare_expr(MUP, _params[i], ErrorMessage, &needVars, &modifyVars)) return false;
    IfExpr.push_back(_params[i]);
  }
  return true;
}

bool CCmExtFunAny::define_if(class C_CM_MUP_Interface *MUP, std::string func, std::string &ErrorMessage, std::vector<class CCmExtFunProc> &_Then)
{
  cmdType=E_CM_EXT_FUNCTIONS_CMD_TYPE_IF;
  std::string IfExpr1 = "(" + func + ")";
  if (!prepare_expr(MUP, IfExpr1, ErrorMessage, &needVars, &modifyVars)) return false;
  IfExpr.push_back(IfExpr1);
  Then = _Then;
  return true;
}

bool CCmExtFunAny::define_case(class C_CM_MUP_Interface *MUP, std::vector<std::string> &_func, std::string &ErrorMessage, std::vector<class CCmExtFunProc> &_Then)
{
  cmdType=E_CM_EXT_FUNCTIONS_CMD_TYPE_CASE;
  for (unsigned i=0;i<_func.size();i++)
  {
    std::string IfExpr1 = "(" + _func[i] + ")";
    if (!prepare_expr(MUP, IfExpr1, ErrorMessage, &needVars, &modifyVars)) return false;
    IfExpr.push_back(IfExpr1);
  }
  Then = _Then;
  return true;
}

bool CCmExtFunAny::define_for(class C_CM_MUP_Interface *MUP, std::string funcStart, std::string funcCompare, std::string funcIncrement, std::string &ErrorMessage, std::vector<class CCmExtFunProc> &_Then)
{
  cmdType=E_CM_EXT_FUNCTIONS_CMD_TYPE_FOR;

  std::string IfExpr1;
  IfExpr1 = funcStart;
  if (!prepare_expr(MUP, IfExpr1, ErrorMessage, &needVars, &modifyVars)) return false;
  IfExpr.push_back(IfExpr1);

  IfExpr1 = "(" + funcCompare + ")";
  if (!prepare_expr(MUP, IfExpr1, ErrorMessage, &needVars, &modifyVars)) return false;
  IfExpr.push_back(IfExpr1);

  IfExpr1 = funcIncrement;
  if (!prepare_expr(MUP, IfExpr1, ErrorMessage, &needVars, &modifyVars)) return false;
  IfExpr.push_back(IfExpr1);

  Then = _Then;
  return true;
}

bool CCmExtFunAny::define_while(class C_CM_MUP_Interface *MUP, std::string funcCompare, std::string &ErrorMessage, std::vector<class CCmExtFunProc> &_Then)
{
  cmdType=E_CM_EXT_FUNCTIONS_CMD_TYPE_WHILE;

  std::string IfExpr1;

  IfExpr1 = "(" + funcCompare + ")";
  if (!prepare_expr(MUP, IfExpr1, ErrorMessage, &needVars, &modifyVars)) return false;
  IfExpr.push_back(IfExpr1);

  Then = _Then;
  return true;
}

bool CCmExtFunAny::define_wait(class C_CM_MUP_Interface *MUP, std::string _Expr, std::string _TimeOut, std::vector<std::string> &_IfExpr, std::string &ErrorMessage)
{
  cmdType=E_CM_EXT_FUNCTIONS_CMD_TYPE_WAIT;
  if (_Expr.size()>0)
  {
    std::string IfExpr1 = "(" + _Expr + ")";
    if (!prepare_expr(MUP, IfExpr1, ErrorMessage, &needVars, &modifyVars)) return false;
    Expr = IfExpr1;
  }
  IfExpr = _IfExpr;
  TimeOut = _TimeOut;
  if (TimeOut.size()>0) { if (!prepare_expr(MUP, TimeOut, ErrorMessage, &needVars, &modifyVars)) return false; }
  for (std::vector<std::string>::iterator it = _IfExpr.begin(); it != _IfExpr.end(); it++) needVars.insert(*it); // waiting vars
  return true;
}

bool CCmExtFunAny::define_call(class C_CM_MUP_Interface *MUP, enum E_CM_EXT_FUNCTIONS_CALL_MODE1 _CallMode1, enum E_CM_EXT_FUNCTIONS_CALL_MODE2 _CallMode2, std::string _Expr, std::vector<std::string> &_IfExpr, std::string &ErrorMessage)
{
  cmdType=E_CM_EXT_FUNCTIONS_CMD_TYPE_CALL;
  CallMode1 = _CallMode1;
  CallMode2 = _CallMode2;
  Expr = _Expr;
  IfExpr = _IfExpr;
  return true;
}


bool CCmExtFunAny::define_kill(class C_CM_MUP_Interface *MUP, std::string _NamePatternExpr, std::string &ErrorMessage)
{
  cmdType=E_CM_EXT_FUNCTIONS_CMD_TYPE_KILL;
  Expr = _NamePatternExpr;
  if (Expr.size()>0)
  {
    if (!prepare_expr(MUP, Expr, ErrorMessage, &needVars, &modifyVars)) return false;
  }
  return true;
}

bool CCmExtFunAny::define_fundef(class C_CM_MUP_Interface *MUP, std::string _NameExpr, std::string _CodeExpr, std::string &ErrorMessage)
{
  cmdType=E_CM_EXT_FUNCTIONS_CMD_TYPE_FUNDEF;
  if ((_NameExpr.size()>0)&&(_CodeExpr.size()>0))
  {
    if (!prepare_expr(MUP, _NameExpr, ErrorMessage, &needVars, &modifyVars)) return false;
    if (!prepare_expr(MUP, _CodeExpr, ErrorMessage, &needVars, &modifyVars)) return false;
    IfExpr.resize(2);
    IfExpr[0] = _NameExpr;
    IfExpr[1] = _CodeExpr;
    return true;
  } else {
    ErrorMessage = "Define fundef ... bad parameter";
    if (_NameExpr.size()<1) ErrorMessage += " 1";
    if (_CodeExpr.size()<1) ErrorMessage += " 2";
    return false;
  }
}

bool CCmExtFunAny::define_sysexec(class C_CM_MUP_Interface *MUP, std::string &pidvar, std::vector<std::string> &_IfExpr, std::string &ErrorMessage)
{
  cmdType=E_CM_EXT_FUNCTIONS_CMD_TYPE_SYSEXEC;
  if (pidvar.size()>0)
  {
//    if (!prepare_expr(MUP, pidvar, ErrorMessage, &needVars, &modifyVars)) return false;
//    for (std::set<std::string>::iterator it = needVars.begin(); it != needVars.end(); it++)  modifyVars.insert(*it);
    needVars.insert(pidvar);
    modifyVars.insert(pidvar);
  }
  Expr = pidvar;
  for (unsigned i=0;i<_IfExpr.size();i++)
  {
    std::string IfExpr1 = _IfExpr[i];
    if (!prepare_expr(MUP, IfExpr1, ErrorMessage, &needVars, &modifyVars)) return false;
    IfExpr.push_back(IfExpr1);
  }
  return true;
}

std::string CCmExtFunAny::print_me(unsigned verbosity, std::string act_tab, std::string one_tab, std::string delimiter)
{
  std::string RetVal = std::string();
  if (cmdType==E_CM_EXT_FUNCTIONS_CMD_TYPE_SET) // set
  {
    RetVal += act_tab + Expr + delimiter;
  }
  if (cmdType==E_CM_EXT_FUNCTIONS_CMD_TYPE_DEF) // def
  {
    RetVal += act_tab + "def";
    for (std::set<std::string>::iterator it = modifyVars.begin(); it != modifyVars.end(); it++)
    {
       RetVal += "," + (*it);
//       doplnit mozna jeste primo variable
    }
    RetVal += act_tab + delimiter;
  }
  if (cmdType==E_CM_EXT_FUNCTIONS_CMD_TYPE_UNDEF) // undef
  {
    RetVal += act_tab + "undef";
    for (std::set<std::string>::iterator it = modifyVars.begin(); it != modifyVars.end(); it++)
    {
       RetVal += "," + (*it);
    }
    RetVal += act_tab + delimiter;
  }
  if (cmdType==E_CM_EXT_FUNCTIONS_CMD_TYPE_DEBUGPRINT) // debugprint
  {
    RetVal += act_tab + "debugprint";
    for (std::vector<std::string>::iterator it = IfExpr.begin(); it != IfExpr.end(); it++)
    {
       RetVal += "," + (*it);
    }
    RetVal += act_tab + delimiter;
  }
  if (cmdType==E_CM_EXT_FUNCTIONS_CMD_TYPE_IF) // if
  {
    RetVal += act_tab + "if," + IfExpr[0] + delimiter;
    RetVal += Then[0].print_me(verbosity, act_tab + one_tab, one_tab, delimiter);
    if (Then.size()>1)
    {
      RetVal += act_tab + "else" + delimiter;
      RetVal += Then[1].print_me(verbosity, act_tab + one_tab, one_tab, delimiter);
    }
  }
  if (cmdType==E_CM_EXT_FUNCTIONS_CMD_TYPE_CASE) // case
  {
    for (unsigned c = 0; c < MIN(IfExpr.size(), Then.size()); c++)
    {
      if (c==0)
      {
        RetVal += act_tab + "case," + IfExpr[c] + delimiter;
      } else {
        RetVal += act_tab + "case-next," + IfExpr[c] + delimiter;
      }
      RetVal += Then[c].print_me(verbosity, act_tab + one_tab, one_tab, delimiter);
    }
    if (IfExpr.size() < Then.size())
    {
      RetVal += act_tab + "case-default" + delimiter;
      RetVal += Then[IfExpr.size()].print_me(verbosity, act_tab + one_tab, one_tab, delimiter);
    }
  }
  if (cmdType==E_CM_EXT_FUNCTIONS_CMD_TYPE_FOR) // for
  {
    RetVal += act_tab + "for";
    for (unsigned c = 0; c < 3; c++)
    {
      RetVal += "," + IfExpr[c];
    }
    RetVal += delimiter;
    RetVal += Then[0].print_me(verbosity, act_tab + one_tab, one_tab, delimiter);
  }
  if (cmdType==E_CM_EXT_FUNCTIONS_CMD_TYPE_WHILE) // while
  {
    RetVal += act_tab + "while";
    for (unsigned c = 0; c < 1; c++)
    {
      RetVal += "," + IfExpr[c];
    }
    RetVal += delimiter;
    RetVal += Then[0].print_me(verbosity, act_tab + one_tab, one_tab, delimiter);
  }
  if (cmdType==E_CM_EXT_FUNCTIONS_CMD_TYPE_WAIT) // wait
  {
    RetVal += act_tab + "wait";
    if (Expr.size()>0)  RetVal += ",expr={" + Expr + "}";
                  else  RetVal += ",expr=NO" ;
    if (TimeOut.size()>0) RetVal += ",TimeOut=" + TimeOut;
                else  RetVal += ",noTimeOut" ;
//      RetVal += act_tab + "else" + delimiter;
    for (unsigned c = 0; c < IfExpr.size(); c++)
    {
      RetVal += "," + IfExpr[c];
    }
    RetVal += delimiter;
  }
  if (cmdType==E_CM_EXT_FUNCTIONS_CMD_TYPE_CALL) // wait
  {
    RetVal += act_tab + "call";
    switch (CallMode1)
    {
      case E_CM_EXT_FUNCTIONS_CALL_MODE1_WAIT:
        RetVal += ",MODE1=WaitForDoneChild";
        break;
      case E_CM_EXT_FUNCTIONS_CALL_MODE1_THR:
        RetVal += ",MODE1=ThreadedCall";
        break;
    }
    switch (CallMode2)
    {
      case E_CM_EXT_FUNCTIONS_CALL_MODE2_KILL:
        RetVal += ",MODE2=IfRunningKillOld";
        break;
      case E_CM_EXT_FUNCTIONS_CALL_MODE2_INST:
        RetVal += ",MODE2=CreateNewInstance";
        break;
      case E_CM_EXT_FUNCTIONS_CALL_MODE2_IF:
        RetVal += ",MODE2=IfRunningDontCall";
        break;
    }
    RetVal += "," + Expr;
//      RetVal += act_tab + "else" + delimiter;
    for (unsigned c = 0; c < IfExpr.size(); c++)
    {
      RetVal += "," + IfExpr[c];
    }
    RetVal += delimiter;
  }
  if (cmdType==E_CM_EXT_FUNCTIONS_CMD_TYPE_SYSEXEC)
  {
    RetVal += act_tab + "sysexec";
    if (Expr.size()>0)
    RetVal += ",pidvar=" + Expr;
    for (unsigned c = 0; c < IfExpr.size(); c++)
    {
      RetVal += "," + IfExpr[c];
    }
    RetVal += delimiter;
  }
  if (cmdType==E_CM_EXT_FUNCTIONS_CMD_TYPE_KILL)
  {
    RetVal += act_tab + "kill";
    if (Expr.size()>0) RetVal += "," + Expr;
    RetVal += delimiter;
  }
  if (cmdType==E_CM_EXT_FUNCTIONS_CMD_TYPE_FUNDEF)
  {
    RetVal += act_tab + "fundef";
    if (IfExpr.size()>1)
    RetVal += "," + IfExpr[0] + "," + IfExpr[1];
    RetVal += delimiter;
  }
  return RetVal;
}

void CCmExtFunAny::cumulateVars(std::set<std::string> &_needVars, std::set<std::string> &_modifyVars)
{
//  if ((cmdType==E_CM_EXT_FUNCTIONS_CMD_TYPE_IF) || (cmdType==E_CM_EXT_FUNCTIONS_CMD_TYPE_CASE) || (cmdType==E_CM_EXT_FUNCTIONS_CMD_TYPE_FOR) || (cmdType==E_CM_EXT_FUNCTIONS_CMD_TYPE_WHILE)) // if or case
  {
    for (unsigned u = 0; u<Then.size(); u++) Then[u].cumulateVars(_needVars, _modifyVars);
  }
  for (std::set<std::string>::iterator it = needVars.begin(); it != needVars.end(); it++)
  {
    _needVars.insert(*it);
  }
  for (std::set<std::string>::iterator it = modifyVars.begin(); it != modifyVars.end(); it++)
  {
    _modifyVars.insert(*it);
  }
}


// ************************************************************************************************
// ************************************************************************************************
// ************************************************************************************************
// ************************************************************************************************




CCmExtFunProc::CCmExtFunProc()
{
  clear();
}

bool CCmExtFunProc::add_set(class C_CM_MUP_Interface *MUP, std::string func, std::string &ErrorMessage, bool &Empty)
{
  CCmExtFunAny n;
  bool RetVal = n.define_set(MUP, func, ErrorMessage, Empty);
  if ((RetVal)&&(!Empty))
  {
    AllCMDs.push_back(n);
  }
  return RetVal;
}

bool CCmExtFunProc::add_def(class C_CM_MUP_Interface *MUP, std::set<std::string> _vars, std::string &ErrorMessage)
{
  CCmExtFunAny n;
  bool RetVal = n.define_def(MUP, _vars, ErrorMessage);
  AllCMDs.push_back(n);
  return RetVal;
}

bool CCmExtFunProc::add_undef(class C_CM_MUP_Interface *MUP, std::set<std::string> _vars, std::string &ErrorMessage)
{
  CCmExtFunAny n;
  bool RetVal = n.define_undef(MUP, _vars, ErrorMessage);
  AllCMDs.push_back(n);
  return RetVal;
}

bool CCmExtFunProc::add_debugprint(class C_CM_MUP_Interface *MUP, std::vector<std::string> _params, std::string &ErrorMessage)
{
  CCmExtFunAny n;
  bool RetVal = n.define_debugprint(MUP, _params, ErrorMessage);
  AllCMDs.push_back(n);
  return RetVal;
}

bool CCmExtFunProc::add_if(class C_CM_MUP_Interface *MUP, std::string func, std::string &ErrorMessage, std::vector<class CCmExtFunProc> &_Then)
{
  CCmExtFunAny n;
  bool RetVal = n.define_if(MUP, func, ErrorMessage, _Then);
  AllCMDs.push_back(n);
  return RetVal;
}

bool CCmExtFunProc::add_case(class C_CM_MUP_Interface *MUP, std::vector<std::string> &_func, std::string &ErrorMessage, std::vector<class CCmExtFunProc> &_Then)
{
  CCmExtFunAny n;
  bool RetVal = n.define_case(MUP, _func, ErrorMessage, _Then);
  AllCMDs.push_back(n);
  return RetVal;
}

bool CCmExtFunProc::add_for(class C_CM_MUP_Interface *MUP, std::string funcStart, std::string funcCompare, std::string funcIncrement, std::string &ErrorMessage, std::vector<class CCmExtFunProc> &_Then)
{
  CCmExtFunAny n;
  bool RetVal = n.define_for(MUP, funcStart, funcCompare, funcIncrement, ErrorMessage, _Then);
  AllCMDs.push_back(n);
  return RetVal;
}

bool CCmExtFunProc::add_while(class C_CM_MUP_Interface *MUP, std::string funcCompare, std::string &ErrorMessage, std::vector<class CCmExtFunProc> &_Then)
{
  CCmExtFunAny n;
  bool RetVal = n.define_while(MUP, funcCompare, ErrorMessage, _Then);
  AllCMDs.push_back(n);
  return RetVal;
}

bool CCmExtFunProc::add_wait(class C_CM_MUP_Interface *MUP, std::string _Expr, std::string _TimeOut, std::vector<std::string> &_IfExpr, std::string &ErrorMessage)
{
  CCmExtFunAny n;
  bool RetVal = n.define_wait(MUP, _Expr, _TimeOut, _IfExpr, ErrorMessage);
  AllCMDs.push_back(n);
  return RetVal;
}

bool CCmExtFunProc::add_call(class C_CM_MUP_Interface *MUP, enum E_CM_EXT_FUNCTIONS_CALL_MODE1 _CallMode1, enum E_CM_EXT_FUNCTIONS_CALL_MODE2 _CallMode2, std::string _Expr, std::vector<std::string> &_IfExpr, std::string &ErrorMessage)
{
  CCmExtFunAny n;
  bool RetVal = n.define_call(MUP, _CallMode1, _CallMode2, _Expr, _IfExpr, ErrorMessage);
  AllCMDs.push_back(n);
  return RetVal;
}

bool CCmExtFunProc::add_kill(class C_CM_MUP_Interface *MUP, std::string _NamePatternExpr, std::string &ErrorMessage)
{
  CCmExtFunAny n;
  bool RetVal = n.define_kill(MUP, _NamePatternExpr, ErrorMessage);
  AllCMDs.push_back(n);
  return RetVal;
}

bool CCmExtFunProc::add_fundef(class C_CM_MUP_Interface *MUP, std::string _NameExpr, std::string _CodeExpr, std::string &ErrorMessage)
{
  CCmExtFunAny n;
  bool RetVal = n.define_fundef(MUP, _NameExpr, _CodeExpr, ErrorMessage);
  AllCMDs.push_back(n);
  return RetVal;
}

bool CCmExtFunProc::add_sysexec(class C_CM_MUP_Interface *MUP, std::string &pidvar, std::vector<std::string> &_IfExpr, std::string &ErrorMessage)
{
  CCmExtFunAny n;
  bool RetVal = n.define_sysexec(MUP, pidvar,  _IfExpr, ErrorMessage);
  AllCMDs.push_back(n);
  return RetVal;
}

void CCmExtFunProc::clear(void)
{
  modifyVars.clear();
  needVars.clear();
  AllCMDs.clear();
}

void CCmExtFunProc::cumulateVars(std::set<std::string> &_needVars, std::set<std::string> &_modifyVars)
{
  for (unsigned i=0; i<AllCMDs.size();i++)
  {
    AllCMDs[i].cumulateVars(needVars, modifyVars);
    for (std::set<std::string>::iterator it = needVars.begin(); it != needVars.end(); it++)
    {
      _needVars.insert(*it);
    }
    for (std::set<std::string>::iterator it = modifyVars.begin(); it != modifyVars.end(); it++)
    {
      _modifyVars.insert(*it);
    }
  }
}


std::string CCmExtFunProc::print_me(unsigned verbosity, std::string act_tab, std::string one_tab, std::string delimiter)
{
  std::string RetVal = std::string();

  if (verbosity > 5)
  {
    std::string needStr   = std::string();
    std::string modifyStr = std::string();
    for (std::set<std::string>::iterator it = needVars.begin(); it != needVars.end(); it++)
    {
      needStr += (needStr.empty() ? "" : ",")  + std::string("\'") + (*it) + std::string("\'");
    }
    for (std::set<std::string>::iterator it = modifyVars.begin(); it != modifyVars.end(); it++)
    {
      modifyStr += (modifyStr.empty() ? "" : ",") + std::string("\'") + (*it) + std::string("\'");
    }


    if (verbosity > 8)
    {
      if (!needStr.empty())  RetVal += act_tab + "... need:"  + needStr   + "\n";
    }
    if (!modifyStr.empty())  RetVal += act_tab + "... modif:" + modifyStr + "\n";
  }

  for (unsigned i=0; i<AllCMDs.size();i++)
  {
    RetVal += AllCMDs[i].print_me(verbosity, act_tab, one_tab, delimiter);
  }
  return RetVal;
}




// ************************************************************************************************
// ************************************************************************************************
// ************************************************************************************************
// ************************************************************************************************
bool CCmExtFun::set_process_one_block(std::string func, class C_CM_MUP_Interface *MUP, class CCmExtFunProc &current_proc, VarEventMap &local_vars, VarMap &AvailExtVariables)
{
  std::string func_without_brackets = func;
  StringList OutParams;
  StringList OutParamsNotInterpreted;
  unsigned previous_size = 0;
  unsigned found_items = 1;
  unsigned found_params = 1;
  std::string ErrorMessage;
  do
  {
    previous_size = func_without_brackets.size();
    OutParams.clear();
    cm_std_string_SplitLineAndInterpretStringsInQuotes(func_without_brackets, OutParams, ";", "[](){}", true);
    found_items = OutParams.size();
    if (found_items<2)
    {
      OutParams.clear();
      cm_std_string_SplitLineAndInterpretStringsInQuotes(func_without_brackets, OutParams, ",", "[](){}", true);
      found_params = OutParams.size();
      if (found_items<2)
      {
        func_without_brackets  = cm_std_string_remove_first_brackets_pair_or_interpret_string(func_without_brackets, "()[]{}", " \t\n\r\f\v");
      }
    }
  } while ((previous_size != func_without_brackets.size()) && (found_items<2) && (found_params<2));

  if (found_items>1)
  {
    OutParams.clear();
    cm_std_string_SplitLineAndInterpretStringsInQuotes(func_without_brackets, OutParams, ";", "[](){}", true);
    for (unsigned i=0;i<OutParams.size();i++)
    {
      if (!set_process_one_block(OutParams[i], MUP, current_proc, local_vars, AvailExtVariables)) return false;
    }
  } else {
    if (found_params>1)
    {
      OutParams.clear();
      cm_std_string_SplitLineAndInterpretStringsInQuotes(func_without_brackets, OutParams, ",", "[](){}", true);
      cm_std_string_SplitLineAndInterpretStringsInQuotes(func_without_brackets, OutParamsNotInterpreted, ",", "[](){}", false, false, false);
      if (OutParams.size()>0)
      {
               if (OutParams[0] == "if")
        {
          if ((OutParams.size()==3)||(OutParams.size()==4))
          {
            std::vector<class CCmExtFunProc> Then; Then.clear();
            class CCmExtFunProc Then1;
            Then1.clear();
            if (!set_process_one_block(OutParams[2], MUP, Then1, local_vars, AvailExtVariables)) return false;
            Then.push_back(Then1);
            if (OutParams.size()==4)
            {
              Then1.clear();
              if (!set_process_one_block(OutParams[3], MUP, Then1, local_vars, AvailExtVariables)) return false;
              Then.push_back(Then1);
            }
            if (!current_proc.add_if(MUP, OutParams[1], ErrorMessage, Then))
            {
              SetLastError(ErrorMessage);
              return false;
            }
          } else {
            SetLastError("WRONG IF (param nums) \"" + func_without_brackets + "\"");
            return false;
          }
        } else if (OutParams[0] == "case")
        {
          if (OutParams.size()>=3)
          {
            std::vector<class CCmExtFunProc> Then; Then.clear();
            std::vector<std::string> func; func.clear();
            unsigned cases = (OutParams.size() - 1) / 2;
            for (unsigned actcase=0; actcase<cases; actcase++)
            {
              class CCmExtFunProc Then1;
              Then1.clear();
              if (!set_process_one_block(OutParams[1+(actcase*2)+1], MUP, Then1, local_vars, AvailExtVariables)) return false;
              Then.push_back(Then1);
              func.push_back(OutParams[1+(actcase*2)+0]);
            }
            if (((OutParams.size() - 1) % 2)==1)
            {
              class CCmExtFunProc Else1;
              Else1.clear();
              if (!set_process_one_block(OutParams[OutParams.size()-1], MUP, Else1, local_vars, AvailExtVariables)) return false;
              Then.push_back(Else1);
            }
            if (!current_proc.add_case(MUP, func, ErrorMessage, Then))
            {
              SetLastError(ErrorMessage);
              return false;
            }
          } else {
            SetLastError("WRONG CASE (param nums) \"" + func_without_brackets + "\"");
            return false;
          }
        } else if (OutParams[0] == "for")
        {
          if (OutParams.size()==5)
          {
            std::vector<class CCmExtFunProc> Then; Then.clear();
            class CCmExtFunProc Then1;
            Then1.clear();
            if (!set_process_one_block(OutParams[4], MUP, Then1, local_vars, AvailExtVariables)) return false;
            Then.push_back(Then1);
            if (!current_proc.add_for(MUP, OutParams[1],OutParams[2],OutParams[3], ErrorMessage, Then))
            {
              SetLastError(ErrorMessage);
              return false;
            }
          } else {
            SetLastError("WRONG FOR (param nums) \"" + func_without_brackets + "\"");
            return false;
          }
        } else if (OutParams[0] == "while")
        {
          if (OutParams.size()==3)
          {
            std::vector<class CCmExtFunProc> Then; Then.clear();
            class CCmExtFunProc Then1;
            Then1.clear();
            if (!set_process_one_block(OutParams[2], MUP, Then1, local_vars, AvailExtVariables)) return false;
            Then.push_back(Then1);
            if (!current_proc.add_while(MUP, OutParams[1],ErrorMessage, Then))
            {
              SetLastError(ErrorMessage);
              return false;
            }
          } else {
            SetLastError("WRONG FOR (param nums) \"" + func_without_brackets + "\"");
            return false;
          }
        } else if ((OutParams[0] == "def")||(OutParams[0] == "set"))
        {
          std::set<std::string> vars; vars.clear();
          for (unsigned p=1; p<OutParams.size(); p++)
          {
            vars.insert(OutParams[p]);
            if (AvailExtVariables.find(OutParams[p]) == AvailExtVariables.end())
            {
              SetLastError("DEF/SET for variable \""+ OutParams[p] +"\" but without definition \"" + func_without_brackets + "\"");
              return false;
            }
          }
          if (vars.size()>0)
          {
            if (!current_proc.add_def(MUP, vars, ErrorMessage))
            {
              SetLastError(ErrorMessage);
              return false;
            }
          } else {
            SetLastError("WRONG DEF (no parameters, but minimally one is required) \"" + func_without_brackets + "\"");
            return false;
          }
        } else if ((OutParams[0] == "undef")||(OutParams[0] == "unset"))
        {
          std::set<std::string> vars; vars.clear();
          for (unsigned p=1; p<OutParams.size(); p++)
          {
            vars.insert(OutParams[p]);
            if (AvailExtVariables.find(OutParams[p]) == AvailExtVariables.end())
            {
              SetLastError("UNDEF/UNSET for variable \""+ OutParams[p] +"\" but without definition \"" + func_without_brackets + "\"");
              return false;
            }
          }
          if (vars.size()>0)
          {
            if (!current_proc.add_undef(MUP, vars, ErrorMessage))
            {
              SetLastError(ErrorMessage);
              return false;
            }
          } else {
            SetLastError("WRONG UNDEF (no parameters, but minimally one is required) \"" + func_without_brackets + "\"");
            return false;
          }
        } else if (OutParams[0] == "debugprint")
        {
          std::vector<std::string> params; params.clear();
          for (unsigned p=1; p<OutParamsNotInterpreted.size(); p++)
          {
            params.push_back(OutParamsNotInterpreted[p]);
          }
          if (params.size()>0)
          {
            if (!current_proc.add_debugprint(MUP, params, ErrorMessage))
            {
              SetLastError(ErrorMessage);
              return false;
            }
          } else {
            SetLastError("WRONG DEBUGPRINT (no parameters, but minimally one is required) \"" + func_without_brackets + "\"");
            return false;
          }
        } else if (OutParams[0] == "wait") // WAIT
        {
          if (OutParams.size()>1)
          {
            std::string Expr;
            std::string TimeOut;
            if (OutParams.size()>1) Expr = OutParams[1]; // name
            if (Expr == "true") Expr.clear();
            if ((OutParams.size()>2)&&(OutParams[2].size()>0)) TimeOut = OutParams[2];
            std::vector<std::string> IfExpr;
            for (size_t i=3;i<OutParams.size();i++) IfExpr.push_back(OutParams[i]);
            if (!current_proc.add_wait(MUP, OutParams[1], TimeOut, IfExpr, ErrorMessage))
            {
              SetLastError(ErrorMessage);
              return false;
            }
          } else {
            SetLastError("WRONG WAIT (no parameters, but minimally one is required) \"" + func_without_brackets + "\"");
            return false;
          }
        } else if (OutParams[0] == "sysexec")
        {
          if ((OutParamsNotInterpreted.size()>1)&&(OutParams.size()>1))
          {
            std::string Expr = OutParams[1];
            std::vector<std::string> IfExpr;
            for (size_t i=2;i<OutParamsNotInterpreted.size();i++) IfExpr.push_back(OutParamsNotInterpreted[i]);


            if (!current_proc.add_sysexec(MUP, Expr, IfExpr, ErrorMessage))
            {
              SetLastError(ErrorMessage);
              return false;
            }
          } else {
            SetLastError("WRONG SYSEXEC (param nums) \"" + func_without_brackets + "\"");
            return false;
          }
        } else if (OutParams[0] == "fundef")
        {
          if ((OutParamsNotInterpreted.size()>2)&&(OutParams.size()>2))
          {
            if (!current_proc.add_fundef(MUP, OutParamsNotInterpreted[1], OutParamsNotInterpreted[2], ErrorMessage))
            {
              SetLastError(ErrorMessage);
              return false;
            }
          } else {
            SetLastError("WRONG FUNDEF (param nums) \"" + func_without_brackets + "\"");
            return false;
          }
        } else if (OutParams[0] == "kill")
        {
          if ((OutParamsNotInterpreted.size()>1)&&(OutParams.size()>1))
          {
            if (!current_proc.add_kill(MUP, OutParamsNotInterpreted[1], ErrorMessage))
            {
              SetLastError(ErrorMessage);
              return false;
            }
          } else {
            SetLastError("WRONG KILL (param nums) \"" + func_without_brackets + "\"");
            return false;
          }
        } else if (OutParams[0] == "call") // call ..... params
        {
          if (OutParams.size()>1)
          {
            enum E_CM_EXT_FUNCTIONS_CALL_MODE1 CallMode1 = E_CM_EXT_FUNCTIONS_CALL_MODE1_WAIT;
            enum E_CM_EXT_FUNCTIONS_CALL_MODE2 CallMode2 = E_CM_EXT_FUNCTIONS_CALL_MODE2_IF;
            std::string FlagsS = OutParams[1];

            if (FlagsS.find("T", 0) != std::string::npos) CallMode1 = E_CM_EXT_FUNCTIONS_CALL_MODE1_THR;
            if (FlagsS.find("K", 0) != std::string::npos) CallMode2 = E_CM_EXT_FUNCTIONS_CALL_MODE2_KILL;
            if (FlagsS.find("I", 0) != std::string::npos) CallMode2 = E_CM_EXT_FUNCTIONS_CALL_MODE2_INST;
//          (default):Wait Here Until Child Done / "T":ThreadedCall
//          (default):If Running Dont Call / "K":If Running Kill Old / "I":Create New Instance
            std::string Expr = OutParams[2];
            std::vector<std::string> IfExpr;
            for (size_t i=3;i<OutParams.size();i++) IfExpr.push_back(OutParams[i]);


            if (!current_proc.add_call(MUP, CallMode1, CallMode2, Expr, IfExpr, ErrorMessage))
            {
              SetLastError(ErrorMessage);
              return false;
            }
          } else {
            SetLastError("WRONG CALL (param nums) \"" + func_without_brackets + "\"");
            return false;
          }
        } else if (OutParams[0] == "var") // define local variables
        {
          if (OutParams.size()>1)
          {
            for (size_t i=1;i<OutParams.size();i++)
            {
              std::string ErrorMessage;
              if (!LibCM_AutoCreateVar(MUP, local_vars, OutParams[i], func_without_brackets, ErrorMessage))
              {
                if (!ErrorMessage.empty())  SetLastError(ErrorMessage);
                return false;
              } else {
                if (!ErrorMessage.empty())  SetLastError(ErrorMessage);
              }
            }
          } else {
            SetLastError("WRONG CALL (param nums) \"" + func_without_brackets + "\"");
            return false;
          }
        } else {
          SetLastError("UNKNOWN COMMAND \"" + OutParams[0] + "\" ::: \"" + func_without_brackets + "\"");
          return false;
        }
      }
    } else {
      if ((func.size()==func_without_brackets.size()))
      {
        bool Empty;
        if (!current_proc.add_set(MUP, func_without_brackets, ErrorMessage, Empty))
        {
          SetLastError(ErrorMessage);
          return false;
        }
      } else {
      // again
        if (!set_process_one_block(func_without_brackets, MUP, current_proc, local_vars, AvailExtVariables)) return false;
      }
    }


  }
  return true;
}

bool CCmExtFun::set(std::string func, class C_CM_MUP_Interface *MUP, VarMap &AvailExtVariables, std::set<std::string> &FunUsedVars)
{
  main.clear();
  LastError.clear();
  bool RetVal = set_process_one_block(func, MUP, main, start_local_vars, AvailExtVariables);
  if (RetVal)
  {
    main.cumulateVars(needVars,modifyVars);
  }

  for (VarMap:: iterator it = AvailExtVariables.begin(); it != AvailExtVariables.end(); it++)
  {
    std::string varname = it->first;
    if (needVars.find(varname) != needVars.end())
    {
      ext_vars[varname] = it->second;
      FunUsedVars.insert(varname);
    }
  }
  return RetVal;
}


bool CCmExtFun::tryLoadVariableInOut(class C_CM_MUP_Interface *MUP, VarMap *InOutVariables,
                                    VarMap *GlobalVariables, std::set<std::string> &LoadedVariables, std::string varname)
{
  if (LoadedVariables.find(varname) != LoadedVariables.end()) return true; // already loaded
  if (InOutVariables!=NULL)
  {
    VarMap::iterator it = InOutVariables->find(varname);
    if (it != InOutVariables->end())
    {
      MUP->SetVariable(&it->second);
      LoadedVariables.insert(varname);
      return true;
    }
  }
  return false;
}

unsigned CCmExtFun::modifyUpdateSet(struct SCmExtFunRunParamStruct &Params, std::string varname, unsigned new_param)
{
  unsigned ErrCode = 0;

  bool actExist = false;
  if (Params.UpdateSetAll->find(varname) != Params.UpdateSetAll->end())
  {
      actExist = ((*Params.UpdateSetAll).at(varname) != CmEventWhat::undefine);
  } else {
    actExist = (Params.AllVariables->find(varname) != Params.AllVariables->end());
    if (!actExist) actExist = (Params.local_vars->find(varname) != Params.local_vars->end());
  }

  if (new_param==0) // delete
  {
    if (!actExist)
    {
      cms_ns_if_print("libcmfun",1,"libcm(%s):DO_PROC\"\%s\"-UNDEF(%s) ERROR:Variable is not exist now, IGNORING this UNDEF",MyLibCM_name.c_str(),name.c_str(),varname.c_str());
      ErrCode = 10;
    } else {
	insert_or_replace(*Params.UpdateSet, varname, CmEventWhat::undefine);
	insert_or_replace(*Params.UpdateSetAll, varname, CmEventWhat::undefine);
      Params.MUP->RemoveVariable(varname);
    }
  } else if (new_param==1) // create
  {
    if (actExist)
    {
      cms_ns_if_print("libcmfun",1,"libcm(%s):DO_PROC\"\%s\"-DEF(%s) ERROR:Variable is exist now, IGNORING this DEF",MyLibCM_name.c_str(),name.c_str(),varname.c_str());
      ErrCode = 21;
    } else {
      if (Params.ext_vars->find(varname) == Params.ext_vars->end())
      {
        cms_ns_if_print("libcmfun",1,"libcm(%s):DO_PROC\"\%s\"-DEF(%s) ERROR:Cannot DEF this variable without ExtDefinition",MyLibCM_name.c_str(),name.c_str(),varname.c_str());
        ErrCode = 22;
      } else {
	  insert_or_replace(*Params.UpdateSet, varname, CmEventWhat::define);
	  insert_or_replace(*Params.UpdateSetAll, varname, CmEventWhat::define);
        Params.MUP->SetVariable( &((*Params.ext_vars)[varname]) );
      }
    }
  } else if (new_param==2) // modify
  {
    if (actExist)
    {
      if (Params.UpdateSet->find(varname) == Params.UpdateSet->end())
      {
	  Params.UpdateSet->insert(std::make_pair(varname, CmEventWhat::update));
      }
      if (Params.UpdateSetAll->find(varname) == Params.UpdateSetAll->end())
      {
	  Params.UpdateSetAll->insert(std::make_pair(varname, CmEventWhat::update));
      }
    } else {
      cms_ns_if_print("libcmfun",1,"libcm(%s):DO_PROC\"\%s\"-MODIFY(%s) ERROR:Variable not exist now",MyLibCM_name.c_str(),name.c_str(),varname.c_str());
      ErrCode = 33;
    }
  }
  return ErrCode;
}


unsigned CCmExtFun::do_Procedure(class CCmExtFunProc &proc, struct SCmExtFunRunParamStruct &Params, std::list<unsigned> &current_run_point)
{
  int print_me_verbosity = 1;
  std::list<unsigned> my_run_point = current_run_point;
  int return_to_code_point = -1;
  if (Params.run_point->size()>0)
  {
    return_to_code_point = Params.run_point->front();
    Params.run_point->pop_front();
    if (Params.run_point->size()==0) return_to_code_point++; // skip last command , it's the interrupt/wait...
  }

  cms_ns_if_print("libcmfun",8,"libcm(%s):DO_PROC\"\%s\"-ENTER:return_to_code_point=%d",MyLibCM_name.c_str(),name.c_str(),return_to_code_point);

  unsigned tests;
  bool can_continue_for = true;
  bool can_continue_while = true;
  int return_to_code_point2 = -1;
  for (unsigned i=0;i<proc.AllCMDs.size();i++)
  {
    if ((return_to_code_point==-1)||(return_to_code_point<=(int)i))
    {
      switch (proc.AllCMDs[i].cmdType)
      {
        case E_CM_EXT_FUNCTIONS_CMD_TYPE_SET:
          {
            cms_ns_if_print("libcmfun",12,"libcm(%s):DO_PROC\"\%s\"-SET(%s)",MyLibCM_name.c_str(),name.c_str(),proc.AllCMDs[i].print_me(print_me_verbosity, "", " ","|").c_str());
            if (cm_std_string_trim(proc.AllCMDs[i].Expr).size()>0)
            {
              unsigned ERR = 0;
              for (std::set<std::string>::iterator itm=proc.AllCMDs[i].modifyVars.begin(); itm!=proc.AllCMDs[i].modifyVars.end();itm++)
              {
                ERR |= modifyUpdateSet(Params, *itm, 2); // update, but can be checked, if is exist and => it is not create
              }
              if (ERR == 0)
              {
                if (!Params.MUP->Evaluate(proc.AllCMDs[i].Expr))
                {
                  SetLastError("MUP::Error (SET)" + proc.AllCMDs[i].Expr + " :: " + Params.MUP->GetLastError());
                  return CM_FUN_RETVAL_STATE_ERROR;
                }
              }
            } else {
              cms_ns_if_print("libcmfun",2,"libcm(%s):DO_PROC\"\%s\"-SET EMPTY!!!",MyLibCM_name.c_str(),name.c_str());
            }
          }
          break;
        case E_CM_EXT_FUNCTIONS_CMD_TYPE_DEF:
          {
            cms_ns_if_print("libcmfun",12,"libcm(%s):DO_PROC\"\%s\"-DEF(%s)",MyLibCM_name.c_str(),name.c_str(),proc.AllCMDs[i].print_me(print_me_verbosity, "", " ","|").c_str());
            for (std::set<std::string>::iterator it = proc.AllCMDs[i].modifyVars.begin(); it != proc.AllCMDs[i].modifyVars.end(); it++)
            {
              std::string varname = *it;
              modifyUpdateSet(Params, varname, 1); // update, but can be checked, if is exist and => it is not create
            }
          }
          break;
        case E_CM_EXT_FUNCTIONS_CMD_TYPE_UNDEF:
          {
            cms_ns_if_print("libcmfun",12,"libcm(%s):DO_PROC\"\%s\"-UNDEF(%s)",MyLibCM_name.c_str(),name.c_str(),proc.AllCMDs[i].print_me(print_me_verbosity, "", " ","|").c_str());
            for (std::set<std::string>::iterator it = proc.AllCMDs[i].modifyVars.begin(); it != proc.AllCMDs[i].modifyVars.end(); it++)
            {
              std::string varname = *it;
              modifyUpdateSet(Params, varname, 0); // update, but can be checked, if is exist and => it is not create
            }
          }
          break;
        case E_CM_EXT_FUNCTIONS_CMD_TYPE_DEBUGPRINT: // untested
          {
            std::string OutStr = "";
            for (std::vector<std::string>::iterator it = proc.AllCMDs[i].IfExpr.begin(); it != proc.AllCMDs[i].IfExpr.end(); it++)
            {
              std::string result = "";
              if (return_to_code_point2==-1)
              {
                if (!Params.MUP->Evaluate((*it), result))
                {
                  SetLastError("MUP::Error (DEBUGPRINT)" + (*it) + " :: " + Params.MUP->GetLastError());
                  return CM_FUN_RETVAL_STATE_ERROR;
                }
              }
              OutStr += result;
            }
           debug_printf("%s",OutStr.c_str());
          }
          break;
        case E_CM_EXT_FUNCTIONS_CMD_TYPE_IF:
          tests = MIN(proc.AllCMDs[i].IfExpr.size(), proc.AllCMDs[i].Then.size());
          if (tests>0)
          {
            return_to_code_point2 = -1;
            if (Params.run_point->size()>0)
            {
              return_to_code_point2 = Params.run_point->front();
              Params.run_point->pop_front();
            }
            bool result = false;
            if (return_to_code_point2==-1)
            {
              if (!Params.MUP->Evaluate(proc.AllCMDs[i].IfExpr[0], result))
              {
                SetLastError("MUP::Error (IF)" + proc.AllCMDs[i].IfExpr[0] + " :: " + Params.MUP->GetLastError());
                return CM_FUN_RETVAL_STATE_ERROR;
              }
//             debug_printf("IF (%s)",(result?"true":"false"));
              cms_ns_if_print("libcmfun",12,"libcm(%s):DO_PROC\"\%s\"-IF_TEST(%s)==>%s",MyLibCM_name.c_str(),name.c_str(),proc.AllCMDs[i].print_me(print_me_verbosity, "", " ","|").c_str(),(result?"true":"false"));
//             debug_printf("  MUP (%s)",Params.MUP->ListVariables().c_str());
//             debug_printf("  MUP (%s)",Params.MUP->ListVariables("Sim_A__*").c_str());

            } else {
              result = ((return_to_code_point2 == 0) ? true : false);
//             debug_printf("IF-resume (%s)",(result?"true":"false"));
              cms_ns_if_print("libcmfun",12,"libcm(%s):DO_PROC\"\%s\"-IF_RESUME(%s)==>%s",MyLibCM_name.c_str(),name.c_str(),proc.AllCMDs[i].print_me(print_me_verbosity, "", " ","|").c_str(),(result?"true":"false"));
            }
            if (result)
            {
              cms_ns_if_print("libcmfun",12,"libcm(%s):DO_PROC\"\%s\"-IF_THEN(%s)",MyLibCM_name.c_str(),name.c_str(),proc.AllCMDs[i].Then[0].print_me(print_me_verbosity, "", " ","|").c_str());
              my_run_point.push_back(i); my_run_point.push_back(0); // Then[0]
              unsigned res = do_Procedure(proc.AllCMDs[i].Then[0], Params, my_run_point);
              if (res>0) return res; // leave || error
              my_run_point.pop_back(); my_run_point.pop_back();
//              if (!do_Procedure(proc.AllCMDs[i].Then[0], Params, my_run_point))
//              return false;
            } else if (proc.AllCMDs[i].Then.size()>1)
            {
              cms_ns_if_print("libcmfun",12,"libcm(%s):DO_PROC\"\%s\"-IF_ELSE(%s)",MyLibCM_name.c_str(),name.c_str(),proc.AllCMDs[i].Then[1].print_me(print_me_verbosity, "", " ","|").c_str());
              my_run_point.push_back(i); my_run_point.push_back(1); // Then[1]
              unsigned res = do_Procedure(proc.AllCMDs[i].Then[1], Params, my_run_point);
              if (res>0) return res; // leave || error
              my_run_point.pop_back(); my_run_point.pop_back();
            }
          } else return CM_FUN_RETVAL_STATE_ERROR;
//          if (DoBreak) return 0;
          break;
        case E_CM_EXT_FUNCTIONS_CMD_TYPE_CASE:
          tests = MIN(proc.AllCMDs[i].IfExpr.size(), proc.AllCMDs[i].Then.size());
          if (tests>0)
          {
            return_to_code_point2 = -1;
            if (Params.run_point->size()>0)
            {
              return_to_code_point2 = Params.run_point->front();
              Params.run_point->pop_front();
            }
            if (return_to_code_point2 == -1)
            {
//           debug_printf("CASE");
              bool result = false;
              for (unsigned u=0;(u<tests)&&(!result);u++)
              {
                if (!Params.MUP->Evaluate(proc.AllCMDs[i].IfExpr[u], result))
                {
                  SetLastError("MUP::Error (CASE)" + proc.AllCMDs[i].IfExpr[u] + " :: " + Params.MUP->GetLastError());
                  return CM_FUN_RETVAL_STATE_ERROR;
                }
                if (result)
                {
//           debug_printf("CASE-THEN[?]");
                  my_run_point.push_back(i); my_run_point.push_back(u);
                  unsigned res = do_Procedure(proc.AllCMDs[i].Then[u], Params, my_run_point);
                  if (res>0) return res; // leave || error
                  my_run_point.pop_back(); my_run_point.pop_back();
                }
              }
              if ((!result)&&(proc.AllCMDs[i].IfExpr.size()<proc.AllCMDs[i].Then.size()))
              { // else
//             debug_printf("CASE-ELSE");

                my_run_point.push_back(i); my_run_point.push_back(proc.AllCMDs[i].IfExpr.size());
                unsigned res = do_Procedure(proc.AllCMDs[i].Then[proc.AllCMDs[i].IfExpr.size()], Params, my_run_point);
                if (res>0) return res; // leave || error
                my_run_point.pop_back(); my_run_point.pop_back();
              }
            } else {
//           debug_printf("CASE-BACK");
              my_run_point.push_back(i); my_run_point.push_back(return_to_code_point2);
              unsigned res = do_Procedure(proc.AllCMDs[i].Then[return_to_code_point2], Params, my_run_point);
              if (res>0) return res; // leave || error
              my_run_point.pop_back(); my_run_point.pop_back();
            }
          } else return CM_FUN_RETVAL_STATE_ERROR;
  //        if (DoBreak) return 0;
          break;
        case E_CM_EXT_FUNCTIONS_CMD_TYPE_FOR:
          return_to_code_point2 = -1;
          if (Params.run_point->size()>0)
          {
            return_to_code_point2 = Params.run_point->front();
            Params.run_point->pop_front();
          }
          if (return_to_code_point2 == -1)
          {
            cms_ns_if_print("libcmfun",12,"libcm(%s):DO_PROC\"\%s\"-FOR_START(%s)",MyLibCM_name.c_str(),name.c_str(),proc.AllCMDs[i].print_me(print_me_verbosity, "", " ","|").c_str());
            unsigned ERR = 0;
            for (std::set<std::string>::iterator itm=proc.AllCMDs[i].modifyVars.begin(); itm!=proc.AllCMDs[i].modifyVars.end();itm++)
            {
              ERR |= modifyUpdateSet(Params, *itm, 2); // update, but can be checked, if is exist and => it is not created
            }
            if (ERR == 0)
            {
              if (!Params.MUP->Evaluate(proc.AllCMDs[i].IfExpr[0])) // start
              {
                SetLastError("MUP::Error (FOR:start)" + proc.AllCMDs[i].IfExpr[0] + " :: " + Params.MUP->GetLastError());
                return CM_FUN_RETVAL_STATE_ERROR;
              }
            }
          }
          can_continue_for = true;
          while (can_continue_for)
          {
            bool result = false;
//           debug_printf("FOR");
            if (return_to_code_point2 == -1)
            {
//           debug_printf("FOR-TEST");
              if (!Params.MUP->Evaluate(proc.AllCMDs[i].IfExpr[1], result))
              {
                SetLastError("MUP::Error (FOR:cond)" + proc.AllCMDs[i].IfExpr[1] + " :: " + Params.MUP->GetLastError());
                return CM_FUN_RETVAL_STATE_ERROR;
              }
            } else result = true;
            if (!result) can_continue_for = false;
            if (can_continue_for)
            {
//           debug_printf("FOR-THEN");
              my_run_point.push_back(i); my_run_point.push_back(0);
              unsigned res = do_Procedure(proc.AllCMDs[i].Then[0], Params, my_run_point);



              if (res>0) return res; // leave || error
              my_run_point.pop_back(); my_run_point.pop_back();
              return_to_code_point2 = -1;
              if (Params.run_point->size()==0) return_to_code_point = -1;


  //            if (DoBreak) return true;
              unsigned ERR = 0;
              cms_ns_if_print("libcmfun",12,"libcm(%s):DO_PROC\"\%s\"-FOR_INCR(%s)",MyLibCM_name.c_str(),name.c_str(),proc.AllCMDs[i].print_me(print_me_verbosity, "", " ","|").c_str());
              for (std::set<std::string>::iterator itm=proc.AllCMDs[i].modifyVars.begin(); itm!=proc.AllCMDs[i].modifyVars.end();itm++)
              {
                ERR |= modifyUpdateSet(Params, *itm, 2); // update, but can be checked, if is exist and => it is not create
              }
              if (ERR == 0)
              {
                if (!Params.MUP->Evaluate(proc.AllCMDs[i].IfExpr[2])) // increment
                {
                  SetLastError("MUP::Error (FOR:incr)" + proc.AllCMDs[i].IfExpr[2] + " :: " + Params.MUP->GetLastError());
                  return CM_FUN_RETVAL_STATE_ERROR;
                }
              }
            }
          }
  //        if (DoBreak) return 0;
          break;
        case E_CM_EXT_FUNCTIONS_CMD_TYPE_WHILE:
          return_to_code_point2 = -1;
          if (Params.run_point->size()>0)
          {
            return_to_code_point2 = Params.run_point->front();
            Params.run_point->pop_front();
          }
          can_continue_while = true;
          while (can_continue_while)
          {
//           debug_printf("WHILE");
            bool result = false;
            if (return_to_code_point2 == -1)
            {
//           debug_printf("WHILE-TEST");
              if (!Params.MUP->Evaluate(proc.AllCMDs[i].IfExpr[0], result))
              {
                SetLastError("MUP::Error (WHILE:cond)" + proc.AllCMDs[i].IfExpr[0] + " :: " + Params.MUP->GetLastError());
                return CM_FUN_RETVAL_STATE_ERROR;
              }
            } else result = true;
            if (!result) can_continue_while = false;
            if (can_continue_while)
            {
//           debug_printf("WHILE-THEN");
              my_run_point.push_back(i); my_run_point.push_back(0);
              unsigned res = do_Procedure(proc.AllCMDs[i].Then[0], Params, my_run_point);
              if (res>0) return res; // leave || error
              my_run_point.pop_back(); my_run_point.pop_back();

//              if (DoBreak) return 0;
              return_to_code_point2 = -1;
              if (Params.run_point->size()==0) return_to_code_point = -1;
            }
          }
//        if (DoBreak) return 0;
          break;
        case E_CM_EXT_FUNCTIONS_CMD_TYPE_KILL:
          if (proc.AllCMDs[i].Expr.size()>0)
          {
            std::string funname;
            if (!Params.MUP->Evaluate(proc.AllCMDs[i].Expr, funname))
            {
              SetLastError("MUP::Error (KILL)" + (proc.AllCMDs[i].Expr) + " :: " + Params.MUP->GetLastError()); // tady to je asi zbytecny!! nejak jinak to chce predat do vedlejsiho threadu
              return CM_FUN_RETVAL_STATE_ERROR;
            }

//           debug_printf("KILL:%s",funname.c_str());
            *Params.now_kill_name = funname;

            my_run_point.push_back(i); *Params.run_point = my_run_point;
            return CM_FUN_RETVAL_STATE_LEAVE; // leave
          }
          break;
        case E_CM_EXT_FUNCTIONS_CMD_TYPE_FUNDEF:
          if (proc.AllCMDs[i].IfExpr.size()>1)
          {
            std::string funname;
            std::string funcode;

            if (!Params.MUP->Evaluate(proc.AllCMDs[i].IfExpr[0], funname))
            {
              SetLastError("MUP::Error (FUNDEF::name)" + (proc.AllCMDs[i].IfExpr[0]) + " :: " + Params.MUP->GetLastError()); // tady to je asi zbytecny!! nejak jinak to chce predat do vedlejsiho threadu
              return CM_FUN_RETVAL_STATE_ERROR;
            }
            if (!Params.MUP->Evaluate(proc.AllCMDs[i].IfExpr[1], funcode))
            {
              SetLastError("MUP::Error (FUNDEF::code)" + (proc.AllCMDs[i].IfExpr[1]) + " :: " + Params.MUP->GetLastError()); // tady to je asi zbytecny!! nejak jinak to chce predat do vedlejsiho threadu
              return CM_FUN_RETVAL_STATE_ERROR;
            }

//           debug_printf("FUN:%s,CODE:%s",funname.c_str(),funcode.c_str());
            *Params.now_fundef_name = funname;
            *Params.now_fundef_code = funcode;

            my_run_point.push_back(i); *Params.run_point = my_run_point;
            return CM_FUN_RETVAL_STATE_LEAVE; // leave

          }
          break;
        case E_CM_EXT_FUNCTIONS_CMD_TYPE_SYSEXEC:
          if (proc.AllCMDs[i].IfExpr.size()>0)
          {
            pid_t child_pid;
            if((child_pid = fork()) < 0 ) // POZOR !!! DULEZITE !!!  nevolat "fork", ale "vfork" ... ten nekopiruje nepotrebne veci a pak stihame i plnit vysilac
            {
              SetLastError("MUP::Error (SYSEXEC:fork)");
              return CM_FUN_RETVAL_STATE_ERROR;
            }
            if(child_pid == 0)
            {
              std::string mycmd;
              std::string result = "";
              std::string actstr = *(proc.AllCMDs[i].IfExpr.begin());
              if (!Params.MUP->Evaluate(actstr, result))
              {
                SetLastError("MUP::Error (SYSEXEC[0])" + (actstr) + " :: " + Params.MUP->GetLastError()); // tady to je asi zbytecny!! nejak jinak to chce predat do vedlejsiho threadu
                return CM_FUN_RETVAL_STATE_ERROR;
              }
              char *command = new char [result.size()+1];
              strcpy(command, result.c_str());
              char **argv = new char* [proc.AllCMDs[i].IfExpr.size()+1];
//              for (unsigned n=0; n<proc.AllCMDs[i].IfExpr.size(); n++)
              size_t n = 0;
              for (std::vector<std::string>::iterator it = proc.AllCMDs[i].IfExpr.begin(); it != proc.AllCMDs[i].IfExpr.end(); it++)
              {
                std::string actstr = *(it);

                if (!Params.MUP->Evaluate(actstr, result))
                {
                  char wstrN [50]; sprintf(wstrN,"%d",(int)n);
                  SetLastError("MUP::Error (SYSEXEC["+std::string(wstrN)+"])" + (actstr) + " :: " + Params.MUP->GetLastError()); // tady to je asi zbytecny!! nejak jinak to chce predat do vedlejsiho threadu
                  return CM_FUN_RETVAL_STATE_ERROR;
                }
                mycmd += "," + result;
                argv[n] = new char [result.size()+1];
                strcpy(argv[n], result.c_str());
                n++;
              }
              argv[proc.AllCMDs[i].IfExpr.size()]=NULL;
//             debug_printf("CMD:%s",mycmd.c_str());

              execv(command, argv);

              for (n=0; n<proc.AllCMDs[i].IfExpr.size(); n++)
              {
                delete argv[n];
              }
              delete[] argv;
              delete[] command;
//              _exit(1);
            } else {
              if (proc.AllCMDs[i].Expr.size()>0)
              {
                unsigned ERR = 0;
                for (std::set<std::string>::iterator itm=proc.AllCMDs[i].modifyVars.begin(); itm!=proc.AllCMDs[i].modifyVars.end();itm++)
                {
                  ERR |= modifyUpdateSet(Params, *itm, 2); // update, but can be checked, if is exist and => it is not create
                }
//                if (ERR)debug_printf("ERROR WHILE STORE CHILD PID=%d",(int)child_pid);
//               debug_printf("CHILD PID=%d",(int)child_pid);
                Params.MUP->SetVariable(proc.AllCMDs[i].Expr,(int)child_pid);
              }
//            wait(NULL);
            }
          }
          break;
        case E_CM_EXT_FUNCTIONS_CMD_TYPE_CALL:
            cms_ns_if_print("libcmfun",12,"libcm(%s):DO_PROC\"\%s\"-CALL(%s)",MyLibCM_name.c_str(),name.c_str(),proc.AllCMDs[i].print_me(print_me_verbosity, "", " ","|").c_str());

            my_run_point.push_back(i);
            *Params.run_point = my_run_point;
//           debug_printf("libcm(%s):DO_PROC\"\%s\"-CALL(%s)",MyLibCM_name.c_str(),name.c_str(),proc.AllCMDs[i].print_me(print_me_verbosity, "", " ","|").c_str());

            *Params.now_call_function = proc.AllCMDs[i].Expr;
            *Params.now_call_function_mode2 = proc.AllCMDs[i].CallMode2;
            *Params.now_call_function_params = proc.AllCMDs[i].IfExpr;


            switch (proc.AllCMDs[i].CallMode1)
            {
              case E_CM_EXT_FUNCTIONS_CALL_MODE1_WAIT:
                *Params.state = E_CM_EXT_FUNCTIONS_STATE_WAIT_CALL;
                break;
              case E_CM_EXT_FUNCTIONS_CALL_MODE1_THR:
                *Params.state = E_CM_EXT_FUNCTIONS_STATE_CALL;
                break;
            }
            return CM_FUN_RETVAL_STATE_LEAVE; // leave

          break;
        case E_CM_EXT_FUNCTIONS_CMD_TYPE_WAIT:
              cms_ns_if_print("libcmfun",12,"libcm(%s):DO_PROC\"\%s\"-WAIT(%s)",MyLibCM_name.c_str(),name.c_str(),proc.AllCMDs[i].print_me(print_me_verbosity, "", " ","|").c_str());
              bool result = true;
              if (proc.AllCMDs[i].Expr.size()>0)
              {
                result = false;
                if (!Params.MUP->Evaluate(proc.AllCMDs[i].Expr, result))
                {
                  SetLastError("MUP::Error (WAIT-COND)" + proc.AllCMDs[i].Expr + " :: " + Params.MUP->GetLastError());
                  return CM_FUN_RETVAL_STATE_ERROR;
                }
              }
              if (result)
              {
                my_run_point.push_back(i);
                *Params.run_point = my_run_point;

                bool condTime = false;
                bool condVars = false;
                double TimeOut = -1.0;

                if (proc.AllCMDs[i].TimeOut.size()>0)
                {
                  if (!Params.MUP->Evaluate(proc.AllCMDs[i].TimeOut, TimeOut))
                  {
                    SetLastError("MUP::Error (WAIT-TIMEOUT)" + proc.AllCMDs[i].TimeOut + " :: " + Params.MUP->GetLastError());
                    return CM_FUN_RETVAL_STATE_ERROR;
                  }
                }

                if (TimeOut>=0.0)
                {
                  condTime = true;
                  (*Params.wait_until) = Params.ActTime + TimeOut;
                }
                Params.wait_vars->clear();
                for (unsigned c = 0; c < proc.AllCMDs[i].IfExpr.size(); c++)
                {
                  condVars = true;
                  Params.wait_vars->add(proc.AllCMDs[i].IfExpr[c]);
                }
                if (condTime && condVars)
                {
                  *Params.state = E_CM_EXT_FUNCTIONS_STATE_WAIT_VARS_AND_TIME;
//                  debug_printf("WAIT-VARS-TIME=(%f => %f) | %s",TimeOut,(*Params.wait_until),Params.wait_vars->fullPrint(",").c_str());
                } else if (condTime)
                {
                  *Params.state = E_CM_EXT_FUNCTIONS_STATE_WAIT_TIME;
//                  debug_printf("WAIT-TIME=(%f => %f)",TimeOut,(*Params.wait_until));
                } else if (condVars)
                {
                  *Params.state = E_CM_EXT_FUNCTIONS_STATE_WAIT_VARS;
//                  debug_printf("WAIT-VARS=%s",Params.wait_vars->fullPrint(",").c_str());
                } else {
                  *Params.state = E_CM_EXT_FUNCTIONS_STATE_WAIT_INTERRUPT;
//                  debug_printf("WAIT-INTERRUPT");
                }
                return CM_FUN_RETVAL_STATE_LEAVE; // leave
              }

          break;
      }
      if (Params.run_point->size()==0) return_to_code_point = -1;
    }
  }
  return CM_FUN_RETVAL_STATE_DONE;
}







unsigned CCmExtFun::do_exec(struct SCmExtFunRunParamStruct &Params)
{
  unsigned RetVal = CM_FUN_RETVAL_STATE_ERROR;
  std::list<unsigned> my_run_point;
  my_run_point.clear();
  cms_ns_if_print("libcmfun",8,"libcm(%s):DO_EXEC\"\%s\"-GO",MyLibCM_name.c_str(),name.c_str());
  RetVal = do_Procedure(main, Params, my_run_point);

  if (RetVal == CM_FUN_RETVAL_STATE_ERROR) // error
  {
    cms_ns_if_print("libcmfun",1,"libcm(%s):DO_EXEC\"\%s\"-ERROR : %s!",MyLibCM_name.c_str(),name.c_str(),LastError.c_str());
  }
  if (RetVal == CM_FUN_RETVAL_STATE_LEAVE) // Leave - interrupt
  {
    cms_ns_if_print("libcmfun",8,"libcm(%s):DO_EXEC\"\%s\"-Leave - interrupt",MyLibCM_name.c_str(),name.c_str());
  }
  return RetVal;
}



std::string CCmExtFun::print_me(unsigned verbosity)
{
  std::string RetVal = std::string();
  RetVal += main.print_me(verbosity, "", "    ");
  return RetVal;
}

void CCmExtFun::SetLastError(std::string Inp)
{
  LastError = Inp;
}

std::string CCmExtFun::GetLastError(void)
{
  std::string RetVal = LastError;
  LastError.clear();
  return RetVal;
}

// ------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------



unsigned CCmExtFun::exec(class C_CM_MUP_Interface *MUP, VarMap &MyAllVariables, VarEventMap &UpdateMap, std::map<std::string, CmEventWhat> &UpdateSetAll, class CCmFiltersSet &CmFunWaits, double &WaitTime, bool &NeedNextLoop, double ActTime)
{
//  cms_ns_if_print("libcmfun",8,"libcm(%s):EXEC\"\%s\"-ENTER",MyLibCM_name.c_str(),name.c_str());

  std::map<std::string, CmEventWhat> MyUpdateSet;
  struct SCmExtFunRunParamStruct Params;
  Params.ActTime = ActTime;
  Params.MUP = MUP;
  Params.UpdateSet = &MyUpdateSet;
  Params.UpdateSetAll = &UpdateSetAll;
  Params.state = &state;
  Params.wait_vars = &wait_vars;
  Params.wait_until = &wait_until;
  Params.run_point = &run_point;
  Params.now_kill_name = &now_kill_name;
  Params.now_fundef_name = &now_fundef_name;
  Params.now_fundef_code = &now_fundef_code;
  Params.now_call_function = &now_call_function;
  Params.now_call_function_mode2 = &now_call_function_mode2;
  Params.now_call_function_params = &now_call_function_params;
  Params.ext_vars = &ext_vars;
  Params.AllVariables = &MyAllVariables;
  Params.local_vars = &local_vars;

  unsigned RetVal = CM_FUN_RETVAL_STATE_NOT_RUNNING;
  if (state == E_CM_EXT_FUNCTIONS_STATE_CALL) state = E_CM_EXT_FUNCTIONS_STATE_RUNNING;
  if (state == E_CM_EXT_FUNCTIONS_STATE_WAIT_INTERRUPT) state = E_CM_EXT_FUNCTIONS_STATE_RUNNING;
  if ((state == E_CM_EXT_FUNCTIONS_STATE_WAIT_TIME) || (state == E_CM_EXT_FUNCTIONS_STATE_WAIT_VARS_AND_TIME))
  {
    if (ActTime >=wait_until)
    {
      state = E_CM_EXT_FUNCTIONS_STATE_RUNNING;
    }
  }

  if (state == E_CM_EXT_FUNCTIONS_STATE_RUNNING)
  {
    for (VarEventMap::iterator itv = local_vars.begin(); itv != local_vars.end();itv++)
    {
      MUP->SetVariable(&itv->second.variable);
    }
    *Params.wait_until = 0.0;
//    cms_ns_if_print("libcmfun",8,"libcm(%s):EXEC\"\%s\"-CALL",MyLibCM_name.c_str(),name.c_str());
    RetVal = do_exec(Params);

    if (RetVal!=CM_FUN_RETVAL_STATE_ERROR) // not error
    {
      if ((RetVal==CM_FUN_RETVAL_STATE_LEAVE)&&(state == E_CM_EXT_FUNCTIONS_STATE_CALL)) NeedNextLoop = true;
      if ((RetVal==CM_FUN_RETVAL_STATE_LEAVE)&&(state == E_CM_EXT_FUNCTIONS_STATE_WAIT_INTERRUPT)) NeedNextLoop = true;
      for (std::map<std::string, CmEventWhat>::iterator its = MyUpdateSet.begin(); its != MyUpdateSet.end(); its++)
      {
        std::string varname = its->first;
        CmEventWhat EventWhat = its->second;
        VarEventMap::iterator itLocal = local_vars.find(varname);
        VarMap::iterator itExt = ext_vars.find(varname);
        VarMap::iterator itAll = MyAllVariables.find(varname);
        if (itLocal != local_vars.end())
        {
	    if ((EventWhat == CmEventWhat::define) || (EventWhat == CmEventWhat::update))
          {
            class CCmVariable Nvar = itLocal->second.variable;
            MUP->GetVariable(&Nvar);
            MUP->RemoveVariable(varname);
	    VariableEvent ev(EventWhat, Nvar);
            // Nvar.EventWhat = EventWhat;
            if (!ev.variable.compare(itLocal->second.variable))
            {
              cms_ns_if_print("libcmfun",10,"libcm(%s):PROC\"\%s\"-UpdatedLocalVar:%s",MyLibCM_name.c_str(),name.c_str(),Nvar.fullPrint().c_str());
              // itLocal->second = Nvar;
	      itLocal->second = ev;
            }
          }
          if (EventWhat == CmEventWhat::undefine)
          {
            local_vars.erase(itLocal); //  a uz to nejde vratit!! asi nesmyslna varianta a vetev
          }
        } else if (itAll != MyAllVariables.end())
        {
          class CCmVariable Nvar = itAll->second;
          bool changed = false;
          if ((EventWhat == CmEventWhat::define) || (EventWhat == CmEventWhat::update))
          {
            MUP->GetVariable(&Nvar);
            if (!Nvar.compare(itAll->second)) changed = true;
          } else if ((EventWhat == CmEventWhat::undefine) && (itExt != ext_vars.end())) // delete
          {
            changed = true;
          }
          if (changed)
          {
            // Nvar.EventWhat = EventWhat;
	      insert_or_replace(UpdateMap, varname, VariableEvent(EventWhat, Nvar));
            cms_ns_if_print("libcmfun",10,"libcm(%s):PROC\"\%s\"-UpdatedGlobalVar:%s",MyLibCM_name.c_str(),name.c_str(),Nvar.fullPrint().c_str());
          }
        } else if ((itExt != ext_vars.end()) && (EventWhat == CmEventWhat::define)) // create Ext
        {
          class CCmVariable Nvar = itExt->second;
          MUP->GetVariable(&Nvar);
          // Nvar.EventWhat = EventWhat;
          // UpdateMap[varname] = Nvar;
	  insert_or_replace(UpdateMap, varname, VariableEvent(EventWhat, Nvar));
          cms_ns_if_print("libcmfun",10,"libcm(%s):PROC\"\%s\"-CreateExtVar:%s",MyLibCM_name.c_str(),name.c_str(),Nvar.fullPrint().c_str());
        } else {
	    cms_ns_if_print("libcmfun",1,"libcm(%s):PROC\"\%s\":UNKNOWN VARIABLE update request: variable=\"%s\", EventWhat=%s",MyLibCM_name.c_str(),name.c_str(),varname.c_str(), EventWhat.to_string().c_str());
        }
      }
      if (RetVal==CM_FUN_RETVAL_STATE_DONE) // not error
      {
        cms_ns_if_print("libcmfun",5,"libcm(%s):EXEC\"\%s\"-DONE",MyLibCM_name.c_str(),name.c_str());
        state = E_CM_EXT_FUNCTIONS_STATE_DONE;
      }
    } else {
      state = E_CM_EXT_FUNCTIONS_STATE_ERROR;
      cms_ns_if_print("libcmfun",1,"libcm(%s):EXEC\"\%s\"-ERROR - STOP",MyLibCM_name.c_str(),name.c_str());
      // ERROR!!!!!!!!!!!!!!!!
    }
//    MUP -> local_vars ... remove from UpdateMap ....
  }
  WaitTime = *Params.wait_until;
  return RetVal;
}


void CCmExtFun::init(void)
{
  name.clear();
  main.clear();
  state = E_CM_EXT_FUNCTIONS_STATE_DONE;
  wait_until = 0.0;
  wait_vars.clear();
  run_point.clear();
  ext_vars.clear();
  local_vars.clear();
  start_local_vars.clear();
}

void CCmExtFun::start_me(class C_CM_MUP_Interface *MUP, const StringList &params)
{
  local_vars = start_local_vars;
  for (size_t i=0;i<params.size();i++)
  {
    std::string ErrorMessage;
    if (!LibCM_AutoCreateVar(MUP, local_vars, params[i], "", ErrorMessage))
    {
      if (!ErrorMessage.empty())  SetLastError(ErrorMessage);
//      return false;
    } else {
      if (!ErrorMessage.empty())  SetLastError(ErrorMessage);
    }
  }
  run_point.clear();
  state = E_CM_EXT_FUNCTIONS_STATE_RUNNING;
}


bool CCmExtFun::changed_var(class C_CM_MUP_Interface *MUP, CCmVariable const *var)
{
  bool RetVal = false;
  if ((state == E_CM_EXT_FUNCTIONS_STATE_WAIT_VARS) || (state == E_CM_EXT_FUNCTIONS_STATE_WAIT_VARS_AND_TIME))
  {
    std::string name = var->getName();
    if (wait_vars.test(name))
    {
      state = E_CM_EXT_FUNCTIONS_STATE_RUNNING;
      RetVal = true;
    }
  }
  return RetVal;
}


bool CCmExtFun::add_my_filters(class CCmFiltersSet &myFiltersFun)
{
  bool RetVal = false;
//  myFiltersFun.add("Sim_A__SW_LOCK");

  for (std::set<std::string>::iterator it = needVars.begin(); it != needVars.end(); it++)
  {
    std::string varname = *it;
    VarEventMap::iterator itLocal = start_local_vars.find(varname);
    if (itLocal == start_local_vars.end())
    {
//      VarMap::iterator itExt = ext_vars.find(varname);
//      if (itExt == ext_vars.end())
//      {
        bool res = myFiltersFun.add(varname);
//       debug_printf("NEED %s ... FiltersSet is : %s",varname.c_str(),(res?"changed":"UNchanged"));
        if (res) RetVal = true;
//      }
    } //else debug_printf("NEED %s ... but local",varname.c_str());
  }
  return RetVal;
}

void CCmExtFun::set_wait_fun(std::string fun)
{
  if (state == E_CM_EXT_FUNCTIONS_STATE_WAIT_CALL)
  {
    wait_done_function = fun;
  }
}

bool CCmExtFun::get_fundef_request(std::string &_now_fundef_name, std::string &_now_fundef_code)
{
  if (!now_fundef_name.empty())
  {
    _now_fundef_name = now_fundef_name;
    _now_fundef_code = now_fundef_code;
    now_fundef_name.clear();
    now_fundef_code.clear();
    return true;
  } else {
    now_fundef_name.clear();
    now_fundef_code.clear();
    return false;
  }
}

bool CCmExtFun::get_kill_request(std::string &_now_kill_name)
{
  if (!now_kill_name.empty())
  {
    _now_kill_name = now_kill_name;
    now_kill_name.clear();
    return true;
  } else {
    now_kill_name.clear();
    return false;
  }
}

bool CCmExtFun::get_call_request(std::string &_now_call_function, enum E_CM_EXT_FUNCTIONS_CALL_MODE2 &_now_call_function_mode2, StringList &_now_call_function_params)
{
  if (!now_call_function.empty())
  {
    _now_call_function = now_call_function;
    _now_call_function_mode2 = now_call_function_mode2;
    _now_call_function_params.clear();
    for (std::vector<std::string>::iterator it = now_call_function_params.begin(); it != now_call_function_params.end(); it++) _now_call_function_params.push_back(*it);
    now_call_function.clear();
    now_call_function_params.clear();
    return true;
  } else {
    _now_call_function.clear();
    _now_call_function_params.clear();
    return false;
  }
}

void CCmExtFun::info_fun_done(std::string fun)
{
  if ((state == E_CM_EXT_FUNCTIONS_STATE_WAIT_CALL) && (wait_done_function == fun))
  {
    state = E_CM_EXT_FUNCTIONS_STATE_RUNNING;
  }
}

#endif // USE_LIB_MUPARSERX
