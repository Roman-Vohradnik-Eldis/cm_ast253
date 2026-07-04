#if USE_LIB_MUPARSERX

#include "cm_ext_muparser_hook.hh"
#include "cm_string.hh"
#include "cm_std_string.hh"
#include "cm_debug.hh"


bool CmMuParserHook::CCmWorkerExt_ExtDef_Action_WorstStatus::SetMe(std::deque<std::string> &cmd_items, std::string line)
{
  if ((cmd_items.size()==4) || (cmd_items.size()==5))
  {
    EvalString = cmd_items[0];
    UndefinedUnavail = cmd_items[1];
    Unknown = cmd_items[2];
    CharsFromWorst = cmd_items[3];
    if (cmd_items.size()>4) OutputVar = cmd_items[4];
    if ((CharsFromWorst.size()<1)||(UndefinedUnavail.size()<1)||(Unknown.size()<1))
    {
	cms_ns_if_print("libcmext",1,"CmMuParserHook::CCmWorkerExt_ExtDef_Action_WorstStatus::SetMe wrong parameters length (too short) on line \"%s\"",line.c_str());
      throw "CmExt::ActionWorstStatus:err_params";
      return false;
    }
    return true;
  } else {
      cms_ns_if_print("libcmext",1,"CmMuParserHook::CCmWorkerExt_ExtDef_Action_WorstStatus::SetMe wrong number of parameters (required 4 or 5, but set %zu) on line \"%s\"",cmd_items.size(),line.c_str());
    throw "CmExt::ActionWorstStatus:err";
    return false;
  }
}
void CmMuParserHook::CCmWorkerExt_ExtDef_Action_WorstStatus::GetUsedVariables(C_CM_MUP_Interface *MUP)
{
  std::list<class CCmFilter> MyUsedVarsRqList;
  if (!MUP->GetUsedVariables(EvalString, &MyUsedVarsRqList, true, false, true)) // input
  {
    cms_ns_if_print("libcmext",1, "MUP::ERROR:GetUsedVariables \"%s\" -> \'%s\'",EvalString.c_str(), MUP->GetLastError().c_str());
    throw "CmExt::ActionWorstStatus:GetUsedVariables";
  }
  MyUsedVarsRq .add(MyUsedVarsRqList );
  if (!OutputVar.empty())
  {
    std::list<class CCmFilter> MyUsedVarsOutList;

    if (!MUP->GetUsedVariables(OutputVar, &MyUsedVarsOutList, true)) // output
    {
      cms_ns_if_print("libcmext",1, "MUP::ERROR:GetUsedVariables \"%s\" -> \'%s\'",OutputVar.c_str(), MUP->GetLastError().c_str());
      throw "CmExt::ActionWorstStatus:GetUsedVariables";
    }
    if (MyUsedVarsOutList.size()>0)
    {
      LastOut = *MyUsedVarsOutList.rbegin();
      LastOutDefined = true;
    }
    MyUsedVarsOut.add(MyUsedVarsOutList);
  }
}
std::string CmMuParserHook::CCmWorkerExt_ExtDef_Action_WorstStatus::Eval(C_CM_MUP_Interface *MUP, double ActTime)
{
  std::string Status;
  char Ret = 'E';
  if (CharsFromWorst.size()>0) Ret=CharsFromWorst.c_str()[CharsFromWorst.size()-1];
  if (!MUP->Evaluate(EvalString, Status))
  {
    if (UndefinedUnavail.size()>0) Ret = UndefinedUnavail.c_str()[0];
  } else {
    std::string s = Status;
    std::string delimiter = "/";
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos)
    {
      token = s.substr(0, pos);
      char Act = 'E';
      if (Unknown.size()>0) Act=Unknown.c_str()[0];
      if ((token.size()>1) && (token.c_str()[1]==':')) Act = token.c_str()[0];
      for (unsigned i=0;(i<CharsFromWorst.size()) && (CharsFromWorst.c_str()[i]!=Ret);i++) if (Act==CharsFromWorst.c_str()[i]) Ret=Act;
      s.erase(0, pos + delimiter.length());
    }
    token = s;
      char Act = 'E';
      if (Unknown.size()>0) Act=Unknown.c_str()[0];
      if ((token.size()>1) && (token.c_str()[1]==':')) Act = token.c_str()[0];
      for (unsigned i=0;(i<CharsFromWorst.size()) && (CharsFromWorst.c_str()[i]!=Ret);i++) if (Act==CharsFromWorst.c_str()[i]) Ret=Act;
  }
  char WStrX[2]; WStrX[0]=Ret; WStrX[1]=0;
  return std::string(WStrX);
}
bool CmMuParserHook::CCmWorkerExt_ExtDef_Action_WorstStatus::Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, double ActTime)
{
  if (!OutputVar.empty())
  {
    std::string Out = Eval(MUP, ActTime);
    MUP->SetVariable(OutputVar,Out);
  }
  return true;
}
bool CmMuParserHook::CCmWorkerExt_ExtDef_Action_WorstStatus::Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, std::string ExtraAssign, double ActTime)
{
  std::string Out = Eval(MUP, ActTime);
  if (!OutputVar.empty()) MUP->SetVariable(OutputVar,Out);
  MUP->SetVariable(ExtraAssign,Out);
  return true;
}

std::string CmMuParserHook::CCmWorkerExt_ExtDef_Action_WorstStatus::PrintMe(void)
{
  std::string RetVal = "WorstStatus:MUP:" + EvalString + ":Undef=" + UndefinedUnavail + ":Unknown=" + Unknown + ":CharsFromWorst=" + CharsFromWorst;
  if (!OutputVar.empty()) RetVal += ":OutputVar=" + OutputVar;
  return RetVal;
}


#endif // USE_LIB_MUPARSERX
