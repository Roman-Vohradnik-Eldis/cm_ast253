
#ifndef CM_LIVE_DEBUG_H
#define CM_LIVE_DEBUG_H

#include "cm_header_internal.hh"
#include "cm_base_functs.hh"
#include "cm_base_variable.hh"
#include "cm_base_filter.hh"
#include "cm_base_filters_set.hh"

class CCmLiveDebug
{
  private:
    class CCmOneLiveDebug
    {
      private:
        StringList OldParams;
        uint64_t DEBUG_FLAG_ADD_TIME;
        uint64_t DEBUG_FLAG_ADD_VAR_AGE;
        uint64_t DEBUG_FLAG_NEWLINE;
      public:
        bool valid;
        uint64_t DebugFlags;  //PrintComun Variant ....... bits 0=Recv, 1=Send, 2=Fragment, 3=PingRecv, 4=PingSend, 5=AccRecv, 6=AccSend, 10=Time
        CMSIP *IPDesc;
        int OtherType; // 0=none, 1=file, 2=fileover, 3=stdout, 4=ns

        std::string FileNameTemplate;
        FILE *OutF;
        std::map<unsigned, std::string> MapOutS; // FILEOVER
        std::map<unsigned, FILE *> MapOutF; // FILE
        std::string OtherFName;
        std::string Prefix;
        std::map<unsigned, std::string> MapPrefixID;
        bool ParamsContainID;
        bool PrefixContainID;
        std::string NS_Name;
        int NS_Num;
        CCmLiveDebug *CmLiveDebug;

        CCmOneLiveDebug();
        virtual ~CCmOneLiveDebug(void);
        virtual void assignPtr(CCmLiveDebug *PTR);
        virtual void CloseAllFilesAndIP(void);
        virtual FILE *CreateNewFile(std::string FileName);
        virtual bool SetFirstParam(StringList &Params, std::map<std::string, std::string> &Constants);
        virtual bool SetSecondParam(StringList &Params, std::map<std::string, std::string> &Constants);
        virtual uint64_t getFlagMask(unsigned char ch);
        virtual uint64_t DecodeDebugFlags(std::string param);
        virtual void NewParams(StringList &Params, std::map<std::string, std::string> &Constants);
        virtual void PrintANY(unsigned conn_id, const char *BeforeData, const char *Data);
        virtual void PrintFilters(unsigned conn_id, const char *BeforeData, class CCmFiltersSet *Filters);
        virtual void PrintVar(unsigned conn_id, const char *BeforeData, class CCmVariable const *var);
        virtual void PrintVars(unsigned conn_id, const char *BeforeData, VarMap *mapVar);
        virtual void fflushAll(void);
    };

    CMSMutex LiveDebugMutex;
    std::list< std::pair< time_t, std::string > > LiveDebugFiles; // list < USED_modify_time, filename>
    std::list< class CCmOneLiveDebug*> LiveDebugList;
    uint64_t LiveDebugFlags;
    std::map<std::string, std::string> Constants;
    uint64_t *FlagsPTR;

    unsigned LastBitAdd;
    std::map<unsigned char, uint64_t> FlagMap;


  public:
    virtual void        PrintANY(uint64_t TestFlags, unsigned conn_id, const char *BeforeData, const char *Data);
    virtual void        PrintANY(uint64_t TestFlags, unsigned conn_id, std::string &BeforeData, std::string &Data);
    virtual void        PrintFilters(uint64_t TestFlags, unsigned conn_id, const char *BeforeData, class CCmFiltersSet *Filters);
    virtual void        PrintVar(uint64_t TestFlags, unsigned conn_id, const char *BeforeData, class CCmVariable const *var);
    virtual void        PrintVars(uint64_t TestFlags, unsigned conn_id, const char *BeforeData, VarMap *mapVar);



    virtual void        Init(void);
    virtual void        Done(bool NeedLock = true);
    virtual void        SetConstants(std::map<std::string, std::string> _Constants) { Constants = _Constants; }
    virtual void        SetAllFlagsPTR(uint64_t *PTR) { FlagsPTR=PTR; if (FlagsPTR!=NULL) *FlagsPTR=LiveDebugFlags; }
    virtual void        SetControlFiles(std::string Files=std::string());
    virtual void        CheckChanges(void);
    virtual uint64_t    getFlagMask(unsigned char ch, bool NeedLock = true);
};



#endif // CM_LIVE_DEBUG_H
