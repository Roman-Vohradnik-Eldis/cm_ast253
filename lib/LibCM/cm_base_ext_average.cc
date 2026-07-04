#if USE_LIB_MUPARSERX

// this average is calculated by time in X axis and value in Y axis. Graph have minimum width in X and maximum in X (if exceed, then cropped).
// output value is calculated from Graph . If input value is not longer time updated, then is still same

#include "cm_ext_muparser_hook.hh"
#include "cm_string.hh"
#include "cm_std_string.hh"
#include "cm_debug.hh"


CmMuParserHook::CCmWorkerExt_ExtDef_Action_Average::CCmWorkerExt_ExtDef_Action_Average()
{
  LastCheckTime = 0.0;
  FirstDataTime = 0.0;
  LastOutputValue = 0.0;
  LastOutputSet   = 0.0;
  LastOutputExist = false;
  DataStorage.clear();
}

void CmMuParserHook::CCmWorkerExt_ExtDef_Action_Average::ResetMe(double ActTime)
{
// debug_printf("ResetMe %s",PrintMe().c_str());
  LastCheckTime = 0.0;
  FirstDataTime = 0.0;
  LastOutputValue = 0.0;
  LastOutputSet   = 0.0;
  LastOutputExist = false;
  DataStorage.clear();
}

bool CmMuParserHook::CCmWorkerExt_ExtDef_Action_Average::SetMe(std::deque<std::string> &cmd_items, std::string line)
{
  if ((cmd_items.size()>=7))
  {
    EvalString = cmd_items[0];
           if (cmd_items[1] == "AVG")
    {
      AvgType = 1;
    } else if (cmd_items[1] == "MAX")
    {
      AvgType = 2;
    } else if (cmd_items[1] == "MIN")
    {
      AvgType = 3;
    } else {
      cms_ns_if_print("libcmext",1,"CmMuParserHook::CCmWorkerExt_ExtDef_Action_Average::SetMe wrong parameter 2:\"%s\" .. known is \"AVG\" \"%s\"",cmd_items[1].c_str(),line.c_str());
      throw "CmExt::ActionAverage:err_params";
      return false;
    }
    AvgMinPeriod            = fabs(cm_std_string_get_value_double(cmd_items[2]));
    AvgStandardPeriod       = fabs(cm_std_string_get_value_double(cmd_items[3]));
    RecalculationIdlePeriod = fabs(cm_std_string_get_value_double(cmd_items[4]));
    ThresholdChange         = fabs(cm_std_string_get_value_double(cmd_items[5]));
    TimeUpdatePrecision     = fabs(cm_std_string_get_value_double(cmd_items[6]));
    if (cmd_items.size()>7) OutputVar = cmd_items[7];
    return true;
  } else {
    cms_ns_if_print("libcmext",1,"CmMuParserHook::CCmWorkerExt_ExtDef_Action_Average::SetMe wrong number of parameters (required 7 or 8, but seted %zu) on line \"%s\"",cmd_items.size(),line.c_str());
    throw "CmExt::ActionAverage:err";
    return false;
  }
}

void CmMuParserHook::CCmWorkerExt_ExtDef_Action_Average::GetUsedVariables(C_CM_MUP_Interface *MUP)
{
  std::list<class CCmFilter> MyUsedVarsRqList;
  if (!MUP->GetUsedVariables(EvalString, &MyUsedVarsRqList, true, false, true)) // input
  {
    cms_ns_if_print("libcmext",1, "MUP::ERROR:GetUsedVariables \"%s\" -> \'%s\'",EvalString.c_str(), MUP->GetLastError().c_str());
    throw "CmExt::ActionAverage:GetUsedVariables";
  }
  MyUsedVarsRq .add(MyUsedVarsRqList );

  if (!OutputVar.empty())
  {
    std::list<class CCmFilter> MyUsedVarsOutList;

    if (!MUP->GetUsedVariables(OutputVar, &MyUsedVarsOutList, true)) // output
    {
      cms_ns_if_print("libcmext",1, "MUP::ERROR:GetUsedVariables \"%s\" -> \'%s\'",OutputVar.c_str(), MUP->GetLastError().c_str());
      throw "CmExt::ActionAverage:GetUsedVariables";
    }
    if (MyUsedVarsOutList.size()>0)
    {
      LastOut = *MyUsedVarsOutList.rbegin();
      LastOutDefined = true;
    }
    MyUsedVarsOut.add(MyUsedVarsOutList);
  }
}

void CmMuParserHook::CCmWorkerExt_ExtDef_Action_Average::UpdateCumul(double &cumul_val, double &cumul_dur, double calc_val, double calc_dur, bool first_write)
{
  if (AvgType==1) // AVG
  {
    cumul_val += calc_val * calc_dur;
    cumul_dur += calc_dur;
  }
  if (AvgType==2) // MAX
  {
    if (first_write) cumul_val = calc_val; else if (cumul_val < calc_val) cumul_val = calc_val;
  }
  if (AvgType==3) // MIN
  {
    if (first_write) cumul_val = calc_val; else if (cumul_val > calc_val) cumul_val = calc_val;
  }
}

void CmMuParserHook::CCmWorkerExt_ExtDef_Action_Average::Eval(C_CM_MUP_Interface *MUP, double ActTime, bool &OutputExist, double &OutputValue)
{
  OutputExist = false;
  OutputValue = 0.0;
  double MyValue;
  if (MUP != NULL)
  {
    if (!MUP->Evaluate(EvalString, MyValue)) return ; // if not exist, then unassign new data to calc fields
    if (DataStorage.empty())
    {
      DataStorage[ActTime] = MyValue;
    } else {
      //      std::map<double, double>::reverse_iterator rit = DataStorage.rbegin();
      // if LAST value is different ... store
      if (DataStorage.rbegin()->second != MyValue) DataStorage[ActTime] = MyValue;
    }


    if (FirstDataTime == 0.0) FirstDataTime = ActTime;
    // strip old data
    double time_thr = ActTime - AvgStandardPeriod;
    std::map<double, double>::iterator it_prev = DataStorage.begin();
    for (std::map<double, double>::iterator it = DataStorage.begin(); it != DataStorage.end(); it++) // first is time, second is value
    {
      if ((it->first < time_thr) && (it_prev != it)) DataStorage.erase(it_prev);
      it_prev = it;
    }
  }
  if ((FirstDataTime != 0.0) && (FirstDataTime <= (ActTime - AvgMinPeriod)))
  {
    double cumul_dur = 0.0;
    double cumul_val = 0.0;
    bool first_write = true;

    double time_thr = ActTime - AvgStandardPeriod;
    std::map<double, double>::iterator it_prev = DataStorage.begin();
    for (std::map<double, double>::iterator it = DataStorage.begin(); it != DataStorage.end(); ) // first is time, second is value
    {
      if ((it->first >= time_thr)&&(it_prev != it))
      {
        double calc_val = it_prev->second;
        double calc_dur = it->first - time_thr;
        UpdateCumul(cumul_val, cumul_dur, calc_val, calc_dur, first_write);
        first_write = false;
        time_thr = it->first;
      }
      it_prev = it;
      it++;
      if (it == DataStorage.end())
      {
        double calc_val = it_prev->second;
        double calc_dur = ActTime - time_thr;
        UpdateCumul(cumul_val, cumul_dur, calc_val, calc_dur, first_write);
        first_write = false;
      }
    }
    if (!first_write)
    {
      OutputExist = true;
      if (AvgType==1) // AVG
      {
        if (cumul_dur>0.0)
        {
          OutputValue = cumul_val / cumul_dur;
        } else OutputExist = false;
      }
      if (AvgType==2) // MAX
      {
        OutputValue = cumul_val;
      }
      if (AvgType==3) // MIN
      {
        OutputValue = cumul_val;
      }
    }
  }
}

bool CmMuParserHook::CCmWorkerExt_ExtDef_Action_Average::Polling_CheckNow(double ActTime)
{  // is dramatic change =- requiring update - FullEval ???
  LastCheckTime = ActTime;
  bool OutputExist = false;
  double OutputValue = 0.0;
  Eval(NULL, ActTime, OutputExist, OutputValue);
  if (OutputExist != LastOutputExist) return true;
  if (OutputExist)
  {
    if (fabs(OutputValue - LastOutputValue) > ThresholdChange) return true;
    if (LastOutputSet==0.0) return true;
    if ((OutputValue != LastOutputValue) && ((LastOutputSet + TimeUpdatePrecision) < ActTime)) return true;
  }
  return false;
}

bool CmMuParserHook::CCmWorkerExt_ExtDef_Action_Average::Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, double ActTime)
{
  LastCheckTime = ActTime;
  bool OutputExist = false;
  double OutputValue = 0.0;
  Eval(MUP, ActTime, OutputExist, OutputValue);
  if (OutputExist)
  {
    LastOutputValue = OutputValue;
    if (!OutputVar.empty()) MUP->SetVariable(OutputVar,OutputValue);
    LastOutputSet = ActTime;
  }
  LastOutputExist = OutputExist;
  return OutputExist;
}

bool CmMuParserHook::CCmWorkerExt_ExtDef_Action_Average::Evaluate(std::string &DebugPrefix, C_CM_MUP_Interface *MUP, std::string ExtraAssign, double ActTime)
{
  LastCheckTime = ActTime;
  bool OutputExist = false;
  double OutputValue = 0.0;
  Eval(MUP, ActTime, OutputExist, OutputValue);
  if (OutputExist)
  {
    LastOutputValue = OutputValue;
    if (!OutputVar.empty()) MUP->SetVariable(OutputVar,OutputValue);
    MUP->SetVariable(ExtraAssign,OutputValue);
    LastOutputSet = ActTime;
  }
  LastOutputExist = OutputExist;
  return OutputExist;
}

double CmMuParserHook::CCmWorkerExt_ExtDef_Action_Average::Polling_GetNextTime(double ActTime)
{
  double RetVal = LastCheckTime + RecalculationIdlePeriod;
  if (RetVal < ActTime) RetVal = ActTime;
  return RetVal;
}

std::string CmMuParserHook::CCmWorkerExt_ExtDef_Action_Average::PrintMe(void)
{
  std::string RetVal = "Average:MUP:" + EvalString
      + ":AvgMinPeriod=" + cm_std_string_print_short_float(AvgMinPeriod)
      + ":AvgStandardPeriod=" + cm_std_string_print_short_float(AvgStandardPeriod)
      + ":RecalculationIdlePeriod=" + cm_std_string_print_short_float(RecalculationIdlePeriod)
      + ":ThresholdChange=" + cm_std_string_print_short_float(ThresholdChange);
      + ":TimeUpdatePrecision=" + cm_std_string_print_short_float(TimeUpdatePrecision);
  if (!OutputVar.empty()) RetVal += ":OutputVar=" + OutputVar;
  return RetVal;
}

#endif // USE_LIB_MUPARSERX
