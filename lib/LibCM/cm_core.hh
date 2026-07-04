#ifndef CM_CORE_H
#define CM_CORE_H

#include "cm_debug.hh"
#include "cm_base.hh"
#include "cm_header_internal.hh"
#include "cm_base_functs.hh"
#include "cm_base_variable.hh"
#include "cm_base_filter.hh"
#include "cm_base_filters_set.hh"
#include "cm_base_partner.hh"
#include "cm_live_debug.hh"
#include "cm_connection.hh"
#include "cm_poll_cond.hh"
#include "cm_ext_hook.hh"
#include "cm_var_changes.hh"
#include "cm_ip.hh"
#include "cm_thread.hh"
#include "cm_scheduler.hh"
#include "cm_db_state.hh"
#include "cm_view.hh"
#include "cm_send_buffer_state.hh"
#include <memory>

class CmCore : CCmConnection
{
    FILE *debug_sent_messages_file;
public:
    std::string MyLibCM_name;

  protected:
    bool use_master_slave;
    CCmPartner *master_partner; // or NULL

    pid_t MyPid;
    std::string MyPidStr;
    std::string MyPrgName;
  // private:
  //   std::string ip_configuration;
  //   std::string IpDesc;

    CmIpDescriptor ip_descriptor;
    Maybe<double> max_variable_send_period;

  protected:
    CMSMutex *CM_Mutex;
  private:
    CMSMutex *SaveFileMutex;

    // CmCommunParams rq_commun_params;
    double   RqPingPeriod;
    double   RqPingTimeOut;
    unsigned RqLengthMax;
    unsigned RqLengthIdeal;
    unsigned RqBaudSpeedMax;
    double   RqBaudSpeedCalcTime;
    
    unsigned RetryCount;
    double   RetryTime;

    int ProtocolFlags;
    int ProtocolVersion;
     // 0=base version - all is sended to and back
     // 1=

    SendBufferSettings send_buffer_settings;
    RecvBufferSettings recv_buffer_settings;
    std::string send_buffer_dict_hash; // Empty if not used
    std::string recv_buffer_dict_hash; // Empty if not used

    
  protected:
    FilterCBFunction FilterCB_function;
    VariableCBFunction VariableCB_function;
    void     *UserPtr;

    bool m_use_notify_cb;
    cm::function<void(CCmNotification)> m_notify_cb;
    
  protected:
    uint16_t CmWorkerFlags;

  protected:

    CmVarChanges m_var_changes;

    // Flag for regenenerate collected list and generate CB_Filters()
    bool PartnerFiltersChanged;
    bool DbChanged;

    TimeAccumulator debug_time_accum1;
    TimeAccumulator debug_time_accum2;
    TimeAccumulator debug_time_accum3;
    TimeAccumulator debug_time_accum4;
    TimeAccumulator debug_time_accum5;
    TimeAccumulator debug_time_accum6;


  private:

    double      PingPeriod; // used values merged with other users
    double      PingTimeOut; // used values merged with other users
    unsigned    LengthMax; // used values merged with other users
    unsigned    LengthIdeal; // used values merged with other users
    unsigned    BaudSpeedMax; // used values merged with other users
    double      BaudSpeedCalcTime; // used values merged with other users

    bool        UDP_MODE;
    unsigned    UDP_PACKET_COUNTER;
    unsigned    UDP_ID; // 0 = TCP(undefined), 1=MASTER, 2..xxx=SLAVES
    unsigned    COM_MODE; // 0=text, 1=binary

    bool use_cmip_ping;
    cm::scoped_ptr<CmIPBase> connection;

    std::unique_ptr<std::ofstream> sent_messages_file;
    
    // CMSServer2 *SERVER; // server*
    // CMSClient  *CLIENT; // client*
    // CMSIP      *IP; // other : udp, mcudp

    std::list <DoubleList> BaudSpeedHistory; // UDP


    // GThread *RecvThread_handler;
    // GThread *PollThread_handler;
   protected:

    // Tohle se pouziva pri zaniku objektu, aby vnitrni thready uz nic
    // nedelaly. Viz ~CmCore.
    bool stop_threads;
    bool has_started;
    
  // public:
  //   GlobalCondition *CB__Condition;

public:
    // CMSCondition *PollCondition;

    cm::scoped_ptr<PollCond> PollCondLive;
    cm::scoped_ptr<PollCond> PollCondSend;
    cm::scoped_ptr<PollCond> PollCondPing;
    cm::scoped_ptr<PollCond> PollCondSave;
    cm::scoped_ptr<PollCond> PollCondExt;
    cm::scoped_ptr<PollCond> PollCondCB;
    cm::scoped_ptr<PollCond> PollCondDb;
    cm::scoped_ptr<PollCond> PollCondRecv;
    cm::scoped_ptr<PollCond> PollCondMasterSlave;
  private:

    void setConfigVariables();

    // kdyz budu predavat pointery, nemel byh to prendat z private na public?

    // double TimeLastSent; // FOR UDP
    // double SendBlockUntil; // FOR UDP

    std::string MyDefaultPrefix; // add to my variables , or empty/blank
    std::string RemotePrefix; // add to my variables , or empty/blank
    std::string m_MyName; // Send to partner
  protected:

    // Tohle jsou filtry, ktere chce uzivatel. 
    class CCmFiltersSet myFiltersIn;
    class CCmFiltersSet myFiltersExt;


    class CCmFiltersSet myFilters;

    // TODO: Prehodit do EXT
public:
    std::string ExtSECTION_FUN; // only for Ext (optional config value)
    std::string ExtSECTION_MUP; // only for Ext (optional config value)
    std::string ExtSECTION_VAR; // only for Ext (optional config value)
    std::string ExtSECTION_DEF; // only for Ext (optional config value)

    bool need_call_cb;

public:
    void debug_size();
  private:

    // myVariables: Poznamka: Pokud je pouzit MyPrefix, promenne ho
    // neobsahuji, ani se s nim neposilaji partnerum. Tento prefix si
    // pridava sam partner, kdyz nasi promennou dostane.
    VarMap myVariables;

    std::list <class CCmPartner> Partners;

    // V tomhle se uchovavaji posledni filtry, ktere sly do callbacku.
    // Pouziva se to k tomu, aby se spocitaly created a deleted do
    // callbacku.
    CCmFiltersSet PartnerFilters;

    // CMSMutex recv_mutex;
    // CMSCondition recv_cond;
    // Thread *recv_thread = nullptr;
    // bool recv_notified = false;
    // bool recv_stop = false;

    std::set<std::string> savefile_enabled_variables;

    // Pro bezne zarizeni je zde true. Router zde dava false a nastavi
    // si jednotlive, ktere chce ukladat.
    bool savefile_policy_save_all = true;
    
    void recv_do_notify();
    void master_slave_do_notify();
  //   void recv_main();
  // protected:
    
  private:
    VarMap PartnerVariables;

    std::list<std::string> SendFragmentDataList;
    unsigned SendFragmentCount;
    unsigned SendFragmentDST;

    unsigned    SaveFile_Type;
    std::string SaveFile_Params;
    double      SaveFile_LastChangeMinDelay;
    double      SaveFile_FirstChangeMaxDelay;

    bool        SaveFile_LOAD;
    bool        SaveFile_USE;
    bool        SaveFile_firstLoop;
    double      SaveFile_LastUnsavedChange;
    double      SaveFile_FirstUnsavedChange;

    // cm::unique_ptr<ThreadWorker<int>> cb_thread;

    // int64_t get_var_changes_counter = 0;
    // int64_t get_filter_changes_counter = 0;
    
    void cb_thread_iter();

    void generate_callback();

    cm::scoped_ptr<CmDbState> db_state;

public:
    CCmLiveDebug LiveDebug;
    uint64_t LiveDebugFlags;
  private:

    uint64_t DEBUG_FLAG_DUMP_FILTERS_MY          ;
    uint64_t DEBUG_FLAG_DUMP_FILTERS_PARTNER_EACH;
    uint64_t DEBUG_FLAG_DUMP_FILTERS_PARTNER_ALL ;
    uint64_t DEBUG_FLAG_PRINT_COMUN_RECV         ;
    uint64_t DEBUG_FLAG_PRINT_COMUN_SEND         ;
    uint64_t DEBUG_FLAG_PRINT_COMUN_FRAG         ;
    uint64_t DEBUG_FLAG_PRINT_COMUN_PING_RECV    ;
    uint64_t DEBUG_FLAG_PRINT_COMUN_PING_SEND    ;
    uint64_t DEBUG_FLAG_PRINT_COMUN_ACC_RECV     ;
    uint64_t DEBUG_FLAG_PRINT_COMUN_ACC_SEND     ;
    uint64_t DEBUG_FLAG_PRINT_COMUN_CONN         ;
    uint64_t DEBUG_FLAG_VAR_CREATED              ;
    uint64_t DEBUG_FLAG_VAR_DELETED              ;
    uint64_t DEBUG_FLAG_VAR_CHANGED              ;
    uint64_t DEBUG_FLAG_VAR_ACTUAL_FULL_STATE    ;
    std::set<unsigned> VAR_ACTUAL_FULL_STATE_changes;

    // std::string ExtSECTION_FUN; // only for Ext (optional config value)
    // std::string ExtSECTION_MUP; // only for Ext (optional config value)
    // std::string ExtSECTION_VAR; // only for Ext (optional config value)
    // std::string ExtSECTION_DEF; // only for Ext (optional config value)

private:
    virtual double      GetActualTimeDouble() ;
    virtual void        PartnerFiltersGoCB(class CCmFiltersSet *PartnerFiltersActive,
                                           class CCmFiltersSet *PartnerFiltersCreated,
                                           class CCmFiltersSet *PartnerFiltersDeleted) ;


    // virtual void        NotifyCallback();

    // virtual void        VariableGoCB      (VarMap *VariablesCreated,
    //                                        VarMap *VariablesUpdated,
    //                                        VarMap *VariablesDeleted) ;
    // virtual void        internalChangePartnerVariable(std::list<CCmPartner>::iterator it_p, std::string name, CCmVariable const *var, CCmVariable const *orig_var = NULL, uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF) ;
    virtual void        internalChangeMyVariable(CCmVariable const *var, CCmPartner *dont_send_it, bool gen_cb, uint8_t newPriority) ;
    virtual void        RecalculateUsedInitValues(CCmPartner *src_it);
    virtual void        CalculateSendBlockUntil(std::list <class CCmPartner>::iterator it, unsigned send_bytes_now) ;

    virtual void        RegenaratePartnerFilters(CCmFiltersSet &PartnerFiltersActive) ;
    virtual void        RegenaratePartnerFiltersBasic(CCmFiltersSet &PartnerFiltersActive) ;
    virtual void        PartnerFiltersDiffForCB(class CCmFiltersSet &PartnerFiltersActive, class CCmFiltersSet *PartnerFiltersCreated, class CCmFiltersSet *PartnerFiltersDeleted) ;

    virtual void        VariableCreated_CB(CCmVariable const *var, unsigned ID, bool WakeUpCB__Thread) ;
    virtual void        VariableDeleted_CB(CCmVariable const *var, unsigned ID, bool WakeUpCB__Thread) ;
    virtual void        VariableUpdated_CB(CCmVariable const *var, CCmVariable const *origo_var, unsigned ID, bool WakeUpCB__Thread, bool RqFiltersConfirm) ;

    virtual void        VariableCreated_No_CB(CCmVariable *var, unsigned ID) ;
    virtual void        VariableDeleted_No_CB(CCmVariable *var, unsigned ID) ;
    virtual void        VariableUpdated_No_CB(CCmVariable *var, unsigned ID) ;

    virtual std::string RemoveMatchingPrefix(std::string variable, std::string onlyPrefix, unsigned *size_add) ;
    virtual void        RefreshFilters(bool useLockCM) ;
    virtual void        RefreshFiltersBasic(bool useLockCM) ;
    // virtual void        GenerateDeleteAllVariablesCBNoMasterSlave(std::list <CCmPartner>::iterator src_it) ;


    virtual void        CCmInternalNew(unsigned _COM_MODE, unsigned _UDP_ID, double _PingPeriod, double _PingTimeOut,
                                       unsigned _LengthMax, unsigned _LengthIdeal, unsigned _BaudSpeedMax, double _BaudSpeedCalcTime, unsigned _RetryCount, double _RetryTime, std::string _MyPrefix, std::string _RemotePrefix,
                                       FilterCBFunction,
                                       VariableCBFunction,
                                       uint16_t _CmWorkerFlags, void *_UserPtr, std::string _name) ;
    virtual void        CCmInternalNew(CMSConfig *config, FilterCBFunction, VariableCBFunction, uint16_t _CmWorkerFlags, void *_UserPtr, std::string _name) ;
    virtual unsigned    SendTextMessage(View<uint8_t> msg, unsigned PACKET_NO, unsigned RETRY_PACKET_NO, uint32_t MyDebugFilterFlag, unsigned DST_ID, unsigned partnerLengthMax); // 0=to al finall
    virtual unsigned    SendTextMessages(std::list<std::string> msgs, unsigned PACKET_NO, unsigned RETRY_PACKET_NO, uint32_t MyDebugFilterFlag, unsigned DST_ID, unsigned partnerLengthMax); // 0=to al finall
    virtual void        ProcessTextCommand(char **item, const char *data1cmd, CCmPartner &src_id) ;
    virtual void        ReceivedTextData(const char *data, unsigned len, unsigned connection_id) ;
    virtual void        ReceivedTextDataOld(const char *data, unsigned len, unsigned connection_id) ;
    virtual void        ReceivedTextDataNew(const char *data, unsigned len, CCmPartner &connection_id) ;

    virtual void        nowConnected(unsigned connection_id, std::string address_str) ;
    virtual void        erasePartner(std::list <CCmPartner>::iterator it_d, bool SendDisconnectUDP) ;
    virtual void        erasePartnerByConnectionId(unsigned connection_id, bool SendDisconnectUDP) ;

    virtual std::list<class CCmPartner>::iterator findVariableOwner(std::string name, class CCmVariable *var) ;
    // virtual void        CheckAndCallCB(void) ;
    virtual bool        doesPartnerVariableCollide(std::string name, CCmPartner& src_it) ;
    virtual void        SaveFileItemChangedNow(void) ;
    virtual void        SaveFile_Load() ;
    virtual void        SaveFile_Save() ;
    virtual void        SaveFile_LoadVariable(std::string const &name, CCmVariable &var);

//    virtual void        CreateDirectoryFromPath(const gchar *path);


    cm::scoped_ptr<CCmExtHook> m_ext_hook;


    void OnRecvFromPartner(char const *buffer, int length, unsigned connection_id);
    void OnPartnerConnected(unsigned connection_id);
    void OnPartnerDisconnected(unsigned connection_id);

    virtual void        LoadConfigVariables(CMSConfig *config, VarMap *LoadedVariables) ;
    virtual void        LoadConfigMyVariables(CMSConfig *config) ;   

    // virtual void        PollThreadOneLoop (double &sleep_time) ;
    // // virtual void        CB__ThreadOneLoop (bool LockMyMutex=false) ;

    // // virtual void        RecvThread (void) ;
    // virtual void        PollThread (void) ;
    
  public:
    CmCore(CMSConfig *config, FilterCBFunction,
              VariableCBFunction,
              uint16_t _CmWorkerFlags, void *_UserPtr, std::string _name);
    CmCore(std::string config_path, std::string section,
              FilterCBFunction,
              VariableCBFunction,
              uint16_t _CmWorkerFlags, void *_UserPtr, std::string _name);

    CmCore(CMSConfig *config, cm::function<void(CCmNotification)> notify_cb, uint16_t flags, std::string name);

    virtual            ~CmCore();
    
    void SetExtHook(CCmExtHook *hook); // Should be unique_ptr<CCmExtHook>
    CCmExtHook *GetExtHook(); // return null if not set
    
    virtual void        InitSaveFile(unsigned _SaveFile_Type, std::string _SaveFile_Params, double _SaveFile_LastChangeMinDelay, double _SaveFile_FirstChangeMaxDelay) ;
    virtual void        StartUseSaveFile(bool _Blocked) ;
    virtual void        StopUseSaveFile(bool _EnableSaveNow) ;
    virtual void        Start() ;
    virtual void        Stop() ;
    virtual unsigned    numConnectedPartners(void) ;
    virtual std::string getPartnersPrefixes(void) ;

    void SaveFileSetPolicy(bool save_all);
    void SaveFileAddVariables(std::set<std::string> const &vars);
    
    // virtual gboolean    onServerConnect(cms_client_t *client); // external call , can be changed to protected: with friend function finals
    // virtual gboolean    onServerDisconnect(cms_client_t *client); // external call , can be changed to protected: with friend function finals

    virtual void        ClearFilters() ;

    // Smaze filtr daneho jmena
    virtual void        DelFilter(const char *name) ;
    virtual void        DelFilter(std::string name) ;

    // Smaze postupne filtry vsech zadanych jmen pomoci DelFilter
    virtual void        DelFilters(std::list<std::string> names) ;

    // Prida jeden filter
    virtual void        AddFilter(class CCmFilter *newFilter) ;

    // Postupne prida vsechny filtry v seznamu pomoci AddFilter
    virtual void        AddFilters(std::list<class CCmFilter> *newFilter) ;
    
    virtual void        SetFilters(std::list<class CCmFilter> *newFilter) ;
    virtual void        SetFilters(std::map<std::string, class CCmFilter> *newFilter) ;
    virtual void        SetFilters(class CCmFiltersSet *newFilter) ;

    virtual CCmFiltersSet GetMyFilters() ;

    // Definuje nasi promennou podle zadani.
    //
    // Pokud uz ja promennou tohoto jmena mam, tak ji predefinuj.
    // Pokud uz nektery partner promennou tohoto jmena ma, tak ho asi
    // odpojime (i v pripade masterslavu). 
    //
    // Funkce vzdycky uspeje. Promenna tohoto jmena bude existovat se
    // zadanym typem a hodnotou, dokud nezavolame Undefine.
    virtual void        DefineMyVariable(CCmVariable const &var, bool _GenerateCB) ;
    virtual void        DefineMyVariable(CCmVariable const *var, bool _GenerateCB) ;
    virtual void        DefineMyVariable(std::string str, bool _GenerateCB) ;
    virtual void        DefineMyVariable(char** params, bool _GenerateCB) ;
    virtual void        DefineMyVariables(VarMap const *newVariables, bool _GenerateCB) ;

    // Oddefinuje nasi promennou.
    //
    // Pokud promenna tohoto jmena neexistuje, nic se neprovede.  Z
    // callbacku uz neprijde zadna zmena ani oddefinovani, dokud ji
    // znovu nenadefinujeme.
    //
    // Vrati true, pokud promenna existovala, jinak false.
    //
    virtual bool        UndefineVariable(std::string name, bool _GenerateCB) ;
    virtual void        ClearMyVariables(bool _GenerateCB) ;
    
    // Nastavi promennou na pozadovanou hodnotu.
    //
    // Pokud se to povedlo, vrati true.
    //
    // Jinak vrati false, kde mohou nastat tyto chyby:
    //
    // (1) Promenna tohoto jmena neexistuje. 
    //
    // (2) Ma jiny typ nez hodnota, na kterou to chceme nastavit, nebo
    // chceme nastavovat polozku mimo rozsah pole.
    //
    // Oboji se muze stat, pokud promenna existovala, a jeste nez nam
    // prisel callback, tak ji nekdo oddefinoval/predefinoval.
    //
    // Pokud z callbacku ta promenna prisla a my nastavovali hodnotu
    // podle toho jejiho typu, co jsme dostali, tak tu zmenu muzeme
    // brat jako provedenou, i kdyz ji LibCM neprovede.  V pripade (1)
    // i (2) nam totiz prijde z callbacku jeji (re)definice nebo
    // oddefinovani.
    //
    // Pokud promenna je nadefinovana a je moje, tak
    // ChangeVariable(CCmVariable *, ...) a DefineMyVariable delaji to
    // same, ale ChangeVariable dovoluje nastavit prioritu teto
    // zmeny. To napriklad znamena, ze muzeme treba pomoci
    // ChangeVariable zmenit typ nebo limity, a udelat tak redefinici.
    // Pokud dodame typ '?', tak je chovani nedefinovane.
    virtual void ChangeVariable(CCmVariable const *var, uint8_t newPriority, bool _GenerateCB) ;

    virtual void ChangeVariable      (std::string name, std::string StrParams                                    , uint8_t newPriority) ;
    virtual void ChangeVariableBool  (std::string name, bool        value, unsigned idx_y, unsigned idx_x, uint8_t newPriority) ;
    virtual void ChangeVariableInt   (std::string name, libcm_integer    value, unsigned idx_y, unsigned idx_x, uint8_t newPriority) ;
    virtual void ChangeVariableFloat (std::string name, double      value, unsigned idx_y, unsigned idx_x, uint8_t newPriority) ;
    virtual void ChangeVariableString(std::string name, std::string value, unsigned idx_y, unsigned idx_x, uint8_t newPriority) ;
    virtual void ChangeVariableEnum  (std::string name, int         value, unsigned idx_y, unsigned idx_x, uint8_t newPriority) ;
    virtual void ChangeVariableEnum  (std::string name, std::string value, unsigned idx_y, unsigned idx_x, uint8_t newPriority) ;

    // Ziska aktualni hodnotu promenne.
    //
    // Pozor: Opakovane volani GetVariable muze davat ruzne hodnoty,
    // pokud mezitim nekdo z partneru tuto promennou modifikoval.
    virtual class CCmVariable   GetVariable( std::string name, bool *found_and_filled) ;

    virtual void        GetAllVariables(VarMap &var_list, uint8_t FlagMaskTest, uint8_t FlagMaskVal) ;
    virtual void        GetMyVariables(VarMap &var_list, uint8_t FlagMaskTest, uint8_t FlagMaskVal) ;
    virtual void        GetPartnersVariables(VarMap &var_list, uint8_t FlagMaskTest, uint8_t FlagMaskVal) ;
    virtual void        AddAllVariables(VarMap &var_list, uint8_t FlagMaskTest, uint8_t FlagMaskVal) ;
    virtual void        AddMyVariables(VarMap &var_list, uint8_t FlagMaskTest, uint8_t FlagMaskVal) ;
    virtual void        AddPartnersVariables(VarMap &var_list, uint8_t FlagMaskTest, uint8_t FlagMaskVal) ;

    virtual std::string ListAllVariables(bool printAge) ;
private:
    
    virtual void        LiveDebugPrintANY(uint64_t TestFlags, unsigned conn_id, const char *BeforeData, const char *Data) ;
    virtual void        LiveDebugPrintANY(uint64_t TestFlags, unsigned conn_id, std::string &BeforeData, std::string &Data) ;
    virtual void        LiveDebugPrintFilters(uint64_t TestFlags, unsigned conn_id, const char *BeforeData, class CCmFiltersSet *Filters) ;
    virtual void        LiveDebugPrintVar(uint64_t TestFlags, unsigned conn_id, const char *BeforeData, CCmVariable const *var) ;
    virtual void        LiveDebugPrintVars(uint64_t TestFlags, unsigned conn_id, const char *BeforeData, VarMap *mapVar) ;

    virtual void        LiveDebugInit(void) ;
    virtual void        LiveDebugDone(bool NeedLock) ;

public:
    virtual void        LiveDebugSetControlFiles(std::string Files) ;
    virtual void        LiveDebugCheckChanges(void) ;

    virtual void        SetMaster(std::string const &value) ;

    virtual std::string        GetMasterName();

    void GetSomeChanges(std::string const &pattern, VarMap &c, VarMap &u, VarMap &d);
    CmChanges GetChanges();

    std::string const &MyName() const;

    std::set<std::string> getCollidingVariables();

    void DbAcceptService(DbService service_name, DbServiceParams);
    void DbSetAcceptedServices(std::map<DbService, DbServiceParams> const &services);
    Maybe<QueryId> DbSendQueryMaybe(DbService service_name, CmDbQuery const &query);
    QueryId DbSendQuery(DbService service_name, CmDbQuery const &query);
    void DbSendResponse(CmDbResponse response);

    void DbForwardResponse(DbMessage msg);
    Maybe<QueryId> DbBeginForwardedQuery(DbService service);
    void DbAddToForwardedQuery(DbMessage msg);

    void DbCancelQuery(QueryId query_id, WeakString reason, bool gen_callback);

    void ApplyVariableChanges(VariableChangesConstRef changes);

    CmStatistics getStatistics();
    
private:

    bool GetFilterChanges(CCmFiltersSet &a, CCmFiltersSet &c, CCmFiltersSet &d);
    void GetVariableChanges(VarMap &c, VarMap &u, VarMap &d);
    
    void tryDefineSomeCollidingVariable(std::string const &name);
    void markPartnersCollidingVariables(std::string const &varname);
    void internalUndefineMyVariable(CCmVariable &v, bool generate_cb);
    void internalUpdateExistingVariable(VarMap::iterator it_my, CCmVariable &var, bool gen_cb, uint8_t newPriority);
    void internalRedefineExistingVariable(VarMap::iterator it_my, CCmVariable &var, bool gen_cb, uint8_t newPriority);
    void internalDefineNewVariable(CCmVariable &var, bool gen_cb, uint8_t newPriority);

    void EvalExt();
    void EvalDb();
    void EvalSend();
    void EvalPing();
    void EvalLiveDebug();
    void EvalSaveFile();
    void EvalRecv();
    void EvalMasterSlave();

    void EvalSendFragments(double ActTime);
    double EvalSendForPartner(std::list<CCmPartner>::iterator it, double ActTime);
    double EvalSendForPartnerWhenInited(std::list<CCmPartner>::iterator it, double ActTime);

    int doRecv(char *buffer, unsigned &connection_id);

    enum FRG_kind
    {
	FRG_NEW,
	FRG_ADD,
	FRG_END,
    };

    void send_init_message(std::list<CCmPartner>::iterator it);
    size_t try_send_filters(std::list<CCmPartner>::iterator it);
    size_t try_send_db_messages(std::list<CCmPartner>::iterator it);
    size_t try_send_variable_messages(std::list<CCmPartner>::iterator it, double ActTime);
    
    void ProcessINIT_ACC(CCmPartner &src_it);
    void ProcessACC(CCmPartner &src_it, char **item, unsigned src_id, const char *data1cmd);
    void ProcessINIT(CCmPartner &src_it, char **item, unsigned src_id) ;
    void ProcessMOD(CCmPartner &src_it, char **item, double ActTime);
    void ProcessSET(CCmPartner &src_it, char **item, double ActTime, char const *data);
    void ProcessDEF(CCmPartner &src_it, char **item, double ActTime, char const *data);
    void ProcessUNDEF(CCmPartner &src_it, char **item);
    void ProcessFRG(CCmPartner &src_it, char **item, FRG_kind kind);

    CmIpDescriptorSub msv_control_ip_descriptor;
    Maybe<std::string> msv_control_variable;

    cm::scoped_ptr<CmIPBase> msv_connection;

    // cm::scoped_ptr<CMSIP> msv_control_client;
    // cm::scoped_ptr<Thread> msv_control_thread;

    // Příklad
    //
    // CMS:LCMS2:LCMS1+LCMS2+RMM2
    //
    // group_name = "CMS"
    // msv_master_name = "LCMS2"
    //
    bool msv_need_refresh; // private use
    bool msv_always_connect; // From ConfigFile
    std::string msv_group_name; // From ConfigFile
    std::string msv_master_name; // Bud nastavi uzivatel, nebo se bere z MSvControl
    bool msv_send_mod_to_all;
    bool msv_changed;
    void msv_control_thread_main();

    void msv_refresh_internal();
    void msv_receive_control_message(std::string const &msg);

    bool will_be_saved(CCmVariable const &v);

    template <typename Change>
    bool ChangeVariableTempl(std::string name, uint8_t newPriority, bool add_to_cb, Change change);

    template <typename Change>
    bool ChangePartnerVariableMasterSlave(std::string name, uint8_t newPriority, bool add_to_cb, Change change);

    template <typename Change>
    bool ChangePartnerVariableNoMasterSlave(std::string name, uint8_t newPriority, bool add_to_cb, Change change);

    void switch_master_variables(CCmPartner *new_master);

    struct LockAndNotify
    {
	CCmWorker &self;
	LockAndNotify(CCmWorker &w);
	~LockAndNotify();
    };

    void internalDefineMyVariable(CCmVariable const &v, bool);
    void check_master_slave_control_variable(CCmPartner &p, std::string const &name, CCmVariable const *maybe_var);
    CCmPartner *find_master_partner(std::list<CCmPartner> &list);

    std::string print_partner(CCmPartner const &p);

    CmChanges GetRawChanges(bool, CCmFiltersSet &, CCmFiltersSet &);
    void debug_sending(CCmPartner &, char const *buffer);

    
    // static gpointer libcm_start_PollThread (gpointer p);
};

#endif // CM_CORE_H
