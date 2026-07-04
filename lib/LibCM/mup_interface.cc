#if USE_LIB_MUPARSERX

#include "cm_header_internal.hh"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <typeinfo>
#include <sys/time.h>
#include <unistd.h>

#include <mpParser.h>
#include <mpDefines.h>
#include <mpTest.h>

// Tohle je Hack, protoze muparser predefinuje nasledujici makra, to
// nechapu, jak je napadlo delat to takhle.
#undef override
#undef unique_ptr
#undef nullptr

#include "cm_base_varevent.hh"
#include "cm_thread.hh"
#include "cm_base_filter.hh"
#include "cm_types.hh"

#include "mup_interface.hh"
#include "cm_header.hh"
#include "cm_base_functs.hh"

#define DEFAULT_USE_GLOBAL_LOCK true

struct LibCM_GLOBAL_Data_Type
{
    CMSMutex MUPMutex;
    CMSMutex Mutex;
};

LibCM_GLOBAL_Data_Type LibCM_GLOBAL_Data;

//--- other includes --------------------------------------------------------


// LibCM
// LibCMparse
// CM : CM_core (LibCM, LibCMparse)
//      CM_gui (LibCM, LibCMparse)
//      CM_automat (LibCM, LibCMparse)
//      CM_example (LibCM)


// toto je do LibCMparse
// Potrebuju funkce:  Funkce opatrit Mutexy() a pri 
//                    pro GUI zavedeme promennou UserLevel a pak pro prvky (UserLevel>5)
//                    pak moznost funkce evaluate ... s moznosti, ze stale udrzuji kontext a pouzivam nejaky zamek....
//                    pozor: case sensitive a nesnasi "-" a "." , takze TxG-A.PRF =>> TxG_A_PRF
//                    u GetUsedVariables - listu secteme seznamy, pak jim upravime VYMAZ *_exist a setridime a uniq pro knihovnu
//                        ale jinak vedeme extra i *_exist (AffectedWidgets....
//          


#ifndef MIN
#define MIN(a,b) ((a<b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a<b)?(b):(a))
#endif


bool CMUP__DefinedGlobalFunctions = false;
std::string CMUP__GlobalLastErrorStr;
std::list<mup::ptr_cal_type> CMUP__UserDefinedGlobalFunctions;
std::list<class mup::ParserX *> CMUP__UsedParserX;

std::string CMUP_GlobalGetLastError(void)
{
  std::string RetVal;
  LibCM_GLOBAL_Data.MUPMutex.Lock();
  RetVal = CMUP__GlobalLastErrorStr;
  CMUP__GlobalLastErrorStr.clear();
  LibCM_GLOBAL_Data.MUPMutex.Unlock();
  return RetVal;
}

void CMUP__DefineGlobalFun(const mup::ptr_cal_type &ptr)
{
//  LibCM_GLOBAL_Data.MUPMutex.Lock();
  LibCM_GLOBAL_Data.MUPMutex.Lock();
  std::string id = ptr->GetIdent();
  for (std::list<mup::ptr_cal_type>::iterator it = CMUP__UserDefinedGlobalFunctions.begin(); it != CMUP__UserDefinedGlobalFunctions.end(); )
  {
    std::list<mup::ptr_cal_type>::iterator itD = it++;
    if (((*itD)->GetIdent()) == id) // Undefine and erase previous
    {
      for (std::list<class mup::ParserX *>::iterator itP = CMUP__UsedParserX.begin() ; itP != CMUP__UsedParserX.end() ; itP++)
      {
        (*itP)->RemoveFun(id);
      }
      CMUP__UserDefinedGlobalFunctions.erase(itD);
    }
  }
  for (std::list<class mup::ParserX *>::iterator it = CMUP__UsedParserX.begin() ; it != CMUP__UsedParserX.end() ; it++)
  {
    (*it)->DefineFun(ptr);
  }
  CMUP__UserDefinedGlobalFunctions.push_back(ptr);
  LibCM_GLOBAL_Data.MUPMutex.Unlock();
}

void CMUP__AddUsedParserX(class mup::ParserX *ParserPtr)
{
  LibCM_GLOBAL_Data.MUPMutex.Lock();
  CMUP__UsedParserX.push_back(ParserPtr);
  for (std::list<mup::ptr_cal_type>::iterator it = CMUP__UserDefinedGlobalFunctions.begin(); it != CMUP__UserDefinedGlobalFunctions.end(); it++)
  {
    ParserPtr->DefineFun(*it);
  }
  LibCM_GLOBAL_Data.MUPMutex.Unlock();
}

void CMUP__RemoveUsedParserX(class mup::ParserX *ParserPtr)
{
  LibCM_GLOBAL_Data.MUPMutex.Lock();
  for (std::list<class mup::ParserX *>::iterator it = CMUP__UsedParserX.begin() ; it != CMUP__UsedParserX.end() ;)
  {
    std::list<class mup::ParserX *>::iterator itD = it++;
    if ((*itD) == ParserPtr) CMUP__UsedParserX.erase(itD);
  }
  LibCM_GLOBAL_Data.MUPMutex.Unlock();
}




class CMUP__FunGeneric : public mup::ICallback
{
private:
  mup::string_type FunGeneric_sIdent;
  mup::string_type FunGeneric_Desc;
public:

  CMUP__FunGeneric(mup::string_type sIdent, mup::string_type sFunction) 
    :mup::ICallback(mup::cmFUNC, sIdent.c_str()) 
    ,m_parser()
    ,m_vars()
    ,m_val()
  {
    FunGeneric_sIdent = sIdent;
    savedFunction = sFunction;
    FunGeneric_Desc = FunGeneric_sIdent + "(...) - Dynamically defined function \"" + sFunction + "\".";
    LibCM_GLOBAL_Data.MUPMutex.Lock();
    for (std::list<mup::ptr_cal_type>::iterator it = CMUP__UserDefinedGlobalFunctions.begin(); it != CMUP__UserDefinedGlobalFunctions.end(); it++)
    {
      m_parser.DefineFun(*it);
    }
    LibCM_GLOBAL_Data.MUPMutex.Unlock();

    try
    {
      m_parser.SetExpr(sFunction);
      m_vars = m_parser.GetExprVar();
      SetArgc(m_vars.size());
    }
    catch(mup::ParserError &e)
    {
      char *wstr = NULL;
      asprintf(&wstr,"MUPerrorFunc:code=%d,pos=%d,%s .......... FUNCNAME=\"%s\", FUNCDEF=\"%s\"",e.GetCode(),e.GetPos(),e.GetMsg().c_str(), sIdent.c_str(), sFunction.c_str());
      if (wstr != NULL)
      {
        LibCM_GLOBAL_Data.MUPMutex.Lock();
        CMUP__GlobalLastErrorStr = std::string(wstr);
        LibCM_GLOBAL_Data.MUPMutex.Unlock();
        free(wstr);
      }
      printf("EXCEPTION\n");
      throw;
    }

    // Create values for the undefined variables and bind them
    // to the variables
    mup::var_maptype::const_iterator item = m_vars.begin();
    mup::string_type ParamsList;
    for (; item!=m_vars.end(); ++item)
    {
      mup::ptr_val_type val(new mup::Value());
      m_val.push_back(val);
      m_names.push_back(item->second->GetIdent());
      ParamsList += (ParamsList.empty()? "" : ",") + item->second->GetIdent();
      // assign a parser variable
      m_parser.DefineVar(item->second->GetIdent(), mup::Variable(val.Get()));
    }
    FunGeneric_Desc = FunGeneric_sIdent + "("+ParamsList+") - Dynamically defined function \"" + sFunction + "\".";
  }

  virtual ~CMUP__FunGeneric()
  {}

  virtual void Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc)
  {
    for (std::size_t i=0; i<(std::size_t)a_iArgc; ++i)
    {
      if (a_pArg[i]->GetType()=='v')
      {
        m_parser.RemoveVar(m_names[i]);
        mup::ptr_val_type val(new mup::Value()); // nevim, zda alokace nebude rostoucin !!!!!!!!!!!!!!! MEMORYLEAK?????????????????
        m_val[i]=val;
        m_parser.DefineVar(m_names[i], mup::Variable(val.Get()));
      } else {
      *m_val[i] = *a_pArg[i];
      }
    }
    *ret = m_parser.Eval();
  }

  virtual const mup::char_type* GetDesc() const
  {
    return _T(FunGeneric_Desc.c_str());
  }

  virtual mup::IToken* Clone() const
  {
    return new CMUP__FunGeneric(*this);
  }

private:

  mup::ParserX m_parser;
  mup::var_maptype m_vars;
  mup::val_vec_type m_val;
  std::vector<std::string> m_names;
  mup::string_type savedFunction;
}; // class CMUP__FunGeneric


class CMUP__FunDefine : public mup::ICallback
{
public:
  CMUP__FunDefine() : ICallback(mup::cmFUNC, _T("define"), 2)
  {}

  virtual void Eval(mup::ptr_val_type &ret, const mup::ptr_val_type *a_pArg, int a_iArgc)
  {
    if (a_iArgc<2)
    {
      char *wstr = NULL;
      asprintf(&wstr,"MUPerrorFunc:define() invalid number of parameters. Required 2, but get %d",a_iArgc);
      if (wstr != NULL)
      {
        LibCM_GLOBAL_Data.MUPMutex.Lock();
        CMUP__GlobalLastErrorStr = std::string(wstr);
        LibCM_GLOBAL_Data.MUPMutex.Unlock();
        free(wstr);
      }
      printf("EXCEPTION\n");

      throw;
    } else {
      mup::string_type sFun = a_pArg[0]->GetString();
      mup::string_type sDef = a_pArg[1]->GetString();
//      mup::ParserXBase &parser = *GetParent();
      CMUP__DefineGlobalFun(new CMUP__FunGeneric(sFun, sDef));
    }
    *ret = (mup::int_type)0;
  }

  virtual const mup::char_type* GetDesc() const
  {
    return _T("define(Function, Definition) - Define a new parser function.");
  }

  virtual mup::IToken* Clone() const
  {
    return new CMUP__FunDefine(*this);
  }
}; // class CMUP__FunDefine


//---------------------------------------------------------------------------
//      string_type sFun = a_pArg[0]->GetString();
//      string_type sDef = a_pArg[1]->GetString();

//      ParserXBase &parser = *GetParent();
//      DefineGlobalFun(parser, new CMUP__FunGeneric(sFun, sDef));


void CMUP__DefineFunctionGeneric(std::string FunName, std::string FunDef)
{
  CMUP__DefineGlobalFun(new CMUP__FunGeneric(FunName, FunDef));
}

void CMUP__DefineFunctionDefine(void)
{
  CMUP__DefineGlobalFun(new CMUP__FunDefine());
}

struct C_CM_MUP_Interface::Impl
{
    CMSMutex *Mutex;
    mup::ParserX  *parser_var;
    mup::ParserX  *parser_var_tester;
    std::map<std::string, mup::Value> VarMap;
};

C_CM_MUP_Interface::C_CM_MUP_Interface()
{
  // LibCM_GLOBAL_TryInit();
  if (!CMUP__DefinedGlobalFunctions)
  {
    CMUP__DefineFunctionDefine();
    CMUP__DefinedGlobalFunctions = true;
  }
  impl = new Impl;
  impl->Mutex = new CMSMutex();
  EnableAutoCreateVar = false;
  impl->parser_var        = new mup::ParserX();
  impl->parser_var       ->EnableAutoCreateVar(EnableAutoCreateVar);
//  parser_var       ->EnableDebugDump(0,0);
  CMUP__AddUsedParserX(impl->parser_var);
  impl->parser_var_tester = new mup::ParserX();
  impl->parser_var_tester->EnableAutoCreateVar(EnableAutoCreateVar);
  impl->parser_var_tester->EnableDebugDump(0,0);
  CMUP__AddUsedParserX(impl->parser_var_tester);
  UseGlobalLock = DEFAULT_USE_GLOBAL_LOCK;
  // LibCM_GLOBAL_AddMyMUP(this);

//  if (!UseGlobalLock) CMUP__GlobalMutex.Unlock();
//                 else Mutex->Unlock();
}
C_CM_MUP_Interface::~C_CM_MUP_Interface()
{
  // LibCM_GLOBAL_RemoveMyMUP(this);
  CMUP__RemoveUsedParserX(impl->parser_var);
  delete impl->parser_var;
  CMUP__RemoveUsedParserX(impl->parser_var_tester);
  delete impl->parser_var_tester;
//  Unlock();
//  Mutex->Unlock();

  delete impl->Mutex;
  delete impl;
}


void C_CM_MUP_Interface::LockMe(void)
{
  if (UseGlobalLock) LibCM_GLOBAL_Data.MUPMutex.Lock();
  else impl->Mutex->Lock();
}

void C_CM_MUP_Interface::UnlockMe(void)
{
  if (UseGlobalLock) LibCM_GLOBAL_Data.MUPMutex.Unlock();
  else impl->Mutex->Unlock();
}

void C_CM_MUP_Interface::SetUseGlobalLock(bool newVal)
{
  if (UseGlobalLock != newVal)
  {
    LockMe();
    UnlockMe();
    UseGlobalLock = newVal;
  }
}


void C_CM_MUP_Interface::DefineFunctionGeneric(std::string FunName, std::string FunDef)
{
  CMUP__DefineGlobalFun(new CMUP__FunGeneric(FunName, FunDef));
}

std::string C_CM_MUP_Interface::GetLastError(void)
{
  LockMe();
  std::string RetVal = LastErrorStr;
  LastErrorStr.clear();
  UnlockMe();
  return RetVal;
}

void C_CM_MUP_Interface::SetEnableAutoCreateVar(bool Enable)
{
  LockMe();
  EnableAutoCreateVar = Enable;
  impl->parser_var       ->EnableAutoCreateVar(EnableAutoCreateVar);
  impl->parser_var_tester->EnableAutoCreateVar(EnableAutoCreateVar);
  UnlockMe();
}

bool C_CM_MUP_Interface::GetUsedVariables(std::string func, std::list<std::string> *UsedVariables_S, std::list<class CCmFilter> *UsedVariables_F, bool with_idx, bool output_vars, bool input_vars, bool with_debug)
{
//    std::deque< std::deque< uint16_t > > indexes;
  CCmFilter WFilter;

  LockMe();
  if (with_debug)
  {
   debug_printf("-------------- debug INPUT:");
   debug_printf("%s",func.c_str());
   debug_printf("-------------- ");
  }
  impl->parser_var_tester       ->EnableAutoCreateVar(true);
  try
  {
    impl->parser_var_tester->SetExpr(func);
//    mup::Value val = parser_var_tester->Eval();
  }
  catch(mup::ParserError &e)
  {
    if (!((e.GetCode()==40) || (e.GetCode()==44) || (e.GetCode()==19)))
    {
      char *wstr = NULL;
      asprintf(&wstr,"MUPerrorA:code=%d,pos=%d,%s .......... FUNC=\"%s\"\n",e.GetCode(),e.GetPos(),e.GetMsg().c_str(), func.c_str());
      if (wstr != NULL)
      {
        LastErrorStr = std::string(wstr);
        free(wstr);
      }
      UnlockMe();
      return false;
    }
  }


    if (with_debug)debug_printf("-------------- debug IDX:");
    mup::string_type str;
    try
    {
      str = impl->parser_var_tester->GetExprVarWithIDX();
// GetExprVarWithIDX:
//    Variable  : A=StrPosition; V=VarName;
//    Value     : A=StrPosition; I=Value(int);
//    IdxClose  : A=StrPosition; X=NumArguments(dimensions);
//    Function  : A=StrPosition; G=NumArgsuments, F=FunctionName
//    Assignment: Q=StrPosition;    { At end ... if is present a assignment like : = += -= *= /= }
    }
    catch(mup::ParserError &e)
    {
      if (!((e.GetCode()==40) || (e.GetCode()==44) || (e.GetCode()==19)))
      {
        char *wstr = NULL;
        asprintf(&wstr,"MUPerrorB:code=%d,pos=%d,%s .......... FUNC=\"%s\"\n",e.GetCode(),e.GetPos(),e.GetMsg().c_str(), func.c_str());
        if (wstr != NULL)
        {
          LastErrorStr = std::string(wstr);
          free(wstr);
        }
        UnlockMe();
        return false;
      }
    }

    char **item = cm_string_strsplit_outside_string(str.c_str(), ";", 0);
    std::string lastvar = "";
    std::string lastvar_F = "";
    int EqualDelimiterPos = -1;
    int lastvarpos = -1;
    int lastpos = -1;
    int lastfunnumargs = 0;
    std::deque< std::deque< uint16_t > > indexes;

#define MAX_IDX_ARR 10
    gint idx_arr[MAX_IDX_ARR];
    gint idx_arr_ptr = 0;
    if ((item) && (item[0]))
    {
      int i=0;
      if ((!output_vars) || (!input_vars))
      {
        while (item[i])
        {
          gchar * i_inside = cm_string_interpret_first_string(item[i]);
          if (i_inside)
          {
            if (!strncmp(item[i],"Q=",2)) // EqualDelimiter  : = / += / -= / *= / /=
            {
              int Val = cm_string_get_value_int(i_inside);
              if (EqualDelimiterPos == -1) EqualDelimiterPos = Val;
              else if (EqualDelimiterPos > Val) EqualDelimiterPos = Val;
            }
            g_free(i_inside);
          }
          i++;
        }
      }
      if ((EqualDelimiterPos != -1) && (with_debug))
      {
       debug_printf ("!!!!!: EqualDelimiterPos=%d\n", EqualDelimiterPos);
      }
      i = 0;
      while (item[i])
      {
        gchar * i_inside = cm_string_interpret_first_string(item[i]);
        if (i_inside)
        {
          if (!strncmp(item[i],"V=",2)) // variable
          {
            if (with_debug)debug_printf("item(Var)=%s",i_inside);
            if ((lastvar != "")||(lastvar_F != ""))
            {
              bool store = false;
              if (EqualDelimiterPos == -1)
              {
                if (input_vars) store = true;
              } else {
                if (( input_vars) && (EqualDelimiterPos < lastvarpos)) store = true;
                if ((output_vars) && (EqualDelimiterPos > lastvarpos)) store = true;
              }
              if (store)
              {
                if ((UsedVariables_S != NULL) && (lastvar != ""))
                {
                  bool found = false;
                  for (std::list<std::string>::iterator it=UsedVariables_S->begin(); it!=UsedVariables_S->end(); it++) if ((*it)==lastvar) found = true;
                  if (!found) UsedVariables_S->push_back(lastvar);
                }
                if ((UsedVariables_F != NULL) && (lastvar_F != ""))
                {
                  bool found = false;
                  WFilter.setName(lastvar_F);
                  for (std::list<class CCmFilter>::iterator it=UsedVariables_F->begin(); it!=UsedVariables_F->end(); it++) if ((*it)==WFilter) found = true;
                  if (!found) UsedVariables_F->push_back(WFilter);
                  WFilter.index_array.clear();
                }
              } else {
                WFilter.index_array.clear();
              }
              if (with_debug)debug_printf("  ==>> %s STORE=%s",lastvar.c_str(), (store?"YES":"no"));
            }
            idx_arr_ptr = 0;
            lastvar   = std::string(i_inside);
            lastvar_F = std::string(i_inside);
            lastvarpos = lastpos;
          } else if (!strncmp(item[i],"X=",2)) // index_close
          {
            if (with_debug)debug_printf("item(IdxCl)=%s",i_inside);
            if (idx_arr_ptr == cm_string_get_value_int(i_inside))
            {
              if ((lastvar != "")||(lastvar_F != ""))
              {
                if (with_idx)
                {
                  std::list<uint16_t> idx_list;
                  idx_list.clear();
                  char WStr[1000];
                  WStr[0] = 0;
                  for (gint x=0; x<idx_arr_ptr; x++)
                  {
                    idx_list.push_back((uint16_t)idx_arr[x]);
                    sprintf(WStr+strlen(WStr),"%s%d",((x==0)?"":","),idx_arr[x]);
                  }
                  if (lastvar_F != "") WFilter.index_array.push_back(idx_list);
                  if (lastvar   != "") lastvar += "[" + std::string(WStr) + "]";
                }
              }
            }
            idx_arr_ptr = 0;
          } else if (!strncmp(item[i],"F=",2)) // fun
          {
            if (with_debug)debug_printf("item(fun)=%s",i_inside);
            if (with_debug)debug_printf("item(funArgs)=%d,%d",lastfunnumargs,idx_arr_ptr);
            if ((idx_arr_ptr == (lastfunnumargs-1)) && (!strcmp(i_inside,"exist_idx")))
            {
              if ((lastvar != "")||(lastvar_F != ""))
              {
                if (with_idx)
                {
                  std::list<uint16_t> idx_list;
                  idx_list.clear();
                  char WStr[1000];
                  WStr[0] = 0;
                  for (gint x=0; x<idx_arr_ptr; x++)
                  {
                    idx_list.push_back((uint16_t)idx_arr[x]);
                    sprintf(WStr+strlen(WStr),"%s%d",((x==0)?"":","),idx_arr[x]);
                  }
                  if (lastvar_F != "") WFilter.index_array.push_back(idx_list);
                  if (lastvar   != "") lastvar += "[" + std::string(WStr) + "]";
                }
              }
            }
            idx_arr_ptr = 0;
          } else if (!strncmp(item[i],"I=",2)) // index
          {
            if (with_debug)debug_printf("item(idx)=%s",i_inside);
            if (idx_arr_ptr < MAX_IDX_ARR)
            {
              idx_arr[idx_arr_ptr] = cm_string_get_value_int(i_inside);
              idx_arr_ptr++;
            }
          } else if (!strncmp(item[i],"G=",2)) // pos
          {
            if (with_debug)debug_printf("item(funArgs)=%s",i_inside);
            lastfunnumargs = cm_string_get_value_int(i_inside);
          } else if (!strncmp(item[i],"A=",2)) // pos
          {
            if (with_debug)debug_printf("item(pos)=%s",i_inside);
            lastpos = cm_string_get_value_int(i_inside);
          }
          g_free(i_inside);
        }
        i++;
      }
      g_strfreev(item);
    }

    if ((lastvar != "")||(lastvar_F != ""))
    {
      bool store = false;
      if (EqualDelimiterPos == -1)
      {
        if (input_vars) store = true;
      } else {
        if (( input_vars) && (EqualDelimiterPos < lastvarpos)) store = true;
        if ((output_vars) && (EqualDelimiterPos > lastvarpos)) store = true;
      }
      if (store)
      {
        if ((UsedVariables_S != NULL) && (lastvar != ""))
        {
          bool found = false;
          for (std::list<std::string>::iterator it=UsedVariables_S->begin(); it!=UsedVariables_S->end(); it++) if ((*it)==lastvar) found = true;
          if (!found) UsedVariables_S->push_back(lastvar);
        }
        if ((UsedVariables_F != NULL) && (lastvar_F != ""))
        {
          bool found = false;
          WFilter.setName(lastvar_F);
          for (std::list<class CCmFilter>::iterator it=UsedVariables_F->begin(); it!=UsedVariables_F->end(); it++) if ((*it)==WFilter) found = true;
          if (!found) UsedVariables_F->push_back(WFilter);
          WFilter.index_array.clear();
        }
      }
      if (with_debug)debug_printf("  ==>> %s STORE=%s",lastvar.c_str(), (store?"YES":"no"));
    }
    if (with_debug)debug_printf("RET:%s",str.c_str());
    if (with_debug)debug_printf("-------------- debug IDX END:");



  if (with_debug)
  {
   debug_printf("-------------- debug RPN:");
    impl->parser_var_tester->DumpRPN();
  }
  if (with_debug)
  {
   debug_printf("-------------- END debug ");
  }
  impl->parser_var_tester->ClearVar();
  impl->parser_var_tester       ->EnableAutoCreateVar(EnableAutoCreateVar);
  UnlockMe();

  return true;
}

bool C_CM_MUP_Interface::IsVarDefined(const std::string &name)
{
  bool RetVal = false;
  LockMe();
  if (impl->parser_var != NULL) RetVal = impl->parser_var->IsVarDefined(name);
  UnlockMe();
  return RetVal;
}

void C_CM_MUP_Interface::SetVariableUnlocked(std::string name, bool value)
{
  if (name.empty()) return;
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=((mup::bool_type)value);
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
}

void C_CM_MUP_Interface::SetVariable(std::string name, bool value)
{
  LockMe();
  SetVariableUnlocked(name, value);
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, int value)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=((mup::int_type)value);
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, libcm_integer value)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=((mup::int_type)((int64_t)value));
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, double value)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=((mup::float_type)value);
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, long double value)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=((mup::float_type)value);
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, float value)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=((mup::float_type)value);
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, std::string value)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }

  impl->VarMap[name]=((mup::string_type)value);
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, bool *values, int dimensionY)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY, 0);
  for (int y=0;y<dimensionY; y++) impl->VarMap[name].At(y) = (mup::bool_type)values[y];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, std::deque<bool> &values, int dimensionY)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY, 0);
  for (int y=0;y<dimensionY; y++) impl->VarMap[name].At(y) = (mup::bool_type)values[y];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, int *values, int dimensionY)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY, 0);
  for (int y=0;y<dimensionY; y++) impl->VarMap[name].At(y) = (mup::int_type)values[y];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, libcm_integer *values, int dimensionY)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY, 0);
  for (int y=0;y<dimensionY; y++) impl->VarMap[name].At(y) = (mup::int_type)((int64_t)values[y]);
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, std::deque<int> &values, int dimensionY)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY, 0);
  for (int y=0;y<dimensionY; y++) impl->VarMap[name].At(y) = (mup::int_type)values[y];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, UIntList const &values, int dimensionY)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY, 0);
  for (int y=0;y<dimensionY; y++) impl->VarMap[name].At(y) = (mup::int_type)((int64_t)values[y]);
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, double *values, int dimensionY)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY, 0);
  for (int y=0;y<dimensionY; y++) impl->VarMap[name].At(y) = (mup::float_type)values[y];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, long double *values, int dimensionY)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY, 0);
  for (int y=0;y<dimensionY; y++) impl->VarMap[name].At(y) = (mup::float_type)values[y];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, DoubleList const &values, int dimensionY)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY, 0);
  for (int y=0;y<dimensionY; y++) impl->VarMap[name].At(y) = (mup::float_type)values[y];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, std::deque<long double> &values, int dimensionY)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY, 0);
  for (int y=0;y<dimensionY; y++) impl->VarMap[name].At(y) = (mup::float_type)values[y];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, float *values, int dimensionY)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//  impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY, 0);
  for (int y=0;y<dimensionY; y++) impl->VarMap[name].At(y) = (mup::float_type)values[y];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, std::deque<float> &values, int dimensionY)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY, 0);
  for (int y=0;y<dimensionY; y++) impl->VarMap[name].At(y) = (mup::float_type)values[y];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}


void C_CM_MUP_Interface::SetVariable(std::string name, std::string *values, int dimensionY)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY, 0);
  for (int y=0;y<dimensionY; y++) impl->VarMap[name].At(y) = (mup::string_type)values[y];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, StringList const &values, int dimensionY)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY, 0);
  for (int y=0;y<dimensionY; y++) impl->VarMap[name].At(y) = (mup::string_type)values[y];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, bool *values, int dimensionY, int dimensionX)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY,dimensionX, 0);
  for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) impl->VarMap[name].At(y, x) = (mup::bool_type)values[(y * dimensionX) + x];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, std::deque<bool> &values, int dimensionY, int dimensionX)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY,dimensionX, 0);
  for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) impl->VarMap[name].At(y, x) = (mup::bool_type)values[(y * dimensionX) + x];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, int *values, int dimensionY, int dimensionX)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY,dimensionX, 0);
  for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) impl->VarMap[name].At(y, x) = (mup::int_type)values[(y * dimensionX) + x];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}


void C_CM_MUP_Interface::SetVariable(std::string name, libcm_integer *values, int dimensionY, int dimensionX)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY,dimensionX, 0);
  for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) impl->VarMap[name].At(y, x) = (mup::int_type)((int64_t)values[(y * dimensionX) + x]);
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}


void C_CM_MUP_Interface::SetVariable(std::string name, std::deque<int> &values, int dimensionY, int dimensionX)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//  impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY,dimensionX, 0);
  for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) impl->VarMap[name].At(y, x) = (mup::int_type)values[(y * dimensionX) + x];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, UIntList const &values, int dimensionY, int dimensionX)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY,dimensionX, 0);
  for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) impl->VarMap[name].At(y, x) = (mup::int_type)((int64_t)values[(y * dimensionX) + x]);
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, double *values, int dimensionY, int dimensionX)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY,dimensionX, 0);
  for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) impl->VarMap[name].At(y, x) = (mup::float_type)values[(y * dimensionX) + x];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, long double *values, int dimensionY, int dimensionX)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY,dimensionX, 0);
  for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) impl->VarMap[name].At(y, x) = (mup::float_type)values[(y * dimensionX) + x];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, DoubleList const &values, int dimensionY, int dimensionX)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY,dimensionX, 0);
  for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) impl->VarMap[name].At(y, x) = (mup::float_type)values[(y * dimensionX) + x];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, std::deque<long double> &values, int dimensionY, int dimensionX)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY,dimensionX, 0);
  for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) impl->VarMap[name].At(y, x) = (mup::float_type)values[(y * dimensionX) + x];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, float *values, int dimensionY, int dimensionX)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY,dimensionX, 0);
  for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) impl->VarMap[name].At(y, x) = (mup::float_type)values[(y * dimensionX) + x];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, std::deque<float> &values, int dimensionY, int dimensionX)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY,dimensionX, 0);
  for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) impl->VarMap[name].At(y, x) = (mup::float_type)values[(y * dimensionX) + x];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, std::string *values, int dimensionY, int dimensionX)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY,dimensionX, 0);
  for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) impl->VarMap[name].At(y, x) = (mup::string_type)values[(y * dimensionX) + x];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}

void C_CM_MUP_Interface::SetVariable(std::string name, StringList const &values, int dimensionY, int dimensionX)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  impl->VarMap[name]=mup::Value(dimensionY,dimensionX, 0);
  for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) impl->VarMap[name].At(y, x) = (mup::string_type)values[(y * dimensionX) + x];
  impl->parser_var->DefineVar(name,mup::Variable(&impl->VarMap[name]));
  UnlockMe();
}



void C_CM_MUP_Interface::SetVariable(CCmVariable const *var)
{
  std::string var_name = var->getName();
  if (var_name.empty()) return;
  char var_type = var->getType();
  unsigned var_array_y = var->getArrayY();
  unsigned var_array_x = var->getArrayX();

  if ((var_type == 'i' || (var_type == 'e')))
    {
      if (var_array_y > 1)
      {
        if (var_array_x > 1)
        {
            SetVariable(var_name, var->getUIntList(), var_array_y, var_array_x);
        } else {
            SetVariable(var_name, var->getUIntList(), var_array_y);
        }
      } else {
        if (var_type == 'e')
        {
          int64_t I;
          if (var->getVariableEnum(I))
	      SetVariable(var_name, (int)I);
        } else {
          libcm_integer I;
          if (var->getVariableInt(I))
	      SetVariable(var_name, I);
        }
      }
    }
    if (var_type == 'f')
    {
      if (var_array_y > 1)
      {
        if (var_array_x > 1)
        {
            SetVariable(var_name, var->getDoubleList(), var_array_y, var_array_x);
        } else {
            SetVariable(var_name, var->getDoubleList(), var_array_y);
        }
      } else {
        double F;
        if (var->getVariableFloat(F)) SetVariable(var_name, F);
      }
    }
    if (var_type == 's')
    {
      if (var_array_y > 1)
      {
        if (var_array_x > 1)
        {
            SetVariable(var_name, var->getStringList(), var_array_y, var_array_x);
        } else {
            SetVariable(var_name, var->getStringList(), var_array_y);
        }
      } else {
        std::string S;
        if (var->getVariableString(S)) SetVariable(var_name, S);
      }
    }
    if (var_type == 'b')
    {
      std::deque<bool> w_dq; // convert int to bool
      for (unsigned idx = 0; idx < (var_array_y * var_array_x); idx++)
      {
        bool B;
        if (var->getVariableBool(B,idx))
        {
          w_dq[idx] = B;
        }
      }
      if (var_array_y > 1) 
      {
        if (var_array_x > 1)
        {
          SetVariable(var_name, w_dq, var_array_y, var_array_x);
        } else {
          SetVariable(var_name, w_dq, var_array_y);
        }
      } else {
        SetVariable(var_name, w_dq[0]);
      }
    }
}




void C_CM_MUP_Interface::RemoveVariable(std::string name)
{
  if (name.empty()) return;
  LockMe();
//  std::map<std::string, mup::Value>::iterator it = impl->VarMap.find(name);
//  if (it != impl->VarMap.end())
  if (impl->parser_var->IsVarDefined(name))
  {
    impl->parser_var->RemoveVar(name);
//    impl->VarMap.erase(it);
  }
  UnlockMe();
}





std::string C_CM_MUP_Interface::mkUnusedVariableNameUnlocked(std::string VariableNameBase)
{
  std::string RetVal;
  if (impl->parser_var->IsVarDefined(VariableNameBase))
  {
    char wstr[100];
    int i = 0;
    do
    {
      i++;
      sprintf(wstr,"%d",i);
      if (!impl->parser_var->IsVarDefined(VariableNameBase+std::string(wstr)))
      {
        RetVal = VariableNameBase+std::string(wstr);
      }
    } while (RetVal.empty());

  } else RetVal = VariableNameBase;
  return RetVal;
}

std::string C_CM_MUP_Interface::mkUnusedVariableName(std::string VariableNameBase)
{
  std::string RetVal;
  LockMe();
  RetVal = mkUnusedVariableNameUnlocked(VariableNameBase);
  UnlockMe();
  return RetVal;
}

std::string C_CM_MUP_Interface::mkUnusedVariableNameAndSet(std::string VariableNameBase, bool value)
{
  LockMe();
  std::string var = mkUnusedVariableNameUnlocked("CmGuiTmpRetVal");
  SetVariableUnlocked(var, value); // obsadit tuto promennou - bude mozna prepsana
  UnlockMe();
  return var;
}


std::string C_CM_MUP_Interface::ReplaceVariableNameInExpression(std::string OldVar, std::string NewVar, std::string ExpressionForEdit)
{
  LockMe();
  impl->parser_var_tester       ->EnableAutoCreateVar(true);
  try
  {
    impl->parser_var_tester->SetExpr(ExpressionForEdit);
    mup::Value val = impl->parser_var_tester->Eval();
  }
  catch(mup::ParserError &e)
  {
  }
  mup::string_type str;
  try
  {
    str = impl->parser_var_tester->GetExprVarWithIDX();
  }
  catch(mup::ParserError &e)
  {
  }

  impl->parser_var_tester->ClearVar();
  impl->parser_var_tester       ->EnableAutoCreateVar(EnableAutoCreateVar);
  UnlockMe();

  std::list<int> Indexes;
  char **item = cm_string_strsplit_outside_string(str.c_str(), ";", 0);
  if (item)
  {
    int i=0;
    int LastA=-1;
    while (item[i])
    {
      gchar * i_inside = cm_string_interpret_first_string(item[i]);
      if (i_inside)
      {
        if (!strncmp(item[i],"A=",2))
        {
          LastA = cm_string_get_value_int(i_inside);
        }
        if (!strncmp(item[i],"V=",2))
        {
          if (!strcmp(i_inside,OldVar.c_str()) && (LastA!=-1)) Indexes.push_back(LastA);
          LastA=-1;
        }
        g_free(i_inside);
      }
      i++;
    }
    g_strfreev(item);
  }
  Indexes.sort();
  Indexes.reverse();
  std::string RetVal = ExpressionForEdit;
  for (std::list<int>::iterator it=Indexes.begin(); it!=Indexes.end(); it++)
  {
    RetVal.replace(*it, OldVar.length(), NewVar);
  }
  return RetVal;
}



void C_CM_MUP_Interface::ClearVariables(void)
{
  LockMe();
  impl->parser_var->ClearVar();
  impl->VarMap.clear();
  UnlockMe();
}



bool C_CM_MUP_Interface::Evaluate(std::string func, char &RetType, std::string &RetValue, std::string &ErrorMessage, unsigned &ErrorPos, std::string *RPN)
{
  bool RetVal = false;
  LockMe();
//  if (EnableAutoCreateVar)  impl->parser_var ->EnableAutoCreateVar(false);
  try
  {
    impl->parser_var->SetExpr(func);
    const mup::Value &val = impl->parser_var->Eval();
    RetVal = true;
    RetType = val.GetType();
    RetValue = val.AsciiDump();
    char *wstr = NULL;
    if (val.GetType()=='i') asprintf(&wstr,"%" PRId64 "",(mup::int_type)val.GetInteger());
    if (val.GetType()=='f') asprintf(&wstr,"%s",cm_std_string_print_short_float(val.GetFloat()).c_str());
    if (val.GetType()=='b') asprintf(&wstr,"%s",(val.GetBool()?"yes":"no"));
    if (val.GetType()=='s') asprintf(&wstr,"\"%s\"",val.GetString().c_str());
    if (val.GetType()=='m')
    {
        if (val.GetCols() == 1)
        {
          asprintf(&wstr,"[%d]", val.GetRows());
        } else {
          asprintf(&wstr,"[%d,%d]", val.GetRows(), val.GetCols());
        }
        if (wstr != NULL)
        {
          RetValue = std::string(wstr) + std::string(":") + val.ToString();
          free(wstr);
          wstr = NULL;
        } else {
          RetValue = "[???]" + std::string(":") + val.ToString();
        }
    }
    if (wstr != NULL)
    {
      RetValue = std::string(wstr);
      free(wstr);
    }

  }
  catch(mup::ParserError &e)
  {
    char *wstr = NULL;
    asprintf(&wstr,"MUPerrorX:code=%d,pos=%d,%s\n",e.GetCode(),e.GetPos(),e.GetMsg().c_str());
    ErrorPos = e.GetPos();
    ErrorMessage = e.GetMsg().c_str();
    if (wstr != NULL)
    {
      ErrorMessage = std::string(wstr); // mozna to je takto uz zbytecne
      free(wstr);
    }
//    LastErrorStr = std::string(wstr);
  }
//  if (EnableAutoCreateVar)  impl->parser_var ->EnableAutoCreateVar(true);
  if (RPN != NULL) *RPN = impl->parser_var->DumpStrRPN();
  UnlockMe();
// debug_printf("aa7  %s",GetRPN().c_str());
  return RetVal;
}

bool C_CM_MUP_Interface::Evaluate(std::string func)
{
  bool RetVal = false;
  LockMe();
  try
  {
    impl->parser_var->SetExpr(func);
    impl->parser_var->Eval();
    RetVal = true;
//   debug_printf("EVAL(%s)=type=%c", func.c_str(), val.GetType());
//    if (val.GetType()=='i') { int i = val.GetInteger();debug_printf("(int)",i); }
//    if (val.GetType()=='f') { mup::float_type f = val.GetFloat();debug_printf("(float)",f); }
//    if (val.GetType()=='b') { mup::bool_type b = val.GetBool();debug_printf("(bool)",(b?"true":"false")); }
//    if (val.GetType()=='s') { mup::string_type s = val.GetString();debug_printf("(string)\"%s",s.c_str()); }
////        case 'c': { std::complex<float_type> c = ans.GetComplex(); console() << c << " (complex)" << "\n"; } break;
//   debug_printf("");
  }
  catch(mup::ParserError &e)
  {
    char *wstr = NULL;
    asprintf(&wstr,"MUPerror:code=%d,pos=%d,%s\n",e.GetCode(),e.GetPos(),e.GetMsg().c_str());
    if (wstr != NULL)
    {
      LastErrorStr = std::string(wstr);
      free(wstr);
    }
  }
  UnlockMe();
  return RetVal;
}

bool C_CM_MUP_Interface::Evaluate(std::string func, bool &RetValue)
{
  bool RetVal = false;
  LockMe();
  try
  {
    impl->parser_var->SetExpr(func);
    const mup::Value &val = impl->parser_var->Eval();
    if (val.GetType()=='b')
    {
      RetValue = val.GetBool();
      RetVal = true;
    }
    if (val.GetType()=='i')
    {
      RetValue = (val.GetInteger() ? true : false);
      RetVal = true;
    }
//   debug_printf("EVAL(%s)=type=%c", func.c_str(), val.GetType());
//    if (val.GetType()=='i') { int i = val.GetInteger();debug_printf("(int)",i); }
//    if (val.GetType()=='f') { mup::float_type f = val.GetFloat();debug_printf("(float)",f); }
//    if (val.GetType()=='b') { mup::bool_type b = val.GetBool();debug_printf("(bool)",(b?"true":"false")); }
//    if (val.GetType()=='s') { mup::string_type s = val.GetString();debug_printf("(string)\"%s",s.c_str()); }
////        case 'c': { std::complex<float_type> c = ans.GetComplex(); console() << c << " (complex)" << "\n"; } break;
//   debug_printf("");
  }
  catch(mup::ParserError &e)
  {
    char *wstr = NULL;
    asprintf(&wstr,"MUPerror:code=%d,pos=%d,%s\n",e.GetCode(),e.GetPos(),e.GetMsg().c_str());
    if (wstr != NULL)
    {
      LastErrorStr = std::string(wstr);
      free(wstr);
    }
    RetVal = false;
  }
  UnlockMe();
  return RetVal;
}

bool C_CM_MUP_Interface::Evaluate(std::string func, int &RetValue)
{
  bool RetVal = false;
  LockMe();
  try
  {
    impl->parser_var->SetExpr(func);
    const mup::Value &val = impl->parser_var->Eval();
    if (val.GetType()=='i')
    {
      RetValue = val.GetInteger();
      RetVal = true;
    }
    if (val.GetType()=='f')
    {
      RetValue = (int)val.GetFloat();
      RetVal = true;
    }
    if (val.GetType()=='b')
    {
      RetValue = (val.GetBool() ? 1 : 0);
      RetVal = true;
    }
//   debug_printf("EVAL(%s)=type=%c", func.c_str(), val.GetType());
//    if (val.GetType()=='i') { int i = val.GetInteger();debug_printf("(int)",i); }
//    if (val.GetType()=='f') { mup::float_type f = val.GetFloat();debug_printf("(float)",f); }
//    if (val.GetType()=='b') { mup::bool_type b = val.GetBool();debug_printf("(bool)",(b?"true":"false")); }
//    if (val.GetType()=='s') { mup::string_type s = val.GetString();debug_printf("(string)\"%s",s.c_str()); }
////        case 'c': { std::complex<float_type> c = ans.GetComplex(); console() << c << " (complex)" << "\n"; } break;
//   debug_printf("");
  }
  catch(mup::ParserError &e)
  {
    char *wstr = NULL;
    asprintf(&wstr,"MUPerror:code=%d,pos=%d,%s\n",e.GetCode(),e.GetPos(),e.GetMsg().c_str());
    if (wstr != NULL)
    {
      LastErrorStr = std::string(wstr);
      free(wstr);
    }
    RetVal = false;
  }
  UnlockMe();
  return RetVal;
}

bool C_CM_MUP_Interface::Evaluate(std::string func, uint64_t &RetValue)
{
  bool RetVal = false;
  LockMe();
  try
  {
    impl->parser_var->SetExpr(func);
    const mup::Value &val = impl->parser_var->Eval();
    if (val.GetType()=='i')
    {
      RetValue = val.GetInteger();
      RetVal = true;
    }
    if (val.GetType()=='f')
    {
      RetValue = (int)val.GetFloat();
      RetVal = true;
    }
    if (val.GetType()=='b')
    {
      RetValue = (val.GetBool() ? 1 : 0);
      RetVal = true;
    }
//   debug_printf("EVAL(%s)=type=%c", func.c_str(), val.GetType());
//    if (val.GetType()=='i') { int i = val.GetInteger();debug_printf("(int)",i); }
//    if (val.GetType()=='f') { mup::float_type f = val.GetFloat();debug_printf("(float)",f); }
//    if (val.GetType()=='b') { mup::bool_type b = val.GetBool();debug_printf("(bool)",(b?"true":"false")); }
//    if (val.GetType()=='s') { mup::string_type s = val.GetString();debug_printf("(string)\"%s",s.c_str()); }
////        case 'c': { std::complex<float_type> c = ans.GetComplex(); console() << c << " (complex)" << "\n"; } break;
//   debug_printf("");
  }
  catch(mup::ParserError &e)
  {
    char *wstr = NULL;
    asprintf(&wstr,"MUPerror:code=%d,pos=%d,%s\n",e.GetCode(),e.GetPos(),e.GetMsg().c_str());
    if (wstr != NULL)
    {
      LastErrorStr = std::string(wstr);
      free(wstr);
    }
    RetVal = false;
  }
  UnlockMe();
  return RetVal;
}

bool C_CM_MUP_Interface::Evaluate(std::string func, int64_t &RetValue)
{
  bool RetVal = false;
  LockMe();
  try
  {
    impl->parser_var->SetExpr(func);
    const mup::Value &val = impl->parser_var->Eval();
    if (val.GetType()=='i')
    {
      RetValue = val.GetInteger();
      RetVal = true;
    }
    if (val.GetType()=='f')
    {
      RetValue = (int)val.GetFloat();
      RetVal = true;
    }
    if (val.GetType()=='b')
    {
      RetValue = (val.GetBool() ? 1 : 0);
      RetVal = true;
    }
//   debug_printf("EVAL(%s)=type=%c", func.c_str(), val.GetType());
//    if (val.GetType()=='i') { int i = val.GetInteger();debug_printf("(int)",i); }
//    if (val.GetType()=='f') { mup::float_type f = val.GetFloat();debug_printf("(float)",f); }
//    if (val.GetType()=='b') { mup::bool_type b = val.GetBool();debug_printf("(bool)",(b?"true":"false")); }
//    if (val.GetType()=='s') { mup::string_type s = val.GetString();debug_printf("(string)\"%s",s.c_str()); }
////        case 'c': { std::complex<float_type> c = ans.GetComplex(); console() << c << " (complex)" << "\n"; } break;
//   debug_printf("");
  }
  catch(mup::ParserError &e)
  {
    char *wstr = NULL;
    asprintf(&wstr,"MUPerror:code=%d,pos=%d,%s\n",e.GetCode(),e.GetPos(),e.GetMsg().c_str());
    if (wstr != NULL)
    {
      LastErrorStr = std::string(wstr);
      free(wstr);
    }
    RetVal = false;
  }
  UnlockMe();
  return RetVal;
}

bool C_CM_MUP_Interface::Evaluate(std::string func, std::string &RetValue)
{
  bool RetVal = false;
  LockMe();
  try
  {
    impl->parser_var->SetExpr(func);
    const mup::Value &val = impl->parser_var->Eval();
    if (val.GetType()=='s')
    {
      RetValue = val.GetString();
      RetVal = true;
    }
//    if (val.GetType()=='i')
//    {
//      RetValue = val.GetInteger();
//      RetVal = true;
//    }
//    if (val.GetType()=='f')
//    {
//      RetValue = (int)val.GetFloat();
//      RetVal = true;
//    }
//    if (val.GetType()=='b')
//    {
//      RetValue = (val.GetBool() ? 1 : 0);
//      RetVal = true;
//    }



//   debug_printf("EVAL(%s)=type=%c", func.c_str(), val.GetType());
//    if (val.GetType()=='i') { int i = val.GetInteger();debug_printf("(int)",i); }
//    if (val.GetType()=='f') { mup::float_type f = val.GetFloat();debug_printf("(float)",f); }
//    if (val.GetType()=='b') { mup::bool_type b = val.GetBool();debug_printf("(bool)",(b?"true":"false")); }
//    if (val.GetType()=='s') { mup::string_type s = val.GetString();debug_printf("(string)\"%s",s.c_str()); }
////        case 'c': { std::complex<float_type> c = ans.GetComplex(); console() << c << " (complex)" << "\n"; } break;
//   debug_printf("");
  }
  catch(mup::ParserError &e)
  {
    char *wstr = NULL;
    asprintf(&wstr,"MUPerror:code=%d,pos=%d,%s\n",e.GetCode(),e.GetPos(),e.GetMsg().c_str());
    if (wstr != NULL)
    {
      LastErrorStr = std::string(wstr);
      free(wstr);
    }
    RetVal = false;
  }
  UnlockMe();
  return RetVal;
}

bool C_CM_MUP_Interface::Evaluate(std::string func, double &RetValue)
{
  bool RetVal = false;
  LockMe();
  try
  {
    impl->parser_var->SetExpr(func);
    const mup::Value &val = impl->parser_var->Eval();
    if (val.GetType()=='i')
    {
      RetValue = (double)val.GetInteger();
      RetVal = true;
    }
    if (val.GetType()=='f')
    {
      RetValue = val.GetFloat();
      RetVal = true;
    }
//   debug_printf("EVAL(%s)=type=%c", func.c_str(), val.GetType());
//    if (val.GetType()=='i') { int i = val.GetInteger();debug_printf("(int)",i); }
//    if (val.GetType()=='f') { mup::float_type f = val.GetFloat();debug_printf("(float)",f); }
//    if (val.GetType()=='b') { mup::bool_type b = val.GetBool();debug_printf("(bool)",(b?"true":"false")); }
//    if (val.GetType()=='s') { mup::string_type s = val.GetString();debug_printf("(string)\"%s",s.c_str()); }
////        case 'c': { std::complex<float_type> c = ans.GetComplex(); console() << c << " (complex)" << "\n"; } break;
//   debug_printf("");
  }
  catch(mup::ParserError &e)
  {
    char *wstr = NULL;
    asprintf(&wstr,"MUPerror:code=%d,pos=%d,%s\n",e.GetCode(),e.GetPos(),e.GetMsg().c_str());
    if (wstr != NULL)
    {
      LastErrorStr = std::string(wstr);
      free(wstr);
    }
    RetVal = false;
  }
  UnlockMe();
  return RetVal;
}

bool C_CM_MUP_Interface::Evaluate(std::string func, long double &RetValue)
{
  bool RetVal = false;
  LockMe();
  try
  {
    impl->parser_var->SetExpr(func);
    const mup::Value &val = impl->parser_var->Eval();
    if (val.GetType()=='i')
    {
      RetValue = (double)val.GetInteger();
      RetVal = true;
    }
    if (val.GetType()=='f')
    {
      RetValue = val.GetFloat();
      RetVal = true;
    }
//   debug_printf("EVAL(%s)=type=%c", func.c_str(), val.GetType());
//    if (val.GetType()=='i') { int i = val.GetInteger();debug_printf("(int)",i); }
//    if (val.GetType()=='f') { mup::float_type f = val.GetFloat();debug_printf("(float)",f); }
//    if (val.GetType()=='b') { mup::bool_type b = val.GetBool();debug_printf("(bool)",(b?"true":"false")); }
//    if (val.GetType()=='s') { mup::string_type s = val.GetString();debug_printf("(string)\"%s",s.c_str()); }
////        case 'c': { std::complex<float_type> c = ans.GetComplex(); console() << c << " (complex)" << "\n"; } break;
//   debug_printf("");
  }
  catch(mup::ParserError &e)
  {
    char *wstr = NULL;
    asprintf(&wstr,"MUPerror:code=%d,pos=%d,%s\n",e.GetCode(),e.GetPos(),e.GetMsg().c_str());
    if (wstr != NULL)
    {
      LastErrorStr = std::string(wstr);
      free(wstr);
    }
    RetVal = false;
  }
  UnlockMe();
  return RetVal;
}


bool C_CM_MUP_Interface::GetVariable(std::string name, std::deque<bool> &values, int &dimensionY, int &dimensionX)
{
  if (name.empty()) return false;
  LockMe();
  bool RetVal = false;
  const mup::var_maptype &vmap = impl->parser_var->GetVar();
  mup::var_maptype::const_iterator item = vmap.find(name);
  if (item != vmap.end())
  {
    mup::Variable &v = (mup::Variable&)(*(item->second));
    dimensionY = v.GetRows();
    dimensionX = v.GetCols();
    values.resize(dimensionY * dimensionX);
    RetVal = true;
    if ((dimensionY * dimensionX) == 1)
    {
      char type = v.GetType();
             if (type == 'i')
      {
        values[0] = (v.GetInteger() ? true : false);
      } else if (type == 'b')
      {
        values[0] = v.GetBool();
      } else {
//        return false;
        RetVal = false;
      }
    } else {
      const mup::Matrix<mup::Value> &varr = v.GetArray();
      for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) 
      {
        const mup::Value &val = varr.At(y, x);
        char type = val.GetType();
               if (type == 'i')
        {
          values[(y * dimensionX) + x] = (val.GetInteger() ? true : false);
        } else if (type == 'b')
        {
          values[(y * dimensionX) + x] = val.GetBool();
        } else {
//          return false;
          RetVal = false;
        }
      }
    }
  }
  UnlockMe();
  return RetVal;
}

bool C_CM_MUP_Interface::GetVariable(std::string name, StringList &values, int &dimensionY, int &dimensionX)
{
  if (name.empty()) return false;
  LockMe();
  bool RetVal = false;
  const mup::var_maptype &vmap = impl->parser_var->GetVar();
  mup::var_maptype::const_iterator item = vmap.find(name);
  if (item != vmap.end())
  {
    mup::Variable &v = (mup::Variable&)(*(item->second));
    dimensionY = v.GetRows();
    dimensionX = v.GetCols();
    values.resize(dimensionY * dimensionX);
    RetVal = true;
    if ((dimensionY * dimensionX) == 1)
    {
      char type = v.GetType();
             if (type == 's')
      {
        values[0] = v.GetString();
      } else {
        RetVal = false;
//        return false;
      }
    } else {
      const mup::Matrix<mup::Value> &varr = v.GetArray();
      for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) 
      {
        const mup::Value &val = varr.At(y, x);
        char type = val.GetType();
               if (type == 's')
        {
          values[(y * dimensionX) + x] = val.GetString();
        } else {
          RetVal = false;
//          return false;
        }
      }
    }
  }
  UnlockMe();
  return RetVal;
}


bool C_CM_MUP_Interface::GetVariable(std::string name, UIntList &values, int &dimensionY, int &dimensionX)
{
  if (name.empty()) return false;
  LockMe();
  bool RetVal = false;
  const mup::var_maptype &vmap = impl->parser_var->GetVar();
  mup::var_maptype::const_iterator item = vmap.find(name);
  if (item != vmap.end())
  {
    mup::Variable &v = (mup::Variable&)(*(item->second));
    dimensionY = v.GetRows();
    dimensionX = v.GetCols();
    values.resize(dimensionY * dimensionX);
    RetVal = true;
    if ((dimensionY * dimensionX) == 1)
    {
      char type = v.GetType();
      if (type == 'i')
      {
        values[0] = v.GetInteger();
      } else if (type == 'f')
      {
        values[0] = (int64_t)(v.GetFloat() + 0.5);
      } else if (type == 'b')
      {
        values[0] = (v.GetBool() ? 1 : 0);
      } else {
//        return false;
        RetVal = false;
      }
    } else {
      const mup::Matrix<mup::Value> &varr = v.GetArray();
      for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) 
      {
        const mup::Value &val = varr.At(y, x);
        char type = val.GetType();
               if (type == 'i')
        {
          values[(y * dimensionX) + x] = val.GetInteger();
        } else if (type == 'f')
        {
          values[(y * dimensionX) + x] = (int64_t)(val.GetFloat() + 0.5);
        } else if (type == 'b')
        {
          values[(y * dimensionX) + x] = (val.GetBool() ? 1 : 0);
        } else {
        RetVal = false;
//          return false;
        }
      }
    }
  }
  UnlockMe();
  return RetVal;
}


bool C_CM_MUP_Interface::GetVariable(std::string name, std::deque<int> &values, int &dimensionY, int &dimensionX)
{
  if (name.empty()) return false;
  LockMe();
  bool RetVal = false;
  const mup::var_maptype &vmap = impl->parser_var->GetVar();
  mup::var_maptype::const_iterator item = vmap.find(name);
  if (item != vmap.end())
  {
    mup::Variable &v = (mup::Variable&)(*(item->second));
    dimensionY = v.GetRows();
    dimensionX = v.GetCols();
    values.resize(dimensionY * dimensionX);
    RetVal = true;
    if ((dimensionY * dimensionX) == 1)
    {
      char type = v.GetType();
             if (type == 'i')
      {
        values[0] = v.GetInteger();
      } else if (type == 'f')
      {
        values[0] = (int)(v.GetFloat() + 0.5);
      } else if (type == 'b')
      {
        values[0] = (v.GetBool() ? 1 : 0);
      } else {
        RetVal = false;
//        return false;
      }
    } else {
      const mup::Matrix<mup::Value> &varr = v.GetArray();
      for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) 
      {
        const mup::Value &val = varr.At(y, x);
        char type = val.GetType();
               if (type == 'i')
        {
          values[(y * dimensionX) + x] = val.GetInteger();
        } else if (type == 'f')
        {
          values[(y * dimensionX) + x] = (int)(val.GetFloat() + 0.5);
        } else if (type == 'b')
        {
          values[(y * dimensionX) + x] = (val.GetBool() ? 1 : 0);
        } else {
        RetVal = false;
//          return false;
        }
      }
    }
  }
  UnlockMe();
  return RetVal;
}

bool C_CM_MUP_Interface::GetVariable(std::string name, DoubleList &values, int &dimensionY, int &dimensionX)
{
  if (name.empty()) return false;
  LockMe();
  bool RetVal = false;
  const mup::var_maptype &vmap = impl->parser_var->GetVar();
  mup::var_maptype::const_iterator item = vmap.find(name);
  if (item != vmap.end())
  {
    mup::Variable &v = (mup::Variable&)(*(item->second));
    dimensionY = v.GetRows();
    dimensionX = v.GetCols();
    values.resize(dimensionY * dimensionX);
    RetVal = true;
    if ((dimensionY * dimensionX) == 1)
    {
      char type = v.GetType();
             if (type == 'i')
      {
        values[0] = (double)v.GetInteger();
      } else if (type == 'b')
      {
        values[0] = (v.GetBool() ? 1.0 : 0.0);
      } else if (type == 'f')
      {
        values[0] = (double)v.GetFloat();
      } else {
        RetVal = false;
//        return false;
      }
    } else {
      const mup::Matrix<mup::Value> &varr = v.GetArray();
      for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) 
      {
        const mup::Value &val = varr.At(y, x);
        char type = val.GetType();
               if (type == 'i')
        {
          values[(y * dimensionX) + x] = (double)val.GetInteger();
        } else if (type == 'b')
        {
          values[(y * dimensionX) + x] = (val.GetBool() ? 1.0 : 0.0);
        } else if (type == 'f')
        {
          values[(y * dimensionX) + x] = (double)val.GetFloat();
        } else {
        RetVal = false;
//          return false;
        }
      }
    }
  }
  UnlockMe();
  return RetVal;
}

bool C_CM_MUP_Interface::GetVariable(std::string name, std::deque<long double> &values, int &dimensionY, int &dimensionX)
{
  if (name.empty()) return false;
  LockMe();
  bool RetVal = false;
  const mup::var_maptype &vmap = impl->parser_var->GetVar();

  mup::var_maptype::const_iterator item = vmap.find(name);
  if (item != vmap.end())
  {
    mup::Variable &v = (mup::Variable&)(*(item->second));
    dimensionY = v.GetRows();
    dimensionX = v.GetCols();
    values.resize(dimensionY * dimensionX);
    RetVal = true;
    if ((dimensionY * dimensionX) == 1)
    {
      char type = v.GetType();
             if (type == 'i')
      {
        values[0] = (long double)v.GetInteger();
      } else if (type == 'b')
      {
        values[0] = (v.GetBool() ? (long double)1.0 : (long double)0.0);
      } else if (type == 'f')
      {
        values[0] = (long double)v.GetFloat();
      } else {
        RetVal = false;
      }
    } else {
      const mup::Matrix<mup::Value> &varr = v.GetArray();
      for (int y=0; y<dimensionY; y++) for (int x=0; x<dimensionX; x++) 
      {
        const mup::Value &val = varr.At(y, x);
        char type = val.GetType();
               if (type == 'i')
        {
          values[(y * dimensionX) + x] = (long double)val.GetInteger();
        } else if (type == 'b')
        {
          values[(y * dimensionX) + x] = (val.GetBool() ? (long double)1.0 : (long double)0.0);
        } else if (type == 'f')
        {
          values[(y * dimensionX) + x] = (long double)val.GetFloat();
        } else {
        RetVal = false;
        }
      }
    }
  }

  UnlockMe();
  return RetVal;
}

bool C_CM_MUP_Interface::GetVariable(std::string name, bool &val)
{
  if (name.empty()) return false;
  std::deque<bool> valarr; int dimX,dimY;
  bool retval = GetVariable(name, valarr, dimY, dimX);
  if ((retval) && (dimY>0) && (dimX>0))
  {
    val = valarr[0];
    return true;
  } else return false;
}

bool C_CM_MUP_Interface::GetVariable(std::string name, libcm_integer &val)
{
  if (name.empty()) return false;
  UIntList valarr; int dimX,dimY;
  bool retval = GetVariable(name, valarr, dimY, dimX);
  if ((retval) && (dimY>0) && (dimX>0))
  {
    val = valarr[0];
    return true;
  } else return false;
}

bool C_CM_MUP_Interface::GetVariable(std::string name, int &val)
{
  if (name.empty()) return false;
  std::deque<int> valarr; int dimX,dimY;
  bool retval = GetVariable(name, valarr, dimY, dimX);
  if ((retval) && (dimY>0) && (dimX>0))
  {
    val = valarr[0];
    return true;
  } else return false;
}

bool C_CM_MUP_Interface::GetVariable(std::string name, double &val)
{
  if (name.empty()) return false;
  DoubleList valarr; int dimX,dimY;
  bool retval = GetVariable(name, valarr, dimY, dimX);
  if ((retval) && (dimY>0) && (dimX>0))
  {
    val = valarr[0];
    return true;
  } else return false;
}

bool C_CM_MUP_Interface::GetVariable(std::string name, long double &val)
{
  if (name.empty()) return false;
  std::deque<long double> valarr; int dimX,dimY;
  bool retval = GetVariable(name, valarr, dimY, dimX);
  if ((retval) && (dimY>0) && (dimX>0))
  {
    val = valarr[0];
    return true;
  } else return false;
}

bool C_CM_MUP_Interface::GetVariable(std::string name, std::string &val)
{
  if (name.empty()) return false;
  StringList valarr; int dimX,dimY;
  bool retval = GetVariable(name, valarr, dimY, dimX);
  if ((retval) && (dimY>0) && (dimX>0))
  {
    val = valarr[0];
    return true;
  } else return false;
}

bool C_CM_MUP_Interface::GetVariable(class CCmVariable *var, bool EnableChangeDimension)
{
  bool RetVal = false;
  std::string var_name = var->getName();
  if (var_name.empty()) return false;
  char var_type = var->getType();
  unsigned var_array_y = var->getArrayY();
  unsigned var_array_x = var->getArrayX();

    if ((var_type == 'i') || (var_type == 'e') || (var_type == 'b'))
    {
      int dimensionY, dimensionX;
      UIntList values;
      if (GetVariable(var_name, values, dimensionY, dimensionX))
      {
        if ((EnableChangeDimension) || (((unsigned)dimensionY==var_array_y)&&((unsigned)dimensionX==var_array_x)))
        {
          for (unsigned y=0; y<MIN((unsigned)dimensionY,var_array_y); y++) for (unsigned x=0; x<MIN((unsigned)dimensionX,var_array_x); x++)
          {
            if (var_type == 'i') var->setVariableInt(values[(y*dimensionX)+x],y,x);
            if (var_type == 'b') var->setVariableBool((values[(y*dimensionX)+x] ? true : false),y,x);
            if (var_type == 'e') var->setVariableEnum(values[(y*dimensionX)+x],y,x);
          }
          if (((unsigned)dimensionY!=var_array_y)||((unsigned)dimensionX!=var_array_x)) SetVariable(var);
          RetVal = true;
        }
      }
    }
    if (var_type == 'f')
    {
      int dimensionY, dimensionX;
      std::deque<long double> values;
      if (GetVariable(var_name, values, dimensionY, dimensionX))
      {
        if ((EnableChangeDimension) || (((unsigned)dimensionY==var_array_y)&&((unsigned)dimensionX==var_array_x)))
        {
          for (unsigned y=0; y<MIN((unsigned)dimensionY,var_array_y); y++) for (unsigned x=0; x<MIN((unsigned)dimensionX,var_array_x); x++)
          {
            var->setVariableFloat((double)values[(y*dimensionX)+x],y,x);
          }
          if (((unsigned)dimensionY!=var_array_y)||((unsigned)dimensionX!=var_array_x)) SetVariable(var);
          RetVal = true;
        }
      }
    }
    if (var_type == 's')
    {
      int dimensionY, dimensionX;
      StringList values;
      if (GetVariable(var_name, values, dimensionY, dimensionX))
      {
        if ((EnableChangeDimension) || (((unsigned)dimensionY==var_array_y)&&((unsigned)dimensionX==var_array_x)))
        {
          for (unsigned y=0; y<MIN((unsigned)dimensionY,var_array_y); y++) for (unsigned x=0; x<MIN((unsigned)dimensionX,var_array_x); x++)
          {
            var->setVariableString(values[(y*dimensionX)+x],y,x);
          }
          if (((unsigned)dimensionY!=var_array_y)||((unsigned)dimensionX!=var_array_x)) SetVariable(var);
          RetVal = true;
        }
      }
    }


  return RetVal;
}

bool C_CM_MUP_Interface::GetVariableFully(class CCmVariable *var, char preferType)
{
  LockMe();
  const mup::var_maptype &vmap = impl->parser_var->GetVar();
  mup::var_maptype::const_iterator item = vmap.find(var->getName());
  if (item != vmap.end())
  {
    mup::Variable &v = (mup::Variable&)(*(item->second));
    var->setArrayYX(v.GetRows(),v.GetCols());
    char var_type = v.GetType();
//    if (var_type=='i') var_type='f'; // can be better for next usage
    if (preferType!=' ') var_type = preferType;
    var->setType(var_type);
    UnlockMe();
    return GetVariable(var);
  } else {
    UnlockMe();
    return false;
  }
}


std::string C_CM_MUP_Interface::ListVariables(void)
{
  std::string RetVal;
  char wstr[100];
  LockMe();
  const mup::var_maptype &vmap = impl->parser_var->GetVar();
  if (vmap.size())
  {
    mup::var_maptype::const_iterator item = vmap.begin();
    for (; item!=vmap.end(); ++item)
    {
      mup::Variable &v = (mup::Variable&)(*(item->second));
      char type = v.GetType();
      RetVal += item->first;
      if (type == 'm')
      {
        if (v.GetCols() == 1)
        {
          sprintf(wstr,"[%d]", v.GetRows());
          const mup::Value &val = v.At(0,0);
          type = val.GetType();
        } else {
          sprintf(wstr,"[%d,%d]", v.GetRows(), v.GetCols());
          const mup::Value &val = v.At(0,0);
          type = val.GetType();
        }
        RetVal += wstr;
      }
      sprintf(wstr,"%c",type);
      RetVal += " {type=" + std::string(wstr) + "} ";
      RetVal += "=" + v.ToString();
      RetVal += "\n";
    }
  } else RetVal = "No variables";
  UnlockMe();
  return RetVal;
}

std::string C_CM_MUP_Interface::ClearAllVoidVariables(void)
{
  std::string RetVal;
  LockMe();
  const mup::var_maptype &vmap = impl->parser_var->GetVar();
  std::set<std::string> ClearVars;
  if (vmap.size())
  {
    mup::var_maptype::const_iterator item = vmap.begin();
    for (; item!=vmap.end(); ++item)
    {
      mup::Variable &v = (mup::Variable&)(*(item->second));
      char type = v.GetType();
      if (type=='v') ClearVars.insert(item->first);
    }
  }
  for (std::set<std::string>::iterator it = ClearVars.begin(); it != ClearVars.end(); it++)
  {
    std::string name = *it;
    if (impl->parser_var->IsVarDefined(name))
    {
      impl->parser_var->RemoveVar(name);
      if (!RetVal.empty()) RetVal += ",";
      RetVal += name;
    }
  }
  UnlockMe();
  return RetVal;
}

std::string C_CM_MUP_Interface::ListVariables(std::string pattern)
{
  std::string RetVal;
  char wstr[100];
  LockMe();
  const mup::var_maptype &vmap = impl->parser_var->GetVar();
  if (vmap.size())
  {
    mup::var_maptype::const_iterator item = vmap.begin();
    for (; item!=vmap.end(); ++item)
    {
      mup::Variable &v = (mup::Variable&)(*(item->second));
      char type = v.GetType();
      if (libCM_isNameMatching(pattern, item->first))
      {
//bool        libCM_isNameMatching(const char *pattern, const char *name);
      RetVal += item->first;
      if (type == 'm')
      {
        if (v.GetCols() == 1)
        {
          sprintf(wstr,"[%d]", v.GetRows());
          const mup::Value &val = v.At(0,0);
          type = val.GetType();
        } else {
          sprintf(wstr,"[%d,%d]", v.GetRows(), v.GetCols());
          const mup::Value &val = v.At(0,0);
          type = val.GetType();
        }
        RetVal += wstr;
      }
      sprintf(wstr,"%c",type);
      RetVal += " {type=" + std::string(wstr) + "} ";
      RetVal += "=" + v.ToString();
      RetVal += "\n";
      }



    }
  } else RetVal = "No variables";
  UnlockMe();
  return RetVal;
}

std::string C_CM_MUP_Interface::ListConstants(void)
{
  std::string RetVal;
  char wstr[100];
  LockMe();
  const mup::var_maptype &vmap = impl->parser_var->GetConst();
  if (vmap.size())
  {
    mup::var_maptype::const_iterator item = vmap.begin();
    for (; item!=vmap.end(); ++item)
    {
      mup::Variable &v = (mup::Variable&)(*(item->second));
      char type = v.GetType();
      RetVal += item->first;
      if (type == 'm')
      {
        if (v.GetCols() == 1)
        {
          sprintf(wstr,"[%d]", v.GetRows());
          const mup::Value &val = v.At(0,0);
          type = val.GetType();
        } else {
          sprintf(wstr,"[%d,%d]", v.GetRows(), v.GetCols());
          const mup::Value &val = v.At(0,0);
          type = val.GetType();
        }
        RetVal += wstr;
      }
      sprintf(wstr,"%c",type);
      RetVal += " {type=" + std::string(wstr) + "} ";
      RetVal += "=" + v.ToString();
      RetVal += "\n";
    }
  } else RetVal = "No constants";
  UnlockMe();
  return RetVal;
}


std::string C_CM_MUP_Interface::ListFunctions(void)
{
  std::string RetVal;
  LockMe();
//  const mup::char_type **pOprtDef = impl->parser_var->GetOprtDef();
//  for (int i=0;pOprtDef[i];i++)
//  {
//   debug_printf(":::\"%s\"",pOprtDef[i]);
//  }
// returns only:
//:::"("
//:::")"
//:::"["
//:::"]"
//:::"{"
//:::"}"
//:::","
//:::"?"
//:::":"


  const mup::oprt_ifx_maptype &imap = impl->parser_var->GetOprtIfxDef();
  if (imap.size())
  {
    RetVal += std::string(" ------ OPRT IFX --------\n");
    mup::oprt_ifx_maptype::const_iterator item = imap.begin();
    for (; item!=imap.end(); ++item)
    {
      mup::ICallback *pFun = (mup::ICallback*)item->second.Get();
      RetVal += "\"" + item->first + "\" - " + pFun->GetDesc() + std::string("\n");
    }
  } else RetVal = "No OPRT IFX";

  const mup::oprt_pfx_maptype &pmap = impl->parser_var->GetOprtPfxDef();
  if (pmap.size())
  {
    RetVal += std::string(" ------ OPRT PFX --------\n");
    mup::oprt_pfx_maptype::const_iterator item = pmap.begin();
    for (; item!=pmap.end(); ++item)
    {
      mup::ICallback *pFun = (mup::ICallback*)item->second.Get();
      RetVal += "\"" + item->first + "\" - " + pFun->GetDesc() + std::string("\n");
    }
  } else RetVal = "No OPRT PFX";

  const mup::oprt_bin_maptype &omap = impl->parser_var->GetOprtBinDef();
  if (omap.size())
  {
    RetVal += std::string(" ------ OPRT --------\n");
    mup::oprt_bin_maptype::const_iterator item = omap.begin();
    for (; item!=omap.end(); ++item)
    {
      mup::ICallback *pFun = (mup::ICallback*)item->second.Get();
      RetVal += "\"" + item->first + "\" - " + pFun->GetDesc() + std::string("\n");
    }
  } else RetVal = "No OPRT";

  const mup::fun_maptype &fmap = impl->parser_var->GetFunDef();
  if (fmap.size())
  {
    RetVal += std::string(" ------ FUN --------\n");
    mup::fun_maptype::const_iterator item = fmap.begin();
    for (; item!=fmap.end(); ++item)
    {
      mup::ICallback *pFun = (mup::ICallback*)item->second.Get();
      RetVal += pFun->GetDesc() + std::string("\n");
    }
  } else RetVal = "No functions";
  UnlockMe();
  return RetVal;
}

std::string C_CM_MUP_Interface::GetLastExprVarWithIDX(void)
{
  std::string RetVal;
  LockMe();
  RetVal = impl->parser_var->GetExprVarWithIDX();
  UnlockMe();
  return RetVal;
}
std::string C_CM_MUP_Interface::GetLastExprVarWithIDX_HumanReadable(void)
{
  std::string RetVal;
  LockMe();
  std::string Data = impl->parser_var->GetExprVarWithIDX();

// GetExprVarWithIDX:
//    Variable  : A=StrPosition; V=VarName;
//    Value     : A=StrPosition; I=Value(int);
//    IdxClose  : A=StrPosition; X=NumArguments(dimensions);
//    Function  : A=StrPosition; G=NumArgsuments, F=FunctionName
//    Assignment: Q=StrPosition;    { At end ... if is present a assignment like : = += -= *= /= }

  char **item = cm_string_strsplit_outside_string(Data.c_str(), ";", 0);
  if ((item) && (item[0]))
  {
    int i=0;
    while (item[i])
    {
      if (strlen(item[i])>0)
      {
        if (!strncmp(item[i],"A=",2)) RetVal += "\n";
        if (!strncmp(item[i],"Q=",2)) RetVal += "\n";
        RetVal += std::string(item[i]) + ";";
      }
      i++;
    }
    g_strfreev(item);
  }

  UnlockMe();
  return RetVal;
}

// ===============================================================

void mup_TESTER_var(class C_CM_MUP_Interface *MUP, std::string str_var)
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
      std::string varN;
      if (!MUP->GetUsedVariables(str_varN, &OutList, false, true, false, false)) // output
      {
       debug_printf("MUP::ERROR:GetUsedVariables for Local Var \"%s\" -> \'%s\'",str_varN.c_str(),MUP->GetLastError().c_str());
        return;
      }
      if (OutList.size()!=1)
      {
       debug_printf("Local Var \"%s\" have bad number of modiffied variables ... need 1 , but %s",str_varN.c_str(),cm_std_string_print_short_float((float)OutList.size()).c_str());
        return;
      }
      varN=*OutList.begin();

      if (!MUP->Evaluate(str_varN))
      {
       debug_printf("MUP::Error (SET Local Var) %s :: %s",str_varN.c_str(),MUP->GetLastError().c_str());
        return;
      }
      class CCmVariable var;
      var.setName(varN);
//      var.setType('f');
//      MUP->GetVariable(&var);
      MUP->GetVariableFully(&var,prefferedType);
      MUP->RemoveVariable(var.getName());
     debug_printf("local var is \"%s\"",var.fullPrint().c_str());
    }

  } else {
   debug_printf("WRONG LOCAL VAR DEFINITION (param nums) VAR:%s have not assigment of forst value",str_var.c_str());
  }

}
void mup_TESTER_Expr(C_CM_MUP_Interface *CM_MUP_Interface, std::string Expr, bool with_idx, bool output_vars, bool input_vars, bool with_debug)
{
  std::list<std::string> UsedVariables;
 debug_printf("===GetVars================================================");
 debug_printf("Expr:%s",Expr.c_str());
  bool RetVal = CM_MUP_Interface->GetUsedVariables(Expr,&UsedVariables,with_idx, output_vars, input_vars, with_debug);
 debug_printf("--RetVal = %s  ---------------------------------",(RetVal ? "true" : "false"));
  if (!RetVal)
  {
   debug_printf("ERROR:%s",CM_MUP_Interface->GetLastError().c_str());
   debug_printf("---------------------------------------------------");
  }
  
  for (std::list<std::string>::iterator it=UsedVariables.begin(); it!=UsedVariables.end(); it++)debug_printf("%s",(*it).c_str());
 debug_printf("---------------------------------------------------");
}

void mup_TESTER_Expr_Eval(C_CM_MUP_Interface *CM_MUP_Interface, std::string Expr)
{
 debug_printf("===Eval================================================");
 debug_printf("Expr:%s",Expr.c_str());

//  if (CM_MUP_Interface->Evaluate(eval,ValI))debug_printf("EVAL: %s==>>%d",eval.c_str(), ValI); elsedebug_printf("EVAL: ERROR: %s",eval.c_str());

  bool RetVal = CM_MUP_Interface->Evaluate(Expr);
 debug_printf("--RetVal = %s  ---------------------------------",(RetVal ? "true" : "false"));
  if (!RetVal)
  {
   debug_printf("ERROR:%s",CM_MUP_Interface->GetLastError().c_str());
   debug_printf("---------------------------------------------------");
  }


}
void mup_TESTER(void)
{
  C_CM_MUP_Interface *CM_MUP_Interface = new C_CM_MUP_Interface();

 debug_printf("==============");
  mup_TESTER_Expr(CM_MUP_Interface,"sum(a,TxG_A_Status_exist)/sum(3,4,5)",true,true,true,true);
  mup_TESTER_Expr(CM_MUP_Interface,"sum(a,TxG_A_Status_exist)/sum(3,4,5)",true,true,true,true);
  mup_TESTER_Expr(CM_MUP_Interface,"ZZZ[21]=sum(TxG_A_PRF[5],c,Measure[4,2],Measure[4,5])/sum(TxG_A_PRF[3],4,5)",true,false,true,true);
  mup_TESTER_Expr(CM_MUP_Interface,"sum(a,TxG_A_Status_exist)/sum(3,4,5)",false,true,true,true);
  mup_TESTER_Expr(CM_MUP_Interface,"ZZZ[21]=sum(TxG_A_PRF[5],c,Measure[4,2],Measure[4,5])/sum(TxG_A_PRF[3],4,5)",false,true,true,true);
  mup_TESTER_Expr(CM_MUP_Interface,"exist(TxG_aa[5])",false,true,true,true);
  mup_TESTER_Expr(CM_MUP_Interface,"exist_idx(TxG_bb,5)",false,true,true,true);
  mup_TESTER_Expr(CM_MUP_Interface,"(((aa==8)&&exist_idx(TxG_cc,5)&&exist_idx(TxG_dd,8) && (  exist(TxG_kk)?(exist_idx(TxG_k1,10)):(exist_idx(TxG_k2,11,12))        )   )?zz=1:zz=2)",false,true,true,true);



  CM_MUP_Interface->ClearVariables();

  try
  {
    CM_MUP_Interface->SetVariable("Boolean",true);
    CM_MUP_Interface->SetVariable("B",(double)10.5);
    CM_MUP_Interface->SetVariable("Integer",(int)125);
    double arr[6] =  {1,2,3,4,5,6};
    CM_MUP_Interface->SetVariable("arr_f1", arr,6);
    CM_MUP_Interface->SetVariable("arr_f2", arr,3,2);
  }
  catch(mup::ParserError &e)
  {
   debug_printf("MUP ERROR!!: code=%d pos=%d %s",e.GetCode(),e.GetPos(),e.GetMsg().c_str());
  }

  int ValI = 0; double ValD=0.0; bool ValB = false; std::string ValS;
  std::string eval;

  eval="sum(3,4,5)";
  if (CM_MUP_Interface->Evaluate(eval,ValI))debug_printf("EVAL: %s==>>%d",eval.c_str(), ValI); else debug_printf("EVAL: ERROR: %s",eval.c_str());

  eval="sum(3.1,4.5,5.8)";
  if (CM_MUP_Interface->Evaluate(eval,ValD))debug_printf("EVAL: %s==>>%g",eval.c_str(), ValD); else debug_printf("EVAL: ERROR: %s",eval.c_str());

  eval="sum(3.1,4.5,5.8)>10";
  if (CM_MUP_Interface->Evaluate(eval,ValB))debug_printf("EVAL: %s==>>%s",eval.c_str(), (ValB?"true":"false")); else debug_printf("EVAL: ERROR: %s",eval.c_str());

  eval="sum(3.1,B,5.5)";
  if (CM_MUP_Interface->Evaluate(eval,ValD))debug_printf("EVAL: %s==>>%g",eval.c_str(), ValD); else debug_printf("EVAL: ERROR: %s",eval.c_str());

  eval="sum(Integer,4,5)";
  if (CM_MUP_Interface->Evaluate(eval,ValD))debug_printf("EVAL: %s==>>%g",eval.c_str(), ValD); else debug_printf("EVAL: ERROR: %s",eval.c_str());

  eval="CurrentTime()";
  if (CM_MUP_Interface->Evaluate(eval,ValD))debug_printf("EVAL: %s==>>%g",eval.c_str(), ValD); else debug_printf("EVAL: ERROR: %s",eval.c_str());


 debug_printf("%s",CM_MUP_Interface->ListVariables().c_str());

 debug_printf("==============");
  sleep(2);
//  CM_MUP_Interface->SetVariable("float",(double)125.88);
  eval="CurrentTime()";      if (CM_MUP_Interface->Evaluate(eval,ValD))debug_printf("EVAL: %s==>>%g",eval.c_str(), ValD); else debug_printf("EVAL: ERROR: %s",eval.c_str());
 debug_printf("%s",CM_MUP_Interface->ListVariables().c_str());
 debug_printf("==============");
  CM_MUP_Interface->SetEnableAutoCreateVar(true);

  eval="arr_f1[ 3] = arr_f1[ 3] | 0x10";
  if (CM_MUP_Interface->Evaluate(eval,ValI))debug_printf("EVAL: %s==>>%d",eval.c_str(), ValI); else debug_printf("EVAL: ERROR: %s",eval.c_str());
  eval="arr_f1[ 4] = arr_f1[ 4] | 0x10";
  CM_MUP_Interface->Evaluate(eval);
 debug_printf("%s",CM_MUP_Interface->ListVariables().c_str());


  mup_TESTER_var(CM_MUP_Interface,  "a=1");
  mup_TESTER_var(CM_MUP_Interface,  "b=3.15");
  mup_TESTER_var(CM_MUP_Interface,  "b=3.0");
  mup_TESTER_var(CM_MUP_Interface,  "(f)b=3");

 debug_printf("==============");
  mup_TESTER_Expr(CM_MUP_Interface,"OA=(exist(arr_f1[5]) ? arr_f1[5] : 0)",true,true,true,true);
  mup_TESTER_Expr(CM_MUP_Interface,"OA=(exist_idx(arr_f1,5) ? arr_f1[5] : 0)",true,true,true,true);

  mup_TESTER_Expr_Eval(CM_MUP_Interface,"OA=(exist(arr_f1[5]) ? arr_f1[5] : 0)");
  mup_TESTER_Expr_Eval(CM_MUP_Interface,"OB=(exist_idx(arr_f1,5) ? arr_f1[5] : 0)");
  mup_TESTER_Expr_Eval(CM_MUP_Interface,"OC=(exist(arr_f1[15]) ? arr_f1[15] : 0)");
  mup_TESTER_Expr_Eval(CM_MUP_Interface,"OD=(exist_idx(arr_f1,15) ? arr_f1[15] : 0)");
  mup_TESTER_Expr_Eval(CM_MUP_Interface,"OE=(exist(arr_f3[15]) ? arr_f3[15] : 0)");
  mup_TESTER_Expr_Eval(CM_MUP_Interface,"OF=(exist_idx(arr_f3,15) ? arr_f3[15] : 0)");
//  "(exist(buf,11)?buf[5]:buf[15])"
  
 debug_printf("==============");
 debug_printf ("%s\n",CM_MUP_Interface->ListVariables().c_str());

  delete CM_MUP_Interface;
}

std::string libCM_ListAllKnownVariablesMUP(C_CM_MUP_Interface *_MUP, std::string pattern, bool printHeadFoot)
{
  std::string RetVal;
  if (printHeadFoot) RetVal += std::string("================================ MUP begin ===========================\n");
  if (pattern.size()<1)
  {
    RetVal += _MUP->ListVariables();
  } else {
    RetVal += _MUP->ListVariables(pattern);
  }
  if (printHeadFoot) RetVal += std::string("================================ MUP end ===========================\n");
  return RetVal;
}

//-------------------------------------------------------------------------------------------------

#endif // USE_LIB_MUPARSERX
