#ifndef CM_BASE_EXT_HH_
#define CM_BASE_EXT_HH_

#if USE_LIB_MUPARSERX

#include "cm_base.hh"
#include "mup_interface.hh"

class CCmWorkerExt : public CCmWorker
{
public: 
    CCmWorkerExt(CmCompatConfig *config, FilterCBFunction _FilterCB_function,
		 VariableCBFunction _VariableCB_function,
		 uint16_t _CmWorkerFlags, void *_UserPtr, std::string _name = std::string());
	
    CCmWorkerExt(std::string config_path, std::string section,
		 FilterCBFunction _FilterCB_function, 
		 VariableCBFunction _VariableCB_function,
		 uint16_t _CmWorkerFlags, void *_UserPtr, std::string _name = std::string());

    CCmWorkerExt(CmCompatConfig *config, cm::function<void(CCmNotification)> notify_cb, uint16_t CmWorkerFlags, std::string name = "");

    // Neni uplne thread-safe, ale to neni potreba
    C_CM_MUP_Interface *GetParserPtr();
};

#endif // USE_LIB_MUPARSERX

#endif // CM_BASE_EXT_HH_
