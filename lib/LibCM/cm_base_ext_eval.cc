#if USE_LIB_MUPARSERX

#include "cm_ext_muparser_hook.hh"
#include "cm_string.hh"
#include "cm_std_string.hh"
#include "cm_debug.hh"



bool CmMuParserHook::CCmWorkerExt_ExtDef_Action_Eval::SetMe(std::deque<std::string> &cmd_items, std::string line)
{
  if (cmd_items.size()==1)
  {
    EvalString = cmd_items[0];
    return true;
  } else {
    cms_ns_if_print("libcmext",1,"CmMuParserHook::CCmWorkerExt_ExtDef_Action_Eval::SetMe wrong number of parameters (required 1, but seted %zu) on line \"%s\"",cmd_items.size(),line.c_str());
    throw "CmExt::ActionEval:err";
    return false;
  }
}
void CmMuParserHook::CCmWorkerExt_ExtDef_Action_Eval::GetUsedVariables(C_CM_MUP_Interface *MUP)
{
  std::list<class CCmFilter> MyUsedVarsOutList;
  std::list<class CCmFilter> MyUsedVarsRqList;
  if (!MUP->GetUsedVariables(EvalString, &MyUsedVarsOutList, true, true, false)) // output
  {
    cms_ns_if_print("libcmext",1, "MUP::ERROR:GetUsedVariables \"%s\" -> \'%s\'",EvalString.c_str(), MUP->GetLastError().c_str());
    throw "CmExt::ActionEval:GetUsedVariables";
  }
  if (!MUP->GetUsedVariables(EvalString, &MyUsedVarsRqList, true, false, true)) // input
  {
    cms_ns_if_print("libcmext",1, "MUP::ERROR:GetUsedVariables \"%s\" -> \'%s\'",EvalString.c_str(), MUP->GetLastError().c_str());
    throw "CmExt::ActionEval:GetUsedVariables";
  }
  if (MyUsedVarsOutList.size()>0)
  {
    LastOut = *MyUsedVarsOutList.rbegin();
    LastOutDefined = true;
  }
  MyUsedVarsOut.add(MyUsedVarsOutList);
  MyUsedVarsRq .add(MyUsedVarsRqList );
}

bool CmMuParserHook::CCmWorkerExt_ExtDef_Action_Eval::Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, double ActTime)
{
  bool MUPRet = MUP->Evaluate(EvalString);
  if (!MUPRet) PrintLastErrorMUP(DebugPrefix, MUP, EvalString);
  return true;
}
bool CmMuParserHook::CCmWorkerExt_ExtDef_Action_Eval::Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, bool &ReturnVal, double ActTime)
{
  ReturnVal = false;
  bool MUPRet = MUP->Evaluate(EvalString, ReturnVal);
  if (!MUPRet) PrintLastErrorMUP(DebugPrefix, MUP, EvalString);
  if (!MUPRet) ReturnVal = false;
  return true;
}

bool CmMuParserHook::CCmWorkerExt_ExtDef_Action_Eval::Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, std::string ExtraAssign, double ActTime)
{
  bool MUPRet = false;
  if (MyUsedVarsOut.size()==0)
  {
    std::string EvalStr = ExtraAssign + "=" + EvalString;
    MUPRet = MUP->Evaluate(EvalStr);
    if (!MUPRet) PrintLastErrorMUP(DebugPrefix, MUP, EvalStr);
  } else {
    MUPRet = MUP->Evaluate(EvalString);
    if (!MUPRet) PrintLastErrorMUP(DebugPrefix, MUP, EvalString);
    if (LastOutDefined)
    {
      std::string EvalStr = ExtraAssign + "=" + LastOut.toString();
      MUPRet = MUP->Evaluate(EvalStr);
      if (!MUPRet) PrintLastErrorMUP(DebugPrefix, MUP, EvalStr);
    }
  }
  return true;
}
std::string CmMuParserHook::CCmWorkerExt_ExtDef_Action_Eval::PrintMe(void)
{
  return "Eval:" + EvalString; 
}

#endif // USE_LIB_MUPARSERX
