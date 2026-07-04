#ifndef CM_BASE_PARTNER_H
#define CM_BASE_PARTNER_H

#include "cm_header_internal.hh"
#include "cm_base_functs.hh"
#include "cm_base_variable.hh"
#include "cm_base_filter.hh"
#include "cm_base_filters_set.hh"
#include "cm_queue.hh"
#include "cm_setcmd.hh"
#include "cm_maybe.hh"
#include "cm_fragments.hh"
#include "cm_base_varevent.hh"
#include "cm_partner_variables.hh"
#include "cm_recv_buffer_state.hh"
#include "cm_send_buffer_state.hh"

struct CmCommunParams
{
    double   RqPingPeriod;
    double   RqPingTimeOut;
    unsigned RqLengthMax;
    unsigned RqLengthIdeal;
    unsigned RqBaudSpeedMax;
    double   RqBaudSpeedCalcTime;
};

class CCmPartner
{
  public:
    unsigned ID; // TCP client_id / UDP_ID

    // For debugging
    std::string libCM_name;

    // Posbirame informace z CMS spojeni a slozime z toho tento
    // string, ktery se pouziva pri debugovacich vypisech/errorech.
    std::string address_string;

    bool received_init_acc;
    bool received_init;
    bool sent_init_acc;
    bool sent_init;

    std::string received_dictionary_hash; // or empty

    Maybe<std::string> received_fragments;

    // Na zacatknu, nebo kdyz poslemem INIT, tak init_wait_until je
    // cas, kdy to budeme zkouset znova.
    double   init_wait_until;

    double TimeLastRecv;
    double TimeLastSent; // FOR TCP
    double SendBlockUntil; // FOR TCP

    // ProtocolFlags: To, co nam poslal ve zprave INIT. Neni pouzito
    int ProtocolFlags;

    // ProtocolVersion: To, co nam poslal ve zprave INIT. Z toho se spocita
    // UsedVersion.
    int ProtocolVersion;

    // UsedVersion: Verze ktera se pouziva: Minimum z moji verze a
    // ProtocolVersion.
    int UsedVersion;

    // UseSetIds: Nastavuje se pri incializaci, pokud ProtocolVersion >= 1
    bool UseSetIds;

    // UseCompression: Nastavuje se pri incializaci, pokud ProtocolVersion >= 2
    bool UseCompression;

    // Dostaneme ve ZPRAVE INIT. Pouziva se napriklad pro MasterSlave.
    std::string partner_name;

    // Aktualni hodnota
    bool master_slave_control_variable_value;

    // PartnerVarPrefix: Dostaneme od nej pri zprave INIT, muze byt prazdne.
    // Pridavame to pridavame k nazvum parterove promenne, pokud dostaneme DEF a
    // SET. A urezavame to, pokud posilame MOD.
    std::string PartnerVarPrefix;

    // MyVarPrefix: Pokud máme prefix A__, a on posle fitr, ze chce A__*, tak z
    // toho musíme oříznout to A__.
    std::string MyVarPrefix;

    size_t n_send_iterations = 0;

    // Parametry komunikace, co nam partner poslal v INIT
    // CmCommunParams his_rq_commun_params;
    double   RqPingPeriod;
    double   RqPingTimeOut;
    unsigned RqLengthMax;
    unsigned RqLengthIdeal;
    unsigned RqBaudSpeedMax;
    double   RqBaudSpeedCalcTime;

    // Parametry komunikace, ktere se pouzivaji. Spocitaji se z jeho a mojich,
    // aby vyhovovaly obema.
    // CmCommunParams used_commun_params;
    double   PingPeriod;
    double   PingTimeOut;
    unsigned LengthMax;
    unsigned LengthIdeal;
    unsigned BaudSpeedMax;
    double   BaudSpeedCalcTime;

    double last_sent_time = 0;

    // Asi je pro UDP?
    std::list<unsigned> SendAckPackets;

    // FOR TCP
    std::list < DoubleList > BaudSpeedHistory;

    // myFiltersChange jsou filtry, co jsem mu poslal. Pokud si ja zmenim
    // filtry, tak vsem partnerum nastavim myFiltersChange na true. Casem se
    // jinej thread podiva na myFiltersChange, a pokud je true, tak partnerovi
    // posle zmeny, updatne myFilters, a nastavi myFiltersChange na false.
    CCmFiltersSet myFilters;
    bool myFiltersChange;

    // Pokud partner pošle změnu/definici filtrů, tak se
    // (1) změní PartnerFilters.
    // (2) nastavi se ParterFiltersChanged na true.
    // (3) Zavola se MergeMyVariablesByChangedFilters, aby se mu poslali nove promenne.
    //
    // Az se bude volat callback, tak ParterFiltersChanged se shodi na false.
    //
    CCmFiltersSet PartnerFilters;

    // Ze mame zkontrolovat filtry do GetChanges
    bool PartnerFiltersChanged;

    // Ze mam zavolat MergeMyVariablesByChangedFilters
    bool filters_changed_now = false;

    // Asi: Tohle se nastavuje, pokud je ve fronte k odeslani zprava s prioritou
    // 0 nebo 1. TODO: Na co se to pouziva.
    bool waitingPriorityMessage;

    // Tohle se pouziva pouze v pripade, ze mu zrovna posilame zpravu pres UDP:
    //
    // RecACCuntil: Cas, do ktereho cekame na odpoved. Az ten cas nastane, tak
    // zkusime zpravu poslat jeste jednou.
    //
    // RecACCretry: Kolikrat mu zpravu jeste zkusime poslat, nez ho odpojime.
    // Postupne dekrementujeme.
    //
    // RecACCpacket_no: Cislo zpravy, dosadi se z globalniho pocitadla. Cekame na
    // odpoved v ktere bude toto cislo.
    //
    // RecACCmsg: Zprava, ktera byla poslana. To mame pro pripad, kdyby ji bylo
    // treba zopakovat.
    //
    double RecACCuntil;
    unsigned RecACCretry;
    unsigned RecACCpacket_no; // 0=unused
    std::string RecACCmsg_txt;

    // DisconnectMe: Pokud neco hodne selze, tak to nastavime na true. Jiny
    // thread se na to casem podiva a odpoji nas.
    bool DisconnectMe;

    std::map<unsigned,std::string> RecvFragData;
    unsigned RecvFragCount;

    std::unique_ptr<RecvBufferState> recv_buffer_state;
    
    std::unique_ptr<SendBufferState> send_buffer_state;
    
    PartnerVariablesState variables_state;

    void     init(double ActTime, unsigned connection_id);

    // Tohle volame po tom, co jsme my oddefinovali nasi promennou.
    bool UndefineMyVariable(std::string const &name);

    // Tohle volame po tom, co my zmenime (update/define/redefine)
    // nasi promemnou nebo jiny partner modifikuje nasi
    // promennnou. Polozky v EventFields se pripravi partnerovi k
    // odeslani.  Vrati true, bude k dispozici neco noveho k odeslani.
    bool ChangeMyVariable(CCmVariable const &var, Maybe<uint8_t> newPriority = Nothing());

    // Toto volame po tom, co tento partner modifikoval nasi
    // promennou.  Polozky v EventFields se pouziji tak, ze se nebou
    // posilat, pokud jsou zrovna ve fronte.  Vrati true, bude k
    // dispozici neco noveho k odeslani.
    bool AddMyVariableModification(CCmVariable const &var);

    // Projdu vsechny moje promenne, ktere parter chce. Podivam se, v
    // jakym stavu je ma, podle toho, co jsem mu uz poslal (treba je
    // vubec nezna, nebo zna jenom cast).  Vrati true, pokud mu mame
    // neco poslat.
    bool MergeMyVariablesByChangedFilters(VarMap const *myWorkerVariables);
    
    bool ClearFilters(void);
    bool ChangeFilter(const char *name);
    bool UnDefineFilter(const char *name);
    bool DefineFilter(const char *name);

    // haveDataToSend: Spocita, za kolik sekund bude treba odesilat data
    // 
    // Pokud ihned, vrati 0.0.
    // Pokud nikdy, vrati nejakou velkou hodnotu, nyni je to 1000.0
    double   haveDataToSend(double ActTime, Maybe<double> max_variable_send_period);

    double   haveDataToSendPrecise(double ActTime);
    
    // @param ret_messages: Where to put messages.  ret_messages are
    // of the form of {"SET,VAR1,10","UNDEF,VAR2"}
    // void     GenerateDataToSend(double ActTime, std::list<std::string> *ret_messages);

    // ChangePartnerVariable je na to, kdyz chci ja zmenit partnerovu promennou.
    // Updatne se hisVariables a pridaji zmeny do hisVariablesToSend.
    //
    // Ignorujou se EventFields, EventWhat, var se cela diffne s existujici
    // hodnotou.
    bool ChangePartnerVariable(std::string name, CCmVariable *var, uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF);

    std::string debug();

// private:
    // void AddHisVariablesToSend(double ActTime, std::list<std::string> *ret_messages, unsigned &length);
    // void AddMyVariablesToSend(double ActTime, std::list<std::string> *ret_messages, unsigned &length);
    // void AddFragmentToSend(double ActTime, std::list<std::string> *ret_messages, unsigned &length);

    // std::string GenerateChangeMyVariable(std::string const &name, VariableEvent const &var, uint8_t Priority, unsigned set_id) const;
    // std::string GenerateChangePartnerVariable(std::string const &reduced_name, CCmVariable &var, uint8_t Priority) const;

    // bool ChangeMyVariableDoUpdate
    // (std::string const &name, CCmVariable *var, bool full_define, bool dont_send, uint8_t newPriority);

    // bool add_msg_to_ret_messages(std::string var_name, std::string const &msg, std::list<std::string> *ret_messages, unsigned &length);
    // void update_my_variable_state_after_send(double ActTime, std::string name, unsigned set_id, VariableEvent const &var_event);
    // void update_his_variable_state_after_send(double ActTime, std::string name, CCmVariable const &var);
};

#endif // CM_BASE_PARTNER_H
