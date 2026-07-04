#if USE_LIB_MUPARSERX

#include "cm_ext_muparser_hook.hh"
#include "cm_string.hh"
#include "cm_std_string.hh"
#include "cm_debug.hh"


std::string CmMuParserHook::CCmWorkerExt_ExtDef_Action_Int2Str::AddCode(const std::string &Inp, int code)
{
  char* wstr = (char*) malloc(Inp.size()+1+(10*20));
  sprintf(wstr, Inp.c_str(), code, code, code, code, code, code, code, code, code, code);
  std::string RetVal = std::string(wstr);
  free(wstr);
  return RetVal;
}

std::string CmMuParserHook::CCmWorkerExt_ExtDef_Action_Int2Str::Eval(C_CM_MUP_Interface *MUP, double ActTime)
{
  int MyValue;
  if (!MUP->Evaluate(EvalString, MyValue)) return StatusUndefInVal;
  std::string StTmpMain;
  std::string StTmpBit;
  std::string StMasterMain;
  std::string StMasterBit;
  std::string OutString;
  for (std::list< std::pair<int,std::string> >::iterator it=StatusList.begin(); it!=StatusList.end(); it++)
  {
    int Type_and_mask = ((it->first >> 16) & 0xffff);
    int Compare_value = ((it->first >>  0) & 0xffff);
    std::string Tested_string = it->second;

    if (Type_and_mask & 0x100)
    { // Tmp
      if (Type_and_mask & 0xff) //multi bit mask
      {
        if ((MyValue & (Type_and_mask & 0xff))==Compare_value)
        {
          StTmpBit += (StTmpBit.empty()?"":"/") + AddCode(Tested_string,MyValue);
        }
      } else { // simple_status
        if (MyValue==Compare_value)
        {
          StTmpMain = Tested_string;
        }
      }
    } else { // Master
      if (Type_and_mask & 0xff) //multi bit mask
      {
        if ((MyValue & (Type_and_mask & 0xff))==Compare_value)
        {
          StMasterBit += (StMasterBit.empty()?"":"/") + AddCode(Tested_string,MyValue);
        }
      } else { // simple_status
        if (MyValue==Compare_value)
        {
          StMasterMain = Tested_string;
        }
      }
    }
  }
  bool AddPrevious = false;
       if (!StMasterMain.empty())   OutString = AddCode(StMasterMain,MyValue);
  else if (!StMasterBit. empty())   OutString = AddCode(StMasterBit ,MyValue);
  else if (!StTmpMain.   empty()) { OutString = StTmpMain   ; AddPrevious=true; }
  else if (!StTmpBit.    empty()) { OutString = StTmpBit    ; AddPrevious=true; }
  if ((AddPrevious) && (!PreviousStatus.empty()))
  {
    if (PreviousStatus.find(OutString, 0) == std::string::npos) // don't add it multiple
    {
      OutString = PreviousStatus + "/" + OutString;
    } else {
      OutString = PreviousStatus;
    }
  }
  if (OutString.empty()) OutString = AddCode(StatusUnknown,MyValue);
  return OutString;
}



bool CmMuParserHook::CCmWorkerExt_ExtDef_Action_Int2Str::SetMe(std::deque<std::string> &cmd_items, std::string line)
{
  if ((cmd_items.size()>=4))
  {
    EvalString = cmd_items[0];
    StatusUndefInVal = cmd_items[1];
    StatusUnknown = cmd_items[2];
    OutputVar = cmd_items[3];
    if ((StatusUndefInVal.size()<1)||(StatusUnknown.size()<1))
    {
      cms_ns_if_print("libcmext",1,"CmMuParserHook::CCmWorkerExt_ExtDef_Action_Int2Str::SetMe wrong parameters length (too short) on line \"%s\"",line.c_str());
      throw "CmExt::ActionInt2Str:err_params";
      return false;
    }

    for (unsigned i=0;i<((cmd_items.size()-4)/3);i++)
    {
      int Type_and_mask = cm_std_string_get_value_int(cmd_items[4+(3*i)+0]);
      int Compare_value = cm_std_string_get_value_int(cmd_items[4+(3*i)+1]);
      std::string Tested_string = cmd_items[4+(3*i)+2];
      std::pair<int,std::string> pp; 
      pp.first  = ((((uint32_t)Type_and_mask) & 0xffff) << 16) | ((((uint32_t)Compare_value) & 0xffff) << 0);
      pp.second = Tested_string;
      StatusList.push_back(pp);
    }
    return true;
  } else {
    cms_ns_if_print("libcmext",1,"CmMuParserHook::CCmWorkerExt_ExtDef_Action_Int2Str::SetMe wrong number of parameters (required 4 or 5, but seted %zu) on line \"%s\"",cmd_items.size(),line.c_str());
    throw "CmExt::ActionInt2Str:err";
    return false;
  }
}

void CmMuParserHook::CCmWorkerExt_ExtDef_Action_Int2Str::GetUsedVariables(C_CM_MUP_Interface *MUP)
{
  std::list<class CCmFilter> MyUsedVarsRqList;
  if (!MUP->GetUsedVariables(EvalString, &MyUsedVarsRqList, true, false, true)) // input
  {
    cms_ns_if_print("libcmext",1, "MUP::ERROR:GetUsedVariables \"%s\" -> \'%s\'",EvalString.c_str(), MUP->GetLastError().c_str());
    throw "CmExt::ActionInt2Str:GetUsedVariables";
  }
  MyUsedVarsRq .add(MyUsedVarsRqList );

  if (!OutputVar.empty())
  {
    std::list<class CCmFilter> MyUsedVarsOutList;

    if (!MUP->GetUsedVariables(OutputVar, &MyUsedVarsOutList, true)) // output
    {
      cms_ns_if_print("libcmext",1, "MUP::ERROR:GetUsedVariables \"%s\" -> \'%s\'",OutputVar.c_str(), MUP->GetLastError().c_str());
      throw "CmExt::ActionInt2Str:GetUsedVariables";
    }
    if (MyUsedVarsOutList.size()>0)
    {
      LastOut = *MyUsedVarsOutList.rbegin();
      LastOutDefined = true;
    }
    MyUsedVarsOut.add(MyUsedVarsOutList);
  }
}

bool CmMuParserHook::CCmWorkerExt_ExtDef_Action_Int2Str::Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, double ActTime)
{
  std::string Out = Eval(MUP, ActTime);
  PreviousStatus=Out;
  if (!OutputVar.empty()) MUP->SetVariable(OutputVar,Out);
  return true;
}
bool CmMuParserHook::CCmWorkerExt_ExtDef_Action_Int2Str::Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, std::string ExtraAssign, double ActTime)
{
  std::string Out = Eval(MUP, ActTime);
  PreviousStatus=Out;
  if (!OutputVar.empty()) MUP->SetVariable(OutputVar,Out);
  MUP->SetVariable(ExtraAssign,Out);
  return true;
}
std::string CmMuParserHook::CCmWorkerExt_ExtDef_Action_Int2Str::PrintMe(void)
{
  std::string RetVal = "Int2Str:MUP:" + EvalString + ":Undef=" + StatusUndefInVal + ":Unknown=" + StatusUnknown;
  if (!OutputVar.empty()) RetVal += ":OutputVar=" + OutputVar;
  for (std::list< std::pair<int,std::string> >::iterator it=StatusList.begin(); it!=StatusList.end(); it++)
  {
    int Type_and_mask = ((it->first >> 16) & 0xffff);
    int Compare_value = ((it->first >>  0) & 0xffff);
    std::string Tested_string = it->second;
    char wstr[200];
    char MaskStr[100];
    MaskStr[0]=0;
    if (Type_and_mask&0xff) sprintf(MaskStr,"Mask=%02x,",Type_and_mask&0xff);
    sprintf(wstr,"%s%sCompare=0x%02x",((Type_and_mask&0x100) ? "Tmp,":""),MaskStr,Compare_value);
    RetVal += ",["+std::string(wstr)+",Str="+Tested_string+"]";
  }
  return RetVal;
}

#endif // USE_LIB_MUPARSERX
