
#if USE_LIB_MUPARSERX

#include "cm_debug.hh"
#include "cm_ext_muparser_hook.hh"
#include "cm_base_ext_fun.hh"
#include "cm_string.hh"
#include "cm_std_string.hh"
#include "cm_utils.hh"
#include "cm_var_utils.hh"

#define LIB_CM_TIME_TOLERANCE 0.005
#define DEFAULT_NeedAllExtVARS false

bool CmMuParserHook::CCmWorkerExt_ExtDef_Action::SetMe(std::deque<std::string> &cmd_items, std::string line)
{
  return false;
}
void CmMuParserHook::CCmWorkerExt_ExtDef_Action::ResetMe(double ActTime)
{
}
void CmMuParserHook::CCmWorkerExt_ExtDef_Action::GetUsedVariables(C_CM_MUP_Interface *MUP)
{
}
bool CmMuParserHook::CCmWorkerExt_ExtDef_Action::Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, double ActTime)
{
  return false;
}
bool CmMuParserHook::CCmWorkerExt_ExtDef_Action::Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, bool &ReturnVal, double ActTime)
{
  ReturnVal = false;
  return false;
}
bool CmMuParserHook::CCmWorkerExt_ExtDef_Action::Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, std::string ExtraAssign, double ActTime)
{
  return false;
}
double CmMuParserHook::CCmWorkerExt_ExtDef_Action::Polling_GetNextTime(double ActTime)
{
  return 0.0;
}
bool CmMuParserHook::CCmWorkerExt_ExtDef_Action::Polling_CheckNow(double ActTime)
{
  return false;
}
std::string CmMuParserHook::CCmWorkerExt_ExtDef_Action::PrintMe(void)
{
  return "Generic";
}

void CmMuParserHook::CCmWorkerExt_ExtDef_Action::PrintLastErrorMUP(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, std::string &EvalString)
{
//enum EErrorCodes
//{
//    // Expression syntax errors
//    ecUNEXPECTED_OPERATOR       =  0, ///< Unexpected binary operator found
//    ecUNASSIGNABLE_TOKEN        =  1, ///< Token cant be identified.
//    ecUNEXPECTED_EOF            =  2, ///< Unexpected end of expression. (Example: "2+sin(")
//    ecUNEXPECTED_COMMA          =  3, ///< An unexpected comma has been found. (Example: "1,23")
//    ecUNEXPECTED_VAL            =  4, ///< An unexpected value token has been found
//    ecUNEXPECTED_VAR            =  5, ///< An unexpected variable token has been found
//    ecUNEXPECTED_PARENS         =  6, ///< Unexpected Parenthesis, opening or closing
//    ecUNEXPECTED_STR            =  7, ///< A string has been found at an inapropriate position
//    ecUNEXPECTED_CONDITIONAL    =  8,
//    ecUNEXPECTED_NEWLINE        =  9,
//    ecSTRING_EXPECTED           = 10, ///< A string function has been called with a different type of argument
//    ecVAL_EXPECTED              = 11, ///< A numerical function has been called with a non value type of argument
//    ecMISSING_PARENS            = 12, ///< Missing parens. (Example: "3*sin(3")
//    ecMISSING_ELSE_CLAUSE       = 13,
//    ecMISPLACED_COLON           = 14,
//    ecUNEXPECTED_FUN            = 15, ///< Unexpected function found. (Example: "sin(8)cos(9)")
//    ecUNTERMINATED_STRING       = 16, ///< unterminated string constant. (Example: "3*valueof("hello)")
//    ecTOO_MANY_PARAMS           = 17, ///< Too many function parameters
//    ecTOO_FEW_PARAMS            = 18, ///< Too few function parameters. (Example: "ite(1<2,2)")
//    ecTYPE_CONFLICT             = 19, ///< Generic type conflict
//    ecTYPE_CONFLICT_FUN         = 20, ///< Function argument type conflict.
//    ecTYPE_CONFLICT_IDX         = 21, ///< Function argument type conflict.
//    ecINVALID_TYPE              = 22,
//    ecINVALID_TYPECAST          = 23, ///< Invalid Value token cast.
//    ecARRAY_SIZE_MISMATCH       = 24, ///< Array size mismatch during a vector operation
//    ecNOT_AN_ARRAY              = 25, ///< Using the index operator on a scalar variable
//    ecUNEXPECTED_SQR_BRACKET    = 26, ///< Invalid use of the index operator
//	ecUNEXPECTED_CURLY_BRACKET  = 27, ///< Invalid use of the index operator
//
//    ecINVALID_NAME              = 28, ///< Invalid function, variable or constant name.
//    ecBUILTIN_OVERLOAD          = 29, ///< Trying to overload builtin operator
//    ecINVALID_FUN_PTR           = 30, ///< Invalid callback function pointer
//    ecINVALID_VAR_PTR           = 31, ///< Invalid variable pointer
//    ecINVALID_PARAMETER         = 32, ///< Invalid function parameter
//    ecINVALID_NUMBER_OF_PARAMETERS = 33,
//
//    ecNAME_CONFLICT             = 34, ///< Name conflict
//    ecOPT_PRI                   = 35, ///< Invalid operator priority
//    ecASSIGNEMENT_TO_VALUE      = 36, ///< Assignment to operator (3=4 instead of a=4)
//
//    //
//    ecDOMAIN_ERROR              = 37, ///< Trying to use func/oprtr with out-of-domain input args
//    ecDIV_BY_ZERO               = 38, ///< Division by zero (currently unused)
//    ecGENERIC                   = 39, ///< Generic error
//
//    ecINDEX_OUT_OF_BOUNDS       = 40, ///< Array index is out of bounds
//    ecINDEX_DIMENSION           = 41,
//    ecMISSING_SQR_BRACKET       = 42, ///< The index operator was not closed properly (i.e. "v[3")
//	ecMISSING_CURLY_BRACKET     = 43,
//    ecEVAL                      = 44, ///< Error while evaluating function / operator
//    ecOVERFLOW                  = 45, ///< Overflow (possibly) occurred
//    
//    // Matrix errors
//    ecMATRIX_DIMENSION_MISMATCH = 46,
//
//    // string related errors
//    ecUNKNOWN_ESCAPE_SEQUENCE   = 47,
//
//    // already-defined item errors
//    ecVARIABLE_DEFINED          = 48, ///< Variable is already defined
//    ecCONSTANT_DEFINED          = 49, ///< Constant is already defined
//    ecFUNOPRT_DEFINED           = 50, ///< Function/operator is already defined
//
//    // internal errors
//    ecINTERNAL_ERROR            = 51, ///< Internal error of any kind.
//
//    // The last two are special entries
//    ecCOUNT,                          ///< This is no error code, It just stores just the total number of error codes
//    ecUNDEFINED                 = -1  ///< Undefined message, placeholder to detect unassigned error messages
//};


//  mozna dodelat (int)MUP->GetLastErrorCode()

  // if (NeedPrintAllMUPErrors)
  cms_ns_if_print("libcmext",1,"For %s At Expr \"%s\": %s",DebugPrefix.c_str(), EvalString.c_str(),MUP->GetLastError().c_str());
}


// ***********************************************************************************************************************************************
// ***********************************************************************************************************************************************
// ***********************************************************************************************************************************************

CmMuParserHook::CCmWorkerExt_ExtDef::CCmWorkerExt_ExtDef()
{
  CompleteActually = false;
  ExistActually    = false;
  double ActTime   = libCM_GetActualTimeDouble();
  FiltersNeedMe    = 0;
  SetEvalDepth(0, ActTime);
}


void CmMuParserHook::CCmWorkerExt_ExtDef::SetAllTmp(C_CM_MUP_Interface *MUP)
{
  if (MUP != NULL) for (std::map<std::string, class CCmVariable *>::iterator it = TmpVariables.begin(); it != TmpVariables.end(); it++)
  {
    if (it->second != NULL) MUP->SetVariable(it->second);
  }
}

void CmMuParserHook::CCmWorkerExt_ExtDef::GetAllTmp(C_CM_MUP_Interface *MUP)
{
  if (MUP != NULL) for (std::map<std::string, class CCmVariable *>::iterator it = TmpVariables.begin(); it != TmpVariables.end(); it++)
  {
    if (it->second != NULL) MUP->GetVariable(it->second);
  }
}
void CmMuParserHook::CCmWorkerExt_ExtDef::RemAllTmp(C_CM_MUP_Interface *MUP)
{
  if (MUP != NULL) for (std::map<std::string, class CCmVariable *>::iterator it = TmpVariables.begin(); it != TmpVariables.end(); it++)
  {
    MUP->RemoveVariable(it->first);
  }
}


void CmMuParserHook::CCmWorkerExt_ExtDef::doAllActionsExist(C_CM_MUP_Interface *MUP, std::list<class CCmWorkerExt_ExtDef_Action *> &Evaluators, bool &ReturnVal, double ActTime)
{
  ReturnVal = false;
  SetAllTmp(MUP);
  for (std::list<class CCmWorkerExt_ExtDef_Action *>::iterator itExt = Evaluators.begin(); itExt != Evaluators.end(); itExt++)
  {
    std::list<class CCmWorkerExt_ExtDef_Action *>::iterator itExtNext = itExt;
    itExtNext++;
    if (itExtNext != Evaluators.end())
    {
    //MyLibCM_name, VarName, "Exist"
      (*itExt)->Evaluate(DebugStringExist, MUP, ActTime);
    } else {
      (*itExt)->Evaluate(DebugStringExist, MUP, ReturnVal, ActTime);
    }
  }
  if (CM_BASE_EXT_KEEP_TMP_VARS) GetAllTmp(MUP); // keep????
  RemAllTmp(MUP);
}


void CmMuParserHook::CCmWorkerExt_ExtDef::doAllActionsEval(C_CM_MUP_Interface *MUP, std::list<class CCmWorkerExt_ExtDef_Action *> &Evaluators, bool &ReturnVal, double ActTime, bool WasNotExist)
{
  ReturnVal = false;
  SetAllTmp(MUP);
  bool isAnyActionWithSettingMe = false;
  unsigned ActualNum=0;
  bool EvalRet = true;
  for (std::list<class CCmWorkerExt_ExtDef_Action *>::iterator itExt = Evaluators.begin(); (itExt != Evaluators.end()) && (EvalRet); itExt++)
  {
    if ((*itExt)->MyUsedVarsOut.test(VarName)) isAnyActionWithSettingMe = true;
    std::list<class CCmWorkerExt_ExtDef_Action *>::iterator itExtNext = itExt;  itExtNext++;
    if ((ActualNum > EvalDepth)||(WasNotExist)) (*itExt)->ResetMe(ActTime);
    if ((itExtNext != Evaluators.end()) || (isAnyActionWithSettingMe))
    {
      EvalRet = (*itExt)->Evaluate(DebugStringEval, MUP, ActTime);
    } else {
      EvalRet = (*itExt)->Evaluate(DebugStringEval, MUP, VarName, ActTime);
    }
    ActualNum++;
  }
  if (!EvalRet) ActualNum--;
  if (EvalDepth != ActualNum) SetEvalDepth(ActualNum, ActTime);
  if (EvalDepth == Evaluators.size())
  {
    class CCmVariable NewVar = Var;
    MUP->GetVariable(&NewVar);
    if (NewVar != Var) ReturnVal = true;
    if (ReturnVal)
    {
      cms_ns_if_print("libcmext",12,"EVAL:%s:changed:%s", VarName.c_str(),NewVar.fullPrint().c_str());
    } else {
      cms_ns_if_print("libcmext",15,"EVAL:%s:unchanged", VarName.c_str());
    }
  } else {
    cms_ns_if_print("libcmext",13,"EVAL:%s:non-complete-eval:step%d/%zusteps", VarName.c_str(), EvalDepth, Evaluators.size());
  }
  if (CM_BASE_EXT_KEEP_TMP_VARS) GetAllTmp(MUP); // keep????
  RemAllTmp(MUP);
}

void CmMuParserHook::CCmWorkerExt_ExtDef::doAllActionsReverse(C_CM_MUP_Interface *MUP, std::list<class CCmWorkerExt_ExtDef_Action *> &Evaluators, VarEventMap &UpdateMap, double ActTime)
{
  SetAllTmp(MUP);
  for (std::list<class CCmWorkerExt_ExtDef_Action *>::iterator itExt = Evaluators.begin(); itExt != Evaluators.end(); itExt++)
  {
////   debug_printf("%s",libCM_ListAllKnownVariablesCM (AllVariablesCM,"BASE").c_str());
//    cms_ns_if_print("libcmext",27,"BEFORE:");
//    cms_ns_if_print("libcmext",27,"%s",libCM_ListAllKnownVariablesMUP(MUP, "BASE").c_str());
//    cms_ns_if_print("libcmext",27,"%s",libCM_ListAllKnownVariablesMUP(MUP, "RDR_PSR_?_ON").c_str());
//    cms_ns_if_print("libcmext",17,"  ... REVERSE: run Evaluate(\"%s\")", (*itExt)->PrintMe().c_str());
    (*itExt)->Evaluate(DebugStringReverse, MUP, ActTime);
//    cms_ns_if_print("libcmext",27,"AFTER:");
//    cms_ns_if_print("libcmext",27,"%s",libCM_ListAllKnownVariablesMUP(MUP, "BASE").c_str());
//    cms_ns_if_print("libcmext",27,"%s",libCM_ListAllKnownVariablesMUP(MUP, "RDR_PSR_?_ON").c_str());

    for(VarMap::iterator itLast = LastUsedVariables.begin(); itLast != LastUsedVariables.end(); itLast++)
    {
      std::string vname = itLast->first;
      if ((*itExt)->MyUsedVarsOut.test(itLast->second, false))
      {
        class CCmVariable modifvar = itLast->second;
        MUP->GetVariable(&modifvar);
        if (itLast->second != modifvar)
        {
	    cms_ns_if_print("libcmext",11,"REVERSE:%s:is setting:%s:%s", VarName.c_str(),vname.c_str(),modifvar.fullPrint().c_str());
	    insert_or_replace(UpdateMap, vname, VariableEvent(CmEventWhat::update, modifvar));
	    itLast->second = modifvar;
        }
      }
    }
  }
  if (CM_BASE_EXT_KEEP_TMP_VARS) GetAllTmp(MUP); // keep????
  RemAllTmp(MUP);
}

bool CmMuParserHook::CCmWorkerExt_ExtDef::doCheckPollings(C_CM_MUP_Interface *MUP, std::list<class CCmWorkerExt_ExtDef_Action *> &Evaluators, double ActTime)
{
  bool RetVal = false; // dramatic change requiring FullEval?
  unsigned ActualNum=0;
  for (std::list<class CCmWorkerExt_ExtDef_Action *>::iterator itExt = Evaluators.begin(); (itExt != Evaluators.end()) && (ActualNum <= EvalDepth); itExt++)
  {
    double P = ((*itExt)->Polling_GetNextTime(ActTime));
    if ((P != 0.0) && (P<=ActTime))
    {
      if ((*itExt)->Polling_CheckNow(ActTime)) RetVal = true;
    }
    ActualNum++;
  }
  return RetVal;
}

double CmMuParserHook::CCmWorkerExt_ExtDef::getFirstPollingTime(double ActTime)
{
  double RetVal = 0.0;
  unsigned ActualNum=0;
  for (std::list<class CCmWorkerExt_ExtDef_Action *>::iterator itExt = EvalExtEvaluators.begin(); (itExt != EvalExtEvaluators.end()) && (ActualNum <= EvalDepth); itExt++)
  {
    double P = ((*itExt)->Polling_GetNextTime(ActTime));
    if (P != 0.0)
    {
      if (RetVal == 0.0)
      {
        RetVal = P;
      } else {
        if (RetVal>P) RetVal = P;
      }
    }
    ActualNum++;
  }
  return RetVal;
}

void CmMuParserHook::CCmWorkerExt_ExtDef::SetEvalDepth(unsigned NewEvalDepth, double ActTime)
{
  EvalDepth = NewEvalDepth;
  havePollings = (getFirstPollingTime(ActTime) != 0.0);
}

double CmMuParserHook::CCmWorkerExt_ExtDef::doPendingActionsOrPoll(C_CM_MUP_Interface *MUP, VarMap &MyAllVariables, VarEventMap &UpdateMap, double ActTime)
{
  if ((EvaluatePending == 0) && (!havePollings))
  {
    return 0.0;
  }
  uint16_t Pending = EvaluatePending;
  int ExportMyVariable = -1; // -1=nothing, 0=delete, 1=create, 2=change
  EvaluatePending = 0;

  if (Pending & (1<<4)) // (1<<4) ReReadKnownVars to LastUsedVariables
  {
    for(VarMap::iterator itGlobKnown = MyAllVariables.begin(); itGlobKnown != MyAllVariables.end(); itGlobKnown++)
    {
      if(AllUsedVariables.test(itGlobKnown->second, false))
      {
        LastUsedVariables[itGlobKnown->first] = itGlobKnown->second;
      }
    }
  }
  bool NewExistActually = ExistActually;
  bool NewCompleteActually = CompleteActually;
  bool OutputChanged = false;
  bool IgnoreReverse = false;

  if (Pending & (1<<0)) // exist
  {
//   debug_printf("PendingExist:%s .. now is %s",VarName.c_str(),(ExistActually?"yes":"no"));
    bool NewExist = false;
    doAllActionsExist(MUP, ExistExtEvaluators, NewExist, ActTime);
    NewExistActually = NewExist;
    if (ExistActually != NewExistActually)
    {
      if (NewExistActually)
      {
        if (NewExistActually) Pending |= (1<<1); // eval must run
        if (MUP != NULL) MUP->SetVariable(&Var);
        SetEvalDepth(0, ActTime);
        cms_ns_if_print("libcmext",10,"EXIST START:%s", VarName.c_str());
      } else {
        if (NewExistActually) Pending &= ~((1<<1)|(1<<2)); // disable eval and reverse
        SetEvalDepth(0, ActTime);
        cms_ns_if_print("libcmext",10,"EXIST STOP:%s", VarName.c_str());
      }
//     debug_printf("PendingExistResult:%s = %s",VarName.c_str(),(NewExist?"yes":"no"));
    }
  }

  if ((Pending & (1<<2))&&(Pending & (1<<1))&&(NewExistActually)&&(CompleteActually)) // reverse
  {
    cms_ns_if_print("libcmext",16,"REVERSE:%s:1:tryDoIt:%s", VarName.c_str(),Var.fullPrint().c_str());
//   debug_printf("PendingReverse:%s",VarName.c_str());
    doAllActionsReverse(MUP, ReverseExtEvaluators, UpdateMap, ActTime);
    IgnoreReverse = true;
  }

// debug_printf(" ...exp_flag2:%s %d",VarName.c_str(),ExportMyVariable);
  if ((Pending & (1<<1))&&(NewExistActually)) // eval
  {
//   debug_printf("PendingEval:%s",VarName.c_str());
    doAllActionsEval(MUP, EvalExtEvaluators, OutputChanged, ActTime, (NewExistActually != ExistActually));
    NewCompleteActually = (EvalDepth == EvalExtEvaluators.size());
  }

  if ((havePollings)&&(!(Pending & (1<<1)))&&(NewExistActually)) // polling
  {
//   debug_printf("polling %s",VarName.c_str());
    bool needFullEval = doCheckPollings(MUP, EvalExtEvaluators, ActTime);
    if (needFullEval)
    {
      cms_ns_if_print("libcmext",11,"POOLING requested full EVAL:%s", VarName.c_str());
      doAllActionsEval(MUP, EvalExtEvaluators, OutputChanged, ActTime);
    }
    NewCompleteActually = (EvalDepth == EvalExtEvaluators.size());
  }

  if ((ExistActually != NewExistActually) || (CompleteActually != NewCompleteActually) || (OutputChanged))
  {
    if (((NewExistActually) && (NewCompleteActually)) && (!((ExistActually)&&(CompleteActually))))
    {
      ExportMyVariable = 1;
    } else if (((ExistActually)&&(CompleteActually)) && (!((NewExistActually) && (NewCompleteActually))))
    {
      ExportMyVariable = 0;
    } else if (((NewExistActually) && (NewCompleteActually)) && (OutputChanged))
    {
      ExportMyVariable = 2;
    }
  }

  ExistActually = NewExistActually;
  CompleteActually = NewCompleteActually;

  if (ExportMyVariable != -1)
  {
    if (MUP != NULL) MUP->GetVariable(&Var);
    LastUsedVariables[VarName] = Var;
    CmEventWhat what = ExportMyVariable == 0 ? CmEventWhat::undefine :
	ExportMyVariable == 1 ? CmEventWhat::define :
	ExportMyVariable == 2 ? CmEventWhat::update : throw std::logic_error("Should not happen");
	
    insert_or_replace(UpdateMap, VarName, VariableEvent(what, Var));
    // UpdateMap[VarName].EventWhat = ExportMyVariable; // -1=nothing, 0=delete, 1=create, 2=change
    if (ExportMyVariable==0) cms_ns_if_print("libcmext",16,"Export:%s DELETE", VarName.c_str());
    if (ExportMyVariable==1) cms_ns_if_print("libcmext",16,"Export:%s CREATE %s", VarName.c_str(), Var.fullPrint().c_str());
    if (ExportMyVariable==2) cms_ns_if_print("libcmext",16,"Export:%s UPDATE %s", VarName.c_str(), Var.fullPrint().c_str());
  }
  if ((!IgnoreReverse)&&(Pending & (1<<2))&&(NewExistActually)&&(NewCompleteActually)) // reverse
  {
    cms_ns_if_print("libcmext",16,"REVERSE:%s:2:tryDoIt:%s", VarName.c_str(),Var.fullPrint().c_str());
//   debug_printf("PendingReverse:%s",VarName.c_str());
    doAllActionsReverse(MUP, ReverseExtEvaluators, UpdateMap, ActTime);
  }
  double RetVal = 0.0;
  if ((havePollings)&&(NewExistActually)) RetVal = getFirstPollingTime(ActTime);
  if (RetVal != 0.0) cms_ns_if_print("libcmext",21,"POOLING:%s:RequestedNextTime %f", VarName.c_str(), RetVal-ActTime);
  return RetVal;
}


bool CmMuParserHook::CCmWorkerExt_ExtDef::CheckChangeOneVariable(CCmVariable const *var, std::string name, int changed)
{
  bool RetVal = false;
  if (AllUsedVariables.test(*var, false))
  {
    uint16_t Pending = 0; // (1<<0)  = exist,  (1<<1) = eval,  (1<<2) = reverse
    if (ExistActually)
    {
      if ((!ExistCondHaveOnlyExistCheck) || (changed==0))
      {
        if (ExistUsedVariables.test(*var, false)) Pending |= (1<<0); // exist
      }
      if (name == VarName)
      {
        if (CompleteActually)
        {
          if ((changed==2)&&(!ReverseExtEvaluators.empty()))
          {
            if (Var != (*var))
            {
              Pending |= (1<<2); // reverse
            }
          }
        }
      } else {
//        std::map<std::string, class CCmFilter>::iterator it_F = EvalUsedVariables.find(name); // tady by bolo trosku lepsi jet po jednotlivyvh Eval .. maximalne po depth?
//        if (it_F != EvalUsedVariables.end())
        if (EvalUsedVariables.test(*var, false))
//        if (EvalUsedVariables.test(var)) // mozna odvazna myslenka s true
        {
          VarMap::iterator itLast = LastUsedVariables.find(name);
          if (itLast != LastUsedVariables.end()) // diff indexes and check with filter
          {
            if (var->isArray())
            {
              itLast->second.EventFields.clear();
              itLast->second.diffFields(*var);
              if (EvalUsedVariables.test(itLast->second)) Pending |= (1<<1); // eval
              itLast->second.EventFields.clear();
            } else Pending |= (1<<1); // eval
          } else Pending |= (1<<1); // eval
        }
      }
    } else {
      if ((!ExistCondHaveOnlyExistCheck) || (changed==1))
      {
      // if is not myName (reverse) ... it's impossible ... nobody can change me in unexisting state
        if (ExistUsedVariables.test(name))
        {
          Pending |= (1<<0); // exist
          Pending |= (1<<1); // eval
        }
//              if (EvalUsedVariables.find(name) != EvalUsedVariables.end()) Pending |= (1<<1); // eval
      }
    }
    if (Pending)
    {
      uint16_t oldV = EvaluatePending;
      uint16_t newV = EvaluatePending | Pending;
      if (oldV != newV)
      {
        cms_ns_if_print("libcmext",22,"PENDING:%s/oldPending=%s%s%s%s/newPending=%s%s%s%s", VarName.c_str(),
			((oldV & (1<<0))?"Exist,":""),((oldV & (1<<1))?"Eval,":""),((oldV & (1<<2))?"Reverse,":""),((oldV & (1<<4))?"ReReadKnown,":""),
			((newV & (1<<0))?"Exist,":""),((newV & (1<<1))?"Eval,":""),((newV & (1<<2))?"Reverse,":""),((newV & (1<<4))?"ReReadKnown,":""));
        EvaluatePending |= Pending;
      }
      RetVal = true;
    }
    LastUsedVariables[name] = *var;
//   debug_printf("ukladam %s",name.c_str());
  }
  return RetVal;
}

void CmMuParserHook::CCmWorkerExt_ExtDef::SetDebugStrings(std::string &MyLibCM_name)
{
  DebugStringExist    = MyLibCM_name + "|" + VarName + "/" + "Exist";
  DebugStringEval     = MyLibCM_name + "|" + VarName + "/" + "Eval";
  DebugStringReverse  = MyLibCM_name + "|" + VarName + "/" + "Reverse";
}

// ***********************************************************************************************************************************************
// ***********************************************************************************************************************************************
// ***********************************************************************************************************************************************


void CmMuParserHook::internalCCmWorkerExtNew(void)
{
  DEBUG_FLAG_PRINT_EXT_CREATE          = LiveDebug->getFlagMask('1');
  DEBUG_FLAG_PRINT_EXT_DELETE          = LiveDebug->getFlagMask('0');
  DEBUG_FLAG_PRINT_EXT_UPDATE          = LiveDebug->getFlagMask('2');
  CM_MUP_Interface = new C_CM_MUP_Interface(); // musi byt pred ctenim configu a inicializaci lib_cm ... aby se pres callbacky nachytaly moje promenne a zapsaly do MUP
  CM_MUP_Interface->SetEnableAutoCreateVar(true);

}

CmMuParserHook::CmMuParserHook(uint16_t worker_flags, CCmLiveDebug *live_debug, uint64_t live_debug_flags, std::string my_libcm_name)
    : LiveDebug(live_debug),
      LiveDebugFlags(live_debug_flags),
      CmWorkerFlags(worker_flags),
      MyLibCM_name(my_libcm_name)
{
    EXT_Mutex = new CMSMutex();
    NeedAllExtVARS = DEFAULT_NeedAllExtVARS;
    // PollCondExt.init(PollCondition, 0.0, true, true);
    internalCCmWorkerExtNew();
}

CmMuParserHook::~CmMuParserHook()
{
    for (std::map <std::string, CCmWorkerExt_ExtDef>::iterator ItMainDef =  ExtDefMap.begin(); ItMainDef !=  ExtDefMap.end(); ItMainDef++)
    {
	std::list<class CCmWorkerExt_ExtDef_Action *> *MyEvaluators = NULL;
	MyEvaluators = &(ItMainDef->second.ExistExtEvaluators  );
	for (std::list<class CCmWorkerExt_ExtDef_Action *>::iterator itAc = MyEvaluators->begin(); itAc != MyEvaluators->end(); )
	{
	    std::list<class CCmWorkerExt_ExtDef_Action *>::iterator itAcD = itAc++;
	    delete (*itAcD);
	}
	MyEvaluators = &(ItMainDef->second.EvalExtEvaluators   );
	for (std::list<class CCmWorkerExt_ExtDef_Action *>::iterator itAc = MyEvaluators->begin(); itAc != MyEvaluators->end(); )
	{
	    std::list<class CCmWorkerExt_ExtDef_Action *>::iterator itAcD = itAc++;
	    delete (*itAcD);
	}
	MyEvaluators = &(ItMainDef->second.ReverseExtEvaluators);
	for (std::list<class CCmWorkerExt_ExtDef_Action *>::iterator itAc = MyEvaluators->begin(); itAc != MyEvaluators->end(); )
	{
	    std::list<class CCmWorkerExt_ExtDef_Action *>::iterator itAcD = itAc++;
	    delete (*itAcD);
	}
    }

    C_CM_MUP_Interface *D_MUP = CM_MUP_Interface;
    if (D_MUP != NULL) delete D_MUP;
    CM_MUP_Interface = NULL;

    delete EXT_Mutex;
}

// CmMuParserHook::CCmWorkerExt(std::string _IpDesc, unsigned _COM_MODE, unsigned _UDP_ID, double _PingPeriod, double _PingTimeOut,
//                           unsigned _LengthMax, unsigned _LengthIdeal, unsigned _BaudSpeedMax, double _BaudSpeedCalcTime, unsigned _RetryCount, double _RetryTime, std::string _MyPrefix, std::string _RemotePrefix,
//                           void (*_FilterCB_function)(class CCmFiltersSet *PartnerFiltersActive, class CCmFiltersSet *PartnerFiltersCreated, class CCmFiltersSet *PartnerFiltersDeleted, void *_UserPtr) ,
//                           void (*_VariableCB_function)(VarMap *VariablesCreated, VarMap *VariablesUpdated, VarMap *VariablesDeleted, void *_UserPtr),
//                           uint16_t _CmWorkerFlags, void *_UserPtr , std::string _name)
//             :CCmWorker(_IpDesc, _COM_MODE, _UDP_ID, _PingPeriod, _PingTimeOut ,_LengthMax, _LengthIdeal, _BaudSpeedMax, _BaudSpeedCalcTime, _RetryCount, _RetryTime, _MyPrefix, _RemotePrefix, _FilterCB_function,  _VariableCB_function, _CmWorkerFlags, _UserPtr, _name)
// {
//   EXT_Mutex = new CMSMutex();
//   NeedAllExtVARS = DEFAULT_NeedAllExtVARS;
//   PollCondExt.init(PollCondition, 0.0, true, true);
//   internalCCmWorkerExtNew();
// }

//      CmMuParserHook::CCmWorkerExt(CMSConfig *config, 
//                           void (*_FilterCB_function)(class CCmFiltersSet *PartnerFiltersActive, class CCmFiltersSet *PartnerFiltersCreated, class CCmFiltersSet *PartnerFiltersDeleted, void *_UserPtr),
//                           void (*_VariableCB_function)(VarMap *VariablesCreated, VarMap *VariablesUpdated, VarMap *VariablesDeleted, void *_UserPtr),
//                           uint16_t _CmWorkerFlags, void *_UserPtr , std::string _name)
//             // :CCmWorker(config, _FilterCB_function,  _VariableCB_function, _CmWorkerFlags, _UserPtr, _name)
// {
//   EXT_Mutex = new CMSMutex();
//   NeedAllExtVARS = DEFAULT_NeedAllExtVARS;
//   PollCondExt.init(PollCondition, 0.0, true, true);
//   internalCCmWorkerExtNew();
// //  if (!ExtSECTION_MUP.empty()) 
//   ReadConfig_Ext(config, ExtSECTION_VAR, ExtSECTION_DEF, ExtSECTION_MUP, ExtSECTION_FUN);
// }

//      CmMuParserHook::CCmWorkerExt(std::string config_path, std::string section,
//                          void (*_FilterCB_function)(class CCmFiltersSet *PartnerFiltersActive, class CCmFiltersSet *PartnerFiltersCreated, class CCmFiltersSet *PartnerFiltersDeleted, void *_UserPtr), 
//                          void (*_VariableCB_function)(VarMap *VariablesCreated, VarMap *VariablesUpdated, VarMap *VariablesDeleted, void *_UserPtr),
//                          uint16_t _CmWorkerFlags, void *_UserPtr , std::string _name)
//             // :CCmWorker(config_path, section, _FilterCB_function,  _VariableCB_function, _CmWorkerFlags, _UserPtr, _name)
// {
//   EXT_Mutex = new CMSMutex();


//   NeedAllExtVARS = DEFAULT_NeedAllExtVARS;
//   PollCondExt.init(PollCondition, 0.0, true, true);
//   internalCCmWorkerExtNew();
// //  if (!ExtSECTION_MUP.empty())
//   {
//     CMSConfig *config = new CMSConfig(config_path,"m4");
//     config->FindSection(section);
//     ReadConfig_Ext(config, ExtSECTION_VAR, ExtSECTION_DEF, ExtSECTION_MUP, ExtSECTION_FUN);
//     delete config;
//   }
// }

//      CmMuParserHook::~CCmWorkerExt()
// {
//   // StopUseSaveFile(false);
//   Stop();


// }


bool CmMuParserHook::internal_Ext_ChangeOneVariable(CCmVariable const *var, uint8_t what) // 0=undefine , 1=define , 2=update
{

  bool RetVal = false;
  bool RetValFun = false;
  bool FiltersMatching = false;

//  if (MyLibCM_name == "Router")debug_printf("SYNC1:%d,%s",what,var->fullPrint().c_str());
//  tady zjistim, zda chceme updatovat vsechno, nebo jenom veci pro Ext a pokud jenom pro Ext, pak otestujeme filtry a kdyztak padame pryc
  if (!(CmWorkerFlags & CM_WORKER_EXT_FLAG_MUPARSER_UPDATE_ALL))
  {
    if(!AllUsedVariables.test(*var, false)) return false;
    FiltersMatching = true;
  }

  std::string name = var->getName();
  if (CM_MUP_Interface != NULL)
  {
         if ((what==1)||(what==2)) CM_MUP_Interface->SetVariable(var);
    else if (what==0)              CM_MUP_Interface->RemoveVariable(name);
  }

  int changed = what;
  VarMap::iterator it = MyAllVariables.find(name);
  if (it != MyAllVariables.end())
  {
    if (what==0)
    {
      MyAllVariables.erase(it);
    } else if ((what==1)||(what==2))
    {
      if (it->second != *var)
      {
        it->second = *var;
        changed = 2;
      } else changed = -1;
    }
  } else {
    if ((what==1)||(what==2))
    {
      MyAllVariables[name] = *var;
      changed = 1;
    } else changed = -1;
  }

  if (changed != -1)
  {
    if ((FiltersMatching)||(AllUsedVariables.test(*var, false)))
    {
      for (std::map <std::string, CCmWorkerExt_ExtDef>::iterator ItMainDef =  ExtDefMap.begin(); ItMainDef !=  ExtDefMap.end(); ItMainDef++)
      {
        if (ItMainDef->second.CheckChangeOneVariable(var, name, changed)) RetVal = true;
      }
    }
  }

  if(!CmFunWaits.test(*var, false)) 
  {
    RetValFun = CmFunChangedVariable(var);
  }

  return RetVal || RetValFun;
}


bool CmMuParserHook::internalChangePartnerVariable(CCmVariable const *var)
{
  // JAN
  // CCmWorker::internalChangePartnerVariable(it_p, name, var, orig_var, newPriority);
  EXT_Mutex->Lock();
  bool changed = internal_Ext_ChangeOneVariable(var,2);
  EXT_Mutex->Unlock();
  return changed;
}

bool CmMuParserHook::internalChangeMyVariable(CCmVariable const *var, int EventWhat)
{
  // JAN
  // CCmWorker::internalChangeMyVariable(var, dont_send_it, orig_var, newPriority);
  EXT_Mutex->Lock();
  bool changed = internal_Ext_ChangeOneVariable(var, EventWhat);
  EXT_Mutex->Unlock();
  return changed;
}



bool CmMuParserHook::VariableGoCB(VarMap *VariablesCreated,
				  VarMap *VariablesUpdated,
				  VarMap *VariablesDeleted)
{
  bool changed = false;
  EXT_Mutex->Lock();
  if ((VariablesDeleted!=NULL)&&(VariablesDeleted->size()>0))
  {
    for (VarMap::iterator it = VariablesDeleted->begin() ; it != VariablesDeleted->end(); it++)
    {
      if (internal_Ext_ChangeOneVariable(&it->second, 0)) changed = true;
    }
  }
  if ((VariablesCreated!=NULL)&&(VariablesCreated->size()>0))
  {
    for (VarMap::iterator it = VariablesCreated->begin() ; it != VariablesCreated->end(); it++)
    {
      if (internal_Ext_ChangeOneVariable(&it->second, 1)) changed = true;
    }
  }
  if ((VariablesUpdated!=NULL)&&(VariablesUpdated->size()>0))
  {
    for (VarMap::iterator it = VariablesUpdated->begin() ; it != VariablesUpdated->end(); it++)
    {
      if (internal_Ext_ChangeOneVariable(&it->second, 2)) changed = true;
    }
  }
  EXT_Mutex->Unlock();
  return changed;
  // JAN
  // CCmWorker::VariableGoCB( VariablesCreated, VariablesUpdated, VariablesDeleted);
}

void CmMuParserHook::MakeForCycleEval(std::deque<std::string> ForCycleParams, std::list<class CCmWorkerExt_ExtDef_Action *> *MyEvaluators, std::string TemplateEval, std::string line)
{
  if (ForCycleParams.size()>1)
  {
    std::string InpVars = ForCycleParams.front();
    ForCycleParams.pop_front();
    std::string InpCycleDef = ForCycleParams.front();
    ForCycleParams.pop_front();

    std::deque<std::string> InpVarsArr;
    cm_std_string_SplitLineAndInterpretStringsInQuotes(InpVars, InpVarsArr, ",", "");

    std::deque<std::string> InpCycleDefArr;
    bool isItList = false;
    cm_std_string_SplitLineAndInterpretStringsInQuotes(InpCycleDef, InpCycleDefArr, "-", "");
    if (InpCycleDefArr.size()!=2)
    {
      InpCycleDefArr.clear();
      cm_std_string_SplitLineAndInterpretStringsInQuotes(InpCycleDef, InpCycleDefArr, ",", "");
      isItList = true;
    }
//   debug_printf("FOR \"%s\"(%d) ... \"%s\"(%d), Template=%s", InpVars.c_str(),InpVarsArr.size(), InpCycleDef.c_str(),InpCycleDefArr.size(),TemplateEval.c_str());
    if (isItList)
    {
      while (InpCycleDefArr.size() >= InpVarsArr.size())
      {
        std::string OutEval = TemplateEval;
        for (unsigned i = 0; i < InpVarsArr.size(); i++)
        {
          std::string Find = "$" + InpVarsArr[i] + "$";
          std::string Replace = InpCycleDefArr.front();
          InpCycleDefArr.pop_front();
          cm_std_string_replace(OutEval, Find, Replace);
//         debug_printf("  ++ --FOR \"%s\" \"%s\" %d .. %s", Find.c_str(), Replace.c_str(), ForCycleParams.size(), OutEval.c_str());
        }
        if (ForCycleParams.size()>1)
        {
          MakeForCycleEval(ForCycleParams, MyEvaluators, OutEval, line);
        } else {
//         debug_printf("ForEval:%s",OutEval.c_str());
            std::deque<std::string> ActionEvalParam; ActionEvalParam.clear(); ActionEvalParam.push_back(OutEval);
          class CCmWorkerExt_ExtDef_Action *new_ev = new CCmWorkerExt_ExtDef_Action_Eval();
          new_ev->SetMe(ActionEvalParam, line);
          MyEvaluators->push_back(new_ev);
        }
      }
    } else {
      int FirstVal = cm_std_string_get_value_int(InpCycleDefArr[0]);
      int LastVal = cm_std_string_get_value_int(InpCycleDefArr[1]);
      int i = FirstVal;
      while ((FirstVal<LastVal) ? (i<=LastVal) : (i>=LastVal))
      {
        char wstr[100];
        sprintf(wstr,"%d",i);
        std::string OutEval = TemplateEval;
        std::string Find = "$" + InpVarsArr[0] + "$";
        std::string Replace = std::string(wstr);
        cm_std_string_replace(OutEval, Find, Replace);
//       debug_printf("--FOR \"%s\" \"%s\" %d .. %s", Find.c_str(), Replace.c_str(), ForCycleParams.size(), OutEval.c_str());
        if (ForCycleParams.size()>1)
        {
          MakeForCycleEval(ForCycleParams, MyEvaluators, OutEval, line);
        } else {
//         debug_printf("ForEval:%s",OutEval.c_str());
            std::deque<std::string> ActionEvalParam; ActionEvalParam.clear(); ActionEvalParam.push_back(OutEval);
          class CCmWorkerExt_ExtDef_Action *new_ev = new CCmWorkerExt_ExtDef_Action_Eval();
          new_ev->SetMe(ActionEvalParam, line);
          MyEvaluators->push_back(new_ev);
        }
        if (FirstVal<LastVal) i++; else i--;
      }
    }
  }
}


std::string CmMuParserHook::MakeForCycleMacro(std::deque<std::string> ForCycleParams, std::string TemplateEval)
{
  std::string RetVal = "";
  if (ForCycleParams.size()>1)
  {
    std::string InpVars = ForCycleParams.front();
    ForCycleParams.pop_front();
    std::string InpCycleDef = ForCycleParams.front();
    ForCycleParams.pop_front();

    std::deque<std::string> InpVarsArr;
    cm_std_string_SplitLineAndInterpretStringsInQuotes(InpVars, InpVarsArr, ",", "");

    std::deque<std::string> InpCycleDefArr;
    bool isItList = false;
    cm_std_string_SplitLineAndInterpretStringsInQuotes(InpCycleDef, InpCycleDefArr, "-", "");
    if (InpCycleDefArr.size()!=2)
    {
      InpCycleDefArr.clear();
      cm_std_string_SplitLineAndInterpretStringsInQuotes(InpCycleDef, InpCycleDefArr, ",", "");
      isItList = true;
    }
//   debug_printf("FOR \"%s\"(%d) ... \"%s\"(%d), Template=%s", InpVars.c_str(),InpVarsArr.size(), InpCycleDef.c_str(),InpCycleDefArr.size(),TemplateEval.c_str());
    if (isItList)
    {
      while (InpCycleDefArr.size() >= InpVarsArr.size())
      {
        std::string OutEval = TemplateEval;
        for (unsigned i = 0; i < InpVarsArr.size(); i++)
        {
          std::string Find = "$" + InpVarsArr[i] + "$";
          std::string Replace = InpCycleDefArr.front();
          InpCycleDefArr.pop_front();
          cm_std_string_replace(OutEval, Find, Replace);
//         debug_printf("  ++ --FOR \"%s\" \"%s\" %d .. %s", Find.c_str(), Replace.c_str(), ForCycleParams.size(), OutEval.c_str());
        }
        if (ForCycleParams.size()>1)
        {
          RetVal += MakeForCycleMacro(ForCycleParams, OutEval);
        } else {
          RetVal += OutEval;
//         debug_printf("ForEval:%s",OutEval.c_str());
        }
      }
    } else {
      int FirstVal = cm_std_string_get_value_int(InpCycleDefArr[0]);
      int LastVal = cm_std_string_get_value_int(InpCycleDefArr[1]);
      int i = FirstVal;
      while ((FirstVal<LastVal) ? (i<=LastVal) : (i>=LastVal))
      {
        char wstr[100];
        sprintf(wstr,"%d",i);
        std::string OutEval = TemplateEval;
        std::string Find = "$" + InpVarsArr[0] + "$";
        std::string Replace = std::string(wstr);
        cm_std_string_replace(OutEval, Find, Replace);
//       debug_printf("--FOR \"%s\" \"%s\" %d .. %s", Find.c_str(), Replace.c_str(), ForCycleParams.size(), OutEval.c_str());
        if (ForCycleParams.size()>1)
        {
          RetVal += MakeForCycleMacro(ForCycleParams, OutEval);
        } else {
          RetVal += OutEval;
//         debug_printf("ForEval:%s",OutEval.c_str());
        }
        if (FirstVal<LastVal) i++; else i--;
      }
    }
  }
  return RetVal;
}




void CmMuParserHook::Config_Ext_Line(std::string name, std::string line)
{

      StringList name_items;
      cm_std_string_SplitLineAndInterpretStringsInQuotes(name, name_items, "/", "[](){}");
      if (name_items.size()>1)
      {

        std::string VarName = name_items[0];
        std::map <std::string, CCmWorkerExt_ExtDef>::iterator ItMainDef =  ExtDefMap.find(VarName);
        if (ItMainDef ==  ExtDefMap.end())
        {
          class CCmWorkerExt_ExtDef NewDef;
          NewDef.VarName = VarName;
          VarMap::iterator VarIt = ExtVariables.find(VarName);
          if (VarIt != ExtVariables.end())
          {
            NewDef.Var = VarIt->second;
            ExtVariables.erase(VarIt);
          } else {
            WORKER_DEBUG("libcmext",1,"libcm(%s):ExtDef variable name not found(%s) : from line : %s", MyLibCM_name.c_str(),VarName.c_str(), line.c_str());
            throw "AssignExtVar";
          }
          ExtDefMap[VarName] = NewDef;
          ItMainDef = ExtDefMap.find(VarName);
        }
        std::deque<std::string> items;
        cm_std_string_SplitLineAndInterpretStringsInQuotes(line, items, ";", "[](){}", true);
        for (unsigned num = 0; num < items.size(); num++)
        {
            std::deque<std::string> cmd_items;
          cm_std_string_SplitLineAndInterpretStringsInQuotes(items[num], cmd_items, ",", "[](){}",true,false,true);
//         debug_printf("ITEM:%s;;;;;%s",items[num].c_str(),line.c_str());
          for (unsigned z=0;z<cmd_items.size();z++)
          {
            cmd_items[z] = cm_std_string_remove_first_brackets_pair_or_interpret_string(cmd_items[z],   "[]{}", " \t\n\r\f\v");
          }
          if ((cmd_items.size()>0) && (cmd_items[0].size()>0))
          {
                   if ((!strcmp(name_items[1].c_str(),"tmp")))
            {
              for (unsigned i=0; i<cmd_items.size(); i++)
              {
                std::map<std::string, class CCmVariable *>::iterator it = ItMainDef->second.TmpVariables.find(cmd_items[i]);
                if (it == ItMainDef->second.TmpVariables.end())
                {
                  ItMainDef->second.TmpVariables[cmd_items[i]]=NULL;
                }
              }
            } else if ((!strcmp(name_items[1].c_str(),"exist")) || (!strcmp(name_items[1].c_str(),"eval")) || (!strcmp(name_items[1].c_str(),"reverse")))
            {
              std::string cmd = cmd_items[0];
              cmd_items.pop_front();
              std::list<class CCmWorkerExt_ExtDef_Action *> *MyEvaluators = &(ItMainDef->second.ExistExtEvaluators);
              if (!strcmp(name_items[1].c_str(),"exist"  )) MyEvaluators = &(ItMainDef->second.ExistExtEvaluators  );
              if (!strcmp(name_items[1].c_str(),"eval"   )) MyEvaluators = &(ItMainDef->second.EvalExtEvaluators   );
              if (!strcmp(name_items[1].c_str(),"reverse")) MyEvaluators = &(ItMainDef->second.ReverseExtEvaluators);

              class CCmWorkerExt_ExtDef_Action *new_ev = NULL;
              bool Unknown = false;
              bool Multiple = false;

                     if ((!strcmp(cmd.c_str(),"Eval")) && (cmd_items.size()>0))
              {
                new_ev = new CCmWorkerExt_ExtDef_Action_Eval();
              } else if ((!strcmp(cmd.c_str(),"Average")) && (cmd_items.size()>0))
              {
                new_ev = new CCmWorkerExt_ExtDef_Action_Average();
              } else if ((!strcmp(cmd.c_str(),"Int2Str")) && (cmd_items.size()>0))
              {
                new_ev = new CCmWorkerExt_ExtDef_Action_Int2Str();
              } else if ((!strcmp(cmd.c_str(),"WorstStatus")) && (cmd_items.size()>0))
              {
                new_ev = new CCmWorkerExt_ExtDef_Action_WorstStatus();
              } else if ((!strcmp(cmd.c_str(),"ForEval")) && (cmd_items.size()>0))
              {
                unsigned ForCycleDimensions = cm_std_string_get_value_int(cmd_items[0]);
                if ((1 + (ForCycleDimensions * 2) + 1) <= cmd_items.size())
                {
                    std::deque<std::string> ForCycleParams;
                  for (unsigned i = 0; i<(ForCycleDimensions * 2); i++) ForCycleParams.push_back(cmd_items[1+i]);
                  std::string TemplateEval = cmd_items[1+(ForCycleDimensions * 2)];
//                 debug_printf("ForCycleDimensions=%d, Template=\"%s\"",ForCycleDimensions,TemplateEval.c_str());
                  MakeForCycleEval(ForCycleParams, MyEvaluators, TemplateEval, line);
                }
                Multiple = true;
                new_ev = NULL;
              } else {
                Unknown = true;
                WORKER_DEBUG("libcmext",1,"libcm(%s):Unknown ExtDef_Action function \"%s\" for \"%s\"",MyLibCM_name.c_str(),cmd.c_str() ,name.c_str());
              }

              if (new_ev != NULL)
              {
                new_ev->SetMe(cmd_items, line);
                MyEvaluators->push_back(new_ev);
              }
              if ((new_ev == NULL) && (! (Unknown || Multiple)))
              {
                WORKER_DEBUG("libcmext",1,"libcm(%s):Parse error ExtDef_Action function \"%s\" for \"%s\"",MyLibCM_name.c_str(),cmd.c_str() ,name.c_str());
              }
            }
          } // if (cmd_items.size()>0)
        } // for (unsigned num = 0; num < name_items.size(); num++)
      } // if (name_items.size()>1)

}


std::string CmMuParserHook::Config_Ext_Line_ExpandMacros(std::string line)
{
  std::string RetVal = "";


  std::deque<std::string> items;
        cm_std_string_SplitLineAndInterpretStringsInQuotes(line, items, ";", "[](){}", true);
        for (unsigned num = 0; num < items.size(); num++)
        {
          bool Generated = false;


          std::deque<std::string> cmd_items;
          cm_std_string_SplitLineAndInterpretStringsInQuotes(items[num], cmd_items, ",", "[](){}",true,false,true);
//         debug_printf("ITEM:%s;;;;;%s",items[num].c_str(),line.c_str());
          for (unsigned z=0;z<cmd_items.size();z++)
          {
            cmd_items[z] = cm_std_string_remove_first_brackets_pair_or_interpret_string(cmd_items[z],   "[]{}", " \t\n\r\f\v");
          }
          if ((cmd_items.size()>0) && (cmd_items[0].size()>0))
          {
              std::string cmd = cmd_items[0];
              cmd_items.pop_front();


                     if ((!strcmp(cmd.c_str(),"Echo")) && (cmd_items.size()>0))
              {
                  for (unsigned z=0;z<cmd_items.size();z++)
                  {
//                    cmd_items[z] = cm_std_string_remove_first_brackets_pair_or_interpret_string(cmd_items[z],   "[]{}", " \t\n\r\f\v");
                    RetVal += cmd_items[z];
                  }
                  Generated = true;
              } else if ((!strcmp(cmd.c_str(),"Macro")) && (cmd_items.size()>0))
              {
                unsigned ForCycleDimensions = cm_std_string_get_value_int(cmd_items[0]);
                if ((1 + (ForCycleDimensions * 2) + 1) <= cmd_items.size())
                {
                    std::deque<std::string> ForCycleParams;
                  for (unsigned i = 0; i<(ForCycleDimensions * 2); i++) ForCycleParams.push_back(cmd_items[1+i]);
                  std::string TemplateEval = cmd_items[1+(ForCycleDimensions * 2)];
//                 debug_printf("ForCycleDimensions=%d, Template=\"%s\"",ForCycleDimensions,TemplateEval.c_str());
                  RetVal += MakeForCycleMacro(ForCycleParams, TemplateEval);
                  Generated = true;
                }
              }
          } // if (cmd_items.size()>0)
          if (!Generated)
          {
            RetVal += items[num];
            if ((num+1)<items.size()) RetVal += ";";
          }
        } // for (unsigned num = 0; num < name_items.size(); num++)

  return RetVal;
}

std::set<std::string> CmMuParserHook::ReadConfig_Ext(CMSConfig *config, std::string Section_Var, std::string Section_Ext, std::string Section_MUP_Fun, std::string Section_CM_Fun, VarMap &VarList)
{
  EXT_Mutex->Lock();
  std::set<std::string> all_variable_names;
  VarMap UnUsedVariables;
  std::set<std::string> FunUsedVars;
  std::string AllUsedStr;
  if (!Section_MUP_Fun.empty())
  {
    config->FindSection(Section_MUP_Fun);
    std::string name, line;
    std::map <std::string, std::string> AllFunctions;
    while (CMSConfigFindNextItem2(*config, name))
    {
      line = cm_std_string_InterpretStringIfIsFullyInQuotes(CMSConfigGetLine2(*config));
      std::map <std::string, std::string>::iterator it = AllFunctions.find(name);
      if (it == AllFunctions.end())
      {
        AllFunctions[name] = line;
      } else {
        it->second += line;
      }
    }
    for (std::map <std::string, std::string>::iterator it = AllFunctions.begin(); it != AllFunctions.end(); it++)
    {
      try
      {
        CMUP__DefineFunctionGeneric(it->first, it->second);
      }
      catch (...)
      {
        WORKER_DEBUG("libcmext",1,"libcm(%s):CMUP__DefineFunctionGeneric(): section=%s %s", MyLibCM_name.c_str(),Section_MUP_Fun.c_str(), CMUP_GlobalGetLastError().c_str());
        throw "FunGeneric::MUP";
      }
    }
  }

  ExtVariables.clear();
//  VarMap ExtVariables;

  if (!Section_Var.empty())
  {
    config->FindSection(Section_Var);
    libCM_LoadConfigVariables(config, &ExtVariables, &MyLibCM_name);
    for (VarMap::iterator it=ExtVariables.begin();it!=ExtVariables.end();it++)
	it->second.setFlagVirtual(true);

    // Tady jsem pripsal test, jestli Ext promenne nekoliduji s existujicima promennyma.
    std::set<std::string> bad_variables;
    for (VarMap::iterator it=ExtVariables.begin();it!=ExtVariables.end();it++)
    {
	if (contains(VarList, it->first))
	{
	    WORKER_DEBUG("libcmext", 1, "Multiple definitions of variable %s. Fix the config file!", it->first.c_str());
	    bad_variables.insert(it->first);
	}
    }
    for (std::set<std::string>::iterator it = bad_variables.begin(); it != bad_variables.end(); ++it)
	ExtVariables.erase(*it);
  }
  for (VarMap::iterator it=ExtVariables.begin();it!=ExtVariables.end();it++)
      all_variable_names.insert(it->first);
  
  if (!Section_Ext.empty())
  {
      // Read Ext section, concatenate lines
      config->FindSection(Section_Ext);
      std::string name;
      std::string line;
      std::map <std::string, std::string> ExtAll; // VarName -> EvalString
      while (CMSConfigFindNextItem2(*config, name))
      {
	  line = CMSConfigGetLine2(*config);
	  std::map <std::string, std::string>::iterator it = ExtAll.find(name);
	  if (it == ExtAll.end()) ExtAll[name]=line; else it->second += line;
      }

      // Delete Bad variables
      std::set<std::string> bad_variables;
      for (std::map<std::string, std::string>::iterator it = ExtAll.begin(); it != ExtAll.end(); ++it)
      {
	  std::string item_name = it->first;
	  size_t slashpos = item_name.find('/');
	  if (slashpos == std::string::npos || slashpos == 0 || slashpos == item_name.size())
	  {
	      WORKER_DEBUG("libcmext", 1, "Invalid Ext item %s. Fix the config file!", it->first.c_str());
	      bad_variables.insert(it->first);
	  }
	  else
	  {
	      std::string varname(item_name.begin(), item_name.begin() + slashpos);
	      if (ExtVariables.find(varname) == ExtVariables.end())
	      {
		  WORKER_DEBUG("libcmext", 1, "%s defined for non-existing variable %s. Fix the config file!", it->first.c_str(), varname.c_str());
		  bad_variables.insert(it->first);
	      }
	  }
      }
      for (std::set<std::string>::iterator it = bad_variables.begin(); it != bad_variables.end(); ++it)
	  ExtAll.erase(*it);
      //////////////////////////////
    
    
      for (std::map <std::string, std::string>::iterator it = ExtAll.begin(); it !=ExtAll.end(); it++)
      {
	  std::string NewLine = Config_Ext_Line_ExpandMacros(it->second);
	  Config_Ext_Line(it->first, NewLine);
	  if (it->second != NewLine)
	  {
	      WORKER_DEBUG("libcmext",4,"libcm(%s):ExpandMacros(%s): ===============\n%s\n=================\n%s\n===================\n", MyLibCM_name.c_str(),it->first.c_str(),it->second.c_str(), NewLine.c_str());
//       debug_printf("ExpandMacros: ===============\n%s\n=================\n%s\n===================",it->second.c_str(), NewLine.c_str());
		  }
//      Config_Ext_Line(it->first, Config_Ext_Line_ExpandMacros(it->second));
      }

      VarMap UsedVariablesTmp;
      for (std::map <std::string, CCmWorkerExt_ExtDef>::iterator ItMainDef =  ExtDefMap.begin(); ItMainDef !=  ExtDefMap.end(); ItMainDef++)
      {
	  for (std::map<std::string, class CCmVariable *>::iterator ItTmp = ItMainDef->second.TmpVariables.begin() ; ItTmp != ItMainDef->second.TmpVariables.end(); ItTmp++)
	  {
	      VarMap::iterator VarIt = ExtVariables.find(ItTmp->first);
	      if (VarIt != ExtVariables.end())
	      {
		  ItTmp->second = &VarIt->second;
		  UsedVariablesTmp[VarIt->first] = VarIt->second;
	      }
	  }
      }


      for (VarMap::iterator VarIt = ExtVariables.begin(); VarIt != ExtVariables.end(); VarIt++)
      {
	  if (UsedVariablesTmp.find(VarIt->first) == UsedVariablesTmp.end())
	  {
//        WORKER_DEBUG("libcmext",2,"libcm(%s):UNASSIGNED Ext Variable:%s", MyLibCM_name.c_str(),VarIt->second.fullPrint().c_str());
	      UnUsedVariables[VarIt->first] = VarIt->second;
	  }
      }

      for (std::map <std::string, CCmWorkerExt_ExtDef>::iterator ItMainDef =  ExtDefMap.begin(); ItMainDef !=  ExtDefMap.end(); ItMainDef++)
      {
	  std::string AllString;
	  std::list<class CCmWorkerExt_ExtDef_Action *> *MyEvaluators = NULL;
	  MyEvaluators = &(ItMainDef->second.EvalExtEvaluators   );
	  AllString.clear();
	  for (std::list<class CCmWorkerExt_ExtDef_Action *>::iterator itAc = MyEvaluators->begin(); itAc != MyEvaluators->end(); itAc++)
	  {
	      AllString += "\n -->>:" + (*itAc)->PrintMe();
	      (*itAc)->GetUsedVariables( CM_MUP_Interface);
	      ItMainDef->second.EvalUsedVariables.add(&((*itAc)->MyUsedVarsOut));
	      ItMainDef->second.EvalUsedVariables.add(&((*itAc)->MyUsedVarsRq));
	  }
	  if (!AllString.empty()) WORKER_DEBUG("libcmext",8,"libcm(%s):DEF:%s:EVAL:::%s",MyLibCM_name.c_str(),ItMainDef->first.c_str(),AllString.c_str());
				      ItMainDef->second.EvalUsedVariables.erase(ItMainDef->first);
	  ItMainDef->second.EvalUsedVariables.erase(ItMainDef->second.TmpVariables);

	  MyEvaluators = &(ItMainDef->second.ReverseExtEvaluators);
	  AllString.clear();
	  for (std::list<class CCmWorkerExt_ExtDef_Action *>::iterator itAc = MyEvaluators->begin(); itAc != MyEvaluators->end(); itAc++)
	  {
	      AllString += "\n -->>:" + (*itAc)->PrintMe();
	      (*itAc)->GetUsedVariables( CM_MUP_Interface);
	      ItMainDef->second.ReverseUsedVariables.add(&((*itAc)->MyUsedVarsOut));
	      ItMainDef->second.ReverseUsedVariables.add(&((*itAc)->MyUsedVarsRq));
	  }
	  if (!AllString.empty()) WORKER_DEBUG("libcmext",8,"libcm(%s):DEF:%s:REVERSE:::%s",MyLibCM_name.c_str(),ItMainDef->first.c_str(),AllString.c_str());
//      ItMainDef->second.ReverseUsedVariables.erase(ItMainDef->first);
				      ItMainDef->second.ReverseUsedVariables.erase(ItMainDef->second.TmpVariables);

	  ItMainDef->second.AllUsedVariables.add(&(ItMainDef->second.EvalUsedVariables));
	  ItMainDef->second.AllUsedVariables.add(&(ItMainDef->second.ReverseUsedVariables));

	  if (ItMainDef->second.ExistExtEvaluators.size()==0)
	  {
	      std::string AutoCond;
	      class CCmFiltersSet WFilters = ItMainDef->second.AllUsedVariables;
	      WFilters.erase(ItMainDef->first);
	      AutoCond = WFilters.getNames(",");
	      if (AutoCond.empty()) AutoCond = "true"; else AutoCond = "exist(" + AutoCond + ")";
	      std::deque<std::string> tmp_cmd_items; tmp_cmd_items.resize(1); tmp_cmd_items[0]=AutoCond;
//       debug_printf("CREATE AUTO EXIST (%s)", AutoCond.c_str());
	      class CCmWorkerExt_ExtDef_Action *new_ev = new CCmWorkerExt_ExtDef_Action_Eval();
	      new_ev->SetMe(tmp_cmd_items, AutoCond);
	      ItMainDef->second.ExistExtEvaluators.push_back(new_ev);
	      ItMainDef->second.ExistCondAutomatic = true; //  (origo/rewriten)
	      ItMainDef->second.ExistCondHaveOnlyExistCheck = true; //  right now only , if cond is automatic
	  } else {
	      ItMainDef->second.ExistCondAutomatic = false; //  (origo/rewriten)
	      ItMainDef->second.ExistCondHaveOnlyExistCheck = false; //  right now only , if cond is automatic
	  }
	  MyEvaluators = &(ItMainDef->second.ExistExtEvaluators  );
	  AllString.clear();
	  for (std::list<class CCmWorkerExt_ExtDef_Action *>::iterator itAc = MyEvaluators->begin(); itAc != MyEvaluators->end(); itAc++)
	  {
	      AllString += "\n -->>:" + (*itAc)->PrintMe();
	      (*itAc)->GetUsedVariables( CM_MUP_Interface);
	      ItMainDef->second.ExistUsedVariables.add(&((*itAc)->MyUsedVarsOut));
	      ItMainDef->second.ExistUsedVariables.add(&((*itAc)->MyUsedVarsRq));
	  }
	  if (!AllString.empty()) WORKER_DEBUG("libcmext",8,"libcm(%s):DEF:%s:EXIST:::%s",MyLibCM_name.c_str(),ItMainDef->first.c_str(),AllString.c_str());
				      ItMainDef->second.ExistUsedVariables.erase(ItMainDef->first);
	  ItMainDef->second.ExistUsedVariables.erase(ItMainDef->second.TmpVariables);
	  ItMainDef->second.AllUsedVariables.add(&(ItMainDef->second.ExistUsedVariables));
	  AllUsedStr = ItMainDef->second.AllUsedVariables.toString();
	  WORKER_DEBUG("libcmext",5,"libcm(%s):AllUsedVariables(%s):%s", MyLibCM_name.c_str(),ItMainDef->first.c_str(), AllUsedStr.c_str());
	      AllUsedVariables.add(&ItMainDef->second.AllUsedVariables);
	  ItMainDef->second.EvaluatePending = (1<<0) | (1<<1)| (1<<4);
      }
      for (std::map <std::string, CCmWorkerExt_ExtDef>::iterator ItMainDef =  ExtDefMap.begin(); ItMainDef !=  ExtDefMap.end(); ItMainDef++)
      {
	  ItMainDef->second.FiltersNeedMe = 0;
	  ItMainDef->second.SetDebugStrings(MyLibCM_name);
      }
  } // if (!Section_Ext.empty())

  AllUsedStr = AllUsedVariables.toString();
  WORKER_DEBUG("libcmext",4,"libcm(%s):AllUsedVariables(--ALL--):%s", MyLibCM_name.c_str(),AllUsedStr.c_str());

  // VarMap VarList;
  // CCmExtHook::GetAllVariables(VarList);
  for (VarMap::iterator it = VarList.begin() ; it != VarList.end(); it++)
  {
    internal_Ext_ChangeOneVariable(&(it->second), 1); // Created all new
  }

  if (!Section_CM_Fun.empty())
  {
    config->FindSection(Section_CM_Fun);
    std::string name, line;
    std::map <std::string, std::string> AllFunctions;
    std::map <std::string, std::list<std::string> > AllFunctionsRAW;
    std::map<std::string, StringList > AutoexecFuncts;

    while (CMSConfigFindNextItem2(*config, name))
    {
      line = cm_std_string_InterpretStringIfIsFullyInQuotes(CMSConfigGetLine2(*config));
      if (name == "AUTOEXEC")
      {
        WORKER_DEBUG("libcmfun",3,"libcm(%s):CmFun:Add autoexec : \"%s\"",MyLibCM_name.c_str(),line.c_str());
        StringList ParamArr;
        StringList ParamArr2;
        cm_std_string_SplitLineAndInterpretStringsInQuotes(line, ParamArr, ",", "");
        if (ParamArr.size()>0)
        {
          for (size_t i=1;i<ParamArr.size();i++) ParamArr2.push_back(ParamArr[i]);
          AutoexecFuncts[ParamArr[0]] = ParamArr2;
        }
      } else if (name == "LOAD")
      {
        StringList ParamArr;
        cm_std_string_SplitLineAndInterpretStringsInQuotes(line, ParamArr, ",", "");
        if (ParamArr.size()>1)
        {
          AllFunctions[ParamArr[0]] = CmFunLoadFromFile(ParamArr[1],ParamArr[0]);
        } else {
          WORKER_DEBUG("libcmfun",1,"libcm(%s):LoadCmFun FAILED, bad string \"\%s\"",MyLibCM_name.c_str(),line.c_str());
          throw "Fun Bad String";
        }

      } else {
        std::map <std::string, std::list<std::string> >::iterator it = AllFunctionsRAW.find(name);
        if (it == AllFunctionsRAW.end())
        {
          AllFunctionsRAW[name].push_back(line);
        } else {
          it->second.push_back(line);
        }
      }
    }
    for (std::map <std::string, std::list<std::string> >::iterator it = AllFunctionsRAW.begin(); it != AllFunctionsRAW.end(); it++)
    {
      std::map<std::string, struct CmFunMacroType> MacroList;
      std::string OutFun;
      for (std::list<std::string>::iterator itX = it->second.begin(); itX != it->second.end(); itX++)
      {
        CmFunLoadFromFile_OneLine("CONFIG", std::string(*itX), OutFun, MacroList);
      }
      AllFunctions[it->first] = OutFun;
    }
    for (std::map <std::string, std::string>::iterator it = AllFunctions.begin(); it != AllFunctions.end(); it++)
    {
      CmFunAdd(it->first, it->second, UnUsedVariables, FunUsedVars, false);
    }
    for (std::map<std::string, StringList >::iterator it_a = AutoexecFuncts.begin(); it_a != AutoexecFuncts.end(); it_a++)
    {
      if (CmFunStart(it_a->first, it_a->second, true)==2)
      {
        WORKER_DEBUG("libcmfun",1,"libcm(%s):Cannot autoexec unexisting function \"\%s\"",MyLibCM_name.c_str(),it_a->first.c_str());
        throw "Cannot Autoexec";
      }
    }
  }

  for (VarMap::iterator VarIt = UnUsedVariables.begin(); VarIt != UnUsedVariables.end(); VarIt++)
  {
    if (FunUsedVars.find(VarIt->first) == FunUsedVars.end())
    {
      WORKER_DEBUG("libcmext",2,"libcm(%s):UNASSIGNED Ext Variable:%s", MyLibCM_name.c_str(),VarIt->second.fullPrint().c_str());
    } else {
      WORKER_DEBUG("libcmext",3,"libcm(%s):Ext Variable handled by a CmFun:%s", MyLibCM_name.c_str(),VarIt->second.fullPrint().c_str());
    }
  }

  EXT_Mutex->Unlock();
  // Predunuto do set hook
  // PollCondExt.Wake();
  // RefreshFilters(true);
  // PollCondExt.Wake();
  return all_variable_names;
}

struct CmFunMacroType
{
  std::string Name;
  std::list<std::string> Args;
  std::list<std::string> Code;
};

void CmMuParserHook::CmFunLoadFromFile_ForGenKeys(std::string FName, std::string LineStd, std::string ParamStr,  StringList &Olist)
{
  std::list<std::string> Ag1;
  cm_std_string_g_strsplit(ParamStr, ",", Ag1);
  if (Ag1.size()<1)
  {
      throw "CmFunLoadFromFile_ForGenKeys::ERR1";
  }
  for (std::list<std::string>::iterator it1 = Ag1.begin(); it1 != Ag1.end(); it1++)
  {
    std::string Param1 = *it1;
    std::list<std::string> Ag2;
    cm_std_string_g_strsplit(Param1, "_", Ag2);
    if (Ag2.size()>0)
    {
      if (Ag2.size()==1)
      {
        Olist.push_back(Param1);
      } else if (Ag2.size()==2) {
        char WStr[100];
        std::string P1 = *(Ag2.begin());
        std::string P2 = *(++(Ag2.begin()));
        if (cm_std_string_get_value_int(P1) != cm_std_string_get_value_int(P2))
        {
          // numbers for
          int P1n = cm_std_string_get_value_int(P1);
          int P2n = cm_std_string_get_value_int(P2);
          if (P1n<P2n)
          {
            for (int i = P1n; i<=P2n; i++)
            {
              sprintf(WStr,"%d",i);
              Olist.push_back(std::string(WStr));
            }
          } else {
            for (int i = P2n; i>=P1n; i--)
            {
              sprintf(WStr,"%d",i);
              Olist.push_back(std::string(WStr));
            }
          }
        } else if ((P1.size()==1) && (P2.size()==1))
        {
          // char for
          char P1c = P1[0];
          char P2c = P2[0];

          if (P1c<P2c)
          {
            for (char i = P1c; i<=P2c; i++)
            {
              sprintf(WStr,"%c",i);
              Olist.push_back(std::string(WStr));
            }
          } else {
            for (char i = P2c; i>=P1c; i--)
            {
              sprintf(WStr,"%c",i);
              Olist.push_back(std::string(WStr));
            }
          }
        }
      } else {
        throw "CmFunLoadFromFile_ForGenKeys::ERR3";
      }
    } else {
      throw "CmFunLoadFromFile_ForGenKeys::ERR2";
    }
  }
}


void CmMuParserHook::CmFunLoadFromFile_OneLine(std::string FName, std::string LineStd, std::string &Output,  std::map<std::string, struct CmFunMacroType> &MacroList)
{
  StringList KEYS;
  KEYS.push_back("%DEF "); // 0
  KEYS.push_back("%DO " ); // 1
  KEYS.push_back("%FOR "); // 2

//  std::string KEY_DEF = "%DEF ";
//  std::string KEY_DO  = "%DO ";
//  std::string KEY_FOR = "%FOR ";
  if (LineStd.substr(0,1) != "#")
  {
    int first_key_id = -1;
    int first_key_pos = -1;
    for (int ii=0; ii<(int)KEYS.size();ii++)
    {
      if (LineStd.find(KEYS[ii], 0) != std::string::npos)
      {
        if ((first_key_pos == -1) || ( first_key_pos > (int)LineStd.find(KEYS[ii], 0)))
        {
          first_key_id = ii;
          first_key_pos = LineStd.find(KEYS[ii], 0);
        }
      }
    }


    if (first_key_id == 0) // DEF
    {
      unsigned ST1 = first_key_pos;
      unsigned KeyLength = KEYS[first_key_id].size();
      unsigned ST2 = ST1 + KeyLength;
      if (ST1>0)
      {
        std::string StrBefore = LineStd.substr(0,ST1);
        CmFunLoadFromFile_OneLine(FName, StrBefore, Output,  MacroList);
      }
      if (LineStd.find("(", ST2) != std::string::npos)
      {
        unsigned BR1 = LineStd.find("(", ST2);
        if (LineStd.find(")", BR1) != std::string::npos)
        {
          unsigned BR2 = LineStd.find(")", BR1);
          std::string DName = LineStd.substr(ST2,BR1-ST2);
          std::string DArgs = LineStd.substr(BR1+1,BR2-(BR1+1));
          std::string DCode = LineStd.substr(BR2+1);
          if (DCode.substr(0,1) == " ") DCode = DCode.substr(1);
          if (MacroList.find(DName) != MacroList.end())
          {
            MacroList.find(DName)->second.Code.push_back(DCode);
          } else {
            std::list<std::string> DArgsL;
            cm_std_string_g_strsplit(DArgs, ",", DArgsL);
            struct CmFunMacroType NMacro;
            NMacro.Name = DName;
            NMacro.Args = DArgsL;
            NMacro.Code.push_back(DCode);
            MacroList[DName] = NMacro;
          }
        }
      }
    }
    else if (first_key_id == 1) // DO
    {
      unsigned ST1 = first_key_pos;
      unsigned KeyLength = KEYS[first_key_id].size();
      unsigned ST2 = ST1 + KeyLength;
      if (ST1>0)
      {
        std::string StrBefore = LineStd.substr(0,ST1);
        CmFunLoadFromFile_OneLine(FName, StrBefore, Output,  MacroList);
      }
      if (LineStd.find("(", ST2) != std::string::npos)
      {
        unsigned BR1 = LineStd.find("(", ST2);
        if (LineStd.find(")", BR1) != std::string::npos)
        {
          unsigned BR2 = LineStd.find(")", BR1);
          std::string DName = LineStd.substr(ST2,BR1-ST2);
          std::string DArgs = LineStd.substr(BR1+1,BR2-(BR1+1));

          std::string DCode = LineStd.substr(BR2+1);

          std::list<std::string> DArgsL;
          cm_std_string_g_strsplit(DArgs, ",", DArgsL);
          std::map<std::string, struct CmFunMacroType>::iterator MacroIt = MacroList.find(DName);
          if (MacroIt != MacroList.end())
          {
            struct CmFunMacroType MyMacro = MacroIt->second;
            if (DArgsL.size() == MyMacro.Args.size())
            {
              for (std::list<std::string>::iterator ItC = MyMacro.Code.begin(); ItC != MyMacro.Code.end(); ItC++)
              {
                std::string WorkLine = *ItC;
                std::list<std::string>::iterator It1 = MyMacro.Args.begin();
                std::list<std::string>::iterator It2 = DArgsL.begin();
                while ((It1 != MyMacro.Args.end()) && (It2 != DArgsL.end()))
                {
                  std::string Find = "$" + (*It1) + "$";
                  std::string Replace = (*It2);
                  cm_std_string_replace(WorkLine, Find, Replace);
                  It1++; It2++;
                }
                CmFunLoadFromFile_OneLine(FName, WorkLine, Output,  MacroList);
              }
            } else {
              WORKER_DEBUG("libcmfun",1,"libcm(%s):LoadCmFun FAILED, load file \"\%s\" ... MACRO \"%s\" ... num params %d != %d",MyLibCM_name.c_str(),FName.c_str(),DName.c_str(), (int)DArgsL.size(), (int)MyMacro.Args.size());
              throw "LOAD Fun FILE - macro params";
            }
          } else {
            WORKER_DEBUG("libcmfun",1,"libcm(%s):LoadCmFun FAILED, load file \"\%s\" ... undefined MACRO \"%s\"",MyLibCM_name.c_str(),FName.c_str(),DName.c_str());
            throw "LOAD Fun FILE - undefined macro";
          }
          if (DCode.size()>0) CmFunLoadFromFile_OneLine(FName, DCode, Output,  MacroList);
        }
      }
    } else if (first_key_id == 2) // FOR
    {
      unsigned ST1 = first_key_pos;
      unsigned KeyLength = KEYS[first_key_id].size();
      unsigned ST2 = ST1 + KeyLength;
      if (ST1>0)
      {
        std::string StrBefore = LineStd.substr(0,ST1);
        CmFunLoadFromFile_OneLine(FName, StrBefore, Output,  MacroList);
      }
      if (LineStd.find("(", ST2) != std::string::npos)
      {
        unsigned BR1 = LineStd.find("(", ST2);
        if (LineStd.find(")", BR1) != std::string::npos)
        {
          unsigned BR2 = LineStd.find(")", BR1);
          std::string DName = LineStd.substr(ST2,BR1-ST2);
          std::string DArgs = LineStd.substr(BR1+1,BR2-(BR1+1));
          std::string DCode = LineStd.substr(BR2+1);
          if (DCode.substr(0,1) == " ") DCode = DCode.substr(1);

          std::list<std::string> DNamesL;
          cm_std_string_g_strsplit(DName, ":", DNamesL);

          std::list<std::string> DArgsL;
          cm_std_string_g_strsplit(DArgs, ":", DArgsL);
          if (DNamesL.size()==DArgsL.size())
          {
            std::list<StringList > DArgsCalc;
            int PrevOListSize = -1;
            for (std::list<std::string>::iterator itAL = DArgsL.begin(); itAL != DArgsL.end(); itAL++)
            {
              StringList OList;
              OList.clear();
              CmFunLoadFromFile_ForGenKeys(FName, LineStd, *itAL,  OList);
              DArgsCalc.push_back(OList);
              if (OList.size()<1)
              {
                // doplnit ktery radek a ktery index key
                WORKER_DEBUG("libcmfun",1,"libcm(%s):LoadCmFun FAILED, load file \"\%s\" ... MACRO \"%s\" ... FOR key output indexes empty",MyLibCM_name.c_str(),FName.c_str(),DName.c_str());
                throw "LOAD Fun FILE - macro FOR out indexes empty";
              }
              if (PrevOListSize != -1)
              {
                if (PrevOListSize != (int)OList.size())
                {
                  // doplnit ktery radek a ktery index key
                  WORKER_DEBUG("libcmfun",1,"libcm(%s):LoadCmFun FAILED, load file \"\%s\" ... MACRO \"%s\" ... FOR key output indexes different size %d != %d",MyLibCM_name.c_str(),FName.c_str(),DName.c_str(), PrevOListSize , (int)OList.size());
                  throw "LOAD Fun FILE - macro FOR out indexes differ";
                }
              }
              PrevOListSize = (int)OList.size();
            }
            for (int groupId = 0; groupId < PrevOListSize; groupId++)
            {
              std::string WorkLine = DCode;
              std::list<std::string>::iterator It1 = DNamesL.begin();
              std::list< StringList >::iterator It2 = DArgsCalc.begin();
              while ((It1 != DNamesL.end()) && (It2 != DArgsCalc.end()))
              {
                std::string Find = "$" + (*It1) + "$";
                std::string Replace = ((*It2)[groupId]);
                cm_std_string_replace(WorkLine, Find, Replace);
                It1++; It2++;
              }
              CmFunLoadFromFile_OneLine(FName, WorkLine, Output,  MacroList);
            }
          } else {
            WORKER_DEBUG("libcmfun",1,"libcm(%s):LoadCmFun FAILED, load file \"\%s\" ... MACRO \"%s\" ... FOR num keys %d != %d",MyLibCM_name.c_str(),FName.c_str(),DName.c_str(), (int)DNamesL.size(), (int)DArgsL.size());
            throw "LOAD Fun FILE - macro FOR num keys";
          }
        }
      }
    } else {
      Output += LineStd;
    }
  }
}

std::string CmMuParserHook::CmFunLoadFromFile(std::string FName, std::string FunName)
{
  std::string RetVal;
  std::map<std::string, struct CmFunMacroType> MacroList;

  FILE *F = fopen(FName.c_str(),"rt");
  if (F!=NULL)
  {
    char *Line  = new char[MSGMAX];
    while (fgets(Line,MSGMAX,F)!=NULL)
    {
      if ((strlen(Line)>0) && (Line[strlen(Line)-1]=='\n')) Line[strlen(Line)-1] = 0;
      if (strlen(Line)>0) CmFunLoadFromFile_OneLine(FName, std::string(Line), RetVal, MacroList);
    }
    fclose(F);
    WORKER_DEBUG("libcmfun",8,"libcm(%s):LoadCmFun SUCCESS,file \"\%s\" fun \"\%s\" content \"\%s\"",MyLibCM_name.c_str(),FName.c_str(),FunName.c_str(),RetVal.c_str());
  } else {
    WORKER_DEBUG("libcmfun",1,"libcm(%s):LoadCmFun FAILED, cannot open load file \"\%s\"",MyLibCM_name.c_str(),FName.c_str());
    throw "LOAD Fun FILE";
  }

  return RetVal;
}

void CmMuParserHook::EvalPollExt(CCmConnection &conn, PollCond &poll_cond_ext, double ActTime)
{
    EXT_Mutex->Lock();

    double NextPollTime = 0.0;
    bool NoPending = false;
    do
    {
	ActTime = GetActualTimeDouble();
	VarEventMap UpdateMap; UpdateMap.clear();

	for (std::map <std::string, CCmWorkerExt_ExtDef>::iterator ItMainDef =  ExtDefMap.begin(); (ItMainDef !=  ExtDefMap.end()) && (UpdateMap.size()==0); ItMainDef++)
	{
	    double nPollTime = ItMainDef->second.doPendingActionsOrPoll(CM_MUP_Interface, MyAllVariables, UpdateMap, ActTime);
	    if ((NextPollTime == 0.0) || ((nPollTime!=0.0)&&(NextPollTime > nPollTime))) NextPollTime = nPollTime;
	}

	if (UpdateMap.size()>0)
	{
	    EXT_Mutex->Unlock();
	    for (VarEventMap::iterator itU = UpdateMap.begin(); itU != UpdateMap.end(); itU++)
	    {
		CmEventWhat EventWhat = itU->second.EventWhat;
//         debug_printf("EVENT%d %s",EventWhat, itU->second.fullPrint().c_str());
		if (EventWhat==CmEventWhat::undefine)
		{
		    conn.UndefineVariable(itU->first,true);
//           debug_printf("++++++UnDefine %s",itU->second.fullPrint().c_str());
		    if (LiveDebugFlags & DEBUG_FLAG_PRINT_EXT_DELETE)
			LiveDebug->PrintANY(DEBUG_FLAG_PRINT_EXT_DELETE, 0, "DeleteVar:", itU->second.variable.fullPrint().c_str());
		}
		if (EventWhat==CmEventWhat::define)
		{
		    conn.DefineMyVariable(&itU->second.variable,true);
		    if (LiveDebugFlags & DEBUG_FLAG_PRINT_EXT_CREATE)
			LiveDebug->PrintANY(DEBUG_FLAG_PRINT_EXT_CREATE, 0, "CreateVar:", itU->second.variable.fullPrint().c_str());

//  DEBUG_FLAG_PRINT_EXT_CREATE          = LiveDebug.getFlagMask('n');
//           debug_printf("++++++Define %s",itU->second.variable.fullPrint().c_str());
		}
		if (EventWhat==CmEventWhat::update)
		{
		    conn.ChangeVariable(&itU->second.variable, CMLIB_VALUE_PRIORITY_UNDEF, true);
		    if (LiveDebugFlags & DEBUG_FLAG_PRINT_EXT_UPDATE)
			LiveDebug->PrintANY(DEBUG_FLAG_PRINT_EXT_UPDATE, 0, "UpdateVar:", itU->second.variable.fullPrint().c_str());
//           debug_printf("++++++Change %s",itU->second.fullPrint().c_str());
		}
	    }
	    EXT_Mutex->Lock();
	} else NoPending = true;

    } while (!NoPending);
//    if (NextPollTime!=0.0)debug_printf("--------NextPollTime=%f",NextPollTime);

    double CmFunRqNextTime = CmFunExecOneLoop(conn);
    double UsedTime = 0.0;
    double TestTime = NextPollTime;
    if ((ActTime<TestTime) &&  ((TestTime < UsedTime) || (UsedTime == 0.0))) UsedTime = TestTime;
    TestTime = CmFunRqNextTime;
    if ((ActTime<TestTime) &&  ((TestTime < UsedTime) || (UsedTime == 0.0))) UsedTime = TestTime;
    if (UsedTime > 0.0)
    {
	poll_cond_ext.ChangeRqTime(UsedTime);
    }
    EXT_Mutex->Unlock();
}


// JAN: Tohle potrebuje ukazatel na CCmWorkera
// void CmMuParserHook::PollThreadOneLoop (double &sleep_time)
// {
//   CCmWorker::PollThreadOneLoop (sleep_time);

//   double ActTime = GetActualTimeDouble();
//   // EXT ------------------============----------------===================----------------
  
//   PollCondExt.UpdateSleepTime(ActTime, sleep_time);
// }

void CmMuParserHook::Start(void)
{
  // JAN
  // CCmWorker::Start();
  //  CM_Mutex->Lock();
  //  CM_Mutex->Unlock();
  EXT_Mutex->Lock();


  for (std::map <std::string, CCmWorkerExt_ExtDef>::iterator ItMainDef =  ExtDefMap.begin(); ItMainDef !=  ExtDefMap.end(); ItMainDef++)
  {
    ItMainDef->second.EvaluatePending = (1<<0) | (1<<1)| (1<<4);
  }

  EXT_Mutex->Unlock();
}

bool CmMuParserHook::checkOtherFilters(CCmFiltersSet *pFilters, int pType, bool UnlimitedStars)
{//pType : 0=myFilters, 1=PartnerFilters, 2=Ext (self)
// pFilters can be NULL ... set/unset flags (1<<pType) and check change
  bool changed = false;
  WORKER_DEBUG("libcmext",9,"libcm(%s):checkOtherFilters ---===-----======-----",MyLibCM_name.c_str());
  for (std::map <std::string, CCmWorkerExt_ExtDef>::iterator ItMainDef =  ExtDefMap.begin(); ItMainDef !=  ExtDefMap.end(); ItMainDef++)
  {
    bool need = false;
    uint8_t OrigoNeedMe = ItMainDef->second.FiltersNeedMe;
    if ((!NeedAllExtVARS)&&(!UnlimitedStars)&&(pFilters!=NULL))
    {
//    ItMainDef->first
//    ItMainDef->second.Var.getName()
      need = pFilters->test(ItMainDef->first);
    } else if ((UnlimitedStars)||(NeedAllExtVARS)) need = true;
    if (need)
    {
      ItMainDef->second.FiltersNeedMe |= (1<<pType);
    } else {
      ItMainDef->second.FiltersNeedMe &= ~(1<<pType);
    }
    WORKER_DEBUG("libcmext",9,"libcm(%s):checkOtherFilters ---NEED %s = %d",MyLibCM_name.c_str(),ItMainDef->second.VarName.c_str(),ItMainDef->second.FiltersNeedMe);
    if (OrigoNeedMe != ItMainDef->second.FiltersNeedMe) changed = true;
  }
  return changed;
}

void CmMuParserHook::RegenerateMyFiltersExt(CCmFiltersSet &myFiltersExt)
{
  myFiltersExt.clear();
  myFiltersExtInternal.clear();
  for (std::map <std::string, CCmWorkerExt_ExtDef>::iterator ItMainDef =  ExtDefMap.begin(); ItMainDef !=  ExtDefMap.end(); ItMainDef++)
  {
    if (ItMainDef->second.FiltersNeedMe)
    {
      myFiltersExt.add(ItMainDef->second.AllUsedVariables); // TEST na moje vlastni a ty nepouzit
      myFiltersExtInternal.add(ItMainDef->second.AllUsedVariables);
    }
  }

  for (std::map <std::string, CCmWorkerExt_ExtDef>::iterator ItMainDef2 =  ExtDefMap.begin(); ItMainDef2 !=  ExtDefMap.end(); ItMainDef2++)
  { // ItMainDef2->first
    myFiltersExt.erase(ItMainDef2->first);
  }
}

// JAN: Tohle presunout do Base
// void CmMuParserHook::RefreshFilters(bool useLockCM) // my filters
// {
//   if (useLockCM)
//   {
//     CM_Mutex->Lock();
//   }
//     EXT_Mutex->Lock();
//     bool UnlimitedStars = ((myFiltersIn.getOnlyStarMaxCount() >= CM_FILTER_ONLYSTARS_UNLIMITED) ? true : false);
//     bool changed = checkOtherFilters(&myFiltersIn, 0, UnlimitedStars);
//     if (changed)
//     {
//       RegenerateMyFiltersExt();
//       bool UnlimitedStarsExt = ((myFiltersExtInternal.getOnlyStarMaxCount() >= CM_FILTER_ONLYSTARS_UNLIMITED) ? true : false);
//       while (checkOtherFilters(&myFiltersExtInternal, 2, UnlimitedStarsExt))
//       {
//         RegenerateMyFiltersExt();
//       }
//     }
//     if (AddCmFunFiltersAll(AllUsedVariables)) changed = true;
//     if (AddCmFunFiltersMy(myFiltersExt)) changed = true;
//     EXT_Mutex->Unlock();
//   if (useLockCM)
//   {
//     CM_Mutex->Unlock();
//   }
//     if (changed)
//     {
//       WORKER_DEBUG("libcmext",4,"libcm(%s):ExtFilterRequest - (myFiltersRq) changed : %s\n",MyLibCM_name.c_str(),myFiltersExt.fullPrint(" | ").c_str());
//     }
//     CCmWorker::RefreshFilters(useLockCM);
//     if (changed && (FilterCB_function != NULL))
//     {
//       PartnerFiltersChanged = true;
//       CB__Condition->Broadcast();
//     }
// }


bool CmMuParserHook::RefreshExtFilters(CCmFiltersSet &myFiltersIn, CCmFiltersSet &myFiltersExt)
{
    EXT_Mutex->Lock();
    WORKER_DEBUG("ext_change", 20, "RefreshExtFilters: in=%s, ext=%s", print_var(myFiltersIn).c_str(), print_var(myFiltersExt).c_str());
    bool UnlimitedStars = ((myFiltersIn.getOnlyStarMaxCount() >= CM_FILTER_ONLYSTARS_UNLIMITED) ? true : false);
    bool changed = checkOtherFilters(&myFiltersIn, 0, UnlimitedStars);
    if (changed)
    {
	RegenerateMyFiltersExt(myFiltersExt);
	bool UnlimitedStarsExt = ((myFiltersExtInternal.getOnlyStarMaxCount() >= CM_FILTER_ONLYSTARS_UNLIMITED) ? true : false);
	while (checkOtherFilters(&myFiltersExtInternal, 2, UnlimitedStarsExt))
	{
	    RegenerateMyFiltersExt(myFiltersExt);
	}
    }
    if (AddCmFunFiltersAll(AllUsedVariables)) changed = true;
    if (AddCmFunFiltersMy(myFiltersExt)) changed = true;
    WORKER_DEBUG("cmext", 20, "RefreshExtFilters changed=%d: new_ext=%s", changed, print_var(myFiltersExt).c_str());
    EXT_Mutex->Unlock();
    return changed;
}

// JAN:Tohle presunout do base
bool CmMuParserHook::RegenaratePartnerFiltersExt(CCmFiltersSet &PartnerFiltersActive, CCmFiltersSet &myFiltersExt) // before CB funct
{
    EXT_Mutex->Lock();
    bool UnlimitedStars = ((PartnerFiltersActive.getOnlyStarMaxCount() >= CM_FILTER_ONLYSTARS_UNLIMITED) ? true : false);

    bool changed = checkOtherFilters(&PartnerFiltersActive, 1, UnlimitedStars);
    if (changed)
    {
	RegenerateMyFiltersExt(myFiltersExt);
	bool UnlimitedStarsExt = ((myFiltersExtInternal.getOnlyStarMaxCount() >= CM_FILTER_ONLYSTARS_UNLIMITED) ? true : false);
	while (checkOtherFilters(&myFiltersExtInternal, 2, UnlimitedStarsExt))
	{
	    RegenerateMyFiltersExt(myFiltersExt);
	}
    }

    PartnerFiltersActive.add(myFiltersExt); // TESTPOINT 2
    EXT_Mutex->Unlock();
    return changed;
}
// {
//   CCmWorker::RegenaratePartnerFilters(PartnerFiltersActive); // before CB funct
//   EXT_Mutex->Lock();
//   bool UnlimitedStars = ((PartnerFiltersActive.getOnlyStarMaxCount() >= CM_FILTER_ONLYSTARS_UNLIMITED) ? true : false);

//   bool changed = checkOtherFilters(&PartnerFiltersActive, 1, UnlimitedStars);
//   if (changed)
//   {
//     RegenerateMyFiltersExt();
//     bool UnlimitedStarsExt = ((myFiltersExtInternal.getOnlyStarMaxCount() >= CM_FILTER_ONLYSTARS_UNLIMITED) ? true : false);
//     while (checkOtherFilters(&myFiltersExtInternal, 2, UnlimitedStarsExt))
//     {
//       RegenerateMyFiltersExt();
//     }
//   }

//   PartnerFiltersActive.add(myFiltersExt); // TESTPOINT 2
//   EXT_Mutex->Unlock();
//   if (changed)
//   {
//     WORKER_DEBUG("libcmext",4,"libcm(%s):ExtFilterRequest - (PartnerFilterCB) changed : %s\n",MyLibCM_name.c_str(),myFiltersExt.fullPrint(" | ").c_str());
//   }
//   if (changed)
//   {
// //    CM_Mutex->Unlock();
//     CCmWorker::RefreshFilters(false);
// //    CM_Mutex->Lock();
//   }
// }

// JAN: Je tahle funkce vubec potreba
void CmMuParserHook::SetNeedAllExtVARS(bool b)
{}
// {
//   CM_Mutex->Lock();
//   EXT_Mutex->Lock();
//   NeedAllExtVARS = b;
//   RefreshFilters(false);
//   EXT_Mutex->Unlock();
//   CM_Mutex->Unlock();
// }


C_CM_MUP_Interface *CmMuParserHook::GetParserPtr(void)
{
  if (!(CmWorkerFlags & CM_WORKER_EXT_FLAG_MUPARSER_UPDATE_ALL))
  {
    WORKER_DEBUG("libcmext",1,"libcm(%s):CmMuParserHook::GetParserPtr() ... in constructor is not seted flag CM_WORKER_EXT_FLAG_MUPARSER_UPDATE_ALL - crazy",MyLibCM_name.c_str());
    throw "CmExt::GetParserPtr::-noUpdateAll";
    return NULL;
  }
  return CM_MUP_Interface;
}


bool CmMuParserHook::CmFunAdd(const std::string name, const std::string fun, VarMap &UnUsedVariables, std::set<std::string> &FunUsedVars, bool UseLock)
{
  if (UseLock) EXT_Mutex->Lock();
  WORKER_DEBUG("libcmfun",14,"libcm(%s):CmFunAdd(%s):%s", MyLibCM_name.c_str(), name.c_str(), fun.c_str());

// debug_printf("CM Funct(%s):%s\n===========",name.c_str(), fun.c_str());
  class CCmExtFun NewFun;
  NewFun.init();
  NewFun.set_name(name);
  NewFun.set_instance(false);
  NewFun.setMyLibCM_name(MyLibCM_name);
  bool RetVal =  NewFun.set(fun, CM_MUP_Interface, UnUsedVariables, FunUsedVars);
  if (RetVal)
  {
    WORKER_DEBUG("libcmfun",3,"libcm(%s):CmFunAdd(%s)\n(%s)\n",MyLibCM_name.c_str(),name.c_str(),NewFun.print_me().c_str());
    NewFun.add_my_filters(myFiltersFun);
    CmFun[name] = NewFun;
  } else {
    WORKER_DEBUG("libcmfun",1,"libcm(%s):CmFunAdd(%s)\nERROR:(%s)\n",MyLibCM_name.c_str(),name.c_str(),NewFun.GetLastError().c_str());
    throw "AddFun";
  }
  if (UseLock) EXT_Mutex->Unlock();
  return RetVal;
}

unsigned CmMuParserHook::CmFunStart(const std::string name, const StringList &params, bool IfRunningDontStart)
{
  std::map <std::string, class CCmExtFun>::iterator it = CmFun.find(name);
  if (it != CmFun.end())
  {
    bool running = it->second.is_running();
    if (IfRunningDontStart && running) return 1; // it's running
    it->second.start_me(CM_MUP_Interface, params);
    WORKER_DEBUG("libcmfun",8,"libcm(%s):AUTOEXEC: SetStart \"\%s\"",MyLibCM_name.c_str(),name.c_str());
    return 0; // success
  } else {
    WORKER_DEBUG("libcmfun",1,"libcm(%s):AUTOEXEC: CANNOT SetStart \"\%s\" .. not found",MyLibCM_name.c_str(),name.c_str());
    return 2; // error
  }
}

std::string CmMuParserHook::CmFunMakeInstanceName(std::string inp)
{
  char WStr[200];
  std::string testname;
  int i = 0;
  std::map <std::string, class CCmExtFun>::iterator it;
  do
  {
    i++;
    sprintf(WStr,"%d",i);
    testname = inp + "__INSTANCE__" + std::string(WStr);
    it = CmFun.find(testname);
  } while (it != CmFun.end());
  return testname;
}

void CmMuParserHook::CmFunSetDone(std::string fun)
{
  for (std::map <std::string, class CCmExtFun>::iterator it = CmFun.begin(); it != CmFun.end(); it++)
  {
    it->second.info_fun_done(fun);
  }
}

void CmMuParserHook::CmFunErase(std::string fun)
{
  std::map <std::string, class CCmExtFun>::iterator it = CmFun.find(fun);
  if (it != CmFun.end())
  {
    CmFun.erase(it);
  }
}


void CmMuParserHook::CmFunKill(std::string kill_name_pattern, bool UseLock)
{
  if (UseLock) EXT_Mutex->Lock();
  WORKER_DEBUG("libcmfun",14,"libcm(%s):CmFunKill():%s", MyLibCM_name.c_str(), kill_name_pattern.c_str());
  for (std::map <std::string, class CCmExtFun>::iterator it = CmFun.begin(); it != CmFun.end(); )
  {
    std::map <std::string, class CCmExtFun>::iterator itD = it++;
    if (libCM_isNameMatching(kill_name_pattern, itD->first))
    {
//      CmFun.erase(itD);
       itD->second.set_done();
//       state = E_CM_EXT_FUNCTIONS_STATE_DONE;
    }
  }


  if (UseLock) EXT_Mutex->Unlock();
}

double CmMuParserHook::CmFunExecOneLoop(CCmConnection &conn)
{
  double ActTime = GetActualTimeDouble();
  VarEventMap UpdateMap;
  CmFunWaits.clear();
  double WaitTime = 0.0;
  bool NeedNextLoop = false;

  std::string now_call_function;
  std::string now_call_function_instance;
  std::string erase_fun;
  std::string now_fundef_name;
  std::string now_fundef_code;
  std::string now_kill_name;
  enum E_CM_EXT_FUNCTIONS_CALL_MODE2 now_call_function_mode2;
  StringList now_call_function_params;
  std::map<std::string, CmEventWhat> UpdateSetAll;

  for (std::map <std::string, class CCmExtFun>::iterator it = CmFun.begin(); (it != CmFun.end()) && (erase_fun.empty()) && (now_call_function.empty()) && (now_fundef_name.empty()) && (now_kill_name.empty()); it++)
  {
    double _WaitTime = 0.0;
    bool _NeedNextLoop = false;
    enum E_CM_EXT_FUNCTIONS_STATE OldState = it->second.get_state();
    unsigned RET = it->second.exec(CM_MUP_Interface, MyAllVariables, UpdateMap, UpdateSetAll, CmFunWaits, WaitTime, NeedNextLoop, ActTime);
    enum E_CM_EXT_FUNCTIONS_STATE NewState = it->second.get_state();
    if (_NeedNextLoop) NeedNextLoop = true;
    if (_WaitTime>0.0)
    {
      if ((WaitTime==0.0)||(_WaitTime<WaitTime)) WaitTime = _WaitTime;
    }
    if ((OldState != NewState)&&((RET == CM_FUN_RETVAL_STATE_DONE)||(RET == CM_FUN_RETVAL_STATE_ERROR)))
    {
      CmFunSetDone(it->first);
      if (RET == CM_FUN_RETVAL_STATE_ERROR) erase_fun = it->first;
      if (it->second.get_instance())
      {
        erase_fun = it->first;
      }
    }

    if (it->second.get_fundef_request(now_fundef_name, now_fundef_code))
    {
    }
    if (it->second.get_kill_request(now_kill_name))
    {
    }
    if (it->second.get_call_request(now_call_function, now_call_function_mode2, now_call_function_params))
    {
      now_call_function_instance.clear();
//     debug_printf ("doCall(%s)\n",now_call_function.c_str()); 
      switch (now_call_function_mode2)
      {
        case E_CM_EXT_FUNCTIONS_CALL_MODE2_INST:
          now_call_function_instance = CmFunMakeInstanceName(now_call_function);
          break;
        case E_CM_EXT_FUNCTIONS_CALL_MODE2_KILL:
        case E_CM_EXT_FUNCTIONS_CALL_MODE2_IF:
          break;
      }
      it->second.set_wait_fun( ((now_call_function_instance.empty()) ? now_call_function : now_call_function_instance) );
    } else now_call_function.clear();
  }
  if (!now_kill_name.empty())
  {
    CmFunKill(now_kill_name,false);
    now_kill_name.clear();
  }
  if (!now_fundef_name.empty())
  {
    try
    {
      VarMap __UnUsedVariables;
      std::set<std::string> __FunUsedVars;
      if (!CmFunAdd(now_fundef_name, now_fundef_code, __UnUsedVariables, __FunUsedVars, false))
      {
        WORKER_DEBUG("libcmfun",1,"libcm(%s):fundef(%s,\"%s\"):1 ",MyLibCM_name.c_str(),now_fundef_name.c_str(), now_fundef_code.c_str());
      }
    }
    catch (...)
    {
        WORKER_DEBUG("libcmfun",1,"libcm(%s):fundef(%s,\"%s\"):2 ",MyLibCM_name.c_str(),now_fundef_name.c_str(), now_fundef_code.c_str());
//      throw;
    }
    now_fundef_name.clear();
  }
  if (!now_call_function.empty())
  {
    bool IfRunningDontStart = true;
    std::string newfunname = ((now_call_function_instance.empty()) ? now_call_function : now_call_function_instance);
      switch (now_call_function_mode2)
      {
        case E_CM_EXT_FUNCTIONS_CALL_MODE2_INST:
           {
             std::map <std::string, class CCmExtFun>::iterator it = CmFun.find(now_call_function);
             if (it != CmFun.end())
             {
               class CCmExtFun FUN = it->second;
               FUN.set_name(now_call_function_instance);
               FUN.set_instance(true);
               CmFun[now_call_function_instance] = FUN;
//              debug_printf("COPIED INSTANCE(%s)->(%s)",now_call_function.c_str(),now_call_function_instance.c_str());
             }
           }
          break;
        case E_CM_EXT_FUNCTIONS_CALL_MODE2_KILL:
           IfRunningDontStart = false;
          break;
        case E_CM_EXT_FUNCTIONS_CALL_MODE2_IF:
           IfRunningDontStart = true;
          break;
      }
//     debug_printf("START_PROC(%s)",newfunname.c_str());
      CmFunStart(newfunname, now_call_function_params,IfRunningDontStart);

    now_kill_name.clear();
  }
  if (!erase_fun.empty())
  {
//   debug_printf("ERASE FUN/INSTANCE %s",erase_fun.c_str());
    CmFunErase(erase_fun);
  }

  if (UpdateMap.size()>0)
  {
    EXT_Mutex->Unlock();
    for (VarEventMap::iterator itU = UpdateMap.begin(); itU != UpdateMap.end(); itU++)
    {
          CmEventWhat EventWhat = itU->second.EventWhat;
//         debug_printf("EVENT%d %s",EventWhat, itU->second.fullPrint().c_str());
          if (EventWhat==CmEventWhat::undefine)
          {
            conn.UndefineVariable(itU->first,true);
//           debug_printf("++++++UnDefine %s",itU->second.fullPrint().c_str());
            if (LiveDebugFlags & DEBUG_FLAG_PRINT_EXT_DELETE) LiveDebug->PrintANY(DEBUG_FLAG_PRINT_EXT_DELETE, 0, "DeleteVar:", itU->second.variable.fullPrint().c_str());
          }
          if (EventWhat==CmEventWhat::define)
          {
	      conn.DefineMyVariable(&itU->second.variable,true);
            if (LiveDebugFlags & DEBUG_FLAG_PRINT_EXT_CREATE) LiveDebug->PrintANY(DEBUG_FLAG_PRINT_EXT_CREATE, 0, "CreateVar:", itU->second.variable.fullPrint().c_str());

//  DEBUG_FLAG_PRINT_EXT_CREATE          = LiveDebug.getFlagMask('n');
//           debug_printf("++++++Define %s",itU->second.variable.fullPrint().c_str());
          }
          if (EventWhat==CmEventWhat::update)
          {
            conn.ChangeVariable(&itU->second.variable, CMLIB_VALUE_PRIORITY_UNDEF, true);
            if (LiveDebugFlags & DEBUG_FLAG_PRINT_EXT_UPDATE) LiveDebug->PrintANY(DEBUG_FLAG_PRINT_EXT_UPDATE, 0, "UpdateVar:", itU->second.variable.fullPrint().c_str());
//           debug_printf("++++++Change %s",itU->second.fullPrint().c_str());
          }
    }
    EXT_Mutex->Lock();
  }
  return (NeedNextLoop ? (ActTime + (LIB_CM_TIME_TOLERANCE / 2)) : WaitTime);
}

bool CmMuParserHook::CmFunChangedVariable(CCmVariable const *var)
{
  bool RetVal = false;
  for (std::map <std::string, class CCmExtFun>::iterator it = CmFun.begin(); it != CmFun.end(); it++)
  {
    if (it->second.changed_var(CM_MUP_Interface, var)) RetVal = true;
  }
  return RetVal;
}

bool CmMuParserHook::AddCmFunFiltersAll(class CCmFiltersSet &_AllUsedVariables)
{
//  if (MyLibCM_name == "Router")debug_printf("FILTERS 55All:");
//  myFiltersFun.add("Sim_A__SW_LOCK");

  return _AllUsedVariables.add(myFiltersFun);

}

bool CmMuParserHook::AddCmFunFiltersMy(class CCmFiltersSet &_myFiltersExt)
{
//  if (MyLibCM_name == "Router")debug_printf("FILTERS 55My:");
  return _myFiltersExt.add(myFiltersFun);

}

double CmMuParserHook::GetActualTimeDouble()
{
  return libCM_GetActualTimeDouble();
}

#endif // USE_LIB_MUPARSERX
