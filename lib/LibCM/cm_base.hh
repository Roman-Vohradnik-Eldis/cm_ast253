#ifndef CM_BASE_HH_
#define CM_BASE_HH_

#include "cm_connection.hh"
#include "cm_compat_function.hh"
#include "cm_db_connection.hh"
#include "cm_db_state.hh"
#include "cm_db_types.hh"
#include <set>

void LibCM_GLOBAL_Init() __attribute__ ((deprecated));
void LibCM_GLOBAL_Done() __attribute__ ((deprecated));

class CmCore;

typedef void (*FilterCBFunction)(CCmFiltersSet *PartnerFiltersActive,
                                 CCmFiltersSet *PartnerFiltersCreated,
                                 CCmFiltersSet *PartnerFiltersDeleted,
                                 void *_UserPtr);

typedef void (*VariableCBFunction)(VarMap *VariablesCreated,
                                   VarMap *VariablesUpdated,
                                   VarMap *VariablesDeleted,
                                   void *_UserPtr);

struct CmChanges
{
    // Zmeny filtru. Tohle je nastavene pouze v pripade, ze filters_changed == true.
    CCmFiltersSet filters_active;

    // Zmeny promennych.  Pokud se nic nezmnenilo, jsou prazdne.
    VarMap vars_created, vars_updated, vars_deleted;

    bool filters_changed;

    // Nekdo se nas na neco pta. Meli bychom mu odpovedet, jinak mu
    // LibCM casem posle timeout error. Pokud nenastavime
    // CCmWorker::DbAcceptServices(), tak to bude vzdy prazdne.
    std::vector<CmDbReceivedQuery> received_queries;

    // Pokud jsme zavolali CCmWorker::DbSendQuery(), tak tady
    // dostaneme odpoved. Pokud je dlouha, nebo se pomalu generuje,
    // muzeme ji dostavat po castech.
    std::vector<CmDbReceivedResponse> received_responses;

    // Pokud pribyly nebo zanikly nejake sluzby, tak se toto nastavi
    // na seznam vsech aktualne dostupnych sluzeb.  Jinak je
    // Nothing.
    Maybe<std::map<DbService, DbAvailableServiceParams>> partners_accepted_services;

    // Tohle je pro potreby routeru, aby umel zpravy posilat bez toho,
    // aby je musel parsovat.
    std::vector<DbMessage> queries_to_forward;
    std::vector<DbMessage> responses_to_forward;

    // Dotazy, ktere nam nekdo polozil, my treba zrovna generujeme
    // odpoved, ale on si vzpomene, uz odpoved nechce (treba kdyz se
    // odpojil).
    // QueryId -> Message
    std::map<QueryId, std::string> cancelled_queries;

    CmChanges() : filters_changed(false) {}
};

struct CmStatistics
{
    size_t variables_to_send = 0;
};

// (1) Kdyz vytvarim zmeny:
//
// Kdyz definuji nebo menim svoji promennou, dam ji do created.
// Kdyz oddefinuji svoji promennou, dam ji do deleted.
// Kdyz modifikuji cizi promennou, dam ji do updated.
//
// (2) Kdyz dostanu zmeny
//
// V created jsou cizi promenne, ktere nekdo nadefinoval nebo zmenil
// V updated jsou

// Kdyz mi prijde v created a ja ji neznam, tak ji vytvorim. Moje
// promenne muzu vzdy menit tak, ze je dam do created.
//
// Kdyz mi prijde v updated a ja ji neznam, tak vypisu chybu a
// ignoruji ji. Tim menim cizi promenne: Kdyz chci menit
// SCS__ROTATION_REQ, a SCS neni pripojeny, tak nechci, aby se ta
// promenna vytvorila.
//
struct VariableChanges
{
    VarMap created;
    VarMap updated;
    VarMap deleted;

    bool empty() const { return created.empty() && updated.empty() && deleted.empty(); }
};

struct VariableChangesRef
{
    VarMap &created;
    VarMap &updated;
    VarMap &deleted;

    VariableChangesRef(VarMap &c, VarMap &u, VarMap &d) : created(c), updated(u), deleted(d) {}
    VariableChangesRef(VariableChanges &ch) : created(ch.created), updated(ch.updated), deleted(ch.deleted) {}
    VariableChangesRef(CmChanges &ch) : created(ch.vars_created), updated(ch.vars_updated), deleted(ch.vars_deleted) {}    
    bool empty() const { return created.empty() && updated.empty() && deleted.empty(); }
};

struct VariableChangesConstRef
{
    VarMap const &created;
    VarMap const &updated;
    VarMap const &deleted;

    VariableChangesConstRef(VarMap const &c, VarMap const &u, VarMap const &d) : created(c), updated(u), deleted(d) {}
    VariableChangesConstRef(VariableChanges const &ch) : created(ch.created), updated(ch.updated), deleted(ch.deleted) {}
    VariableChangesConstRef(CmChanges const &ch) : created(ch.vars_created), updated(ch.vars_updated), deleted(ch.vars_deleted) {}    
    bool empty() const { return created.empty() && updated.empty() && deleted.empty(); }
};

void print_changes(CmChanges const &, WeakString ns2 = "");

// Kdyz LibCM uzivateli knihovny oznami, ze ma nejake zmeny, preda mu
// strukturu CCmNotification. Tim poskytne informace, aby uzivatel
// zhruba vedel, jake zmeny ma ocekavat.
//
// Pouziva to napriklad cm_router. Tomu muzou prijit tyto callbacky z
// desitek zarizeni najednou a on se podle toho rozhoduje, ktere
// zarizeni drive zpracuje.
//
// Pouziva to napriklad cm_gui, kde GTK pouziva jen jeden thread,
// ktery postupne zpracovava udalosti.  Kdyz nam prijde notifikace z
// LibCM, tak muzeme podle CCmNotification rozhodnotut, s jakou
// prioritou ho do GTK naplanujeme.
struct CCmNotification
{
    bool master_slave_changed;
    
    CCmNotification() : master_slave_changed(false) {}
};

class CCmWorker : public CCmConnection, public CmDbConnection
{
    CCmWorker(CCmWorker const &);
    CCmWorker &operator=(CCmWorker);
protected:
    CmCore *impl;

public:
    CCmWorker(CmCompatConfig *config, FilterCBFunction,
              VariableCBFunction,
              uint16_t _CmWorkerFlags, void *_UserPtr, std::string _name = std::string());
    CCmWorker(std::string config_path, std::string section,
              FilterCBFunction,
              VariableCBFunction,
              uint16_t _CmWorkerFlags, void *_UserPtr, std::string _name = std::string());

    CCmWorker(CmCompatConfig *config, cm::function<void(CCmNotification)> notify_cb, uint16_t flags, std::string name = "");

    virtual ~CCmWorker();

    // Ostatnim zarizenim se posle informace, ze poskytujeme sluzbu
    // zadaneho jmena.  Moznost forward == true vyuziva router:
    // prichozi dotazy budou chodit nenparsovane.
    void DbAcceptService(DbService service_name, DbServiceParams);
    void DbSetAcceptedServices(std::map<DbService, DbServiceParams> const &services);

    // Posle dotaz.  Pokud nikdo neposkytuje zadane `service_name`,
    // vrati to Nothing().  Pokud ano, tak se mu dotaz posle. Vrati to
    // id dotazu. Odpoved prijde v CmChanges s timto id.
    Maybe<QueryId> DbSendQueryMaybe(DbService service_name, CmDbQuery const &query);

    // Posle dotaz.  Pokud nikdo neposkytuje zadane `service_name`,
    // v pristi CmChanges prijde error, ze to nejde poslat.
    QueryId DbSendQuery(DbService service_name, CmDbQuery const &query);

    // Predpoklada se, ze v CmChanges prisel dotaz (s nejakym id), a
    // my na toto id ted odpovidame, jinak to vypise chybu a neudela
    // to nic. Na jeden dotaz muzeme odpovidat opakovane, dokud na
    // vsechny prikazy dotazu neodpovime CM_DB_DONE, cimz se bere
    // odpoved jako kompletni.
    void DbSendResponse(CmDbResponse response);

    // Jako DbSendResponse, ale posilame jiz serializovanou zpravu. Tohle
    // pouziva router.
    void DbForwardResponse(DbMessage msg);
    
    // Zacne posilat zpravu partnerovi Jako DbSendQuery, ale posilame
    // jiz serializovanou zpravu. Tohle pouziva router.
    Maybe<QueryId> DbBeginForwardedQuery(DbService service);
    void DbAddToForwardedQuery(DbMessage msg);

    // Tohle muze zavolat uzivatel kdykoliv po polozeni dotazu.  Do
    // CmChanges zadna odpoved ohledne tohoto dotazu neprijde.
    // Partnerovi se posle informace, ze se tento dotaz rusi.
    void DbCancelQuery(QueryId query_id, WeakString reason, bool gen_callback);

    void        StartUseSaveFile(bool _Blocked = true) ;
    void        StopUseSaveFile(bool _EnableSaveNow = true) ;

    // Co se ma ukladat do SaveFile.
    //
    // Bezna zarizeni zde maji true: Ukladej vsechny moje promenne,
    // ktere maji flag SaveFile.
    //
    // Router zde ma false a nastavuje si jednotlive promenne, ktere
    // chce ukladat.
    //
    void SaveFileSetPolicy(bool save_all);

    // Pokud je `SaveFileSetPolicy` nastaveno na false, tak timhle si
    // vybira, ktere se maji ukladat. Tohle pouzivat router. Na
    // zacatku jsou automaticky pridany vsechny promenne z
    // konfiguraku.
    void SaveFileAddVariables(std::set<std::string> const &vars);

    void        Start() ;
    void        Stop() ;

    void        ClearFilters() ;

    // Smaze filtr daneho jmena
    void        DelFilter(const char *name);
    void        DelFilter(std::string name);

    // Smaze postupne filtry vsech zadanych jmen pomoci DelFilter
    void        DelFilters(std::list<std::string> names);

    // Prida jeden filter
    void        AddFilter(CCmFilter *newFilter);
    void        AddFilter(CCmFilter newFilter);

    // Postupne prida vsechny filtry v seznamu pomoci AddFilter
    void        AddFilters(std::list<CCmFilter> *newFilter);
    
    void        SetFilters(std::list<CCmFilter> *newFilter);
    void        SetFilters(std::map<std::string, CCmFilter> *newFilter);
    void        SetFilters(CCmFiltersSet *newFilter);

    CCmFiltersSet GetMyFilters() ;

    // Definuje nasi promennou podle zadani.
    //
    // Pokud uz ja promennou tohoto jmena mam, tak ji predefinuj.
    // Pokud uz nektery partner promennou tohoto jmena ma, tak ta
    // partnerova promenna se pro nas skryje, dokud ta nase bude
    // nadefinovana.
    //
    // Funkce vzdycky uspeje. Promenna tohoto jmena bude existovat se
    // zadanym typem a hodnotou az do te doby, dokud nezavolame Undefine.
    void        DefineMyVariable(CCmVariable const &var, bool _GenerateCB  = false) ;
    void        DefineMyVariable(CCmVariable const *var, bool _GenerateCB  = false) ;
    void        DefineMyVariable(std::string str, bool _GenerateCB  = false) ;
    void        DefineMyVariable(char** params, bool _GenerateCB  = false) ;
    void        DefineMyVariables(VarMap const *newVariables, bool _GenerateCB = false) ;

    void ApplyVariableChanges(VariableChangesConstRef);
    
    // Oddefinuje nasi promennou.
    //
    // Pokud promenna tohoto jmena neexistuje, nic se neprovede.  Z
    // callbacku uz neprijde zadna zmena ani oddefinovani, dokud ji
    // znovu nenadefinujeme.
    //
    // Vrati true, pokud promenna existovala, jinak false.
    //
    bool        UndefineVariable(std::string name, bool _GenerateCB  = false) ;
    void        ClearMyVariables(bool _GenerateCB  = false) ;
    
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
    void ChangeVariable(CCmVariable const *var, uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF, bool _GenerateCB  = false) ;

    void ChangeVariable      (std::string name, std::string StrParams                                    , uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF) ;
    void ChangeVariableBool  (std::string name, bool          value, unsigned idx_y = 0, unsigned idx_x = 0, uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF) ;
    void ChangeVariableInt   (std::string name, libcm_integer value, unsigned idx_y = 0, unsigned idx_x = 0, uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF) ;
    void ChangeVariableFloat (std::string name, double        value, unsigned idx_y = 0, unsigned idx_x = 0, uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF) ;
    void ChangeVariableString(std::string name, std::string   value, unsigned idx_y = 0, unsigned idx_x = 0, uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF) ;
    void ChangeVariableEnum  (std::string name, int           value, unsigned idx_y = 0, unsigned idx_x = 0, uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF) ;
    void ChangeVariableEnum  (std::string name, std::string   value, unsigned idx_y = 0, unsigned idx_x = 0, uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF) ;

    // void ChangeVariable(CCmVariable *var, uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF, bool _GenerateCB  = false) ;

    // Ziska aktualni hodnotu promenne.
    //
    // Pozor: Opakovane volani GetVariable muze davat ruzne hodnoty,
    // pokud mezitim nekdo z partneru tuto promennou modifikoval.
    CCmVariable   GetVariable( std::string name, bool *found_and_filled) ;

    void GetAllVariables(VarMap &var_list, uint8_t FlagMaskTest=0, uint8_t FlagMaskVal=0) ;
    void GetMyVariables(VarMap &var_list, uint8_t FlagMaskTest=0, uint8_t FlagMaskVal=0) ;
    void GetPartnersVariables(VarMap &var_list, uint8_t FlagMaskTest=0, uint8_t FlagMaskVal=0) ;
    void AddAllVariables(VarMap &var_list, uint8_t FlagMaskTest=0, uint8_t FlagMaskVal=0) ;
    void AddMyVariables(VarMap &var_list, uint8_t FlagMaskTest=0, uint8_t FlagMaskVal=0) ;
    void AddPartnersVariables(VarMap &var_list, uint8_t FlagMaskTest=0, uint8_t FlagMaskVal=0) ;
    void LiveDebugSetControlFiles(std::string Files=std::string()) ;
    void LiveDebugCheckChanges(void) ;

    void SetMaster(std::string const &value) ;

    std::string ListAllVariables(bool printAge = true);
    
    std::string GetMasterName();
    
    void GetSomeChanges(std::string const &pattern, VarMap &c, VarMap &u, VarMap &d);
    CmChanges GetChanges();

    std::string const &MyName() const;

    std::set<std::string> getCollidingVariables();

    unsigned    numConnectedPartners();
    std::string getPartnersPrefixes();

    CmStatistics getStatistics();
};

#endif // CM_BASE_HH_
