
#include "cm_live_debug.hh"
#include "cm_string.hh"
#include "cm_std_string.hh"
#include "cm_debug.hh"


void CCmLiveDebug::Init(void)
{
  FlagsPTR=NULL;
  LiveDebugFlags = 0;
  LiveDebugList.clear();
  LastBitAdd = 0;
  FlagMap.clear();
}
void CCmLiveDebug::Done(bool NeedLock)
{
  if (NeedLock) LiveDebugMutex.Lock();

  LiveDebugFlags = 0; if (FlagsPTR!=NULL) *FlagsPTR=LiveDebugFlags;
  for (std::list< class CCmOneLiveDebug*>::iterator itLD = LiveDebugList.begin(); itLD != LiveDebugList.end(); itLD++) delete(*itLD);
  LiveDebugList.clear();

  if (NeedLock) LiveDebugMutex.Unlock();
}

void CCmLiveDebug::CheckChanges(void)
{
  bool changed = false;
  bool found_usable = false;
  LiveDebugMutex.Lock();
  std::string FirstUsableFile;
  for (std::list< std::pair< time_t, std::string > >::iterator it = LiveDebugFiles.begin(); (it != LiveDebugFiles.end()) && (!(found_usable)); it++)
  {
    time_t file_mtime = 0;
    struct stat file_stat;
    int retval = stat(it->second.c_str(),&file_stat);
    if (retval==0) file_mtime = file_stat.st_mtime;
    if (file_mtime != 0)
    {
      found_usable = true;
      FirstUsableFile = it->second;
    }
    if (file_mtime != it->first)
    {
      changed = true;
      it->first = file_mtime;
    }
  }
  if (changed)
  {
    if (!FirstUsableFile.empty())
    {
      CMSConfig *config = new CMSConfig(FirstUsableFile,"m4");

      std::string name;
      std::string param;
      StringList params;
      std::deque<StringList> allparams;
      while (CMSConfigFindNextItem2(*config, name))
      {
        params.clear();
        params.push_back(name);
        while (!(param = config->GetValueString2 ()).empty ())
        {
          params.push_back(param);
        }
        allparams.push_back(params);
      }

//    std::list< class CCmOneLiveDebug*> LiveDebugList;
//    uint64_t LiveDebugFlags;
      if(allparams.size()!=LiveDebugList.size())
      {
        for (std::list< class CCmOneLiveDebug*>::iterator itLD = LiveDebugList.begin(); itLD != LiveDebugList.end(); itLD++) delete(*itLD);
        LiveDebugList.clear();
        for (unsigned u=0;u<allparams.size();u++)
        {
          class CCmOneLiveDebug*N=new CCmOneLiveDebug();
          N->assignPtr(this);
          LiveDebugList.push_back(N);
        }
      }
      int i=0;
      for (std::list< class CCmOneLiveDebug*>::iterator itLD = LiveDebugList.begin(); itLD != LiveDebugList.end(); itLD++, i++)
      {
        (*itLD)->NewParams(allparams[i], Constants);
      }
      LiveDebugFlags = 0;if (FlagsPTR!=NULL) *FlagsPTR=LiveDebugFlags;
      for (std::list< class CCmOneLiveDebug*>::iterator itLD = LiveDebugList.begin(); itLD != LiveDebugList.end(); itLD++)
      {
        if ((*itLD)->valid) LiveDebugFlags |= (*itLD)->DebugFlags;
      }
      if (FlagsPTR!=NULL) *FlagsPTR=LiveDebugFlags;
      delete config;
      cms_ns_if_print("libcm", 3,"CFGLive::Changed ... use \"%s\"",FirstUsableFile.c_str());
    } else {
      cms_ns_if_print("libcm", 3,"CFGLive::Changed ... EMPTY");
      LiveDebugFlags = 0;if (FlagsPTR!=NULL) *FlagsPTR=LiveDebugFlags;
      for (std::list< class CCmOneLiveDebug*>::iterator itLD = LiveDebugList.begin(); itLD != LiveDebugList.end(); itLD++) delete(*itLD);
      LiveDebugList.clear();
    }
  }
  for (std::list< class CCmOneLiveDebug*>::iterator itLD = LiveDebugList.begin(); itLD != LiveDebugList.end(); itLD++) (*itLD)->fflushAll();

  LiveDebugMutex.Unlock();
}

void CCmLiveDebug::SetControlFiles(std::string Files)
{
  LiveDebugMutex.Lock();
  Done(false);
  StringList FilesArr;
  cm_std_string_SplitLineAndInterpretStringsInQuotes(Files, FilesArr, ",", "");
  if (FilesArr.size()>1)
  {
    for (unsigned x=1;x<FilesArr.size(); x++)
    {
      std::string FName = FilesArr[0] + FilesArr[x];
      cm_std_string_replace_by_map(FName, Constants);
      cm_std_string_replace(FName, "$ID", "0");
      std::pair< time_t, std::string > ITEM;
      ITEM.first = 0;
      ITEM.second = FName;
      LiveDebugFiles.push_back(ITEM);
    }
  }
  LiveDebugMutex.Unlock();
}

uint64_t CCmLiveDebug::getFlagMask(unsigned char ch, bool NeedLock)
{
  uint64_t RetVal = 0;
  if (NeedLock) LiveDebugMutex.Lock();

  std::map<unsigned char, uint64_t>::iterator it = FlagMap.find(ch);
  if (it == FlagMap.end())
  {
    if (LastBitAdd<63)
    {
      uint64_t NewFlag = 0;
      NewFlag |= (1LL << LastBitAdd);
      LastBitAdd++;
      RetVal = NewFlag;
      FlagMap[ch] = NewFlag;
    } else {
     debug_printf("ERROR!!! many debug flags ... more than 63 ");
    }
  } else RetVal = it->second;

  if (NeedLock) LiveDebugMutex.Unlock();
  return RetVal;
}


void CCmLiveDebug::PrintANY(uint64_t TestFlags, unsigned conn_id, const char *BeforeData, const char *Data)
{
  LiveDebugMutex.Lock();
  for (std::list< class CCmOneLiveDebug*>::iterator itLD = LiveDebugList.begin(); itLD != LiveDebugList.end(); itLD++)
  {
    if (((*itLD)->valid) && (((*itLD)->DebugFlags) & TestFlags))
    {
      (*itLD)->PrintANY(conn_id, BeforeData, Data);
    }
  }
  LiveDebugMutex.Unlock();
}
void CCmLiveDebug::PrintANY(uint64_t TestFlags, unsigned conn_id, std::string &BeforeData, std::string &Data)
{
  PrintANY(TestFlags, conn_id, BeforeData.c_str(), Data.c_str());
}

void CCmLiveDebug::PrintFilters(uint64_t TestFlags, unsigned conn_id, const char *BeforeData, class CCmFiltersSet *Filters)
{
  LiveDebugMutex.Lock();
  for (std::list< class CCmOneLiveDebug*>::iterator itLD = LiveDebugList.begin(); itLD != LiveDebugList.end(); itLD++)
  {
    if (((*itLD)->valid) && (((*itLD)->DebugFlags) & TestFlags))
    {
      (*itLD)->PrintFilters(conn_id, BeforeData, Filters);
    }
  }
  LiveDebugMutex.Unlock();
}

void CCmLiveDebug::PrintVar(uint64_t TestFlags, unsigned conn_id, const char *BeforeData, class CCmVariable const *var)
{
  LiveDebugMutex.Lock();
  for (std::list< class CCmOneLiveDebug*>::iterator itLD = LiveDebugList.begin(); itLD != LiveDebugList.end(); itLD++)
  {
    if (((*itLD)->valid) && (((*itLD)->DebugFlags) & TestFlags))
    {
      (*itLD)->PrintVar( conn_id, BeforeData, var);
    }
  }
  LiveDebugMutex.Unlock();
}

void CCmLiveDebug::PrintVars(uint64_t TestFlags, unsigned conn_id, const char *BeforeData, VarMap *mapVar)
{
  LiveDebugMutex.Lock();
  for (std::list< class CCmOneLiveDebug*>::iterator itLD = LiveDebugList.begin(); itLD != LiveDebugList.end(); itLD++)
  {
    if (((*itLD)->valid) && (((*itLD)->DebugFlags) & TestFlags))
    {
      (*itLD)->PrintVars( conn_id, BeforeData, mapVar);
    }
  }
  LiveDebugMutex.Unlock();
}


// ===================================================================================================================================================================================
// ===================================================================================================================================================================================





CCmLiveDebug::CCmOneLiveDebug::CCmOneLiveDebug()
{
  valid=false; IPDesc=NULL; OutF=NULL; DebugFlags=0;
  ParamsContainID = PrefixContainID = false;
  CmLiveDebug = NULL;
  DEBUG_FLAG_ADD_TIME = 0xffffffff;
  DEBUG_FLAG_ADD_VAR_AGE = 0xffffffff;
  DEBUG_FLAG_NEWLINE = 0xffffffff;
}
CCmLiveDebug::CCmOneLiveDebug::~CCmOneLiveDebug(void)
{
  CloseAllFilesAndIP();
}

void CCmLiveDebug::CCmOneLiveDebug::assignPtr(CCmLiveDebug *PTR)
{
  CmLiveDebug = PTR;
  DEBUG_FLAG_ADD_TIME = getFlagMask('T');
  DEBUG_FLAG_ADD_VAR_AGE = getFlagMask('G');
  DEBUG_FLAG_NEWLINE = getFlagMask('n');
}

void CCmLiveDebug::CCmOneLiveDebug::CloseAllFilesAndIP(void)
{
  if (IPDesc!=NULL) delete IPDesc;
  IPDesc = NULL;
  if (OutF!=NULL) fclose(OutF);
  OutF = NULL;
  if (!MapOutF.empty())
  {
    for(std::map<unsigned, FILE *>::iterator it = MapOutF.begin(); it != MapOutF.end(); it++)
    {
      fclose(it->second);
    }
    MapOutF.clear();
  }
  MapOutS.clear();
  ParamsContainID = PrefixContainID = false;
  OtherType = 0;
  MapPrefixID.clear();
}

FILE *CCmLiveDebug::CCmOneLiveDebug::CreateNewFile(std::string FileName)
{
  FILE *RetVal = fopen(FileName.c_str(),"w");
  if (RetVal==NULL)
  {
    // try also all required directories
    libCM_CreateDirectoryFromPath(FileName.c_str());
    RetVal = fopen(FileName.c_str(),"w");
  }
  return RetVal;
}

bool CCmLiveDebug::CCmOneLiveDebug::SetFirstParam(StringList &Params, std::map<std::string, std::string> &Constants)
{
  CloseAllFilesAndIP();
  bool OK = true;
  StringList P0;
  std::string Params0 = Params[1];
  cm_std_string_replace_by_map(Params0, Constants);
  std::string Params0x =  Params0; cm_std_string_replace(Params0x, "$ID"    , ""); ParamsContainID   = (Params0 != Params0x);
  cm_std_string_SplitLineAndInterpretStringsInQuotes(Params0, P0, ":", "");

  if (P0.size()>0)
  {
           if (P0[0] == "STDOUT")
    {
      OtherType = 3;
    } else if (P0[0] == "NS")
    {
      if (P0.size()>2)
      {
        NS_Name = P0[1];
        NS_Num = cm_std_string_get_value_int(P0[2]); // i to muze byt dynamicke, ale to je hovadina!
        OtherType = 4;
      } else OK=false;
    } else if (P0[0] == "FILE")
    {
      if (P0.size()>1)
      {
        FileNameTemplate = P0[1];
        if (!ParamsContainID) OutF = CreateNewFile(FileNameTemplate);
        OtherType = 1;
      } else OK=false;
    } else if (P0[0] == "FILEOVER")
    {
      if (P0.size()>1)
      {
        FileNameTemplate = P0[1];
//        if (!ParamsContainID) OutF = CreateNewFile(FileNameTemplate);
        OtherType = 2;
      } else OK=false;
    } else {
     // cmsIP?
      if (IPDesc!=NULL) delete IPDesc;
      IPDesc = NULL;
      if (P0.size()>1)
      {
        try { IPDesc = new CMSIP(CMS_FLAG_SEND, Params0); }
        catch (...)
        {
//          throw;
          OK=false;
          IPDesc = NULL;
        }
//        OtherType = 4;
      } else OK=false;
    }
  }


  // doplnit varianty a hlavne files a CMSIP arrays, kdyz $ID bude v parameetru a dynamicky Prefix by $ID

          // pokud bude neco spatne, shodime what na 0, CMSIP na NULL a OtherType na 0
  if (!OK) CloseAllFilesAndIP();

  return OK;
}

bool CCmLiveDebug::CCmOneLiveDebug::SetSecondParam(StringList &Params, std::map<std::string, std::string> &Constants)
{
  std::string Params1 = Params[2];
  cm_std_string_replace_by_map(Params1, Constants);
  std::string Params1x =  Params1; cm_std_string_replace(Params1x, "$ID"    , ""); PrefixContainID   = (Params1 != Params1x);
  Prefix = Params1;
  MapPrefixID.clear();
  return true;
}

uint64_t CCmLiveDebug::CCmOneLiveDebug::getFlagMask(unsigned char ch)
{
  uint64_t RetVal = 0;
  if (CmLiveDebug != NULL)
  {
    RetVal = CmLiveDebug->getFlagMask(ch, false);
  }
  return RetVal;
}


uint64_t CCmLiveDebug::CCmOneLiveDebug::DecodeDebugFlags(std::string param)
{
  uint64_t RetVal=0;
  for (size_t i=0;i<param.size();i++)
  {
    RetVal |= getFlagMask(param[i]);
  }
  return RetVal;
}

void CCmLiveDebug::CCmOneLiveDebug::NewParams(StringList &Params, std::map<std::string, std::string> &Constants)
{
  if (Params != OldParams)
  {
    DebugFlags=0;
    valid = false;
    if (Params.size()>2)
    {
      DebugFlags = DecodeDebugFlags(Params[0]);
      valid = true;
      if (!((OldParams.size()>1) && (OldParams[1] == Params[1]))) if (!SetFirstParam (Params, Constants)) valid = false;
      if (!((OldParams.size()>2) && (OldParams[2] == Params[2]))) if (!SetSecondParam(Params, Constants)) valid = false;
    }
    OldParams = Params;
  }
}
void CCmLiveDebug::CCmOneLiveDebug::PrintANY(unsigned conn_id, const char *BeforeData, const char *Data)
{
  std::string *PrefixPTR = &Prefix;
  std::string ChangedPrefix;
  if (PrefixContainID)
  {
    std::map<unsigned, std::string>::iterator it = MapPrefixID.find(conn_id);
    if (it == MapPrefixID.end())
    {
      ChangedPrefix = Prefix;
      char WStr2[50]; sprintf(WStr2,"%d",conn_id);
      cm_std_string_replace(ChangedPrefix, "$ID", std::string(WStr2));
      MapPrefixID[conn_id] = ChangedPrefix;
      PrefixPTR = &ChangedPrefix;
    } else PrefixPTR = &(it->second);
  }

  if ((IPDesc!=NULL) || (OtherType==1) || (OtherType==2))
  {
    char WStr[1000];
    if (DebugFlags & DEBUG_FLAG_ADD_TIME)
    {
      sprintf(WStr,"%s:%s:%s", libCM_TimeToString(libCM_GetActualTimeDouble(), 0).c_str(), PrefixPTR->c_str(), BeforeData);
    } else {
      sprintf(WStr,"%s:%s",PrefixPTR->c_str(),BeforeData);
    }

    std::string DataSEND = std::string(WStr)+std::string(Data)+std::string("\n");

           if (IPDesc!=NULL)
    {
      IPDesc->Send(DataSEND.c_str(),DataSEND.size()-1);
    } else if (OtherType==1) // FILE
    {
      if (ParamsContainID)
      {
        std::map<unsigned, FILE *>::iterator it = MapOutF.find(conn_id);
        FILE *FW;
        if (it == MapOutF.end())
        {
          char WStr2[50]; sprintf(WStr2,"%d",conn_id);
          std::string NewFileName = FileNameTemplate;
          cm_std_string_replace(NewFileName, "$ID", std::string(WStr2));
          FILE *NewOutF = CreateNewFile(NewFileName);
          if (NewOutF != NULL) MapOutF[conn_id] = NewOutF;
          FW = NewOutF;
        } else FW = it->second;
        fputs(DataSEND.c_str(),FW);
      } else {
        if (OutF!=NULL) fputs(DataSEND.c_str(),OutF);
      }
    } else if (OtherType==2) // FILEOVER
    {


      if (ParamsContainID)
      {
        std::map<unsigned, std::string>::iterator it = MapOutS.find(conn_id);
        std::string FS;
        if (it == MapOutS.end())
        {
          char WStr2[50]; sprintf(WStr2,"%d",conn_id);
          std::string NewFileName = FileNameTemplate;
          cm_std_string_replace(NewFileName, "$ID", std::string(WStr2));
          MapOutS[conn_id] = NewFileName;
          FS = NewFileName;
        } else FS = it->second;
        FILE *NewOutF = CreateNewFile(FS);
        fputs(DataSEND.c_str(),NewOutF);
        fclose(NewOutF);
      } else {
        FILE *NewOutF = CreateNewFile(FileNameTemplate);
        fputs(DataSEND.c_str(),NewOutF);
        fclose(NewOutF);
      }



    }
  } else if (OtherType==3) //STDOUT
  {
    if (DebugFlags & DEBUG_FLAG_ADD_TIME)
    {
     debug_printf("%s:%s:%s%s", libCM_TimeToString(libCM_GetActualTimeDouble(), 0).c_str(), PrefixPTR->c_str(), BeforeData, Data);
    } else {
     debug_printf("%s:%s%s",PrefixPTR->c_str(),BeforeData,Data);
    }
  } else if (OtherType==4) //NS
  {
    if (DebugFlags & DEBUG_FLAG_ADD_TIME)
    {
      cms_ns_if_print(NS_Name.c_str(), (unsigned)NS_Num,"%s:%s:%s%s", libCM_TimeToString(libCM_GetActualTimeDouble(), 0).c_str(), PrefixPTR->c_str(), BeforeData, Data);
    } else {
      cms_ns_if_print(NS_Name.c_str(), (unsigned)NS_Num,"%s:%s%s",PrefixPTR->c_str(),BeforeData,Data);
    }
  }
}

void CCmLiveDebug::CCmOneLiveDebug::fflushAll(void)
{
  if (OutF!=NULL) fflush(OutF);
  if (!MapOutF.empty())
  {
    for(std::map<unsigned, FILE *>::iterator it = MapOutF.begin(); it != MapOutF.end(); it++)
    {
      fflush(it->second);
    }
  }
}

void CCmLiveDebug::CCmOneLiveDebug::PrintFilters(unsigned conn_id, const char *BeforeData, class CCmFiltersSet *Filters)
{
  std::string Delimiter = ",";
  std::string ID = "My";
  if (conn_id != 0) ID = "Partner";
  std::string Prefix = ID+":";
  std::string Suffix = "";
  if (DebugFlags & DEBUG_FLAG_NEWLINE)
  {
    Delimiter = "\n ";
    Prefix = ID+":"+"....\n================================================\n ";
    Suffix = "\n================================================\n";
  }
 
  std::string full = Prefix + Filters->toString(Delimiter) + Suffix;
  PrintANY(conn_id, ("|FILTERS|"+std::string(BeforeData)).c_str(), full.c_str());
  fflushAll();
}

void CCmLiveDebug::CCmOneLiveDebug::PrintVar(unsigned conn_id, const char *BeforeData, class CCmVariable const *var)
{
  double ActTime = 0.0;
  if (DebugFlags & DEBUG_FLAG_ADD_VAR_AGE)
  {
    ActTime = libCM_GetActualTimeDouble();
  }
  PrintANY(conn_id, BeforeData, var->fullPrint(ActTime).c_str());
}

void CCmLiveDebug::CCmOneLiveDebug::PrintVars(unsigned conn_id, const char *BeforeData, VarMap *mapVar)
{
  double ActTime = 0.0;
  if (DebugFlags & DEBUG_FLAG_ADD_VAR_AGE)
  {
    ActTime = libCM_GetActualTimeDouble();
  }

  std::string Delimiter = ",";
  std::string ID = "My";
  if (conn_id != 0) ID = "Partner";
  std::string Prefix = ID+":";
  std::string Suffix = "";
  if (DebugFlags & DEBUG_FLAG_NEWLINE)
  {
    Delimiter = "\n ";
    Prefix = ID+":"+"....\n================================================\n ";
    Suffix = "\n================================================\n";
  }
 
//  std::string full = Prefix + Filters->toString(Delimiter) + Suffix;
  std::string full = Prefix;
  for (VarMap::iterator it = mapVar->begin(); it != mapVar->end(); it++)
  {
    if (it != mapVar->begin()) full += Delimiter;
    full += it->second.fullPrint(ActTime);
  }


  PrintANY(conn_id, ("|VARIABLES|"+std::string(BeforeData)).c_str(), full.c_str());
  fflushAll();
}

