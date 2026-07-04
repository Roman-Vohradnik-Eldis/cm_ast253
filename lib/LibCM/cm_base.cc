#include "cm_base.hh"
#include "cm_core.hh"

void LibCM_GLOBAL_Init() {}
void LibCM_GLOBAL_Done() {}

// CCmWorker::CCmWorker(std::string _IpDesc, unsigned _COM_MODE, unsigned _UDP_ID, double _PingPeriod, double _PingTimeOut,
//           unsigned _LengthMax, unsigned _LengthIdeal, unsigned _BaudSpeedMax, double _BaudSpeedCalcTime, unsigned _RetryCount, double _RetryTime, std::string _MyPrefix, std::string _RemotePrefix,
//           FilterCBFunction _FilterCB_function ,
//           VariableCBFunction _VariableCB_function,
//           uint16_t _CmWorkerFlags, void *_UserPtr, std::string _name)
//     : impl(new CmCore(_IpDesc, _COM_MODE, _UDP_ID, _PingPeriod, _PingTimeOut ,_LengthMax, _LengthIdeal, _BaudSpeedMax, _BaudSpeedCalcTime, _RetryCount, _RetryTime, _MyPrefix, _RemotePrefix, _FilterCB_function,  _VariableCB_function, _CmWorkerFlags, _UserPtr, _name))
// {}
    
CCmWorker::CCmWorker(CMSConfig *config, FilterCBFunction _FilterCB_function,
          VariableCBFunction _VariableCB_function,
          uint16_t _CmWorkerFlags, void *_UserPtr, std::string _name)
    : impl(new CmCore(config, _FilterCB_function,  _VariableCB_function, _CmWorkerFlags, _UserPtr, _name))     
{}
	
CCmWorker::CCmWorker(std::string config_path, std::string section,
          FilterCBFunction _FilterCB_function, 
          VariableCBFunction _VariableCB_function,
          uint16_t _CmWorkerFlags, void *_UserPtr, std::string _name)
    : impl(new CmCore(config_path, section, _FilterCB_function,  _VariableCB_function, _CmWorkerFlags, _UserPtr, _name))
{}

CCmWorker::CCmWorker(CMSConfig *config, cm::function<void(CCmNotification)> notify_cb, uint16_t CmWorkerFlags, std::string name)
    : impl(new CmCore(config, notify_cb, CmWorkerFlags, name))
{}

CCmWorker::~CCmWorker()
{
    delete impl;
}

void CCmWorker::StartUseSaveFile(bool _Blocked) { impl->StartUseSaveFile(_Blocked); }
void CCmWorker::StopUseSaveFile(bool _EnableSaveNow) { impl->StopUseSaveFile(_EnableSaveNow); }

void CCmWorker::Start() { impl->Start(); }
void CCmWorker::Stop() { impl->Stop(); }
void CCmWorker::ClearFilters()  { impl->ClearFilters(); }

void CCmWorker::DelFilter(const char *name) { impl->DelFilter(name); }
void CCmWorker::DelFilter(std::string name) { impl->DelFilter(name); }

// Smaze postupne filtry vsech zadanych jmen pomoci DelFilter
void CCmWorker::DelFilters(std::list<std::string> names) { impl->DelFilters(names); }

// Prida jeden filter
void CCmWorker::AddFilter(CCmFilter *newFilter) { impl->AddFilter(newFilter); }
void CCmWorker::AddFilter(CCmFilter newFilter) { impl->AddFilter(&newFilter); }

// Postupne prida vsechny filtry v seznamu pomoci AddFilter
void CCmWorker::AddFilters(std::list<CCmFilter> *newFilter) { impl->AddFilters(newFilter); }
    
void CCmWorker::SetFilters(std::list<CCmFilter> *newFilter) { impl->SetFilters(newFilter); }
void CCmWorker::SetFilters(std::map<std::string, CCmFilter> *newFilter) { impl->SetFilters(newFilter); }
void CCmWorker::SetFilters(CCmFiltersSet *newFilter) { impl->SetFilters(newFilter); }

CCmFiltersSet CCmWorker::GetMyFilters() { return impl->GetMyFilters(); }

void CCmWorker::DefineMyVariable(CCmVariable const &var, bool _GenerateCB) { impl->DefineMyVariable(var, _GenerateCB); }
void CCmWorker::DefineMyVariable(CCmVariable const *var, bool _GenerateCB) { impl->DefineMyVariable(var, _GenerateCB); }
void CCmWorker::DefineMyVariable(std::string str, bool _GenerateCB) { impl->DefineMyVariable(str, _GenerateCB); }
void CCmWorker::DefineMyVariable(char** params, bool _GenerateCB) { impl->DefineMyVariable(params, _GenerateCB); }
void CCmWorker::DefineMyVariables(VarMap const *newVariables, bool _GenerateCB) { impl->DefineMyVariables(newVariables, _GenerateCB); }

bool CCmWorker::UndefineVariable(std::string name, bool _GenerateCB) { return impl->UndefineVariable(name, _GenerateCB); }
void CCmWorker::ClearMyVariables(bool _GenerateCB) { impl->ClearMyVariables(_GenerateCB); }

void CCmWorker::ChangeVariable(CCmVariable const *var, uint8_t newPriority, bool _GenerateCB)                                    { impl->ChangeVariable(var, newPriority, _GenerateCB); }
void CCmWorker::ChangeVariable      (std::string name, std::string StrParams, uint8_t newPriority)                               { impl->ChangeVariable(name, StrParams, newPriority); }
void CCmWorker::ChangeVariableBool  (std::string name, bool          value, unsigned idx_y, unsigned idx_x, uint8_t newPriority) { impl->ChangeVariableBool(name, value, idx_y, idx_x, newPriority); }
void CCmWorker::ChangeVariableInt   (std::string name, libcm_integer value, unsigned idx_y, unsigned idx_x, uint8_t newPriority) { impl->ChangeVariableInt(name, value, idx_y, idx_x, newPriority); }
void CCmWorker::ChangeVariableFloat (std::string name, double        value, unsigned idx_y, unsigned idx_x, uint8_t newPriority) { impl->ChangeVariableFloat(name, value, idx_y, idx_x, newPriority); }
void CCmWorker::ChangeVariableString(std::string name, std::string   value, unsigned idx_y, unsigned idx_x, uint8_t newPriority) { impl->ChangeVariableString(name, value, idx_y, idx_x, newPriority); }
void CCmWorker::ChangeVariableEnum  (std::string name, int           value, unsigned idx_y, unsigned idx_x, uint8_t newPriority) { impl->ChangeVariableEnum(name, value, idx_y, idx_x, newPriority); }
void CCmWorker::ChangeVariableEnum  (std::string name, std::string   value, unsigned idx_y, unsigned idx_x, uint8_t newPriority) { impl->ChangeVariableEnum(name, value, idx_y, idx_x, newPriority); }

CCmVariable CCmWorker::GetVariable( std::string name, bool *found_and_filled) { return impl->GetVariable(name, found_and_filled); }

void CCmWorker::GetAllVariables(VarMap &var_list, uint8_t FlagMaskTest, uint8_t FlagMaskVal) { impl->GetAllVariables(var_list, FlagMaskTest, FlagMaskVal); }
void CCmWorker::GetMyVariables(VarMap &var_list, uint8_t FlagMaskTest, uint8_t FlagMaskVal) { impl->GetMyVariables(var_list, FlagMaskTest, FlagMaskVal); }
void CCmWorker::GetPartnersVariables(VarMap &var_list, uint8_t FlagMaskTest, uint8_t FlagMaskVal) { impl->GetPartnersVariables(var_list, FlagMaskTest, FlagMaskVal); }
void CCmWorker::AddAllVariables(VarMap &var_list, uint8_t FlagMaskTest, uint8_t FlagMaskVal) { impl->AddAllVariables(var_list, FlagMaskTest, FlagMaskVal); }
void CCmWorker::AddMyVariables(VarMap &var_list, uint8_t FlagMaskTest, uint8_t FlagMaskVal) { impl->AddMyVariables(var_list, FlagMaskTest, FlagMaskVal); }
void CCmWorker::AddPartnersVariables(VarMap &var_list, uint8_t FlagMaskTest, uint8_t FlagMaskVal) { impl->AddPartnersVariables(var_list, FlagMaskTest, FlagMaskVal); }

void CCmWorker::LiveDebugSetControlFiles(std::string Files) { impl->LiveDebugSetControlFiles(Files); }
void CCmWorker::LiveDebugCheckChanges() { impl->LiveDebugCheckChanges(); }

void CCmWorker::SetMaster(std::string const &value) { impl->SetMaster(value); }

std::string CCmWorker::GetMasterName() { return impl->GetMasterName(); }
    
void CCmWorker::GetSomeChanges(std::string const &pattern, VarMap &c, VarMap &u, VarMap &d) { impl->GetSomeChanges(pattern, c, u, d); }
CmChanges CCmWorker::GetChanges() { return impl->GetChanges(); }

std::string const &CCmWorker::MyName() const { return impl->MyName(); }

std::set<std::string> CCmWorker::getCollidingVariables() { return impl->getCollidingVariables(); }

std::string CCmWorker::ListAllVariables(bool printAge) { return impl->ListAllVariables(printAge); }

unsigned CCmWorker::numConnectedPartners() { return impl->numConnectedPartners(); }
std::string CCmWorker::getPartnersPrefixes() { return impl->getPartnersPrefixes(); }

void CCmWorker::DbAcceptService(DbService service_name, DbServiceParams params) { impl->DbAcceptService(service_name, params); }
void CCmWorker::DbSetAcceptedServices(std::map<DbService, DbServiceParams> const &services) { impl->DbSetAcceptedServices(services); }

Maybe<QueryId> CCmWorker::DbSendQueryMaybe(DbService service_name, CmDbQuery const &query) { return impl->DbSendQueryMaybe(service_name, query); }

QueryId CCmWorker::DbSendQuery(DbService service_name, CmDbQuery const &query) { return impl->DbSendQuery(service_name, query); }

void CCmWorker::DbSendResponse(CmDbResponse response) { impl->DbSendResponse(std::move(response)); }

void CCmWorker::DbForwardResponse(DbMessage msg) { impl->DbForwardResponse(msg); }

    
Maybe<QueryId> CCmWorker::DbBeginForwardedQuery(DbService service) { return impl->DbBeginForwardedQuery(service); }
void CCmWorker::DbAddToForwardedQuery(DbMessage msg) { impl->DbAddToForwardedQuery(msg); }

void CCmWorker::DbCancelQuery(QueryId query_id, WeakString reason, bool gen_callback)
{
    impl->DbCancelQuery(query_id, reason, gen_callback);
}

void CCmWorker::ApplyVariableChanges(VariableChangesConstRef changes)
{
    impl->ApplyVariableChanges(changes);
}

void print_changes(CmChanges const &changes, WeakString who)
{
    if (changes.partners_accepted_services)
        cms_ns2_if_print("main", who, 3, "Changes: Available services: '%s'",
                         to_string(cm_concat_first(changes.partners_accepted_services->begin(),
                                                changes.partners_accepted_services->end(),
                                                "', '")).c_str());
    for (CmDbReceivedQuery const &q : changes.received_queries)
        cms_ns2_if_print("main", who, 3, "Changes: %s", to_string(q).c_str());
    for (CmDbReceivedResponse const &q : changes.received_responses)
        cms_ns2_if_print("main", who, 3, "Changes: %s: ", to_string(q).c_str());
    for (DbMessage const &msg : changes.queries_to_forward)
        cms_ns2_if_print("main", who, 3, "Changes: query to forward: %s", to_string(msg).c_str());
    for (DbMessage const &msg : changes.responses_to_forward)
        cms_ns2_if_print("main", who, 3, "Changes: response to forward: %s", to_string(msg).c_str());
    for (std::pair<QueryId const, std::string> const &p : changes.cancelled_queries)
        cms_ns2_if_print("main", who, 3, "Changes: cancelled query %llx: %s", p.first.value, p.second.c_str());
}

void CCmWorker::SaveFileSetPolicy(bool save_all)
{
    impl->SaveFileSetPolicy(save_all);
}

void CCmWorker::SaveFileAddVariables(std::set<std::string> const &vars)
{
    impl->SaveFileAddVariables(vars);
}

CmStatistics CCmWorker::getStatistics()
{
    return impl->getStatistics();
}

