#if USE_LIB_MUPARSERX

#include "cm_base_ext.hh"
#include "cm_core.hh"
#include "cm_ext_muparser_hook.hh"
#include "cm_scheduler.hh"

CCmWorkerExt::CCmWorkerExt(CMSConfig *config, FilterCBFunction _FilterCB_function,
             VariableCBFunction _VariableCB_function,
             uint16_t _CmWorkerFlags, void *_UserPtr, std::string _name)
    : CCmWorker(config, _FilterCB_function,  _VariableCB_function, _CmWorkerFlags, _UserPtr, _name)

{
    CmMuParserHook *hook = new CmMuParserHook(_CmWorkerFlags, &impl->LiveDebug, impl->LiveDebugFlags, _name);
    VarMap all_variables;
    GetAllVariables(all_variables);
    std::set<std::string> ext_vars = hook->ReadConfig_Ext(config, impl->ExtSECTION_VAR, impl->ExtSECTION_DEF, impl->ExtSECTION_MUP, impl->ExtSECTION_FUN, all_variables);
    impl->SetExtHook(hook);
    impl->SaveFileAddVariables(ext_vars);
}
	
CCmWorkerExt::CCmWorkerExt(std::string config_path, std::string section,
             FilterCBFunction _FilterCB_function, 
             VariableCBFunction _VariableCB_function,
             uint16_t _CmWorkerFlags, void *_UserPtr, std::string _name)
    : CCmWorker(config_path, section, _FilterCB_function,  _VariableCB_function, _CmWorkerFlags, _UserPtr, _name)
{
    CmMuParserHook *hook = new CmMuParserHook(_CmWorkerFlags, &impl->LiveDebug, impl->LiveDebugFlags, _name);
    VarMap all_variables;
    GetAllVariables(all_variables);
    std::set<std::string> ext_vars;
    {
        CMSConfig config(config_path,"m4");
        config.FindSection(section);
        ext_vars = hook->ReadConfig_Ext(&config, impl->ExtSECTION_VAR, impl->ExtSECTION_DEF, impl->ExtSECTION_MUP, impl->ExtSECTION_FUN, all_variables);
    }
    impl->SetExtHook(hook);
    impl->SaveFileAddVariables(ext_vars);
}

CCmWorkerExt::CCmWorkerExt(CMSConfig *config, cm::function<void(CCmNotification)> notify_cb, uint16_t CmWorkerFlags, std::string name)
    : CCmWorker(config, notify_cb, CmWorkerFlags, name)
{
    CmMuParserHook *hook = new CmMuParserHook(CmWorkerFlags, &impl->LiveDebug, impl->LiveDebugFlags, name);
    VarMap all_variables;
    GetAllVariables(all_variables);
    std::set<std::string> ext_vars = hook->ReadConfig_Ext(config, impl->ExtSECTION_VAR, impl->ExtSECTION_DEF, impl->ExtSECTION_MUP, impl->ExtSECTION_FUN, all_variables);
    impl->SetExtHook(hook);
    impl->SaveFileAddVariables(ext_vars);
}

// Neni uplne thread-safe, ale to neni potreba
C_CM_MUP_Interface *CCmWorkerExt::GetParserPtr()
{
    return static_cast<CmMuParserHook*>(impl->GetExtHook())->GetParserPtr();
}

#endif // USE_LIB_MUPARSERX
