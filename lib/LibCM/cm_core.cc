#include "cm_core.hh"
#include "cm_base_partner.hh"
#include "cm_generate_changes.hh"
#include "cm_ip.hh"
#include "cm_recv_buffer_state.hh"
#include "cm_string.hh"
#include "cm_std_string.hh"
#include "cm_utils.hh"
#include "cm_maybe.hh"
#include "cm_thread.hh"
#include "cm_var_utils.hh"
#include "cm_debug.hh"
#include "cm_generate_filter_changes.hh"
#include <unistd.h>
#include <functional>
#include <fstream>
#include "cm_view.hh"

#define LIBCM_UNINITED_MSG "WARNING!!!!: please call : LibCM_GLOBAL_Init() before use LibCM ..."

// #define DEBUG_LOCK ([&](){WORKER_DEBUG("cmlock", 4, "+ %d: Locking ...", __LINE__);; CM_Mutex->Lock(); WORKER_DEBUG("cmlock", 4, "+ %d: Locked", __LINE__);; })()
// #define DEBUG_UNLOCK ([&](){WORKER_DEBUG("cmlock", 4, "+ %d: Unlocking", __LINE__);; CM_Mutex->Unlock(); })()
// #define DEBUG_TRYLOCK ([&](){WORKER_DEBUG("cmlock", 4, "+ %d: TryLock", __LINE__);; return CM_Mutex->TryLock(); })()

// #define DEBUG_LOCK CM_Mutex->Lock()
// #define DEBUG_UNLOCK CM_Mutex->Unlock()
// #define DEBUG_TRYLOCK CM_Mutex->TryLock()

void write_debug_message_info(std::ostream &out, const char *type, View<uint8_t> msg, unsigned partner_id)
{
    View<uint8_t> head(msg.begin(), msg.begin() + std::min(msg.size(), (size_t)30));
    View<uint8_t> hex_head(msg.begin(), msg.begin() + std::min(msg.size(), (size_t)16));
    out << format("%s %lld %02u %5zu %-30s %s\n",
                  type,
                  (long long)get_current_time_usec(),
                  partner_id,
                  msg.size(),
                  replace_nonprintable(head).c_str(),
                  str_to_hex(hex_head).c_str())
        << std::flush;
}

CCmPartner *find_partner_by_id(std::list <CCmPartner> &partners, unsigned connection_id)
{
    for (std::list <CCmPartner>::iterator it = partners.begin(); it != partners.end(); ++it)
	if (it->ID == connection_id)
	    return &*it;
    return 0;
}

struct DebugScopedLock
{
    CMSMutex *CM_Mutex;
    std::string MyLibCM_name;
    bool need_lock;
    int64_t time_start = 0;
    
    DebugScopedLock(CMSMutex &m, std::string name, bool need_lock = true)
        : CM_Mutex(&m), MyLibCM_name(name), need_lock(need_lock)
    {
        if (need_lock)
        {
            CM_Mutex->Lock();
            time_start = get_current_time_usec();
        }
    }
    ~DebugScopedLock()
    {
        if (need_lock)
        {
            CM_Mutex->Unlock();
            int64_t elapsed_usec = get_current_time_usec() - time_start;
            if (elapsed_usec > 2000)
            {
                cms_ns2_if_print("libcm", MyLibCM_name, 3, "LibCM locked for %.3f msec", elapsed_usec / 1000.0);
            }
        }
    }
};

struct DebugScopedUnlock
{
    CMSMutex *CM_Mutex;
    std::string MyLibCM_name;
    DebugScopedUnlock(CMSMutex &m, std::string name) : CM_Mutex(&m), MyLibCM_name(name)
    {
        CM_Mutex->Unlock();
    }
    ~DebugScopedUnlock()
    {
        CM_Mutex->Lock();
    }
};

int var_value(CCmVariable &v)
{
    if (v.getType() == 'i')
    {
        libcm_integer x = 0;
        assert(v.getVariableInt(x));
        return x;
    }
    return -1;
}

// void LibCM_GLOBAL_AddMyWorker(CCmWorker *worker, )
// {
//     // LibCM_add_regular_task(worker, [=](){ worker->CB__ThreadOneLoop(true); });
// }

// void LibCM_GLOBAL_RemoveMyWorker(CCmWorker *worker)
// {
//     // LibCM_remove_regular_task(worker);
// }

// static double last_time_debug = 0;
// static std::ofstream debug_out("/dev/shm/worker-debug");

// void CCmWorker::debug_size()
// {
//     DEBUG_LOCK;
//     size_t partner_var_count = 0;
//     for (CCmPartner &partner : Partners)
//     {
//         partner_var_count += partner.myVariables_State.size();
//         partner_var_count += partner.hisVariables.size();
//     }
//     debug_out << "Worker " << MyLibCM_name
//               << ": my=" << myVariables.size()
//               << ", partner_variables=" << PartnerVariables.size()
//               << ", partners_sum=" << partner_var_count
//               << std::endl;
//     DEBUG_UNLOCK;
// }


// Called by CMS until it returns true
// gboolean libcm_CALLBACK_onServerConnect (cms_client_t *client, void * data)
// {
//   return ((CCmWorker*)data)->onServerConnect(client);
// }

// gboolean libcm_CALLBACK_onServerDisconnect (cms_client_t *client, void * data)
// {
//   return ((CCmWorker*)data)->onServerDisconnect(client);
// }


// gpointer libcm_start_RecvThread (gpointer p)
// {
//     add_debug_thread("recv");
//   try
//   {
//     cms_ns_if_print("libcm",4,"RecvThread start");
//     ((CCmWorker*)p)->RecvThread ();
//     cms_ns_if_print("libcm",4,"RecvThread stop");
//   }
//   catch (std::runtime_error &e)
//   {
//       cms_ns_if_print("libcm",1,"RecvThrad runtime error %s", e.what());
//       throw;
//   }
//   catch (...)
//   {
//       cms_ns_if_print("libcm",1,"RecvThread error");
//     throw;
//   }
//   return NULL;
// }

// gpointer CCmWorker::libcm_start_PollThread (gpointer p)
// {
//     add_debug_thread("poll");
//     // debug_printf("Starting poll thread");
//     try
//     {
// 	cms_ns_if_print("libcm",4,"PollThread start");
// 	((CCmWorker*)p)->PollThread ();
// 	cms_ns_if_print("libcm",4,"PollThread stop");
//     }
//     catch (std::runtime_error &e)
//     {
// 	cms_ns_if_print("libcm",1,"PollThread runtime error %s", e.what());
// 	throw;
//     }
//     catch (...)
//     {
// 	cms_ns_if_print("libcm",1,"PollThread error");
// 	throw;
//     }
//     return NULL;
// }

// #if (!LIBCM_CB_THREAD_ONCE)
// gpointer libcm_start_CB__Thread (gpointer p)
// {
//   try
//   {
//     cms_ns_if_print("libcm",4,"CB__Thread start");
//     ((CCmWorker*)p)->CB__Thread ();
//     cms_ns_if_print("libcm",4,"CB__Thread stop");
//   }
//   catch (...)
//   {
//     cms_ns_if_print("libcm",1,"CB__Thread error");
//     throw;
//   }
//   return NULL;
// }
// #endif




void CmCore::CCmInternalNew(unsigned _COM_MODE, unsigned _UDP_ID, double _PingPeriod, double _PingTimeOut,
                               unsigned _LengthMax, unsigned _LengthIdeal, unsigned _BaudSpeedMax, double _BaudSpeedCalcTime, unsigned _RetryCount, double _RetryTime, std::string _MyPrefix, std::string _RemotePrefix,
                               void (*_FilterCB_function)(class CCmFiltersSet *PartnerFiltersActive, class CCmFiltersSet *PartnerFiltersCreated, class CCmFiltersSet *PartnerFiltersDeleted, void *_UserPtr), 
                               void (*_VariableCB_function)(VarMap *VariablesCreated, VarMap *VariablesUpdated, VarMap *VariablesDeleted, void *_UserPtr),
                               uint16_t _CmWorkerFlags, void *_UserPtr, std::string _name )

{
    // printf("CCmInternalNew from params\n");
    MyLibCM_name = _name;
    msv_changed = false;
    use_cmip_ping = false;
    MyPid = getpid();
    char WStrPID[50];
    sprintf(WStrPID,"%lu",(unsigned long)MyPid);
    MyPidStr = std::string(WStrPID);
    MyPrgName = libCM_GetLastNameFromPath(libCM_GetProgramPath());
// debug_printf("PRG:%lu,%s",(unsigned long)MyPid,MyPrgName.c_str());


    CM_Mutex = new CMSMutex();
    // SaveFileMutex = new CMSMutex();

// #if (!LIBCM_CB_THREAD_ONCE)
//     CB__Condition = new CMSCondition();
//     CB__Thread_handler = NULL;
// #else
//     CB__Condition = LibCM_GLOBAL_Data->GLOBAL_CB__Condition;
// #endif

    // PollCondition = new CMSCondition();

    // PollCondLive.init(PollCondition, 0.0, true, true);
    // PollCondSend.init(PollCondition, 0.0, true, true);
    // PollCondPing.init(PollCondition, 0.0, true, true);
    // PollCondSave.init(PollCondition, 0.0, true, true);
    // PollCondExt.init(PollCondition, 0.0, true, true);

    PollCondLive = new PollCond(cm::bind_front(&CmCore::EvalLiveDebug, this), MyLibCM_name+"/Live");
    PollCondSend = new PollCond(cm::bind_front(&CmCore::EvalSend, this), MyLibCM_name+"/Send");
    PollCondPing = new PollCond(cm::bind_front(&CmCore::EvalPing, this), MyLibCM_name+"/Ping");
    PollCondSave = new PollCond(cm::bind_front(&CmCore::EvalSaveFile, this), MyLibCM_name+"/Save");
    PollCondDb   = new PollCond(cm::bind_front(&CmCore::EvalDb, this), MyLibCM_name+"/Db");
    PollCondExt = new PollCond(cm::bind_front(&CmCore::EvalExt, this), MyLibCM_name+"/Ext");
    PollCondRecv = new PollCond(cm::bind_front(&CmCore::EvalRecv, this), MyLibCM_name+"/Recv");
    PollCondMasterSlave = new PollCond(cm::bind_front(&CmCore::EvalMasterSlave, this), MyLibCM_name+"/MS");
    PollCondCB = new PollCond(cm::bind_front(&CmCore::cb_thread_iter, this), MyLibCM_name+"/CB");

    db_state = new CmDbState(MyLibCM_name);
    DbChanged = false;
    
    SaveFile_Type = 0;   // none
    SaveFile_Params = std::string("");
    SaveFile_LastChangeMinDelay = 0.5;
    SaveFile_FirstChangeMaxDelay = 10.0;

    SaveFile_LOAD = false;
    SaveFile_USE  = false;
    SaveFile_LastUnsavedChange=0.0;
    SaveFile_FirstUnsavedChange=0.0;

    ProtocolFlags = 0; // for feature
    
    // ProtocolVersion = 0; // Initial version
    // ProtocolVersion = 1; // SetIDs introduced
    // ProtocolVersion = 2; // Compression introduced
    ProtocolVersion = 3; // DBCompression changed

    PingPeriod = RqPingPeriod = _PingPeriod;
    PingTimeOut = RqPingTimeOut = _PingTimeOut;
    LengthMax = RqLengthMax = _LengthMax;
    LengthIdeal = RqLengthIdeal = _LengthIdeal;
    BaudSpeedMax = RqBaudSpeedMax = _BaudSpeedMax;
    BaudSpeedCalcTime = RqBaudSpeedCalcTime = _BaudSpeedCalcTime;
    
    RetryCount = _RetryCount;
    RetryTime = _RetryTime;
    MyDefaultPrefix = _MyPrefix;
    RemotePrefix = _RemotePrefix;
    if (MyDefaultPrefix.empty()) MyDefaultPrefix = std::string("");
    if (RemotePrefix.empty()) RemotePrefix = std::string("");

    FilterCB_function = _FilterCB_function;
    VariableCB_function = _VariableCB_function;
    CmWorkerFlags = _CmWorkerFlags;
    UserPtr = _UserPtr;

    UDP_ID = _UDP_ID;



    stop_threads = false;
    has_started = false;
    // RecvThread_handler = NULL;
    // PollThread_handler = NULL;
    UDP_MODE = true;
    UDP_PACKET_COUNTER = 1;
    COM_MODE = _COM_MODE; // 0=text, 1=binary

    // SendBlockUntil = 0.0; // UDP
    PartnerFiltersChanged = false;

    UDP_MODE = false;
    
    // IP = NULL;
    // CLIENT = NULL;
    // SERVER = NULL;
    // char **item = g_strsplit (IpDesc.c_str(), ":", 0);
    // if (item && item[0])
    // {

    //     if (!strcmp (item[0], "server")
    //         || !strcmp (item[0], "server-text")
    //         || !strcmp (item[0], "server-raw")
    //         || !strcmp (item[0], "client")
    //         || !strcmp (item[0], "client-text")
    //         || !strcmp (item[0], "client-raw"))
    //     {
    //         UDP_MODE = false;
    //         if (!strcmp (item[0], "server-text") || !strcmp (item[0], "client-text"))
    //         {
    //             if (COM_MODE!=0) // 0=text, 1=binary
    //             {
    //                 cms_ns_error("libcm","COM_MODE and cms server/client type incompatible");
    //                 throw 202;
    //             }
    //         }
    // 	    else
    // 	    {
    //             if (COM_MODE!=1) // 0=text, 1=binary
    //             {
    //                 cms_ns_error("libcm","COM_MODE and cms server/client type incompatible");
    //                 throw 201;
    //             }
    //             cms_ns_error("libcm","BINARY mode is unsupported now");
    //             throw 100;
    //         }

    //     }
    // 	else
    // 	{
    //         // CMSIP
    //         if (UDP_ID == 0)
    //         {
    //             cms_ns_error("libcm","For CMSIP must be defined UDP_ID and greather than 0");
    //             throw 101;
    //         }

    //         char WStrUDPID[50]; sprintf(WStrUDPID,"%d",UDP_ID);
    //         IpDesc_Changed = IpDesc;
    //         IpDesc_Changed+=":udp_id:"+std::string(WStrUDPID);
    //         cm_std_string_replace(IpDesc_Changed, ":", "_");

    //     }
    //     g_strfreev (item);
    // }
    LiveDebugInit();
    DEBUG_FLAG_DUMP_FILTERS_MY           = LiveDebug.getFlagMask('M');
    DEBUG_FLAG_DUMP_FILTERS_PARTNER_EACH = LiveDebug.getFlagMask('E');
    DEBUG_FLAG_DUMP_FILTERS_PARTNER_ALL  = LiveDebug.getFlagMask('L');
    DEBUG_FLAG_PRINT_COMUN_RECV          = LiveDebug.getFlagMask('R');
    DEBUG_FLAG_PRINT_COMUN_SEND          = LiveDebug.getFlagMask('S');
    DEBUG_FLAG_PRINT_COMUN_FRAG          = LiveDebug.getFlagMask('F');
    DEBUG_FLAG_PRINT_COMUN_PING_RECV     = LiveDebug.getFlagMask('P');
    DEBUG_FLAG_PRINT_COMUN_PING_SEND     = LiveDebug.getFlagMask('p');
    DEBUG_FLAG_PRINT_COMUN_ACC_RECV      = LiveDebug.getFlagMask('A');
    DEBUG_FLAG_PRINT_COMUN_ACC_SEND      = LiveDebug.getFlagMask('a');
    DEBUG_FLAG_PRINT_COMUN_CONN          = LiveDebug.getFlagMask('N');
    DEBUG_FLAG_VAR_CREATED               = LiveDebug.getFlagMask('C');
    DEBUG_FLAG_VAR_DELETED               = LiveDebug.getFlagMask('D');
    DEBUG_FLAG_VAR_CHANGED               = LiveDebug.getFlagMask('c');
    DEBUG_FLAG_VAR_ACTUAL_FULL_STATE     = LiveDebug.getFlagMask('l');
//  DEBUG_FLAG_PRINT_EXT_CREATE          = LiveDebug.getFlagMask('1');
//  DEBUG_FLAG_PRINT_EXT_DELETE          = LiveDebug.getFlagMask('0');
//  DEBUG_FLAG_PRINT_EXT_UPDATE          = LiveDebug.getFlagMask('2');
}

std::string const &CmCore::MyName() const
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    return m_MyName;
}

void CmCore::CCmInternalNew(CMSConfig *config, 
                               void (*_FilterCB_function)(class CCmFiltersSet *PartnerFiltersActive, class CCmFiltersSet *PartnerFiltersCreated, class CCmFiltersSet *PartnerFiltersDeleted, void *_UserPtr), 
                               void (*_VariableCB_function)(VarMap *VariablesCreated, VarMap *VariablesUpdated, VarMap *VariablesDeleted, void *_UserPtr),
                               uint16_t _CmWorkerFlags, void *_UserPtr , std::string _name)
{
    // printf("CCmInternalNew from config\n");
//  Connection = new CMSIP(CMS_FLAG_BOTH, myConfig->GetValueString("IP"));
  MyLibCM_name = _name;

#ifdef USE_LIB_ZSTD
  send_buffer_dict_hash = "yes";
#else
  send_buffer_dict_hash = "no";
#endif

  unsigned _UDP_ID = 0;

  config->FindItem("IP");
  ip_descriptor.descriptor.ip_descriptors = parse_ip_descriptor_list(CMSConfigGetLine2(*config));
  if (config->FindItem2("IPOnlyWhenMaster"))
      ip_descriptor.only_when_master = parse_master_slave_params(CMSConfigGetLine2(*config));
  if (config->FindItem2("IPUseSingleConnection"))
      ip_descriptor.descriptor.use_single_connection = config->GetValueBool();

  if (config->FindItem2("UDP_ID")) // nepovinne
  {
      _UDP_ID = config->GetValueInt("UDP_ID");
  }
  unsigned _COM_MODE          = config->GetValueInt("COM_MODE");
  if (config->FindItem2("UseCMIPPing"))
      use_cmip_ping = config->GetValueBool("UseCmIPPing");
  double   _PingPeriod        = config->GetValueDouble("PingPeriod");
  double   _PingTimeOut       = config->GetValueDouble("PingTimeOut");
  unsigned _LengthMax         = config->GetValueInt("LengthMax");
  unsigned _LengthIdeal       = config->GetValueInt("LengthIdeal");
  unsigned _BaudSpeedMax      = config->GetValueInt("BaudSpeedMax");
  double   _BaudSpeedCalcTime = config->GetValueDouble("BaudSpeedCalcTime");
  unsigned _RetryCount        = config->GetValueInt("RetryCount");
  double   _RetryTime         = config->GetValueDouble("RetryTime");

  std::string _MyPrefix       = std::string("");
  std::string _RemotePrefix   = std::string("");
  if (config->FindItem2("MyPrefix")) // nepovinne
  {
    _MyPrefix = config->GetValueString("MyPrefix");
  }
  if (config->FindItem2("MyName")) //nepovinne
  {
      m_MyName = config->GetValueString("MyName");
  }
  if (config->FindItem2("RemotePrefix")) // nepovinne
  {
    _RemotePrefix = config->GetValueString("RemotePrefix");
  }
  if (config->FindItem2("VariableSendPeriod"))
  {
      max_variable_send_period = config->GetValueDouble();
  }
  else
  {
      cms_ns_if_print("libcm", 4, "VariableSendPeriod not found");
  }
  // debug_sent_messages_file = NULL;
  // if (config->FindItem2("DebugSentMessages"))
  // {
  //     std::string fname = config->GetValueString("DebugSentMessages");
  //     debug_sent_messages_file = fopen(fname.c_str(), "w");
  //     if (!debug_sent_messages_file)
  //         cms_ns_if_print("libcm", 1, "Failed to open file '%s'", fname.c_str());
  //     cms_ns_if_print("libcm", 2, "Sent messages will be saved to '%s'", fname.c_str());
  // }

  db_state = new CmDbState(MyLibCM_name);
  DbChanged = false;

  // MASTER SLAVE

  if (config->FindItem2("MasterSlaveEnabled"))
  {
      WORKER_DEBUG("msv", 3, "Enabled");
      use_master_slave = config->GetValueBool("MasterSlaveEnabled");
  }
  if (config->FindItem2("MasterSlaveAlwaysConnect"))
  {
      msv_always_connect = config->GetValueBool("MasterSlaveAlwaysConnect");
  }
  msv_send_mod_to_all = false;
  if (config->FindItem2("MasterSlaveMODPolicy"))
  {
      std::string policy = config->GetValueString("MasterSlaveMODPolicy");
      if (policy == "SendToAll")
      {
	  msv_send_mod_to_all = true;
          db_state->set_master_slave_policy(CM_DB_SEND_TO_ANY);
      }
      else if (policy == "SendToMaster")
      {
	  msv_send_mod_to_all = false;
          db_state->set_master_slave_policy(CM_DB_SEND_TO_MASTER);
      }
      else
      {
	  throw std::runtime_error("Unknown MasterSlaveMODPolicy: Expected SendToAll/SendToMaster");
      }
  }
  if (config->FindItem2("MasterSlaveControlGroup"))
  {
      msv_group_name = config->GetValueString("MasterSlaveControlGroup");
  }
  if (config->FindItem2("MasterSlaveControlVariable"))
  {
      msv_control_variable = config->GetValueString("MasterSlaveControlVariable");
  }
  if (config->FindItem2("MasterSlaveControlIP"))
  {
      if (msv_group_name.empty())
          throw std::runtime_error("Missing MasterSlaveControlGroup");
      std::string l = config->GetValueString("MasterSlaveControlIP");
      msv_control_ip_descriptor.ip_descriptors = parse_ip_descriptor_list(l);
  }
  if (use_master_slave)
  {
      for (std::string const &s : msv_control_ip_descriptor.ip_descriptors)
          WORKER_DEBUG("msv", 3, "Starting control IP: %s", s.c_str());
  }

  CCmInternalNew(_COM_MODE, _UDP_ID, _PingPeriod, _PingTimeOut ,_LengthMax, _LengthIdeal, _BaudSpeedMax, _BaudSpeedCalcTime, _RetryCount, _RetryTime, _MyPrefix, _RemotePrefix, _FilterCB_function, _VariableCB_function, _CmWorkerFlags,_UserPtr,_name);

  unsigned _SaveFile_Type = 0;
  std::string _SaveFile_Params = std::string("");
  double _SaveFile_LastChangeMinDelay = 0.5;
  double _SaveFile_FirstChangeMaxDelay = 10.0;

  if (config->FindItem2("DebugSentMessagesFile"))
  {
      sent_messages_file.reset(new std::ofstream(config->GetValueString()));
  }
  
  if (config->FindItem2("CompressionDebugFile"))
  {
      send_buffer_settings.debug_fname = config->GetValueString();
  }
  if (config->FindItem2("CompressionDictionary"))
  {
      std::string fname = config->GetValueString();
      libCM_CreateDirectoryFromPath(fname.c_str());
      if (access(fname.c_str(), F_OK) == 0)
      {
          // File exists
          send_buffer_settings.dictionary = read_whole_file(fname);
          send_buffer_dict_hash = compute_md5_string(*send_buffer_settings.dictionary);
          cms_ns_if_print("libcm", 3, "Using compression directory, md5=%s", send_buffer_dict_hash.c_str());
      }
      else
      {
          send_buffer_settings.write_dictionary_fname = fname;
      }
      send_buffer_settings.write_dictionary_max_size = config->GetValueInt("CompressionDictionaryWriteSize");
  }
  if (config->FindItem2("DecompressionDictionary"))
  {
      std::string fname = config->GetValueString();
      libCM_CreateDirectoryFromPath(fname.c_str());
      if (access(fname.c_str(), F_OK) == 0)
      {
          // File exists
          recv_buffer_settings.dictionary = read_whole_file(fname);
          recv_buffer_dict_hash = compute_md5_string(*recv_buffer_settings.dictionary);
          cms_ns_if_print("libcm", 3, "Using decompression directory, md5=%s", recv_buffer_dict_hash.c_str());
      }
  }

  send_buffer_settings.use_compression = false;
  if (config->FindItem2("CompressionEnabled"))
  {
      send_buffer_settings.use_compression = config->GetValueBool();

      if (send_buffer_settings.use_compression)
      {
          send_buffer_settings.compression_level = config->GetValueInt("CompressionLevel");
      }
  }

  if (!is_compression_supported())
  {
      if (send_buffer_settings.use_compression)
      {
          cms_ns_if_print("libcm", 3, "Compression not supported, you must compile libCM with zstd");
          send_buffer_settings.use_compression = false;
      }
  }

  if (config->FindItem2("SaveFileType"))
  {
    std::string SaveFile_TypeStr = config->GetValueString("SaveFileType");
           if (!strcasecmp(SaveFile_TypeStr.c_str(),"NO"))
    {
      _SaveFile_Type = 0;
    } else if (!strcasecmp(SaveFile_TypeStr.c_str(),"LOCAL"))
    {
      _SaveFile_Type = 1;
    } else if (!strcasecmp(SaveFile_TypeStr.c_str(),"REMOTE_SCP"))
    {
      _SaveFile_Type = 2;
    } else {
      WORKER_DEBUG("libcm",1,"libcm(%s):unknown SaveFileType \"%s\"",MyLibCM_name.c_str(),SaveFile_TypeStr.c_str());
    }
  }
  if (_SaveFile_Type)
  {
    if (config->FindItem2("SaveFileParams"))
    {
      _SaveFile_Params = config->GetValueString("SaveFileParams");
    }
    if (config->FindItem2("SaveFileLastChangeMinDelay"))
    {
      _SaveFile_LastChangeMinDelay = config->GetValueDouble("SaveFileLastChangeMinDelay");
    }
    if (config->FindItem2("SaveFileFirstChangeMaxDelay"))
    {
      _SaveFile_FirstChangeMaxDelay = config->GetValueDouble("SaveFileFirstChangeMaxDelay");
    }
    InitSaveFile(_SaveFile_Type, _SaveFile_Params, _SaveFile_LastChangeMinDelay, _SaveFile_FirstChangeMaxDelay);
  }

  if (config->FindItem2("ExtSections"))
  {
    ExtSECTION_VAR = config->GetValueString("ExtSections");
    ExtSECTION_DEF = config->GetValueString2 ();
    if (!ExtSECTION_DEF.empty())
    {
      ExtSECTION_MUP = config->GetValueString2 ();
    }
  }
  ExtSECTION_FUN.clear();
  if (config->FindItem2("ExtFunctions"))
  {
    ExtSECTION_FUN = config->GetValueString("ExtFunctions");
  }

  if (config->FindItem2("LiveDebugControlFiles"))
  {
    std::string Files = config->GetValueString("LiveDebugControlFiles");
    std::string F2;
    while (  !(F2 = config->GetValueString2()).empty() )
    {
      Files += "," + F2;
    }
    LiveDebugSetControlFiles(Files);
  } else {
    LiveDebugSetControlFiles("/tmp/,CM_LIVE_DEBUG");
  }
  LiveDebugCheckChanges();

  if (config->FindItem2("MyVariables"))
  {
    config->GetValueString("MyVariables");
    if (config->FindSection2(config->GetValueString("MyVariables"))) // if find, then immediately switch  !!!!!!!!!!!!! at end!!!
    {
      LoadConfigMyVariables(config);
    }
  }
}

     CmCore::CmCore(CMSConfig *config, 
                          void (*_FilterCB_function)(class CCmFiltersSet *PartnerFiltersActive, class CCmFiltersSet *PartnerFiltersCreated, class CCmFiltersSet *PartnerFiltersDeleted, void *_UserPtr),
                          void (*_VariableCB_function)(VarMap *VariablesCreated, VarMap *VariablesUpdated, VarMap *VariablesDeleted, void *_UserPtr),
                          uint16_t _CmWorkerFlags, void *_UserPtr , std::string _name)
{
  // LibCM_GLOBAL_TryInit();
  use_master_slave = false;
  master_partner = NULL;
  msv_always_connect = false;
  msv_need_refresh = false;
  m_use_notify_cb = false;
  need_call_cb = false;
  CCmInternalNew(config, _FilterCB_function,  _VariableCB_function, _CmWorkerFlags, _UserPtr,_name);
}

     CmCore::CmCore(std::string config_path, std::string section,
                         void (*_FilterCB_function)(class CCmFiltersSet *PartnerFiltersActive, class CCmFiltersSet *PartnerFiltersCreated, class CCmFiltersSet *PartnerFiltersDeleted, void *_UserPtr), 
                         void (*_VariableCB_function)(VarMap *VariablesCreated, VarMap *VariablesUpdated, VarMap *VariablesDeleted, void *_UserPtr),
                         uint16_t _CmWorkerFlags, void *_UserPtr , std::string _name)
{
  // LibCM_GLOBAL_TryInit();
  use_master_slave = false;
  master_partner = NULL;
  msv_always_connect = false;
  msv_need_refresh = false;
  m_use_notify_cb = false;
  need_call_cb = false;
  CMSConfig *config = new CMSConfig(config_path,"m4");
  config->FindSection(section);
  CCmInternalNew(config, _FilterCB_function,  _VariableCB_function, _CmWorkerFlags, _UserPtr, _name);
  delete config;
}

CmCore::CmCore(CMSConfig *config, cm::function<void(CCmNotification)> notify_cb, uint16_t flags, std::string name)
{
    // LibCM_GLOBAL_TryInit();
    use_master_slave = false;
    master_partner = NULL;
    msv_always_connect = false;
    msv_need_refresh = false;
    m_notify_cb = notify_cb;
    m_use_notify_cb = true;
    need_call_cb = false;
    CCmInternalNew(config, NULL,  NULL, flags, NULL, name);
}

// Dulezita konvence: Kdyz EvalXXX zamkne mutex, tak si otestuje,
// jestli neni nastaveno stop_threads.  Pokud jo tak ihned skoncej.
//
void CmCore::Stop()
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);

    // Informace interni thready, aby na nic nesahaly
    stop_threads = true;

    // Nyni jen optimalizace: Zacnou se ukoncovat prijimaci thready

    if (msv_connection)
        msv_connection->shutdown();

    if (connection)
        connection->shutdown();
}

CmCore::~CmCore()
{
    Stop();

    {
        DebugScopedLock lock(*CM_Mutex, MyLibCM_name);

        // Nejprve ukoncime msv_connection, protoze pouziva PollCondCB
        msv_connection = 0;

        // Nejdrive ukoncime connection, protoze pouziva PollCondRecv
        connection = 0;
    }

    // Odted uz zadny vnitrni thread nebude sahat na CmCore, vyjma
    // toho, ze zamkne, zkontroluje si, ze stop_threads = true, a zase
    // vybehne.

    // Odregistrujeme vsechny callbacky z planovace.
    PollCondLive = 0;
    PollCondSend = 0;
    PollCondPing = 0;
    PollCondSave = 0;
    PollCondDb = 0;
    PollCondExt = 0;
    PollCondRecv = 0;
    PollCondMasterSlave = 0;
    PollCondCB = 0;

    // Nyni jsou vsechny vnitrni thready ukoncene. Tedy nikdo nebude
    // sahat na polozky v CmCore.
    
    LiveDebugDone(true);

    delete CM_Mutex;
}

void CmCore::debug_sending(CCmPartner &p, char const *buffer)
{
    // if (!debug_sent_messages_file)
    //     return;

    // // Replace newlines. I am not sure if it is needed, messages
    // // should not contain newlines...
    // std::string copy;
    // if (strchr(buffer, '\n'))
    // {
    //     copy = buffer;
    //     for (char &c : copy)
    //         if (c == '\n')
    //             c = ' ';
    //     buffer = copy.c_str();
    // }
    
    // std::string p_name = p.partner_name;
    // if (p_name.empty())
    //     p_name = format("Client%u", p.ID);
    // fprintf(debug_sent_messages_file, "%lld %10s %6zu %s\n",
    //         (long long)get_current_time_usec(),
    //         p_name.c_str(),
    //         strlen(buffer),
    //         buffer);
    // fflush(debug_sent_messages_file);
}

void CmCore::InitSaveFile(unsigned _SaveFile_Type, std::string _SaveFile_Params, double _SaveFile_LastChangeMinDelay, double _SaveFile_FirstChangeMaxDelay)
{
  SaveFile_Type = _SaveFile_Type;
  SaveFile_Params = _SaveFile_Params;
  SaveFile_LastChangeMinDelay = _SaveFile_LastChangeMinDelay;
  SaveFile_FirstChangeMaxDelay = _SaveFile_FirstChangeMaxDelay;
}

double CmCore::GetActualTimeDouble()
{
  return libCM_GetActualTimeDouble();
}


void CmCore::erasePartner(std::list <CCmPartner>::iterator it_d, bool SendDisconnectUDP)
{
  // printf("nowDisconnected\n");
  if ((UDP_MODE)&&(SendDisconnectUDP))
  {
      std::string msg = "DISCONNECT";
      SendTextMessage(view_bytes(msg),0,0,DEBUG_FLAG_PRINT_COMUN_SEND,it_d->ID, 0);
  }

  // Vzdy prepocitame filtry
  if (it_d->ClearFilters())
      PartnerFiltersChanged = true;

  db_state->disconnect_partner(it_d->ID);
  generate_callback();
  PollCondSend->Wake();

  if (use_master_slave)
  {
      // Pokud se odpojil Master, tak zkusime zvolime jineho, jinak nedelame nic
      if (master_partner && &*it_d == master_partner)
      {
          CCmPartner *new_master = NULL;
          if (msv_always_connect && !Partners.empty())
          {
              // Find any other partner, if possible
              CM_for (CCmPartner &p, Partners)
              {
                  if (&p != master_partner)
                  {
                      new_master = &p;
                      break;
                  }
              }
          }
          WORKER_DEBUG("msv", 3, "Master %p has disconnected, Settings new master to %p", master_partner, new_master);
          switch_master_variables(new_master);
          master_partner = new_master;

          // NotifyCallback();
	  generate_callback();
      }
      Partners.erase(it_d);
  }
  else
  {
      // Vola se mj. NotifyCallback()
      std::set<std::string> his_colliding_variables;
      VarMap his_vars;
      unsigned id = it_d->ID;
      std::string his_address = it_d->address_string;
      std::swap(it_d->variables_state.hisVariables, his_vars);
      std::swap(it_d->variables_state.his_colliding_variables, his_colliding_variables);

      // Nejdrive smazeme
      Partners.erase(it_d);

      // Ted se postarame o zanik promennych.
      for (VarMap::iterator it = his_vars.begin(); it != his_vars.end(); ++it)
      {
          if (contains(his_colliding_variables, it->first))
          {
              WORKER_DEBUG("libcm", 2, "Colliding variable %s was undefined because partner disconnected: %s", it->first.c_str(), his_address.c_str());
          }
          else
          {
              VariableDeleted_CB(&it->second, id, false);
          }
          tryDefineSomeCollidingVariable(it->first);
      }
      generate_callback();
  }
  if (UDP_MODE)
      RecalculateUsedInitValues(0);
}


void CmCore::erasePartnerByConnectionId(unsigned connection_id, bool SendDisconnectUDP)
{
  for (std::list <CCmPartner>::iterator it = Partners.begin(); it != Partners.end(); )
  {
    std::list <CCmPartner>::iterator it_d = it;
    it++;
    if (it_d->ID == connection_id)
    {
      erasePartner(it_d, SendDisconnectUDP);
    }
  }
}


void CmCore::nowConnected(unsigned connection_id, std::string address_str)
{
  double ActTime = GetActualTimeDouble();
  erasePartnerByConnectionId(connection_id, false);
  CCmPartner newPartner;
  WORKER_DEBUG("libcm", 3, "nowConnected: %s", address_str.c_str());

  newPartner.init(ActTime, connection_id);
  newPartner.libCM_name = MyLibCM_name;

  newPartner.PingPeriod = newPartner.RqPingPeriod = RqPingPeriod;
  newPartner.PingTimeOut = newPartner.RqPingTimeOut = RqPingTimeOut;
  newPartner.LengthMax = newPartner.RqLengthMax = RqLengthMax;
  newPartner.LengthIdeal = newPartner.RqLengthIdeal = RqLengthIdeal;
  newPartner.BaudSpeedMax = newPartner.RqBaudSpeedMax = RqBaudSpeedMax;
  newPartner.BaudSpeedCalcTime = newPartner.RqBaudSpeedCalcTime = RqBaudSpeedCalcTime;

  newPartner.address_string = address_str;

  if (myFilters.size()>0)
  {
    newPartner.myFiltersChange = true;
  }

  Partners.push_back(std::move(newPartner));

  if (use_master_slave)
  {
      WORKER_DEBUG("msv", 4, "New Client connected: ID=%u", connection_id);
      msv_refresh_internal();
  }

  generate_callback();
  PollCondPing->Wake();
  PollCondSend->Wake();
}

// Tyhle dve funkce vytvreji string, ktery se pouziva pro debugovani.
//
// std::string create_client_address(unsigned connection_id, CMSClient &client)
// {
//     char buffer[128]; 
//     snprintf(buffer, sizeof buffer, "(id=%u,port=0x%x,dest=%s:0x%x)",
// 	     connection_id,
// 	     client.getLocalPort(),
// 	     client.getRemoteAddr().c_str(),
// 	     client.getRemotePort());
//     return buffer;
// }

// // Kdyz jsem server
// std::string create_client_address(unsigned connection_id, cms_client_t *client)
// {
//     char buffer[128]; 
//     snprintf(buffer, sizeof buffer, "(id=%u,port=0x%x,dest=%s:0x%x)",
// 	     connection_id,
// 	     cms_client_get_local_port(client),
// 	     cms_client_get_remote_addr(client),
// 	     cms_client_get_remote_port(client));
//     return buffer;
// }

// gboolean CmCore::onServerConnect(cms_client_t *client)
// {
//     DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
//     // if (!DEBUG_TRYLOCK) return false;
//     unsigned connection_id = client->connection_id + 2;
//     WORKER_DEBUG("libcm",3,"SERVER: NEW CLIENT CONNECTION - connection_id=%d \"%s\"",connection_id, cms_client_get_remote_addr(client));

//     if (LiveDebugFlags & DEBUG_FLAG_PRINT_COMUN_CONN)
//     {	
// 	char xstr[64000];
// 	sprintf(xstr,"info:TCP server Connected (%d)",connection_id);
// 	LiveDebugPrintANY(DEBUG_FLAG_PRINT_COMUN_CONN, connection_id, "R:", xstr);
//     }


//     nowConnected(connection_id, create_client_address(connection_id, client));
//     // DEBUG_UNLOCK;
//     return true;
// }

// gboolean CmCore::onServerDisconnect(cms_client_t *client)
// {
//     if (!DEBUG_TRYLOCK) return false;
//     unsigned connection_id = client->connection_id + 2;
//     cms_ns_if_print("libcm",5,"libcm(%s):SERVER: DISCONNECT - connection_id=%d", MyLibCM_name.c_str(),connection_id);
//     if (LiveDebugFlags & DEBUG_FLAG_PRINT_COMUN_CONN)
//     {
//         char xstr[64000];
//         sprintf(xstr,"info:TCP server DISConnected (%d)",connection_id);
//         LiveDebugPrintANY(DEBUG_FLAG_PRINT_COMUN_CONN, connection_id, "R:", xstr);
//     }
//     erasePartnerByConnectionId(connection_id,false);
//     DEBUG_UNLOCK;
//     if (PartnerFiltersChanged) NotifyCallback();
//     return true;
// }

// void CmCore::onClientConnect(void)
// {
//   unsigned connection_id = 2;
//   cms_ns_if_print("libcm",5,"libcm(%s):CONNECT - TCP client",MyLibCM_name.c_str());
//   DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
//     if (LiveDebugFlags & DEBUG_FLAG_PRINT_COMUN_CONN)
//     {
//        char xstr[64000];
//        sprintf(xstr,"info:TCP Connected (%d)",connection_id);
//        LiveDebugPrintANY(DEBUG_FLAG_PRINT_COMUN_CONN, connection_id, "R:", xstr);
//     }
//     nowConnected(connection_id, create_client_address(connection_id, *CLIENT));
// }

// void CmCore::onClientDisconnect(void)
// {
//   unsigned connection_id = 2;
//   cms_ns_if_print("libcm",5,"libcm(%s):DISCONNECT - TCP client",MyLibCM_name.c_str());
//   DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
//     if (LiveDebugFlags & DEBUG_FLAG_PRINT_COMUN_CONN)
//     {
//        char xstr[64000];
//        sprintf(xstr,"info:TCP DISConnected (%d)",connection_id);
//        LiveDebugPrintANY(DEBUG_FLAG_PRINT_COMUN_CONN, connection_id, "R:", xstr);
//     }
//   erasePartnerByConnectionId(connection_id,false);
// }



void debug_ext_cb(std::string MyLibCM_name, VarMap &c, VarMap &u, VarMap &d)
{
    CM_for (VarMap::value_type &p, c)
	WORKER_DEBUG("libcm_cb", 22, "VariableGoCB created %s", print_var(p.second).c_str());
    CM_for (VarMap::value_type &p, u)
	WORKER_DEBUG("libcm_cb", 22, "VariableGoCB updated %s", print_var(p.second).c_str());
    CM_for (VarMap::value_type &p, d)
	WORKER_DEBUG("libcm_cb", 22, "VariableGoCB deleted %s", print_var(p.second).c_str());
}

// void CmCore::VariableGoCB(VarMap *VariablesCreated, VarMap *VariablesUpdated, VarMap *VariablesDeleted)
// {
//     assert(VariablesCreated);
//     assert(VariablesUpdated);
//     assert(VariablesDeleted);

//     if (!stop_threads)
//     {
// 	if (m_ext_hook)
// 	{
// 	    // debug_ext_cb(MyLibCM_name, *VariablesCreated, *VariablesUpdated, *VariablesDeleted);
// 	    m_ext_hook->VariableGoCB(VariablesCreated, VariablesUpdated, VariablesDeleted);
// 	}
// 	if (!m_just_notify && VariableCB_function)
// 	{
// 	    debug_ext_cb(MyLibCM_name, *VariablesCreated, *VariablesUpdated, *VariablesDeleted);
// 	    (*VariableCB_function)(VariablesCreated, VariablesUpdated, VariablesDeleted, UserPtr);
// 	}
//     }
// }


// TODO: Tahle funkce je pouzita jenom po zaniku Partnera
// void CmCore::GenerateDeleteAllVariablesCBNoMasterSlave(VarMap &map)
// {
//     for (VarMap::iterator it_var = map.begin(); it_var != map.end(); ++it_var)
//     {
//         if (contains(src_it->his_colliding_variables, it_d->first))
//         {
//             WORKER_DEBUG("libcm", 2, "Colliding variable %s was undefined because partner disconnected: %s", it_d->first.c_str(), src_it->address_string.c_str());
//         }
//         else
//         {
//             VariableDeleted_CB(&(it_d->second),src_it->ID,false);
//         }
//         tryDefineSomeCollidingVariable(it_d->first);
//     }
//     generate_callback();
// }

// static int64_t cm_get_current_time_usec()
// {
//     struct timeval tv;
//     gettimeofday(&tv, NULL);
//     return (tv.tv_sec * 1000L * 1000L) + (uint64_t)tv.tv_usec;
// }

// vypisuj pouze prvni minutu
// static void debug_callbacks(char const *libcm_name, char const *fun_name, char const *var_name, bool c, bool u, bool d)
// {
//     // static int64_t first_time = cm_get_current_time_usec();
//     // int64_t time_now = cm_get_current_time_usec();
//     // if (time_now - first_time <= 60*1000*1000)
//     // 	printf("%s: %s: %s: %d %d %d\n", fun_name, libcm_name, var_name, c, u, d);
// }

#define DEBUG_CB WORKER_DEBUG("libcm_cb", 22, "Change CB %s: Variable %s", __FUNCTION__, print_var(*var).c_str());

    // debug_callbacks(MyLibCM_name.c_str(),				
    // 		    __FUNCTION__,					
    // 		    var->getName().c_str(),				
    // 		    contains(CB_VariablesCreated, var->getName()),	
    // 		    contains(CB_VariablesUpdated, var->getName()),	
    // 		    contains(CB_VariablesDeleted, var->getName()))

// #define DEBUG_CB (void)0

void CmCore::VariableDeleted_No_CB(CCmVariable *var, unsigned ID)
{
    DEBUG_CB;
    m_var_changes.add_my_changes(VarMap(), VarMap(), make_single_change(*var));
    // CB_VariablesCreated.erase(var->getName());
    // CB_VariablesUpdated.erase(var->getName());
    // CB_VariablesDeleted.erase(var->getName());

    if (LiveDebugFlags & DEBUG_FLAG_VAR_DELETED) LiveDebugPrintVar(DEBUG_FLAG_VAR_DELETED, ID, "-:",var);
    if (LiveDebugFlags & DEBUG_FLAG_VAR_ACTUAL_FULL_STATE) VAR_ACTUAL_FULL_STATE_changes.insert(ID);
}

void CmCore::VariableDeleted_CB(CCmVariable const *var, unsigned ID, bool WakeUpCB__Thread)
{
    DEBUG_CB;
    m_var_changes.add_foreign_changes(VarMap(), VarMap(), make_single_change(*var));
    //debug_printf("%s: VariableDeleted_CB: %s", MyLibCM_name.c_str(), var->getName().c_str());
  // bool changed = false;
  //   std::string name = var->getName();

  // // Z updated se smaze vzdy, to je jasne
  // CB_VariablesUpdated.erase(name);
  // CB_VariablesCreated.erase(name);

  // VarMap::iterator it = CB_VariablesCreated.find(name);
  // if (it != CB_VariablesCreated.end())
  // {
  //     // Nikdo zatim nevi, ze byla vytvorena, takze ji muzeme tajne smazat, jako
  //     // by se nic nestalo
  //     CB_VariablesCreated.erase(it);
  // }
  // else
  // {
  //    CB_VariablesDeleted[name] = *var;
  // }

  // NOTE: Pokud prijde created a pak hned deleted, tak do callbacku pujde
  // deleted bez predchoziho created.
  // CB_VariablesDeleted[name] = CCmVariable();
  // CB_VariablesDeleted[name].setName(name);
  
  if (LiveDebugFlags & DEBUG_FLAG_VAR_DELETED) LiveDebugPrintVar(DEBUG_FLAG_VAR_DELETED, ID, "-:", var);
  if (LiveDebugFlags & DEBUG_FLAG_VAR_ACTUAL_FULL_STATE) VAR_ACTUAL_FULL_STATE_changes.insert(ID);
  if (WakeUpCB__Thread)
  {
      generate_callback();
  }
}


void CmCore::VariableCreated_No_CB(CCmVariable *var, unsigned ID)
{
    DEBUG_CB;
    m_var_changes.add_my_changes(make_single_change(*var), VarMap(), VarMap());
    // Pozor, nechavame ji v deleted, pokud tam je
    // CB_VariablesCreated.erase(var->getName());
    // CB_VariablesUpdated.erase(var->getName());
    // assert(CB_VariablesCreated.find(var->getName()) == CB_VariablesCreated.end());
    // assert(CB_VariablesUpdated.find(var->getName()) == CB_VariablesUpdated.end());

    if (LiveDebugFlags & DEBUG_FLAG_VAR_CREATED) LiveDebugPrintVar(DEBUG_FLAG_VAR_CREATED, ID, "+:",var);
    if (LiveDebugFlags & DEBUG_FLAG_VAR_ACTUAL_FULL_STATE) VAR_ACTUAL_FULL_STATE_changes.insert(ID);
}

void CmCore::VariableCreated_CB(CCmVariable const *var, unsigned ID, bool WakeUpCB__Thread)
{
    DEBUG_CB;
    bool changed = m_var_changes.add_foreign_changes(make_single_change(*var), VarMap(), VarMap());
    //debug_printf("%s: VariableCreated_CB: %s", MyLibCM_name.c_str(), var->getName().c_str());
    // WORKER_DEBUG("libcm_change", 3, "VariableCreated_CB: %s, wakeup=%d", var->getName().c_str(), WakeUpCB__Thread);
  // bool changed = false;
  // std::string name = var->getName();
  // VarMap::iterator it;

  // Snad by nemela bejt tady
  // assert(CB_VariablesCreated.find(name) == CB_VariablesCreated.end());
  // assert(CB_VariablesUpdated.find(name) == CB_VariablesUpdated.end());

  // it = CB_VariablesUpdated.find(name); if (it != CB_VariablesUpdated.end()) CB_VariablesUpdated.erase(it);
  // it = CB_VariablesDeleted.find(name); if (it != CB_VariablesDeleted.end()) CB_VariablesDeleted.erase(it);

//  if (myFilters.test(var, true))
//  {

//   CB_VariablesUpdated.erase(name); // Tady muze byt, pokud toto je redefinice.
  
//   CB_VariablesCreated[name] = *var;
//   CB_VariablesCreated[name].EventFields.clear();
//   changed = true;
// //  }
    if (changed)
    {
	if (LiveDebugFlags & DEBUG_FLAG_VAR_CREATED) LiveDebugPrintVar(DEBUG_FLAG_VAR_CREATED, ID, "+:",var);
	if (LiveDebugFlags & DEBUG_FLAG_VAR_ACTUAL_FULL_STATE) VAR_ACTUAL_FULL_STATE_changes.insert(ID);
	if (WakeUpCB__Thread)
	    generate_callback();
    }
}

// Promennou jsme modifikovali my
void CmCore::VariableUpdated_No_CB(CCmVariable *var, unsigned ID)
{
    DEBUG_CB;
    m_var_changes.add_my_changes(VarMap(), make_single_change(*var), VarMap());
    // Muze byt v created nebo v updated:
    //
    // Napr. prisel update, pridal se do CB_VariablesUpdated. Zatimco tam jeste
    // je, tak my jsme promennou modifikovali. Takze v tomhle pripade musime
    // upravit ten CB, aby nam nevratil starou hodnotu.
    //
    // Pozor: Zaroven muze byt take v deleted, tam ji nechavame.

    // VarMap::iterator it = CB_VariablesCreated.find(var->getName());
    // if (it != CB_VariablesCreated.end())
    // {
    //     it->second = *var;
    //     it->second.EventFields.clear();
    // }
    // else
    // {
    //     VarMap::iterator it = CB_VariablesUpdated.find(var->getName());
    //     if (it != CB_VariablesUpdated.end())
    //     {
    //         CCmVariable copy = it->second;
    //         bool keep = erase_variable_changes_from(copy, *var); // Erase Event Fields
    //         if (keep)
    //         {
    //             it->second = *var;
    //             it->second.EventFields = copy.EventFields;
    //         }
    //         else
    //         {
    //             CB_VariablesUpdated.erase(it);
    //         }
    //     }
    // }
    // assert(CB_VariablesCreated.find(var->getName()) == CB_VariablesCreated.end());
    // CB_VariablesUpdated.erase(var->getName());

    // VarMap::iterator it = CB_VariablesUpdated.find(var->getName());
    // if (it != CB_VariablesUpdated.end())
    // {
    //     CCmVariable &orig_var = it->second;
    //     if (orig_var.isArray() && !orig_var.EventFields.empty())
    //     {
    //         // TODO: Tohle neni uplne presne
    //         CCmVariable new_var = *var;
    //         new_var.EventFields = orig_var.EventFields;
    //         CB_VariablesUpdated[var->getName()] = new_var;
    //     }
    //     else
    //     {
    //         CB_VariablesUpdated.erase(it);
    //     }
    // }

    if (LiveDebugFlags & DEBUG_FLAG_VAR_DELETED) LiveDebugPrintVar(DEBUG_FLAG_VAR_DELETED, ID, "*:",var);
    if (LiveDebugFlags & DEBUG_FLAG_VAR_ACTUAL_FULL_STATE) VAR_ACTUAL_FULL_STATE_changes.insert(ID);
}

// Vola se pri obdrseni zpravy SET
//
// var - nova hodnota promenne, ignoruji se EventFields
void CmCore::VariableUpdated_CB(CCmVariable const *var, CCmVariable const *origo_var, unsigned ID, bool WakeUpCB__Thread, bool RqFiltersConfirm)
{
    DEBUG_CB;
    bool changed = m_var_changes.add_foreign_changes(VarMap(), make_single_change(*var), VarMap());
//     WORKER_DEBUG("libcm", 22, "VariableUpdated_CB begin: %s ", print_var(*var).c_str());
//     //debug_printf("VariableUpdated_CB: %s", var->getName().c_str());
//     bool changed = false;
//     std::string name = var->getName();
//     // VarMap::iterator it;
//     // it = CB_VariablesDeleted.find(name);
//     // if (it != CB_VariablesDeleted.end())
//     //     CB_VariablesDeleted.erase(it);
// //  if ((!RqFiltersConfirm) || myFilters.test(var, true))

//     if ((origo_var == NULL) || (var->diffAllFields(*origo_var)))
//     {
// 	VarMap::iterator it = CB_VariablesCreated.find(name);
// 	if (it != CB_VariablesCreated.end())
// 	{
// 	    // Prepiseme
// 	    CB_VariablesCreated[name] = *var;
// 	    CB_VariablesCreated[name].EventFields.clear();
// 	    changed = true;
// 	    WORKER_DEBUG("libcm", 22, "VariableUpdated_CB: %s created", print_var(*var).c_str());
// 	}
// 	else
// 	{
// 	    it = CB_VariablesUpdated.find(name);
// 	    if (it != CB_VariablesUpdated.end())
// 	    {
// 		// tak tady se zmeny sectou .......
// 		if (!it->second.EventFields.empty())
// 		{
// 		    if (it->second.EventFields.empty())
// 			var->EventFields.clear();
// 		    else
// 			var->addEventFields(&(it->second.EventFields));
// 		}
// 		CB_VariablesUpdated[name] = *var;
// 		changed = true;
// 		WORKER_DEBUG("libcm", 22, "VariableUpdated_CB: %s add event_fields update", print_var(*var).c_str());
// 	    }
// 	    else
// 	    {
// 		// nova zmena
// 		CB_VariablesUpdated[name] = *var;
// 		// CB_VariablesUpdated[name].EventFields.clear();
// 		changed = true;
// 		WORKER_DEBUG("libcm", 22, "VariableUpdated_CB: %s new updated", print_var(*var).c_str());
// 	    }
// 	}
//     }
//     else
//     {
// 	WORKER_DEBUG("libcm", 22, "VariableUpdated_CB: No need to update, origo=%s",
// 		     origo_var ? print_var(*origo_var).c_str() : "NULL");
//     }

    if (changed)
    {
	if (LiveDebugFlags & DEBUG_FLAG_VAR_DELETED) LiveDebugPrintVar(DEBUG_FLAG_VAR_DELETED, ID, "*:",var);
	if (LiveDebugFlags & DEBUG_FLAG_VAR_ACTUAL_FULL_STATE) VAR_ACTUAL_FULL_STATE_changes.insert(ID);
	if (WakeUpCB__Thread)
	    generate_callback();
    }
}






void CmCore::RecalculateUsedInitValues(CCmPartner *src_it)
{
    src_it->PingPeriod        = MIN(RqPingPeriod       , src_it->RqPingPeriod       );
    src_it->PingTimeOut       = MAX(RqPingTimeOut      , src_it->RqPingTimeOut      );
    src_it->LengthMax         = MIN(RqLengthMax        , src_it->RqLengthMax        );
    src_it->LengthIdeal       = MIN(RqLengthIdeal      , src_it->RqLengthIdeal      );
    src_it->BaudSpeedCalcTime = MAX(RqBaudSpeedCalcTime, src_it->RqBaudSpeedCalcTime);
    if ((RqBaudSpeedMax == 0) || (src_it->RqBaudSpeedMax == 0))
    {
        src_it->BaudSpeedMax      = MAX(RqBaudSpeedMax     , src_it->RqBaudSpeedMax     );
    }
    else
    {
        src_it->BaudSpeedMax      = MIN(RqBaudSpeedMax     , src_it->RqBaudSpeedMax     );
    }

    WORKER_DEBUG("libcm",2,"libcm(%s):calculated values TCP(%d):"
                 "  PingPeriod(my:%s,partner:%s=>%s)"
                 "  PingTimeOut(my:%s,partner:%s=>%s)"
                 "  LengthMax(my:%d,partner:%d=>%d)"
                 "  LengthIdeal(my:%d,partner:%d=>%d)"
                 "  BaudSpeedMax(my:%d,partner:%d=>%d)"
                 "  BaudSpeedCalcTime(my:%s,partner:%s=>%s)",
                 MyLibCM_name.c_str(),src_it->ID,
                 cm_std_string_print_short_float(RqPingPeriod       ).c_str(), cm_std_string_print_short_float(src_it->RqPingPeriod       ).c_str(), cm_std_string_print_short_float(src_it->PingPeriod       ).c_str(),
                 cm_std_string_print_short_float(RqPingTimeOut      ).c_str(), cm_std_string_print_short_float(src_it->RqPingTimeOut      ).c_str(), cm_std_string_print_short_float(src_it->PingTimeOut      ).c_str(),
                 RqLengthMax        , src_it->RqLengthMax        , src_it->LengthMax        ,
                 RqLengthIdeal      , src_it->RqLengthIdeal      , src_it->LengthIdeal      ,
                 RqBaudSpeedMax     , src_it->RqBaudSpeedMax     , src_it->BaudSpeedMax     ,
                 cm_std_string_print_short_float(RqBaudSpeedCalcTime).c_str(), cm_std_string_print_short_float(src_it->RqBaudSpeedCalcTime).c_str(), cm_std_string_print_short_float(src_it->BaudSpeedCalcTime).c_str());
}

void CmCore::ProcessINIT_ACC(CCmPartner &src_p)
{
    CCmPartner *src_it = &src_p;
    src_it->received_init_acc = true;

    // Tak ted uz partner zna nase komunikacni parametry, takze posila
    // ping podle toho, jak jsme mu rekli. Takze ho zacneme hlidat.
    CmIPParams params;
    params.ping_period = src_it->PingPeriod * 1E6;
    params.resend_delay = src_it->PingPeriod * 1E6 * 1.5;
    params.timeout = src_it->PingTimeOut * 1E6;
    connection->set_params(src_it->ID, params);

    PollCondSend->Wake();
}

void CmCore::ProcessACC(CCmPartner &src_p, char **item, unsigned src_id, const char *data1cmd)
{
    CCmPartner *src_it = &src_p;
    if (LiveDebugFlags & DEBUG_FLAG_PRINT_COMUN_ACC_RECV)
        LiveDebugPrintANY(DEBUG_FLAG_PRINT_COMUN_ACC_RECV, src_id, "R:", data1cmd);
    unsigned packet_no   = cm_string_get_value_int   (item[1]);
    if (packet_no == src_it->RecACCpacket_no)
    {
        src_it->RecACCpacket_no = 0; // accepted
        //if (src_it->waitingPriorityMessage) PollCondSend.Wake();
        PollCondSend->Wake();
        WORKER_DEBUG("libcm",22,"libcm(%s):packet Accepted src_id=%d packet_no=%d",MyLibCM_name.c_str(),src_id,packet_no);
    }
    else
    {
        if (src_it->RecACCpacket_no == 0)
        {
            WORKER_DEBUG("libcm",22,"libcm(%s):packet Accepted (SECOND TIME) src_id=%d packet_no=%d",MyLibCM_name.c_str(),src_id,packet_no);
        }
        else
        {
            WORKER_DEBUG("libcm",2,"libcm(%s):UNKNOWN packet ACCEPT src_id=%d packet_no=%d but excepted packet_no=%d",MyLibCM_name.c_str(),src_id,packet_no,src_it->RecACCpacket_no);
        }
    }
}

// item[0]: "INIT"
// item[1]: ProtocolVersion
// item[2]: ProtocolFlags
// item[3]: PingPeriod[sec]
// item[4]: PingTimeOut[sec]
// item[5]: LengthMax[bytes]
// item[6]: LengthIdeal[bytes]
// item[7]: BaudSpeedMax[bits/sec]
// item[8]: BaudSpeedCalcTime[sec]
// item[9]: HisPrefix
// item[10]: MyVarPrefix
// item[11]: HisName
void CmCore::ProcessINIT(CCmPartner &src_p, char **item, unsigned src_id)
{
    CCmPartner *src_it = &src_p;
    if (true)
    {
        src_it->RecvFragData.clear();
        src_it->ProtocolVersion     = cm_string_get_value_int   (item[1]);

        src_it->UsedVersion = std::min(src_it->ProtocolVersion, ProtocolVersion);
        src_it->UseSetIds = src_it->UsedVersion >= 1;

        src_it->ProtocolFlags       = cm_string_get_value_int   (item[2]);
        src_it->RqPingPeriod        = cm_string_get_value_double(item[3]);
        src_it->RqPingTimeOut       = cm_string_get_value_double(item[4]);
        src_it->RqLengthMax         = cm_string_get_value_int   (item[5]);
        src_it->RqLengthIdeal       = cm_string_get_value_int   (item[6]);
        src_it->RqBaudSpeedMax      = cm_string_get_value_int   (item[7]);
        src_it->RqBaudSpeedCalcTime = cm_string_get_value_double(item[8]);
        src_it->PartnerVarPrefix    =                std::string(item[9]);
        if (RemotePrefix.size()>0) src_it->PartnerVarPrefix = RemotePrefix;

        src_it->MyVarPrefix         =                std::string(item[10]);
        if (src_it->MyVarPrefix.size()<1) src_it->MyVarPrefix = MyDefaultPrefix;

        std::string his_decompression_capabilities = "no";
        std::string his_compression_dictionary = "";
        
        // std::string received_compression_mode_str = "no";
        if (item[11])
        {
            src_it->partner_name = item[11];

            if (item[12])
            {
                his_decompression_capabilities = item[12];
                if (item[13])
                {
                    his_compression_dictionary = item[13];
                }
            }
        }

        // NOTE: Schvalne nekontrolujeme pocet polozek, aby v budoucich verzich
        // bylo mozno pridavat dalsi.

        RecalculateUsedInitValues(&*src_it);

        if (use_master_slave)
	{
	    WORKER_DEBUG("msv", 4, "Received init from client %s:'%s'",
                         src_it->address_string.c_str(), src_it->partner_name.c_str());
	    msv_refresh_internal(); // Kvuli novemu jmenu
	}

	src_it->received_init = true;
	src_it->sent_init_acc = false;

        RecvBufferSettings used_recv_settings = recv_buffer_settings;
        SendBufferSettings used_send_settings = send_buffer_settings;


        if (his_decompression_capabilities == "no")
        {
            // Partner neumi kompresi, takze mu nic komprimovaneho nesmime poslat
            used_send_settings.use_compression = false;
            cms_ns_if_print("libcm", 4, "Compression disabled");
        }
        else if (his_decompression_capabilities == "yes")
        {
            // Partner umi kompresi, nema slovnik
            used_send_settings.dictionary = Nothing();
            cms_ns_if_print("libcm", 4, "Compression enabled, no dictionary");
        }
        else
        {
            if (send_buffer_dict_hash == his_decompression_capabilities)
            {
                // Partner umi kompresi a ma slovnik stejny jako my
                cms_ns_if_print("libcm", 4, "Compression enabled with dictionary: md5=%s",
                                send_buffer_dict_hash.c_str());
            }
            else
            {
                // Umi, ale ma jiny slovnik
                cms_ns_if_print("libcm", 4, "Compression enabled, no dictionary:"
                                "md5 mismatch: my=%s vs his=%s",
                                send_buffer_dict_hash.c_str(),
                                his_decompression_capabilities.c_str());
                used_send_settings.dictionary = Nothing();
            }
        }

        if (!is_compression_supported())
        {
            cms_ns_if_print("libcm", 4, "Decompression not supported");            
        }
        else if (recv_buffer_dict_hash.empty())
        {
            cms_ns_if_print("libcm", 4, "Decompression supported, no dictionary");
            assert(!used_recv_settings.dictionary);
        }
        else if (recv_buffer_dict_hash == his_compression_dictionary)
        {
            cms_ns_if_print("libcm", 4, "Decompression supported with dictionary md5=%s",
                            recv_buffer_dict_hash.c_str());
            assert(used_recv_settings.dictionary);
        }
        else
        {
            cms_ns_if_print("libcm", 4, "Decompression supported, no dictionary:"
                            "md5 mismatch: my=%s vs his=%s",
                            recv_buffer_dict_hash.c_str(),
                            his_compression_dictionary.c_str());
            used_recv_settings.dictionary = Nothing();
        }

        src_it->recv_buffer_state.reset(new RecvBufferState(used_recv_settings));
        src_it->send_buffer_state.reset(new SendBufferState(';', used_send_settings, src_it->UsedVersion >= 2));

        // Pro databazi nepouzivame slovnik. 
        PartnerParams params = {
            false,
            {
                {},
                false,
                {},
            }
        };
        if (src_it->UsedVersion >= 3)
        {
            params = {
                true,
                {
                    (used_send_settings.use_compression)
                    ? Maybe<int>(used_send_settings.compression_level)
                    : Nothing(),
                    true,
                    used_send_settings.debug_fname,
                }
            };
        }

        if (params.send_settings.compression_level)
        {
            cms_ns_if_print("libcm", 4, "DB compression enabled (level=%d)",
                            *params.send_settings.compression_level);            
        }
        else
        {
            cms_ns_if_print("libcm", 4, "DB compression disabled");
        }
        db_state->connect_partner(src_it->ID, params);

        PollCondSend->Wake();
    }
}

// Command:
//
//     CMD,SetID,Name,Value...
// or
//     CMD,Name,Value...
//
static char *item_ref_setid(CCmPartner &p, char **item) { return p.UseSetIds ? item[1] : 0; }
static char *item_ref_name(CCmPartner &p, char **item) { return p.UseSetIds ? item[2] : item[1]; }
static char **item_ref_values(CCmPartner &p, char **item) { return p.UseSetIds ? item + 3: item + 2; }
static char **item_ref_definition(CCmPartner &p, char **item) { return p.UseSetIds ? item + 2: item + 1; }

static bool split_name_priority(char *item, std::string &name, uint8_t &priority)
{
    char *slash_pos = strchr(item, '/');
    if (!slash_pos)
    {
        name = item;
        priority = 0;
        return false;
    }
    else
    {
        name = std::string(item, slash_pos);
        priority = cm_string_get_value_int(slash_pos + 1);
        return true;
    }
}

// CmMaybe<CCmVariable> apply_modification(CCmVariable const &orig, CmSetCmdMap &setcmd_map, double ActTime, char **changes, unsigned received_setcmd_id)
// {
//     // Spocitej nove hodnoty, a EventFields obsahuje vse co se zmenilo
//     CCmVariable var = orig; // copy
//     var.EventFields.clear();
//     var.setModifTime(ActTime);
//     bool changed = var.setVALUES(changes);
//     var.diffAllFields(orig);

//     if (!changed)
//     {
// 	// Nic se nezmenilo
// 	return CmNothing();
//     }
    
//     // Z EventFields se vyhodi indexy, ktere uz jsme prepsali
//     debug_printf("Changed = true: %s", print_var(var).c_str());
//     changed = setcmd_map.diff_event_fields(var.getName(), var.getArrayY(), var.getArrayX(), var.EventFields, received_setcmd_id);

//     if (!changed)
//     {
// 	// Vsechny zmeny jsou zastarale
// 	return CmNothing();
//     }

//     // Tu promennou `var` nepouzijeme, protoze muze mit
//     // nastaveny i indexy, ktere nechce kvuli set_id.  Takze
//     // vytvorime var2 takto:
//     CCmVariable var2;
//     if (var.EventFields.empty())
//     {
// 	// Meni se vsechny
// 	var2 = var;
//     }
//     else
//     {
// 	// Vem soucasnou hodnotu promenne, a ze zmen aplikujem jen ty aktualni
// 	var2 = orig;
// 	var2.setModifTime(ActTime);
// 	var2.copy_indexes(var, var.EventFields);
//     }
//     return var2;
// }

// Partner chce zmenit nasi promennou
//
// item: MOD
// item: Optional SetID
// item: Varname[/Priority]
// item: value(s)
void CmCore::ProcessMOD(CCmPartner &src_p, char **item, double ActTime)
{
    CCmPartner *src_it = &src_p;
    // assert(src_it != Partners.end());
    src_it->RecvFragData.clear();

    // Nacti setcmd_id
    unsigned received_setcmd_id = 0;
    if (src_it->UseSetIds)
        received_setcmd_id = cm_string_get_value_int(item_ref_setid(*src_it, item));

    // Nacti jmeno a prioritu
    std::string name = item_ref_name(*src_it, item);
    uint8_t newPriority = 0;
    bool newPrioritySeted = split_name_priority(item_ref_name(*src_it, item), name, newPriority);
    if (newPrioritySeted)
	newPriority = CMLIB_VALUE_PRIORITY_UNDEF;
    // printf("MOD %u %s %u\n", received_setcmd_id, name.c_str(), newPriority);

    // Zde osetrime pripad, kdy promenna zmenila typ nebo zanikla.
    // Bud uz mu tato informace byla poslana, a potom to pozname podle
    // set_id, nebo je to teprve ve fronte, tak se tam podivame.
    if (MyVariableToSend const *vts = src_it->variables_state.myVariablesToSend.find_variable(name))
    {
        if (vts->EventWhat == CmEventWhat::undefine || vts->EventWhat == CmEventWhat::define)
        {
            return;
        }
    }
    
    if (!src_it->variables_state.my_vars_setcmd_id.is_safe_to_apply(name, received_setcmd_id))
    {
        return;
    }

    VarMap::iterator it_var = myVariables.find(name);
    if (it_var == myVariables.end())
    {
        // TODO: Tohle neni error: Muze se stat, ze promenna existovala, on ji
        // modifikoval a nez k nam zmena dosla, tak ta promenna zanikla.
        WORKER_DEBUG("libcm", 1, "libcm(%s):cannot modify variable - not defined (%s)",
                        MyLibCM_name.c_str(), name.c_str());
        return;
    }

    if (!(it_var->second.haveFlagExternal() && it_var->second.haveFlagWritable()))
    {
        WORKER_DEBUG("libcm", 1, "libcm(%s):cannot modify variable - (%s) ... External=%s Writable=%s",
                        MyLibCM_name.c_str(),
                        item[1],
                        (it_var->second.haveFlagExternal() ? "Yes" : "No"),
                        (it_var->second.haveFlagWritable() ? "Yes" : "No"));
        return;
    }

    // Spocitej nove hodnoty, a EventFields obsahuje vse co se zmenilo
    CCmVariable var = it_var->second;
    var.EventFields.clear();
    var.setModifTime(ActTime);
    bool changed = var.setVALUES(item_ref_values(*src_it, item));
    var.diffAllFields(it_var->second);

    WORKER_DEBUG("libcm_change", 4, "Received MOD %s from %s", print_var(var).c_str(), print_partner(*src_it).c_str());

    // Pokud se nic neprepsalo
    if (changed)
    {
        // Z EventFields se vyhodi indexy, ktere uz jsme prepsali
       WORKER_DEBUG("libcm", 22, "Changed = true: %s", print_var(var).c_str());
       changed = src_it->variables_state.my_vars_setcmd_id.diff_event_fields(var.getName(),
                                                                             var.getArrayY(),
                                                                             var.getArrayX(),
                                                                             var.EventFields,
                                                                             received_setcmd_id);

       WORKER_DEBUG("libcm_change", 4, "After erasing old set_ids var=%s", print_var(var).c_str());

       // V EventFields zustanou pouze ty indexy, ktere zmenil on

        if (changed)
        {
            CCmVariable var2;
            if (var.EventFields.empty())
            {
                // Meni se vsechny
                var2 = var;
            }
            else
            {
                // Musime vratit zmenu tech polozek, ktere neprosly pres set_id test.
                var2 = it_var->second;
                var2.setModifTime(ActTime);
		var2.EventFields = var.EventFields;
                var2.copy_indexes(var, var.EventFields);
            }
	    WORKER_DEBUG("libcm_change", 4, "Changed after setcmd diff: var2=%s", print_var(var2).c_str());
            // var2.EventWhat = 2;

            // Nasledujici internalChangeMyVariable updatuje i vsechny partnery
            if (newPrioritySeted)
            {
                internalChangeMyVariable(&var2, src_it, true, newPriority);
            }
            else
            {
                internalChangeMyVariable(&var2, src_it, true, CMLIB_VALUE_PRIORITY_UNDEF);
            }
        }
    }
    else
    {
        WORKER_DEBUG("libcm", 22, "Changed = false: %s", print_var(var).c_str());
    }
}

// void CmCore::ProcessMOD(std::list<CCmPartner>::iterator src_it, char **item, double ActTime)
// {
//     assert(src_it != Partners.end());
//     src_it->RecvFragData.clear();

//     // Nacti setcmd_id
//     unsigned received_setcmd_id = 0;
//     if (src_it->UseSetIds)
//         received_setcmd_id = cm_string_get_value_int(item_ref_setid(*src_it, item));

//     // Nacti jmeno a prioritu
//     std::string name = item_ref_name(*src_it, item);
//     uint8_t newPriority = CMLIB_VALUE_PRIORITY_UNDEF;
//     bool newPrioritySeted = split_name_priority(item_ref_name(*src_it, item), name, newPriority);
//     // printf("MOD %u %s %u\n", received_setcmd_id, name.c_str(), newPriority);

//     // Zde osetrime pripad, kdy promenna zmenila typ nebo zanikla.
//     // Bud uz mu tato informace byla poslana, a potom to pozname podle
//     // set_id, nebo je to teprve ve fronte, tak se tam podivame.
//     auto it = find_first_by_name(src_it->myVariablesToSend.begin(),
// 				 src_it->myVariablesToSend.end(),
// 				 name);
//     if (it != src_it->myVariablesToSend.end())
//     {
// 	CCmVariable &v = *it;
// 	if (v.EventWhat == CmEventWhat::undefine || v.EventWhat == CmEventWhat::define) // undefine or define
// 	{
// 	    return;
// 	}
//     }
//     if (!src_it->my_vars_setcmd_id.is_safe_to_apply(name, received_setcmd_id))
// 	return;

//     VarMap::iterator it_var = myVariables.find(name);
//     if (it_var == myVariables.end())
//     {
//         // TODO: Tohle neni error: Muze se stat, ze promenna existovala, on ji
//         // modifikoval a nez k nam zmena dosla, tak ta promenna zanikla.
//         WORKER_DEBUG("libcm", 1, "libcm(%s):cannot modify variable - not defined (%s)",
//                         MyLibCM_name.c_str(), name.c_str());
//         return;
//     }

//     if (!(it_var->second.haveFlagExternal() && it_var->second.haveFlagWritable()))
//     {
//         WORKER_DEBUG("libcm", 1, "libcm(%s):cannot modify variable - (%s) ... External=%s Writable=%s",
//                         MyLibCM_name.c_str(),
//                         item[1],
//                         (it_var->second.haveFlagExternal() ? "Yes" : "No"),
//                         (it_var->second.haveFlagWritable() ? "Yes" : "No"));
//         return;
//     }

//     CmMaybe<CCmVariable> var = apply_modification(it_var->second, src_it->my_vars_setcmd_id, ActTime, item_ref_values(*src_it, item), received_setcmd_id);

//     if (!var)
//     {
// 	// No changes to apply.
// 	return;
//     }
    
//     WORKER_DEBUG("libcm", 22, "Changed after setcmd diff = true: var2 = %s", print_var(*var).c_str());
//     if (newPrioritySeted)
//     {
// 	var->setFlagPriorityChanged(true);
// 	var->setPriority(newPriority);
//     }
//     var->EventWhat = 2; // update

//     internalChangeMyVariable(&*var, &src_it, &it_var->second, newPriority);
    
//     // Dej zmenu do callbacku
//     // VariableUpdated_CB(&*var, &it_var->second, src_it->ID, true/*wakeup_cb*/, false /*unused*/);

//     // // Dej zmenu ostatnim partnerum
//     // for (auto it = Partners.begin(); it != Partners.end(); ++it)
//     // {
//     // 	if (it != src_it)
//     // 	    it->ChangeMyVariable(&*var, false /* send to partner */, newPriority);
//     // }

//     // // Nastav nasi promennou
//     // it_var->second = *var;
// }

void CmCore::check_master_slave_control_variable(CCmPartner &p, std::string const &name, CCmVariable const *maybe_var)
{
    if (!use_master_slave || !msv_control_variable)
        return;

    if (name == *msv_control_variable)
    {
        bool val = false;
        if (maybe_var)
        {
            bool ok = maybe_var->getVariableBool(val);
            if (!ok)
            {
                cms_ns_if_print("msv", 1, "Control variable %s of %s has type '%c'", name.c_str(), p.address_string.c_str(), maybe_var->getType());
            }
        }
        cms_ns_if_print("msv", 3, "Control variable %s of %s has changed to %d", name.c_str(), p.address_string.c_str(), val);
        p.master_slave_control_variable_value = val;
        msv_need_refresh = true;
    }
}

// Partner si zmenil svoji promennou
//
// item: "SET"
// item: Optional SetcmdID
// item: Varname[/Priority]
// item: Values...
void CmCore::ProcessSET(CCmPartner &src_p, char **item, double ActTime, char const *)
{
    // assert(src_it != Partners.end());
    CCmPartner *src_it = &src_p;

    // Nacti setcmd_id
    unsigned setcmd_id = 0;
    if (src_it->UseSetIds)
        setcmd_id = cm_string_get_value_int(item_ref_setid(*src_it, item));

    // Nacti jmeno a prioritu
    std::string name = item_ref_name(*src_it, item);
    uint8_t newPriority = 0;
    bool newPrioritySeted = split_name_priority(item_ref_name(*src_it, item), name, newPriority);

    // Uprav jmeno
    if (!src_it->PartnerVarPrefix.empty())
    {
        name = src_it->PartnerVarPrefix + name;
    }

    // Proved update
    src_it->RecvFragData.clear();
    VarMap::iterator it_var = src_it->variables_state.hisVariables.find(name);

    if (it_var == src_it->variables_state.hisVariables.end())
    {
        // Tohle by se nemelo stat. Nejdriv ten partner mel tu promennou
        // definovat.
        WORKER_DEBUG("libcm", 1, "libcm(%s):cannot set variable - not defined (%s)",
                        MyLibCM_name.c_str(),name.c_str());
        return;
    }

    // Vytvorime novou hodnotu, a spocitame, co bylo zmeneno, tj. nastavime ji EventFields.
    CCmVariable var = it_var->second;
    var.EventFields.clear();
    WORKER_DEBUG("libcm", 22, "ProcessSET: Original var = %s", print_var(var).c_str());
    bool changed = var.setVALUES(item_ref_values(*src_it, item), NULL);
    var.diffAllFields(it_var->second);
    WORKER_DEBUG("libcm", 22, "ProcessSET: After change = %s", print_var(var).c_str());

    WORKER_DEBUG("libcm_change", 4, "Received SET %s from %s", print_var(var).c_str(), print_partner(*src_it).c_str());

    // Update his_vars_setcmd_id. Tohle se provadi, i kdyz nebyla promenna
    // zmenena.
    assert(contains(src_it->variables_state.his_vars_setcmd_id, name));
    src_it->variables_state.his_vars_setcmd_id.at(name) = setcmd_id;

    // Smaz zmeny, ktere jsou nyni stare, z fronty, popripade tu promenou vyhod
    // celou.
    if (!changed)
    {
	WORKER_DEBUG("libcm", 22, "SET %s: Was not changed", name.c_str());
	return;
    }

    // Invariant inv(std::bind(&CCmPartner::debug, *src_it), "Erasing changes to send " + print_var(var));
    erase_variable_changes_from_queue(src_it->variables_state.his_variables_to_send, var);

    // Priprav promennou.  TODO: Mozna, ze setModifTime by se mel volat
    // i kdyz nebyla changed.
    WORKER_DEBUG("libcm", 22, "SET %s: Was changed", name.c_str());
    var.setModifTime(ActTime);
    var.SendBlockUntil = it_var->second.SendBlockUntil;
    if (newPrioritySeted)
    {
	var.setFlagPriorityChanged(true);
	var.setPriority(newPriority);
    }

    // Udelej Callback
    if (!contains(src_it->variables_state.his_colliding_variables, name))
    {
	if (use_master_slave)
	{
	    if (master_partner && &*src_it == master_partner)
	    {
		VariableUpdated_CB(&var, &(it_var->second),src_it->ID,true, false);
	    }
	    else
	    {
		WORKER_DEBUG("libcm", 22, "SET %s: No CB, partner is a slave", name.c_str());
	    }
	}
	else
	{
	    VariableUpdated_CB(&var, &(it_var->second),src_it->ID,true, false);
	}
    }
    else
    {
	WORKER_DEBUG("libcm", 22, "SET %s: No CB, variable collides", name.c_str());
    }

    // Az nakonec: Update hisVariables
    src_it->variables_state.hisVariables[name] = var;
    check_master_slave_control_variable(*src_it, name, &var);
}

// Oznameni, ze Partner si nadefinoval nebo predefinoval promennou
//
// item: "DEF"
// item: Optional SetID
// item: definition arguments ...
void CmCore::ProcessDEF(CCmPartner &src_p, char **item, double ActTime, const char *data)
{
    CCmPartner *src_it = &src_p;
    src_it->RecvFragData.clear();

    // Nacti setcmd_id
    unsigned setcmd_id = 0;
    if (src_it->UseSetIds)
        setcmd_id = cm_string_get_value_int(item_ref_setid(*src_it, item));

    CCmVariable var;
    std::string error_str = var.defineFromArray(item_ref_definition(*src_it, item),data);

    if (!error_str.empty())
    {
        WORKER_DEBUG("libcm", 1, "libcm(%s):received variable error (%s)",
                        MyLibCM_name.c_str(),error_str.c_str());
        return;
    }

    std::string name = var.getName();
    if (!src_it->PartnerVarPrefix.empty())
    {
        name = src_it->PartnerVarPrefix + name;
        var.setName(name);
    }
    WORKER_DEBUG("libcm_change", 4, "Received DEF %s from %s", print_var(var).c_str(), print_partner(*src_it).c_str());
    
    var.setModifTime(ActTime);

    // Pokud mu budeme posilat nejake modifikace teto promenne, tak je
    // zahodime, protoze promenna je cela predefinovana.  Pokud uz
    // jsme pred chvilkou odeslali nejakou modifikace na tuto
    // promennou, tak jsme to posilali se starym setid, a on tu nasi
    // modifikaci zahodil.
    find_and_erase_by_name(src_it->variables_state.his_variables_to_send, name);
    // src_it->his_variables_to_send.erase(name);

    if (doesPartnerVariableCollide(name, *src_it))
    {
	// Nic nedavame do callbacku, bude se pouzivat `jiná'
	// promenna, dokud ji nekdo neodefinuje.
	src_it->variables_state.his_colliding_variables.insert(name);
    }
    else
    {
	// Pridej ji do callbacku
	WORKER_DEBUG("libcm", 16,"libcm(%s):received DEF \"%s\"", MyLibCM_name.c_str(),var.toStringDefine().c_str());
	if (!use_master_slave || (master_partner && &*src_it == master_partner))
	{
	    VariableCreated_CB(&var,src_it->ID,false);
	    need_call_cb = true;
	}
    }

    // his_vars_setcmd_id: Pokud je to prvni definice promenne, tak se
    // vytvori novy zaznam, jinak se prepise minula hodnota. Poznamka:
    // Frontu jsme vymazali vyse, takze vsechno nove se bude posilat s
    // timto setcmd_id.
    src_it->variables_state.hisVariables[name] = var;
    src_it->variables_state.his_vars_setcmd_id[var.getName()] = setcmd_id;

    check_master_slave_control_variable(*src_it, name, &var);
}

// Oznameni, ze partner odefinoval svoji promennou
//
// item: "UNDEF"
// item: Optional SetID
// item: Variable name
void CmCore::ProcessUNDEF(CCmPartner &src_p, char **item)
{
    CCmPartner *src_it = &src_p;
    src_it->RecvFragData.clear();

    // Nacti setcmd_id
    unsigned setcmd_id = 0;
    if (src_it->UseSetIds)
        setcmd_id = cm_string_get_value_int(item_ref_setid(*src_it, item));

    // Nacti jmeno
    std::string name = std::string(item_ref_name(*src_it, item));
    if (!src_it->PartnerVarPrefix.empty())
    {
        name = src_it->PartnerVarPrefix + name;
    }
    WORKER_DEBUG("libcm", 14,"libcm(%s):received UNDEF \"%s\"",MyLibCM_name.c_str(),name.c_str());


    WORKER_DEBUG("libcm_change", 4, "Received UNDEF %s from %s", name.c_str(), print_partner(*src_it).c_str());

    // Pokud jsme mu meli posilat nejaky zmeny, tak je zahodime,
    // protoze on si tu promennou redefinoval.
    find_and_erase_by_name(src_it->variables_state.his_variables_to_send, name);
    // src_it->his_variables_to_send.erase(name);

    // Aktualizujem si set_id
    assert(contains(src_it->variables_state.his_vars_setcmd_id, name));
    src_it->variables_state.his_vars_setcmd_id.at(name) = setcmd_id;

    // Tu promennou urcite musime mit. Pokud ne, znamena to, ze nam
    // prisel UNDEF bez predchoziho DEFu, nebo dva UNDEFy za sebou.
    VarMap::iterator it_var = src_it->variables_state.hisVariables.find(name);
    if (it_var == src_it->variables_state.hisVariables.end())
    {
        WORKER_DEBUG("libcm", 2, "Received UNDEF %s from %s, but variable does not exist", name.c_str(), src_it->address_string.c_str());
        return;
    }

    // Dame ji do callbacku.
    if (!contains(src_it->variables_state.his_colliding_variables, name))
    {
	if (!use_master_slave || (master_partner && &*src_it == master_partner))
	{
	    VariableDeleted_CB(&(it_var->second),src_it->ID,true);
	}
    }

    if (contains(src_it->variables_state.his_colliding_variables, name))
    {
	WORKER_DEBUG("libcm", 2, "Colliding variable %s was undefined by %s", name.c_str(), src_it->address_string.c_str());
    }
    // Az nakonec ji smazeme.
    src_it->variables_state.his_colliding_variables.erase(name);
    src_it->variables_state.hisVariables.erase(it_var);

    // Zkontrolujeme kolize
    tryDefineSomeCollidingVariable(name);

    if (!use_master_slave || master_partner == &*src_it)
        check_master_slave_control_variable(*src_it, name, nullptr);

    // if (it_var != src_it->hisVariables.end())
    // {
    // }
    // else
    // {
    //     WORKER_DEBUG("libcm", 1, "libcm(%s):cannot undef variable - not defined (%s)", MyLibCM_name.c_str(),item[1]);
    // }
}

void CmCore::ProcessFRG(CCmPartner &src_p, char **item, FRG_kind kind)
{
    CCmPartner *src_it = &src_p;
    if (kind == FRG_NEW)
    {
	src_it->received_fragments = std::string();
    }
    if (!src_it->received_fragments)
    {
	cms_ns_if_print("libcm", 1, "Missing FRG_NEW");
	return;
    }
    WeakString input(item[1]);
    std::string msg = interpret_first_string_and_advance(input);
    // cms_ns_if_print("libcm", 2, "FRG adding `%s'", msg.c_str());
    *src_it->received_fragments += msg;

    if (kind == FRG_END)
    {
	// Prepare for possible new fragments
	std::string complete;
	std::swap(*src_it->received_fragments, complete);
	src_it->received_fragments = Nothing();
	
	char **item = cm_string_strsplit_outside_string_and_brackets(complete.c_str(), ",","[](){}", 0);
	if (item)
	{
	    // cms_ns_if_print("libcm", 2, "FRG_END, complete=%s", complete.c_str());
	    ProcessTextCommand(item, complete.c_str(), *src_it);
	}
	else
	{
	    cms_ns_if_print("libcm", 1, "Failed to split fragment");
	}
    }
}

// Přijdou data: ty obsahují víc příkazů.
// Příkaz, který se má zpracovat je v data1cmd.
// Už je rozsekaný v items.
void CmCore::ProcessTextCommand(char **item, const char *data1cmd, CCmPartner &src_p)
{
    double ActTime = GetActualTimeDouble();

    CCmPartner *src_it = &src_p;
    unsigned src_id = src_it->ID;
    
    // WORKER_DEBUG("libcm", 22, "THR:received \"%s\" src_id=%d dst_id=%d",data,src_id,dst_id);

    // bool filters_changed_now = false;

    // WORKER_DEBUG("libcm", 22, "%s: Process %s", MyLibCM_name.c_str(), data);
    WORKER_DEBUG("libcm", 22, "ProcessTextCommand: %s: %s", MyLibCM_name.c_str(), data1cmd);

    // if (src_it == Partners.end())
    // {
    //     WORKER_DEBUG("libcm", 2, "Received text command %s from id %d, but partner not present", data1cmd, src_id);
    //     return;
    // }

   // DebugTimer t;

    if ((!strcmp(item[0],"PING")))
    {
        if (LiveDebugFlags & DEBUG_FLAG_PRINT_COMUN_PING_RECV)
            LiveDebugPrintANY(DEBUG_FLAG_PRINT_COMUN_PING_RECV, src_id, "R:",data1cmd);

        // WORKER_DEBUG("libcm",28,"libcm(%s):ping %s received src_id=%d dst_id=%d",
        //                 MyLibCM_name.c_str(),(UDP_MODE ? "UDP" : "TCP"), src_id,dst_id);
    }
    else if ((!strcmp(item[0],"ACC")) && (item[1]))
    {
        ProcessACC(src_p, item, src_id, data1cmd);
    }
    else
    {
        if (LiveDebugFlags & DEBUG_FLAG_PRINT_COMUN_RECV)
            LiveDebugPrintANY(DEBUG_FLAG_PRINT_COMUN_RECV, src_id, "R:", data1cmd);

        if ((!strcmp(item[0],"INIT-ACC")))
        {
            ProcessINIT_ACC(src_p);
	    PollCondSend->Wake();
        }
        else if ((!strcmp(item[0],"INIT"))
                 && (item[1]) && (item[2]) && (item[3]) && (item[4]) && (item[5])
                 && (item[6]) && (item[7]) && (item[8]) && (item[9]) && (item[10]))
        {
            ProcessINIT(src_p, item, src_id);
        }
        else if ((!strcmp(item[0],"DEF")) && (item[1]))
        {
            ProcessDEF(src_p, item, ActTime, data1cmd);
        }
        else if ((!strcmp(item[0],"UNDEF")) && (item[1]))
        {
            ProcessUNDEF(src_p, item);
        }
        else if ((!strcmp(item[0],"SET")) && (item[1]) && (item[2]))
        {
            ProcessSET(src_p, item, ActTime, data1cmd);
        }
        else if ((!strcmp(item[0],"MOD")) && (item[1]) && (item[2])) // modify my variables by partner
        {
            ProcessMOD(src_p, item, ActTime);
        }
	else if (strcmp(item[0], "FRG_ADD") == 0 && item[1])
	{
	    ProcessFRG(src_p, item, FRG_ADD);
	}
	else if (strcmp(item[0], "FRG_NEW") == 0 && item[1])
	{
	    ProcessFRG(src_p, item, FRG_NEW);
	}
	else if (strcmp(item[0], "FRG_END") == 0 && item[1])
	{
	    ProcessFRG(src_p, item, FRG_END);
	}
        else if ((!strcmp(item[0],"FILT_CHG")) && (item[1]))
        {
            src_it->RecvFragData.clear();
            unsigned i=1;
            while (item[i]!=NULL)
            {
                    if (src_it->ChangeFilter(item[i]))
                        src_it->filters_changed_now = true;
                    i++;
            }
        }
        else if ((!strcmp(item[0],"FILT_DEF")) && (item[1]))
        {
            src_it->RecvFragData.clear();
            unsigned i=1;
            while (item[i]!=NULL)
            {
                if (src_it->DefineFilter(item[i]))
                    src_it->filters_changed_now = true;
		i++;
            }
        }
        else if ((!strcmp(item[0],"FILT_UNDEF")) && (item[1]))
        {
            src_it->RecvFragData.clear();
            unsigned i=1;
            while (item[i]!=NULL)
            {
                if (src_it->UnDefineFilter(item[i]))
                    src_it->filters_changed_now = true;
                i++;
            }
        }
        // else if ((!strcmp(item[0],"FILT_CLEAR")))
        // {
	//     printf("JAN FILT_CLEAR: %s\n", item[1] ? "yes" : "no");
	//     if (item[1])
	//     {
	// 	src_it->RecvFragData.clear();
	// 	if (src_it->ClearFilters())
	// 	    filters_changed_now = true;
	//     }
        // }
        else
        {
            WORKER_DEBUG("libcm",7, "libcm(%s):unknown command \"%s\" \"%s\"",
                            MyLibCM_name.c_str(),item[0],data1cmd);
        }
    }

    // printf("Process %s: %.3lf msec\n", item[0], t.elapsed_msec_double());

        // if (src_it->FiltersChanged)
        // {
        //     src_it->FiltersChanged = false;
        //     src_it->PartnerFiltersChanged = true;
        // }
}

void CmCore::ReceivedTextData(const char *data, unsigned len, unsigned connection_id)
{
    CCmPartner *p = find_partner_by_id(Partners, connection_id);
    if (!p)
    {
	WORKER_DEBUG("libcm_change", 1, "ReceivedTextData: Partner %u does not exist", connection_id);
        return;
    }

    // Update time (used for timeout)
    double ActTime = GetActualTimeDouble();
    p->TimeLastRecv = ActTime;

    if (len > 2 && data[0] == 'D' && data[1] == 'B')
    {
        bool changed = db_state->receive_text_data(connection_id, view_bytes(data + 2, data + len));
        // cms_ns_if_print("libcm_change", 1, "after receive_text_data, changed=%d", changed);
        if (changed)
        {
            DbChanged = true;
            generate_callback();
        }
    }
    else if (len > 2 && data[0] == 'I' && data[1] == 'N')
    {
        // Asi je to INIT nebo INIT_ACC, ty se posilaj nekomprimovane
        ReceivedTextDataNew(data, len, *p);
    }
    else if (len > 2 && data[0] == 'F' && data[1] == 'I')
    {
        // Asi je to FILT_CHG, FILT_DEF, FILT_UNDEF, ty se ve stary verzi  posilaj nekomprimovane
        ReceivedTextDataNew(data, len, *p);
    }
    else if (len > 2 && data[0] == 'P' && data[1] == 'I')
    {
        // Asi je to PING, ty se posilaj nekomprimovane
        ReceivedTextDataNew(data, len, *p);
    }
    else if (p->UsedVersion >= 2)
    {
        if (!p->recv_buffer_state)
        {
            assert(false);
            cms_ns_if_print("libcm", 1, "Partner has no recv buffer state: %u", connection_id);
            return;
        }

        p->recv_buffer_state->add_data(view_bytes(data, data + len));
        std::vector<uint8_t> &vec = p->recv_buffer_state->get();
        std::string msg_str(vec.begin(), vec.end()); // TODO: Vyhodit
        vec.clear();
        
        ReceivedTextDataNew(msg_str.c_str(), msg_str.size(), *p);
    }
    else if (p->UsedVersion >= 1)
    {
        ReceivedTextDataNew(data, len, *p);
    }
    else
    {
        ReceivedTextDataOld(data, len, connection_id);
    }
}

struct MsgStats
{
    typedef std::map<std::string, int> Map;
    Map stats;
    void add(std::string str)
    {
	++stats[str];
    }
    void print()
    {
	std::stringstream ss;
	CM_for (Map::value_type &p, stats)
	    ss << " " << p.first << "=" << p.second;
	cms_ns_if_print("libcm", 3, "ReceivedTextDataNew%s", ss.str().c_str());
    }
};

void CmCore::ReceivedTextDataNew(const char *data, unsigned len, CCmPartner &src_p)
{
    CCmPartner *src_it = &src_p;
    char **msg_split = cm_string_strsplit_outside_string_and_brackets(data, ";","[](){}", 0);
    // MsgStats stats;
    if (msg_split)
    {
	for (char **messages = msg_split; messages[0]; ++messages)
	{
	    char **item = cm_string_strsplit_outside_string_and_brackets(messages[0], ",","[](){}", 0);
	    if (item && item[0])
	    {
		// stats.add(item[0]);
		ProcessTextCommand(item, messages[0], src_p);
		if (msv_need_refresh)
		{
		    msv_refresh_internal();
		    msv_need_refresh = false;
		}
		g_strfreev(item);
	    }
	    else
	    {
		cms_ns_if_print("libcm", 1, "ReceivedTextDataNew split by , failed");
	    }
	}
	g_strfreev(msg_split);
	// stats.print();

        if (src_it->filters_changed_now)
        {
            src_it->PartnerFiltersChanged = true;
            PartnerFiltersChanged = true;
            if (src_it->MergeMyVariablesByChangedFilters(&myVariables))
            {
                PollCondSend->Wake();
            }
            src_it->filters_changed_now = false;
        }

    }
    else
    {
	cms_ns_if_print("libcm", 1, "ReceivedTextDataNew split by ; failed");
    }
}

void CmCore::ReceivedTextDataOld(const char *data, unsigned len, unsigned connection_id)
{
    WORKER_DEBUG("libcm_change", 5, "ReceivedTextData");
  double ActTime = GetActualTimeDouble();
  unsigned conn_id = connection_id;
  const char * data_stripped = NULL;
// predrozebrat na carky
//  v udp frag_no,src,dst,CMD
//  v tcp                 CMD
//                           =FRAG,NO,COUNT,data
  WORKER_DEBUG("libcm_change", 5, "ReceivedTextData spliting begin");
  char **item_cmd_test = cm_string_strsplit_outside_string_and_brackets(data, ",", "[](){}", 0);
  WORKER_DEBUG("libcm_change", 5, "ReceivedTextData spliting end");
  unsigned UDP_packet_no = 0;
  unsigned UDP_src_id = 0;
  unsigned UDP_dst_id = 0;
  unsigned UDP_item_shift = 0;

  WORKER_DEBUG("libcm_change", 5, "ReceivedTextData at line %d", __LINE__);
  
  if (item_cmd_test)
  {
    if (UDP_MODE)
    {
      if ((item_cmd_test[0])&&(item_cmd_test[1])&&(item_cmd_test[2])&&(item_cmd_test[3]))
      {
        char *item_cmd_test_0 = cm_string_strdup_nospace_outside_string(item_cmd_test[0]);
        char *item_cmd_test_1 = cm_string_strdup_nospace_outside_string(item_cmd_test[1]);
        char *item_cmd_test_2 = cm_string_strdup_nospace_outside_string(item_cmd_test[2]);

        if  (item_cmd_test_0) UDP_packet_no = atoi(item_cmd_test_0);
        if  (item_cmd_test_1) UDP_src_id = atoi(item_cmd_test_1);
        if ((item_cmd_test_2) && (strlen(item_cmd_test_2)>0)) UDP_dst_id = atoi(item_cmd_test_2);
        unsigned offset = strlen(item_cmd_test[0])+1+strlen(item_cmd_test[1])+1+strlen(item_cmd_test[2])+1;
        if (offset < strlen(data))
        {
          data_stripped = data + offset;
        }

        if (item_cmd_test_0) g_free(item_cmd_test_0);
        if (item_cmd_test_1) g_free(item_cmd_test_1);
        if (item_cmd_test_2) g_free(item_cmd_test_2);
        conn_id = UDP_src_id;
        UDP_item_shift = 3;
      }
    } else {
      data_stripped = data;
      UDP_item_shift = 0;
    }


    WORKER_DEBUG("libcm_change", 5, "ReceivedTextData at line %d", __LINE__);

    bool for_me = true;
    if (UDP_MODE)
    {
      for_me = ((UDP_dst_id==UDP_ID) || ( (UDP_dst_id==0) && (  ((UDP_src_id == 1) && (UDP_ID  > 1)) || ((UDP_src_id  > 1) && (UDP_ID == 1))  ))) ;
    }

    WORKER_DEBUG("libcm_change", 5, "ReceivedTextData at line %d", __LINE__);

    if (for_me)
    {
      bool found_partner = false;
      for (std::list <CCmPartner>::iterator it = Partners.begin(); it != Partners.end(); it++)
      {
        if (conn_id == it->ID)
        {
          if (UDP_packet_no != 0)
          {
            it->SendAckPackets.push_back(UDP_packet_no);
          }
          it->TimeLastRecv = ActTime;
          found_partner = true;
        }
      }
      WORKER_DEBUG("libcm_change", 5, "ReceivedTextData at line %d", __LINE__);
      if ((UDP_MODE) && (!found_partner))
      {
        if (LiveDebugFlags & DEBUG_FLAG_PRINT_COMUN_CONN)
        {
           char xstr[64000];
           sprintf(xstr,"info:UDP Connected (%d), because received: %s",conn_id,data);
           LiveDebugPrintANY(DEBUG_FLAG_PRINT_COMUN_CONN, conn_id, "R:", xstr);
        }
        nowConnected(conn_id, "UDP");
      }
      if (UDP_packet_no != 0)
      {
        PollCondSend->Wake(); // send ACK
      }

      WORKER_DEBUG("libcm_change", 5, "ReceivedTextData at line %d", __LINE__);

      unsigned FRAG_no=0;
      unsigned FRAG_count=0;
      const char * FRAG_ptr=NULL;
      // TEST FRAG && UDP DISCONNECT
      if (item_cmd_test[UDP_item_shift])
      {
        char *item_cmd_test_0 = cm_string_strdup_nospace_outside_string(item_cmd_test[UDP_item_shift+0]);
        if (!strcmp(item_cmd_test_0,"FRAG"))
        {
          if (LiveDebugFlags & DEBUG_FLAG_PRINT_COMUN_FRAG) LiveDebugPrintANY(DEBUG_FLAG_PRINT_COMUN_FRAG, conn_id, "R:", data);
          if ((item_cmd_test[UDP_item_shift+1]) && (item_cmd_test[UDP_item_shift+2]) && (item_cmd_test[UDP_item_shift+3]))
          {
            char *item_cmd_test_1 = cm_string_strdup_nospace_outside_string(item_cmd_test[UDP_item_shift+1]);
            char *item_cmd_test_2 = cm_string_strdup_nospace_outside_string(item_cmd_test[UDP_item_shift+2]);

            if  (item_cmd_test_1) FRAG_no    = atoi(item_cmd_test_1);
            if  (item_cmd_test_2) FRAG_count = atoi(item_cmd_test_2);

            unsigned offset = strlen(item_cmd_test[UDP_item_shift+0])+1+strlen(item_cmd_test[UDP_item_shift+1])+1+strlen(item_cmd_test[UDP_item_shift+2])+1;
            if (offset < strlen(data_stripped))
            {
              FRAG_ptr = data_stripped + offset;
            }
            if (item_cmd_test_1) g_free(item_cmd_test_1);
            if (item_cmd_test_2) g_free(item_cmd_test_2);
          }
        } else if ((!strcmp(item_cmd_test_0,"DISCONNECT"))&&(UDP_MODE))
        {
          WORKER_DEBUG("libcm",7,"libcm(%s):Received DISCONNECT UDP (%d) - Disconnected",MyLibCM_name.c_str(),UDP_src_id);

          if (LiveDebugFlags & DEBUG_FLAG_PRINT_COMUN_CONN)
          {
             char xstr[64000];
             sprintf(xstr,"info:UDP DISConnected (%d), because received : %s",UDP_src_id,data);
             LiveDebugPrintANY(DEBUG_FLAG_PRINT_COMUN_CONN, UDP_src_id, "R:", xstr);
          }
          erasePartnerByConnectionId(UDP_src_id,false);
          g_strfreev(item_cmd_test);
          if (item_cmd_test_0) g_free(item_cmd_test_0);
          return;
        }
        if (item_cmd_test_0) g_free(item_cmd_test_0);
      }
      WORKER_DEBUG("libcm_change", 5, "ReceivedTextData at line %d", __LINE__);

      gchar * frag_data_joined = NULL;
      if (FRAG_ptr !=NULL )
      {
//       debug_printf ("RECEIVED FRAGMENT from=%d NO=%d/%d DATA=\"%s\"  /////// \"%s\"\n",conn_id,FRAG_no,FRAG_count,FRAG_ptr, data);
        // do slozeni fragmentu musi chodit jenom FRAG, ACK a nebo PING, vse ostatni rozpracovany FRAG zrusi!!
        data_stripped = NULL;
        for (std::list <CCmPartner>::iterator it = Partners.begin(); it != Partners.end(); it++)
        {
          if (conn_id == it->ID)
          {
            // zapis
            if (it->RecvFragCount != FRAG_count)
            {
              it->RecvFragCount = FRAG_count;
              it->RecvFragData.clear();
            }
            it->RecvFragData[FRAG_no] = std::string(FRAG_ptr);

            bool all_ok = true;
            for (unsigned i = 0; i < FRAG_count; i++)
            {
              if (it->RecvFragData.find(i) == it->RecvFragData.end()) all_ok = false;
            }
            if (all_ok)
            {
              std::string frag_output;
              for (unsigned i = 0; i < FRAG_count; i++)
              {
                frag_output += it->RecvFragData[i];
              }

              if (frag_data_joined) g_free(frag_data_joined);
              frag_data_joined = g_new(gchar, frag_output.size()+1);
              strcpy(frag_data_joined, frag_output.c_str());
//             WORKER_DEBUG("libcm", 22, "finished receive frag \"%s\"",frag_output.c_str());
              it->RecvFragCount = 0;
              it->RecvFragData.clear();
            }
          }
        }
        data_stripped = frag_data_joined;
      }

      WORKER_DEBUG("libcm_change", 5, "ReceivedTextData at line %d", __LINE__);

      if (data_stripped) // standard command
      {
        // char *data_nospace = cm_string_strdup_nospace_outside_string(data_stripped);
        // if (data_nospace)
        // {
          char **item_cmd = cm_string_strsplit_outside_string_and_brackets(data_stripped, ";", "[](){}", 0);
          if (item_cmd)
          {
	      // cms_ns_if_print("jan", 2, "msg=%s", data_nospace);
	      // for (char **s = item_cmd; *s; ++s)
	      // {
	      // 	  cms_ns_if_print("jan", 2, "split=%s", *s);
	      // }
	      
            unsigned cmd_cnt = 0;
	    WORKER_DEBUG("libcm_change", 5, "ReceivedTextData: begining process loop");
            while (item_cmd[cmd_cnt])
            {
              char **item = cm_string_strsplit_outside_string_and_brackets(item_cmd[cmd_cnt], ",","[](){}", 0);
              if (item)
              {
                if ((item[0]) && (for_me))
                {
                    ProcessTextCommand(item+0,item_cmd[cmd_cnt], *find_partner_by_id(Partners, conn_id));
                  if (msv_need_refresh)
                  {
                      msv_refresh_internal();
                      msv_need_refresh = false;
                  }
                }
                g_strfreev(item);
              } else {
                WORKER_DEBUG("libcm",7, "libcm(%s):bad command - no enough params\"%s\"",MyLibCM_name.c_str(),data_stripped);
              }
              cmd_cnt++;
            }
            g_strfreev(item_cmd);
          } else {
            WORKER_DEBUG("libcm",7, "libcm(%s):bad command set - no enough params\"%s\"",MyLibCM_name.c_str(),data_stripped);
          }
        //   g_free(data_nospace);
        // }
	// else {
        //   WORKER_DEBUG("libcm",7, "libcm(%s):bad command set - empty string \"%s\"",MyLibCM_name.c_str(),data_stripped);
        // }
      } else {
//        WORKER_DEBUG("libcm",7, "bad command set - empty string after strip \"%s\"",data_stripped);
      }
      g_strfreev(item_cmd_test);
      if (frag_data_joined) g_free(frag_data_joined);
    }
  } else {
    WORKER_DEBUG("libcm",7, "libcm(%s):bad command set - empty string for test \"%s\"",MyLibCM_name.c_str(),data);
  }
}

// Sets connection_id
// int CmCore::doRecv(char *buffer, unsigned &connection_id)
// {
//     int length = 0;
//     if (SERVER)
//     {
//         DEBUG_UNLOCK;
//         length = SERVER->Recv(buffer, MSGMAX, &connection_id);
//         connection_id += 2;
//         DEBUG_LOCK;
//         // if (stop_threads)
//         //    WORKER_DEBUG("libcm", 22, "wake up SERVER");
//     }
//     else if (CLIENT)
//     {
//         DEBUG_UNLOCK;
//         if (!CLIENT->IsConnected())
//         {
//             if (CLIENT->Connect())
//             {
//                 onClientConnect();
//             } else {
//                 usleep(100000);
//             }
//         }
//         else
//         {
//             length = CLIENT->Recv(buffer, MSGMAX);
//             // if (stop_threads)
//             //    WORKER_DEBUG("libcm", 22, "wake up CLIENT 1");
//             if (length<0)
//             {
//                 onClientDisconnect();
//             }
//             connection_id=2;
//         }
//         DEBUG_LOCK;
//         // if (stop_threads)
//         //    WORKER_DEBUG("libcm", 22, "wake up CLIENT");
//     }
//     else if (IP)
//     {
//         DEBUG_UNLOCK;
//         length = IP->Recv(buffer, MSGMAX); connection_id=0;
//         DEBUG_LOCK;
//         // if (stop_threads)
//         //    WORKER_DEBUG("libcm", 22, "wake up IP");
//     }
//     // DEBUG_UNLOCK;
//     // int sleep = rand() % 5000;
//     // printf("LibCM: Sleeping for %d usec\n", sleep);
//     // usleep(sleep);
//     // DEBUG_LOCK;
//     return length;
// }


// void CmCore::OnPartnerConnected(unsigned connection_id)
// {
//     cms_ns_if_print("cmip", 5, "OnPartnerConnected: %u", connection_id);
//     nowConnected(connection_id, "");
//     cms_ns_if_print("cmip", 5, "OnPartnerConnected done: %u", connection_id);
// }

void CmCore::OnPartnerDisconnected(unsigned connection_id)
{
    cms_ns2_if_print("cmip", MyLibCM_name, 3, "OnPartnerDisconnected: %u", connection_id);
    erasePartnerByConnectionId(connection_id,false);
    if (PartnerFiltersChanged)
	generate_callback();
    cms_ns2_if_print("cmip", MyLibCM_name, 3, "OnPartnerDisconnected done: %u", connection_id);
}

void CmCore::OnRecvFromPartner(char const *buffer, int length, unsigned connection_id)
{
    WORKER_DEBUG("libcm", 4, "ReceiveTextData from partner %u", connection_id);
    if (sent_messages_file)
        write_debug_message_info(*sent_messages_file, "recv", view_bytes(buffer, buffer + length), connection_id);
    
    ThreadTimer timer;
    try
    {
        ReceivedTextData(buffer, length, connection_id);
    }
    catch (std::runtime_error &e)
    {
        WORKER_DEBUG("libcm", 1, "ReceiveTextData from partner %u failed: %s", connection_id, e.what());
    }

    if (need_call_cb)
    {
	need_call_cb = false;
	generate_callback();
    }

    // TODO: Je tohle potreba??
    for (std::list <CCmPartner>::iterator it = Partners.begin(); it != Partners.end();)
    {
	std::list <CCmPartner>::iterator itd = it++;
	if (itd->DisconnectMe)
	{
	    WORKER_DEBUG("libcm",7,"libcm(%s):May collision variable flag set  (ID:%d) - Disconnected - successor",MyLibCM_name.c_str(),itd->ID);
	    if (LiveDebugFlags & DEBUG_FLAG_PRINT_COMUN_CONN)
	    {
		char xstr[64000];
		sprintf(xstr,"info:DISConnected (%d), sucessor",itd->ID);
		LiveDebugPrintANY(DEBUG_FLAG_PRINT_COMUN_CONN, itd->ID, "S:", xstr);
	    }
	    erasePartner(itd,true);
	}
    }
    if ((PartnerFiltersChanged) || !m_var_changes.empty()) 
    {
	generate_callback();
	//CheckAndCallCB();
    }

    int64_t usec = timer.elapsed_usec();
    if (usec >= 1000)
    {
        std::string msg_str(buffer, buffer + std::max(length, 50));
        cms_ns2_if_print("cmip", MyLibCM_name, 3, "OnRecvFromPartner %u: elapsed=%.3f msec, size=%d: %s",
                         connection_id,
                         usec/1000.0,
                         length,
                         std::string(msg_str, 0, 50).c_str());
    }
}

// void CmCore::RecvThread (void)
// {
//     char buffer[MSGMAX];
//     DEBUG_LOCK;

//     while (!stop_threads)
//     {
//         unsigned connection_id=0;
//         WORKER_DEBUG("libcm", 22, "Doing recv");
//         int length = doRecv(buffer, connection_id); // Sets connection_id
//         WORKER_DEBUG("libcm", 22, "Recv done");

//         if (length>0)
//         {
//             if (COM_MODE==0) // TEXT
//             {
//                 WORKER_DEBUG("libcm",25,"libcm(%s):Received TEXT data(from con=%d) ... len=%d ", MyLibCM_name.c_str(),connection_id, length);
//                 buffer[length] = 0;
//                 //WORKER_DEBUG("libcm", 22, "%s: Received %s", MyLibCM_name.c_str(), buffer);
//                 // DebugTimer t;
//                 WORKER_DEBUG("libcm", 22, "ReceivedTextData");
//                 ReceivedTextData(buffer, length, connection_id);
//                 WORKER_DEBUG("libcm", 22, "ReceivedTextData done");
//                 // printf("Timer: ReceivedTextData %.3lf ms\n", t.elapsed_msec_double());
//             }
//             if (COM_MODE==1) // BIN
//             {
//                 WORKER_DEBUG("libcm",25,"libcm(%s):Received BINARY data(from con=%d) ... len=%d ", MyLibCM_name.c_str(),connection_id, length);
//             }

//             for (std::list <CCmPartner>::iterator it = Partners.begin(); it != Partners.end();)
//             {
//                 std::list <CCmPartner>::iterator itd = it++;
//                 if (itd->DisconnectMe)
//                 {
//                     WORKER_DEBUG("libcm",7,"libcm(%s):May collision variable flag set  (ID:%d) - Disconnected - successor",MyLibCM_name.c_str(),itd->ID);
//                     if (LiveDebugFlags & DEBUG_FLAG_PRINT_COMUN_CONN)
//                     {
//                         char xstr[64000];
//                         sprintf(xstr,"info:DISConnected (%d), sucessor",itd->ID);
//                         LiveDebugPrintANY(DEBUG_FLAG_PRINT_COMUN_CONN, itd->ID, "S:", xstr);
//                     }
//                     erasePartner(itd,true);
//                 }
//             }

//         }
//         if ((PartnerFiltersChanged) || !m_var_changes.empty()) 
//         {
//             NotifyCallback();
//             //CheckAndCallCB();
//         }
//     }
//     DEBUG_UNLOCK;
// }


void CmCore::RegenaratePartnerFiltersBasic(CCmFiltersSet &PartnerFiltersActive)
{
  // MERGE   Partners :: PartnerFilter ===>>> (*PartnerFiltersActive)
  for (std::list <CCmPartner>::iterator it_partner = Partners.begin(); it_partner != Partners.end(); it_partner++)
  {
    PartnerFiltersActive.add(it_partner->PartnerFilters);
  }
}

void CmCore::RegenaratePartnerFilters(CCmFiltersSet &PartnerFiltersActive)
{
    bool changed = false;
    RegenaratePartnerFiltersBasic(PartnerFiltersActive); // before CB funct
    if (m_ext_hook)
	m_ext_hook->RegenaratePartnerFiltersExt(PartnerFiltersActive, myFiltersExt);
  if (changed)
  {
    WORKER_DEBUG("libcmext",4,"libcm(%s):ExtFilterRequest - (PartnerFilterCB) changed : %s\n",MyLibCM_name.c_str(),myFiltersExt.fullPrint(" | ").c_str());
  }
  if (changed)
  {
//    CM_Mutex->Unlock();
    CmCore::RefreshFilters(false);
//    CM_Mutex->Lock();
  }
}


// Diffne PartnerFiltersActive oproti PartnerFilter (posledni filtry,
// ktere sly do callbacku).  Zaroven se z nich spocita Created a
// Deleted.
void CmCore::PartnerFiltersDiffForCB(class CCmFiltersSet &PartnerFiltersActive, class CCmFiltersSet *PartnerFiltersCreated, class CCmFiltersSet *PartnerFiltersDeleted)
{
    CCmFiltersSet CreatedSet;
    CCmFiltersSet DeletedSet;
    PartnerFiltersActive.diff(PartnerFilters, &CreatedSet, &DeletedSet, NULL, true, true);
    PartnerFilters = PartnerFiltersActive;

    if (PartnerFiltersCreated != NULL) *PartnerFiltersCreated = CreatedSet;//.getMapAll();
    if (PartnerFiltersDeleted != NULL) *PartnerFiltersDeleted = DeletedSet;//.getMapAll();

    if (LiveDebugFlags & DEBUG_FLAG_DUMP_FILTERS_PARTNER_ALL) LiveDebugPrintFilters(DEBUG_FLAG_DUMP_FILTERS_PARTNER_ALL, 49999, "R:",&PartnerFilters);

    PartnerFiltersChanged = false;
}


void CmCore::PartnerFiltersGoCB(class CCmFiltersSet *PartnerFiltersActive, class CCmFiltersSet *PartnerFiltersCreated, class CCmFiltersSet *PartnerFiltersDeleted)
{ // UNLOCKED!!! ONLY CALL CB
    (*FilterCB_function)(PartnerFiltersActive, PartnerFiltersCreated, PartnerFiltersDeleted, UserPtr);
}

// CmCore::LockAndNotify::LockAndNotify(CCmWorker &w) : self(w)
// {
//     self.CM_Mutex->Lock();
// }

// CmCore::LockAndNotify::~LockAndNotify()
// {
//     // CM_Mutex is Locked
//     bool need_notify = self.need_call_notify_callback;
//     self.need_call_notify_callback = false;

//     // Unlock and call CB
//     if (need_notify)
// 	self.cb_thread->push(1);
//     self.CM_Mutex->Unlock();
    
//     // if (need_notify && self.m_notify_cb)
//     // 	self.m_notify_cb();
// }

// Tohle se vola pri odemcenym workeru
// void CmCore::NotifyCallback()
// {
//     cms_ns_if_print("libcm", 4, "notifycallback");
//     if (!m_just_notify)
//     {
// 	if (cb_thread)
// 	    cb_thread->push(0);
//     }
//     else
//     {
// 	if (PartnerFiltersChanged)
// 	{
// 	    if (FilterCB_function)
// 		(*FilterCB_function)(nullptr, nullptr, nullptr, UserPtr);
// 	}
// 	if (!m_var_changes.empty())
// 	{
// 	    if (VariableCB_function)
// 		(*VariableCB_function)(nullptr, nullptr, nullptr, UserPtr);
// 	}
//     }
// }

// void CmCore::CheckAndCallCB()
// {
//     WORKER_DEBUG("libcm",5, "CheckAndCallCB");

//     if (!m_var_changes.empty())
//     {
// 	if (m_just_notify)
// 	{
// 	    if (VariableCB_function)
// 	    {
// 		DEBUG_UNLOCK;
// 		VarMap empty;
// 		(*VariableCB_function)(&empty, &empty, &empty, UserPtr);
// 		// VariableGoCB(&empty, &empty, &empty);
// 		DEBUG_LOCK;
// 	    }
// 	}
// 	else
// 	{
// 	    VarMap copyCB_VariablesCreated; // = CB_VariablesCreated;
// 	    VarMap copyCB_VariablesUpdated; // = CB_VariablesUpdated;
// 	    VarMap copyCB_VariablesDeleted; // = CB_VariablesDeleted;
// 	    m_var_changes.move_all_changes(copyCB_VariablesCreated,
// 					   copyCB_VariablesUpdated,
// 					   copyCB_VariablesDeleted);
// 	    // CB_VariablesDeleted.clear();
// 	    // CB_VariablesCreated.clear();
// 	    // CB_VariablesUpdated.clear();

// 	    // Adjust recreated variables
// 	    for (auto &p : copyCB_VariablesCreated)
// 		copyCB_VariablesDeleted.erase(p.first);

// 	    if ((LiveDebugFlags & DEBUG_FLAG_VAR_ACTUAL_FULL_STATE) && (!VAR_ACTUAL_FULL_STATE_changes.empty()))
// 	    {
// 		for (std::set<unsigned>::iterator it_s = VAR_ACTUAL_FULL_STATE_changes.begin(); it_s != VAR_ACTUAL_FULL_STATE_changes.end(); it_s++)
// 		{
// 		    unsigned XID = *it_s;
// 		    if (XID==0)
// 		    {
// 			LiveDebugPrintVars(DEBUG_FLAG_VAR_ACTUAL_FULL_STATE, XID, "FULL:",&myVariables);
// 		    } else {
// 			for (std::list <CCmPartner>::iterator it_partner = Partners.begin(); it_partner != Partners.end(); it_partner++)
// 			{
// 			    if (it_partner->ID == XID)
// 			    {
// 				LiveDebugPrintVars(DEBUG_FLAG_VAR_ACTUAL_FULL_STATE, XID, "FULL:",&(it_partner->hisVariables));
// 			    }
// 			}
// 		    }
// 		}
// 		VAR_ACTUAL_FULL_STATE_changes.clear();
// 	    }

// 	    DEBUG_UNLOCK;
// 	    WORKER_DEBUG("libcm",5, "CheckAndCallCB: Calling Var CB: created=%s, updated=%s, deleted=%s",
// 			 print_map(copyCB_VariablesCreated).c_str(),
// 			 print_map(copyCB_VariablesUpdated).c_str(),
// 			 print_map(copyCB_VariablesDeleted).c_str());

// 	    VariableGoCB(&copyCB_VariablesCreated, &copyCB_VariablesUpdated, &copyCB_VariablesDeleted);
// 	    DEBUG_LOCK;
// 	}
//     }
// }

// Funguje, pokud se nefragmentuje...
void debug_sending(std::string MyLibCM_name, unsigned client_id, char const *buffer)
{
    char *data_nospace = cm_string_strdup_nospace_outside_string(buffer);
    if (data_nospace)
    {
        char **item_cmd = cm_string_strsplit_outside_string_and_brackets(data_nospace, ";", "[](){}", 0);
        assert(item_cmd);
        for (unsigned pos = 0; item_cmd[pos]; ++pos)
        {
	    WORKER_DEBUG("libcm", 22, "Sending to %u: %s", client_id, item_cmd[pos]);
        }
        g_strfreev(item_cmd);
    }
    g_free(data_nospace);
}

unsigned CmCore::SendTextMessage(View<uint8_t> msg, unsigned PACKET_NO, unsigned RETRY_PACKET_NO, uint32_t MyDebugFilterFlag, unsigned DST_ID, unsigned partnerLengthMax) // 0=to all
{
    cms_ns_if_print("libcm", 4, "SendTextMessage: %zu bytes", msg.size());
  // unsigned RetVal = 0;
  // unsigned USED_PACKET_NO = 0;
  // char buffer[MSGMAX];
  // std::string act_msg = msg;
  double ActTime = GetActualTimeDouble();

  if (partnerLengthMax != 0 && msg.size() > partnerLengthMax)
  {
      cms_ns_if_print("libcm", 1, "Message too long size=%zu > max=%u", msg.size(), partnerLengthMax);
  }
  
  // if (partnerLengthMax!=0)
  // {
  //     if (msg.size() > partnerLengthMax)
  //     {
  // 	  unsigned pos = 0;
  // 	  cms_ns_if_print("libcm", 1, "BUDEME FRAGMENTOVAT (msg.size=%zu, partnerLengthMax=%u, %s", msg.size(), partnerLengthMax, msg.c_str());
  // 	  assert(false);
      
  // 	  SendFragmentDataList.clear();
  // 	  SendFragmentCount = 0;
  // 	  SendFragmentDST = DST_ID;
  // 	  while (pos < msg.size())
  // 	  {
  // 	      if (pos == 0)
  // 	      {
  // 		  act_msg = msg.substr(pos,partnerLengthMax);
  // 	      } else {
  // 		  SendFragmentDataList.push_back(msg.substr(pos,partnerLengthMax));
  // 	      }
  // 	      pos += partnerLengthMax;
  // 	      SendFragmentCount++;
  // 	  }
  // 	  char wstr[40];
  // 	  sprintf(wstr,"FRAG,%d,%d,", SendFragmentCount - ((int)SendFragmentDataList.size() + 1), SendFragmentCount);
  // 	  act_msg = std::string(wstr) + act_msg;
  //     }
  // }

  if (sent_messages_file)
  {
      write_debug_message_info(*sent_messages_file, "send", msg, DST_ID);
  }
  
// WORKER_DEBUG("libcm", 22, "send:%s",msg.c_str());
  for (std::list <CCmPartner>::iterator it = Partners.begin(); it != Partners.end(); it++)
  {
      if ((DST_ID==0) || (it->ID == DST_ID))
      {
	  it->TimeLastSent = ActTime;
	  if (COM_MODE==0) // TEXT
	  {
	      // sprintf(buffer,"%s",act_msg.c_str());
	      // RetVal = strlen(buffer);

	      // if (rand() % 20 != 0)
	      // {
	      // if (LiveDebugFlags & MyDebugFilterFlag)
	      //     LiveDebugPrintANY(MyDebugFilterFlag, DST_ID, "S:", buffer);
	      // debug_sending(*it, buffer);
	      
	      //WORKER_DEBUG("libcm", 22, "%s: Sending %s", MyLibCM_name.c_str(), buffer);
	      // cms_ns_if_print("cmip", 3, "Send2");
	      connection->send((char const *)msg.begin(), msg.size(), it->ID);
	      
	      // WORKER_DEBUG("libcm",23,"libcm(%s):send TCP DST=%d message \"%s\"",MyLibCM_name.c_str(),it->ID,buffer);
	  }
      }
  }
  return msg.size();
}

unsigned CmCore::SendTextMessages(std::list<std::string> msgs, unsigned PACKET_NO, unsigned RETRY_PACKET_NO, uint32_t MyDebugFilterFlag, unsigned DST_ID, unsigned partnerLengthMax) // 0=to all
{
  unsigned RetVal = 0;
  std::string msg;
  for (std::list<std::string>::iterator it = msgs.begin(); it != msgs.end(); it++)
  {
    if (it != msgs.begin())
	msg += std::string(";");
    msg += (*it);
    if (LiveDebugFlags & MyDebugFilterFlag)
	LiveDebugPrintANY(MyDebugFilterFlag, DST_ID, "S:", it->c_str());
  }
  if (msg.size()>0)
  {
      RetVal += SendTextMessage(view_bytes(msg), PACKET_NO, RETRY_PACKET_NO, 0, DST_ID, partnerLengthMax);
  }
  return RetVal;
}


std::string CmCore::RemoveMatchingPrefix(std::string variable, std::string onlyPrefix, unsigned *size_add)
{
  std::string RetStr = std::string("");
  *size_add = 0;
  if (onlyPrefix.size() != 0)
  {
    if (variable.size() > onlyPrefix.size())
    {
      if (onlyPrefix == variable.substr (0,onlyPrefix.size())) 
      {
        RetStr = variable.substr (onlyPrefix.size(), variable.size() - onlyPrefix.size());
        *size_add = RetStr.size() + 1;
      }
    }
  } else {
    RetStr = variable;
    *size_add = RetStr.size() + 1;
  }
  return RetStr;

}


// Tohle se vola pokazde, kdyz posleme zpravu partnerovi.
// send_bytes_now: kolik jsme toho poslali v predchozi zprave.
void CmCore::CalculateSendBlockUntil(std::list <CCmPartner>::iterator it, unsigned send_bytes_now)
{
    WORKER_DEBUG("libcm", 10, "CalculateSendBlockUntil partner=%u, history_size=%zu, sent_now=%u, BaudSpeedMax=%u, BaudSpeedCalcTime=%lf",
                 it->ID,
                 it->BaudSpeedHistory.size(),
                 send_bytes_now,
                 it->BaudSpeedMax,
                 it->BaudSpeedCalcTime);
  unsigned *__BaudSpeedMax = &it->BaudSpeedMax;
  double   *__BaudSpeedCalcTime = &it->BaudSpeedCalcTime;
  double   *__SendBlockUntil = &it->SendBlockUntil;
  std::list< DoubleList > * __BaudSpeedHistory = &it->BaudSpeedHistory;
  // if (UDP_MODE)
  // {
  //   __BaudSpeedMax = &BaudSpeedMax;
  //   __BaudSpeedCalcTime = &BaudSpeedCalcTime;
  //   __SendBlockUntil = &SendBlockUntil;
  //   __BaudSpeedHistory = &BaudSpeedHistory;

  // }

  if ((*__BaudSpeedMax > 0) && (*__BaudSpeedCalcTime > 0.1))
  {
      double ActTime = GetActualTimeDouble();
      // add actual
      DoubleList myItem;
      myItem.push_back(ActTime);
      myItem.push_back((double)send_bytes_now);
      __BaudSpeedHistory->push_back(myItem);
      // remove old
      for (std::list< DoubleList >::iterator itlx = __BaudSpeedHistory->begin(); itlx != __BaudSpeedHistory->end(); )
      {
	  std::list< DoubleList >::iterator itl = itlx++;
	  if ((((*itl)[0]) + *__BaudSpeedCalcTime) < ActTime)
	  {
	      __BaudSpeedHistory->erase(itl);
	  }
      }
      // calculate: sent_bytes in last BaudSpeedCalcTime
      unsigned sended_bytes = 0;
      for (std::list< DoubleList >::iterator itl = __BaudSpeedHistory->begin(); itl != __BaudSpeedHistory->end(); itl++)
      {
	  sended_bytes += (unsigned)((*itl)[1]);
      }
      if (sended_bytes>0)
      {
	  unsigned allowed_bytes = (unsigned)(((double)*__BaudSpeedMax * *__BaudSpeedCalcTime) / 8.);
	  if (sended_bytes > allowed_bytes)
	  {
	      double add_wait = (double)((sended_bytes - allowed_bytes) * 8) / (double)*__BaudSpeedMax;
              // WORKER_DEBUG("libcm", 2, "CalculateSendBlockUntil Add wait: %.3f seconds", add_wait);
	      *__SendBlockUntil = ActTime + add_wait;
	  }
          else
          {
              // WORKER_DEBUG("libcm", 2, "CalculateSendBlockUntil: No need to add wait");
          }
      }
      else
      {
          // WORKER_DEBUG("libcm", 2, "CalculateSendBlockUntil: no bytes sent");          
      }
  }
  else
  {
      // WORKER_DEBUG("libcm", 2, "CalculateSendBlockUntil: no limits");                
  }
}

void CmCore::EvalLiveDebug()
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    double ActTime = GetActualTimeDouble();
    if (stop_threads)
	return;
    PollCondLive->ChangeRqTime(ActTime+0.5);
}

void CmCore::EvalDb()
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    if (stop_threads)
	return;
    bool changed = db_state->wakeup();
    Maybe<int64_t> wake_time = db_state->get_next_wakeup_time();
    // WORKER_DEBUG("libcm", 1, "DbSendQuery wake_time=%" PRId64, wake_time ? *wake_time : -1);
    if (wake_time)
        PollCondDb->ChangeRqTimeUsec(*wake_time);
    if (changed)
    {
        cms_ns_if_print("libcm", 4, "EvalDB changed");
        DbChanged = true;
        PollCondSend->Wake();
        generate_callback();
    }
}

// PollCondSave se vzbudi pri kazde zmene promenne, ktera ma nastaveno SaveFile
// Dale se naplanuje podle podmínek
void CmCore::EvalSaveFile()
{
    cms_ns_if_print("libcm", 4, "EvalSaveFile");
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    double ActTime = GetActualTimeDouble();
    if (stop_threads)
    {
	return;
    }
    
    double sleep_time_local = 0.0;
    // PollCondSave.ClearCallRq();
    if ((SaveFile_USE)&&(SaveFile_firstLoop))
    {
        cms_ns_if_print("libcm", 4, "EvalSaveFile first use: Creating SaveFile directory");
	if (SaveFile_Type == 1)
	{
	    std::string SaveFile_Name = SaveFile_Params;
	    libCM_CreateDirectoryFromPath(SaveFile_Name.c_str());
	}
	SaveFile_firstLoop = false;
    }

    // Tohle se provadi v pripade, ze bylo zavolano
    // StartUseSaveFile(false), cimz se loadovani hodnot jenom
    // naplanovalo, a ted to mame vykonat.  Shodi to samo
    // SaveFile_LOAD na false.
    if (SaveFile_LOAD)
    {
	SaveFile_Load();
    }
    
    if (SaveFile_USE)
    {
	bool SaveNow = false;
	if ((SaveFile_FirstUnsavedChange != 0.0) && ((SaveFile_FirstUnsavedChange + SaveFile_FirstChangeMaxDelay) < ActTime))
	    SaveNow = true;
	else if ((SaveFile_LastUnsavedChange != 0.0) && ((SaveFile_LastUnsavedChange + SaveFile_LastChangeMinDelay) < ActTime))
	    SaveNow = true;
	if (SaveNow)
	{
	    SaveFile_Save();
	}
	else
	{
	    // calc_sleep_time
	    if (SaveFile_FirstUnsavedChange != 0.0)
	    {
		double calc_sleep_time = (SaveFile_FirstUnsavedChange + SaveFile_FirstChangeMaxDelay) - ActTime;
		if ((calc_sleep_time > 0.0) && ((sleep_time_local > calc_sleep_time) || (sleep_time_local == 0.0)))
		    sleep_time_local = calc_sleep_time;
	    }
	    if (SaveFile_LastUnsavedChange  != 0.0)
	    {
		double calc_sleep_time = (SaveFile_LastUnsavedChange  + SaveFile_LastChangeMinDelay ) - ActTime;
		if ((calc_sleep_time > 0.0) && ((sleep_time_local > calc_sleep_time) || (sleep_time_local == 0.0)))
		    sleep_time_local = calc_sleep_time;
	    }
	}
    }
    cms_ns_if_print("libcm", 4, "EvalSaveFile: Sleep time = %lf", sleep_time_local);
    if (sleep_time_local>0.0)
	PollCondSave->ChangeRqTime(ActTime + sleep_time_local);
}

void CmCore::EvalPing()
{
    if (use_cmip_ping)
        return;
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    double ActTime = GetActualTimeDouble();
    if (stop_threads)
	return;
    
    double sleep_time_local = 0.0;

    for (std::list <CCmPartner>::iterator it = Partners.begin(); it != Partners.end(); )
    {
        std::list <CCmPartner>::iterator itp = it;
        it++;
        if ((ActTime - itp->TimeLastSent) >= itp->PingPeriod)
        {
            std::string msg = "PING";
            SendTextMessage(view_bytes(msg),0,0,DEBUG_FLAG_PRINT_COMUN_PING_SEND,itp->ID, 0);
        }
        double calc_sleep_time = itp->PingPeriod - (ActTime - itp->TimeLastSent);
        if ((calc_sleep_time > 0.0) && ((sleep_time_local > calc_sleep_time) || (sleep_time_local == 0.0)))
            sleep_time_local = calc_sleep_time;

        double RecvAge = ActTime - itp->TimeLastRecv; // EVALUATE TIMEOUT

		
        if (RecvAge > itp->PingTimeOut)
        {
            cms_ns_if_print("libcm", 1, "Disconnecting: %s: timeout", itp->address_string.c_str());
            connection->disconnect(itp->ID);
            erasePartnerByConnectionId(itp->ID,false);
        }
        else
        {
            double calc_sleep_time = itp->PingTimeOut - RecvAge;
            if ((calc_sleep_time > 0.0) && ((sleep_time_local > calc_sleep_time) || (sleep_time_local == 0.0)))
                sleep_time_local = calc_sleep_time;
        }
    }

    if (sleep_time_local>0.0)
        PollCondPing->ChangeRqTime(ActTime + sleep_time_local);
}

size_t CmCore::try_send_filters(std::list<CCmPartner>::iterator it)
{
    NsTimer main_timer("libcm", MyLibCM_name, 3, "try_send_filters");
    if (it->myFiltersChange)
    {
        FilterParams params =
        {
            it->PartnerVarPrefix,
            myVariables,
        };

        assert(it->send_buffer_state->empty());
        it->myFiltersChange = CompareFiltersAndGenerateChangeMessage(*it->send_buffer_state,
                                                                     myFilters, // src
                                                                     it->myFilters, // dst
                                                                     params);

        assert(it->send_buffer_state->empty());
        ByteBuffer &buff = it->send_buffer_state->get();
        if (!buff.empty())
        {
            unsigned n = SendTextMessage(view(buff.begin(), buff.end()), 1, 0,
                                         DEBUG_FLAG_PRINT_COMUN_SEND, it->ID, it->LengthMax);
            buff.clear();
            return n;
        }
    }
    return 0;
}

size_t CmCore::try_send_db_messages(std::list<CCmPartner>::iterator it)
{
    NsTimer main_timer("libcm", MyLibCM_name, 3, "try_send_db_messages");
    if (it->LengthMax > 2)
    {
        // NsTimer timer("libcm", MyLibCM_name, 3, format("EvanSend (iter=%ld) DB", iterations));
        std::vector<uint8_t> dbmsg = db_state->generate_data_to_send(it->ID, it->LengthMax - 2);
        if (!dbmsg.empty())
        {
            std::string prefix("DB");
            dbmsg.insert(dbmsg.begin(), prefix.begin(), prefix.end());
            // cms_ns_if_print("libcm", 3, "Sending DB message to %u: %s", it->ID, dbmsg.c_str());
            return SendTextMessage(dbmsg, 0, 0, 0, it->ID, it->LengthMax);
        }
    }
    return 0;
}

size_t CmCore::try_send_variable_messages(std::list<CCmPartner>::iterator it, double ActTime)
{
    // Pokud je to nastavene, tak nesmime posilat zpravy moc casto.
    if (max_variable_send_period)
    {
        if (ActTime < it->last_sent_time + *max_variable_send_period)
            return 0;
    }
    
    NsTimer main_timer("libcm", MyLibCM_name, 3, "try_send_variable_messages");
    DataToSendParams params = {
        it->UseSetIds,
        it->PartnerVarPrefix,
        MyLibCM_name
    };
    it->send_buffer_state->set_max_size(it->LengthIdeal);
    bool has_changes = generate_data_to_send(*it->send_buffer_state, it->variables_state, params, ActTime);
    if (has_changes)
    {
        ByteBuffer &ret_messages = it->send_buffer_state->get();
        assert(!ret_messages.empty());
        if (!ret_messages.empty())
        {
            unsigned bytes = SendTextMessage(View<uint8_t>(ret_messages.begin(), ret_messages.end()),
                                             1,0, DEBUG_FLAG_PRINT_COMUN_SEND, it->ID, it->LengthMax);
            ret_messages.clear();
            return bytes;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        // Neni nic k odeslani.
        assert(it->send_buffer_state->get().empty());
        return 0;
    }
}

// Posila zpravy, dokud to kapacita linky zvlada. Pak vrati cas, za
// jako dlouho se ma posilat znovu.
double CmCore::EvalSendForPartnerWhenInited(std::list<CCmPartner>::iterator it, double ActTime)
{
    for (;;)
    {
    again:
	// Zkontroluj, jestli vubec muzeme posilat.
	if (it->SendBlockUntil > ActTime)
	{
	    return it->SendBlockUntil - ActTime;
	}

        // V nasledujicim kodu posleme nejvyse jednu zpravu.  Kod
        // navic zajistuje, ze v kazde iteraci for cyklu zkousime
        // poslat neco jineho, aby se jednotlive druhy zprav
        // rovnomerne stridaly.
        size_t const n_kinds = 3;
        for (size_t i = 0; i < n_kinds; ++i)
        {
            size_t bytes = 0;
            switch (it->n_send_iterations % n_kinds)
            {
            case 0:
                bytes = try_send_filters(it);
                break;
            case 1:
                bytes = try_send_db_messages(it);
                break;
            case 2:
                bytes = try_send_variable_messages(it, ActTime);
                if (bytes > 0)
                    it->last_sent_time = ActTime;
                break;
            }
            ++it->n_send_iterations;          
            if (bytes > 0)
            {
                // Zprava byla odeslana
                CalculateSendBlockUntil(it, bytes);
                goto again;
            }
        }

        // Pokud jsme se dostali az sem, tak muzeme posilat, ale
        // nemame nic k odeslani
        double sleep_until_next_send = it->haveDataToSend(ActTime, max_variable_send_period);

        // Tohle je ochrana, kdyby byla chyba v implementaci
        // try_send_XXXX, aby to necyklilo
        if (sleep_until_next_send == 0.0)
        {
            cms_ns_if_print("libcm", 1, "No data to send, but sleep_time = 0");
            sleep_until_next_send += 0.5;
        }
        
        return sleep_until_next_send;
    }
}

void CmCore::send_init_message(std::list<CCmPartner>::iterator it)
{
    char WStr[MSGMAX];
    std::string decompression_capabilities = "no";
    if (is_compression_supported())
    {
        if (!recv_buffer_dict_hash.empty())
        {
            // Umime dekompresi a dokonce mame slovnik
            decompression_capabilities = recv_buffer_dict_hash;
        }
        else
        {
            // Umime dekompresi, ale nemame slovnik
            decompression_capabilities = "yes";
        }
    }

    std::string compression_capabilities = send_buffer_dict_hash; // hash or empty
    
    // TODO: Check size.....
    sprintf(WStr,"INIT,%d,%d,%s,%s,%d,%d,%d,%s,%s,%s,%s,%s,%s",
            ProtocolVersion,
            ProtocolFlags,
            cm_std_string_print_short_float(RqPingPeriod).c_str(),
            cm_std_string_print_short_float(RqPingTimeOut).c_str(),
            RqLengthMax,
            RqLengthIdeal,
            RqBaudSpeedMax,
            cm_std_string_print_short_float(RqBaudSpeedCalcTime).c_str(),
            MyDefaultPrefix.c_str(),
            RemotePrefix.c_str(),
            m_MyName.c_str(),
            decompression_capabilities.c_str(),
            compression_capabilities.c_str()
        );
    unsigned bytes = SendTextMessage(view_bytes(WStr, WStr + strlen(WStr)),
                                     0,0,DEBUG_FLAG_PRINT_COMUN_SEND,it->ID, 0);
    CalculateSendBlockUntil(it, bytes);
}

double CmCore::EvalSendForPartner(std::list<CCmPartner>::iterator it, double ActTime)
{
    cms_ns_if_print("cmip", 4, "eval send:"
		    " received_init_acc=%d"
		    " received_init=%d"
		    " sent_init_acc=%d"
		    " sent_init=%d",
		    it->received_init_acc,
		    it->received_init,
		    it->sent_init_acc,
		    it->sent_init);

    if (it->received_init)
    {
	if (!it->sent_init_acc)
	{
            std::string msg = "INIT-ACC";
            SendTextMessage(view_bytes(msg),0,0,DEBUG_FLAG_PRINT_COMUN_SEND,it->ID, 0);
            it->sent_init_acc = true;
	}
	if (it->received_init_acc)
	{
	    // Muzeme normalne komunikovat
            NsTimer main_timer("libcm", MyLibCM_name, 3, "EvalSendForPartnerWhenInited");
	    return EvalSendForPartnerWhenInited(it, ActTime);
	}
	else if (it->sent_init)
	{
	    // Cekame na INIT-ACC
	    return 10000;
	}
    }

    if (!it->sent_init || it->init_wait_until <= ActTime)
    {
        send_init_message(it);
	it->init_wait_until = ActTime + RetryTime;
	it->sent_init = true;
    }
    return it->init_wait_until - ActTime;
}

// TODO: tohle je cele spatne
// void CmCore::EvalSendFragments(double ActTime)
// {
//     if (!SendFragmentDataList.empty())
//     {
// 	// Tady nevim, no..
// 	bool partner_exist = false;
// 	if ((UDP_MODE) && (SendFragmentDST==0) && (Partners.size()>0)) partner_exist = true;
// 	if (!partner_exist)
// 	{
// 	    for (std::list <CCmPartner>::iterator it = Partners.begin(); it != Partners.end(); it++)
// 	    {
// 		if (it->ID == SendFragmentDST) partner_exist = true;
// 	    }
// 	}
// 	if (partner_exist)
// 	{
// 	    std::string act_msg = SendFragmentDataList.front();
// 	    SendFragmentDataList.pop_front();
// 	    char wstr[40];
// 	    sprintf(wstr,"FRAG,%d,%d,", SendFragmentCount - ((int)SendFragmentDataList.size() + 1), SendFragmentCount);
// 	    act_msg = std::string(wstr) + act_msg;
// 	    unsigned bytes = SendTextMessage(act_msg,1,0,DEBUG_FLAG_PRINT_COMUN_SEND,SendFragmentDST,0);
// 	    for (std::list <CCmPartner>::iterator it = Partners.begin(); it != Partners.end(); it++)
// 	    {
// 		if ((it->ID == SendFragmentDST) || ((UDP_MODE) && (SendFragmentDST==0)))
// 		{
// 		    CalculateSendBlockUntil(it, bytes);
// 		}
// 	    }
// 	}
// 	else
// 	{
// 	    SendFragmentDataList.clear();
// 	}
//     }
// }

void CmCore::EvalSend()
{
    cms_ns_if_print("libcm", 4, "EvalSend");
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    double ActTime = GetActualTimeDouble();
    if (stop_threads)
	return;

    // EvalSendFragments(ActTime);

    double sleep_time = 10000; // neco velkeho
    for (std::list <CCmPartner>::iterator it = Partners.begin(); it != Partners.end(); it++)
    {
	sleep_time = std::min(sleep_time, EvalSendForPartner(it, ActTime));
    }
        
    for (std::list <CCmPartner>::iterator it = Partners.begin(); it != Partners.end();)
    {
	std::list <CCmPartner>::iterator itd = it++;
	if (itd->DisconnectMe)
	{
	    WORKER_DEBUG("libcm",7,"libcm(%s):ACC not received (%d times, with period %g sec) UDP (%d) - Disconnected - successor",MyLibCM_name.c_str(),RetryCount,RetryTime,itd->ID);
	    if (LiveDebugFlags & DEBUG_FLAG_PRINT_COMUN_CONN)
	    {
		char xstr[64000];
		sprintf(xstr,"info:DISConnect (%d), successor",itd->ID);
		LiveDebugPrintANY(DEBUG_FLAG_PRINT_COMUN_CONN, itd->ID, "S:", xstr);
	    }
	    erasePartner(itd,true);
	}
    }
    // cms_ns_if_print("libcm", 3, "sleep_time_local=%lf", sleep_time_local);
    // if (sleep_time_local>0.0)
    PollCondSend->ChangeRqTime(ActTime + sleep_time);
}

void CmCore::EvalExt()
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    if (stop_threads)
	return;
    double ActTime = GetActualTimeDouble();
    DebugScopedUnlock unlock(*CM_Mutex, MyLibCM_name);
    if (m_ext_hook)
	m_ext_hook->EvalPollExt(*this, *PollCondExt, ActTime);
}

// void CmCore::PollThreadOneLoop (double &sleep_time)
// {

//     // LiveDebug ------------------============----------------===================----------------
//     double ActTime = GetActualTimeDouble();
//     EvalLiveDebug(ActTime);
//     PollCondLive.UpdateSleepTime(ActTime, sleep_time);

//     // SaveFile ------------------============----------------===================----------------
//     ActTime = GetActualTimeDouble();
//     EvalSaveFile(ActTime);
//     PollCondSave.UpdateSleepTime(ActTime, sleep_time);


//     // PING ------------------============----------------===================----------------
//     ActTime = GetActualTimeDouble();
//     EvalPing(ActTime);
//     PollCondPing.UpdateSleepTime(ActTime, sleep_time);

//     // SEND ------------------============----------------===================----------------
//     ActTime = GetActualTimeDouble();
//     EvalSend(ActTime);
//     PollCondSend.UpdateSleepTime(ActTime, sleep_time);

//     // EXT --------------------------------------------------
//     // NOTE: UpdateSleepTime : Je ActTime aktualni?????
//     ActTime = GetActualTimeDouble();
//     EvalExt(ActTime);
//     PollCondExt.UpdateSleepTime(ActTime, sleep_time);
// }

// void CmCore::PollThread (void)
// {

//   DEBUG_LOCK;
//   while (!stop_threads)
//   {
//     double sleep_time = 0.0;
//     PollThreadOneLoop (sleep_time);
//     WORKER_DEBUG("libcm", 22, "PollThreadOneLoop set sleep_time to %lf sec", sleep_time);
// //    if (sleep_time < 0.0001) sleep_time = 0.2; // commented 2019-06-24  !!!!!!!!
// //   WORKER_DEBUG("libcm", 22, "SaveThread wait %g sec ",sleep_time);

//          if (sleep_time == 0.0                  ) PollCondition->Wait(CM_Mutex);
//     else if (sleep_time >= LIB_CM_TIME_TOLERANCE) PollCondition->WaitTime(CM_Mutex, (long)(sleep_time * 1000000.0));

//   }
//   DEBUG_UNLOCK;
// }


// void CmCore::CB__ThreadOneLoop(bool LockMyMutex)
// {
//     WORKER_DEBUG("libcm", 22, "CB__ThreadOneLoop begin");
//     if (LockMyMutex) DEBUG_LOCK;
//     WORKER_DEBUG("libcm", 22, "CB__ThreadOneLoop begin2");
//     bool varcb_notified = false;
//     // Nevim, jestli je nutne to mit v cyklu. Pokud je m_just_notify == true,
//     // tak pri zmene promennych volame callback jenom jednou.
//     while (!stop_threads && (PartnerFiltersChanged || ((!m_just_notify || !varcb_notified) && !m_var_changes.empty())))
//     {
//         varcb_notified = true;
//         CheckAndCallCB();
//     }
//     WORKER_DEBUG("libcm", 22, "CB__ThreadOneLoop end1");
//     if (LockMyMutex) DEBUG_UNLOCK;
//     WORKER_DEBUG("libcm", 22, "CB__ThreadOneLoop end2");
// }


// #if (!LIBCM_CB_THREAD_ONCE)
// void CmCore::CB__Thread(void)
// {
//   DEBUG_LOCK;
//   while (!stop_threads)
//   {
// //    double ActTime = GetActualTimeDouble();
//     double sleep_time = 0.0;

// //    double calc_sleep_time = 0.0;
// //    if (sleep_time < 0.0001) sleep_time = 0.1;

//          if (sleep_time == 0.0                  ) CB__Condition->Wait(CM_Mutex);
//     else if (sleep_time >= LIB_CM_TIME_TOLERANCE) CB__Condition->WaitTime(CM_Mutex, (long)(sleep_time * 1000000.0));

//     CB__ThreadOneLoop();
//   }
//   DEBUG_UNLOCK;
// }
// #endif


void CmCore::RefreshFiltersBasic(bool useLockCM)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name, useLockCM);
    myFilters = myFiltersIn;
    myFilters.add(myFiltersExt); // TESTPOINT 1
    if (msv_control_variable)
        myFilters.add(*msv_control_variable);
    for (std::list <CCmPartner>::iterator it = Partners.begin(); it != Partners.end(); it++)
        it->myFiltersChange = true;
    WORKER_DEBUG("libcm_change", 22, "Refreshed myFilters: %s", print_var(myFilters).c_str());
    if (Partners.size()>0) PollCondSend->Wake();
    if (LiveDebugFlags & DEBUG_FLAG_DUMP_FILTERS_MY)
        LiveDebugPrintFilters(DEBUG_FLAG_DUMP_FILTERS_MY, 0, "S:",&myFilters);
}

void CmCore::RefreshFilters(bool useLockCM)
{
    bool changed = false;
    {
        DebugScopedLock lock(*CM_Mutex, MyLibCM_name, useLockCM);
        if (m_ext_hook)
        {
            WORKER_DEBUG("libcm", 22, "ExtHook: %s: RefreshExtFilters: in=%s",
                         MyLibCM_name.c_str(), myFiltersIn.toString().c_str());
            WORKER_DEBUG("libcm", 22, "ExtHook: %s: RefreshExtFilters: old_ext=%s",
                         MyLibCM_name.c_str(), myFiltersExt.toString().c_str());
            changed = m_ext_hook->RefreshExtFilters(myFiltersIn, myFiltersExt);
            WORKER_DEBUG("libcm", 22, "ExtHook: %s: RefreshExtFilters: changed=%d, new_ext=%s",
                         MyLibCM_name.c_str(), changed, myFiltersExt.toString().c_str());
        }
    }
    if (changed)
    {
	WORKER_DEBUG("libcmext",4,"libcm(%s):ExtFilterRequest - (myFiltersRq) changed : %s\n",
                     MyLibCM_name.c_str(),myFiltersExt.fullPrint(" | ").c_str());
    }
    RefreshFiltersBasic(useLockCM);
    if (changed)
    {
	PartnerFiltersChanged = true;
	generate_callback();
    }
}

void CmCore::ClearFilters()
{
  {
      DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
      myFiltersIn.clear();
  }
  RefreshFilters(true);
}


void  CmCore::DelFilter(std::string name)
{
    bool changed = false;
    {
        DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
        changed = myFiltersIn.erase(name);
    }
    if (changed)
        RefreshFilters(true);
}

void  CmCore::DelFilter(const char *name)
{
    DelFilter(std::string(name));
}

void  CmCore::DelFilters(std::list<std::string> names)
{
    bool changed = false;
    {
        DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
        changed = myFiltersIn.erase(names);
    }
    if (changed)
        RefreshFilters(true);
}

void CmCore::AddFilter(class CCmFilter *newFilter)
{
    bool changed = false;
    {
        DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
        WORKER_DEBUG("libcm_change", 4, "Add filter: %s", print_var(*newFilter).c_str());
        changed = myFiltersIn.add(newFilter);
    }
    if (changed)
        RefreshFilters(true);
}

void CmCore::AddFilters(std::list<class CCmFilter> *newFilter)
{
    bool changed = false;
    {
        DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
        WORKER_DEBUG("libcm_change", 4, "Add filters: %s", print_list(*newFilter).c_str());
        changed = myFiltersIn.add(newFilter);
    }
    if (changed)
        RefreshFilters(true);
}

CCmFiltersSet CmCore::GetMyFilters()
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    return myFilters;
}

void CmCore::SetFilters(std::list<class CCmFilter> *newFilter)
{
  bool changed = false;
  {
      DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
      WORKER_DEBUG("libcm_change", 4, "Set filters: %s", print_list(*newFilter).c_str());
      if (!myFiltersIn.empty()) changed = true;
      myFiltersIn.clear();
      if (myFiltersIn.add(newFilter)) changed = true;
  }
  if (changed) RefreshFilters(true);
}

void CmCore::SetFilters(std::map<std::string, class CCmFilter> *newFilter)
{
  bool changed = false;
  {
      DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
      WORKER_DEBUG("libcm_change", 4, "Set filters (%zu): %s", newFilter->size(), print_map(*newFilter).c_str());
      if (!myFiltersIn.empty()) changed = true;
      myFiltersIn.clear();
      if (myFiltersIn.add(newFilter)) changed = true;
  }
  if (changed) RefreshFilters(true);
}

void CmCore::SetFilters(class CCmFiltersSet *newFilter)
{
  bool changed = false;
  {
      DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
      WORKER_DEBUG("libcm_change", 4, "Set filters (%zu)): %s", newFilter->size(), newFilter->toString().c_str());
      if (!myFiltersIn.empty()) changed = true;
      myFiltersIn = *newFilter;
      if (!myFiltersIn.empty()) changed = true;
  }
  if (changed) RefreshFilters(true);
}
// void CmCore::recv_do_notify()
// {
//     cms_ns_if_print("cmip",3, "Notify");
//     DebugScopedLock lock(recv_mutex);
//     recv_notified = true;
//     recv_cond.Broadcast();
// }

// Nesmime zamykat.
void CmCore::recv_do_notify()
{
    PollCondRecv->Wake();
}

// Nesmime zamykat
void CmCore::master_slave_do_notify()
{
    PollCondMasterSlave->Wake();
}

// Tohle je callback z CmIP, ze se nekdo pripojil/odpojil nebo jsme
// prijali zpravu.
void CmCore::EvalRecv()
{
    cms_ns_if_print("cmip", 4, "recv_do_notify locking ...");
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    cms_ns_if_print("cmip", 4, "recv_do_notify locked");
    if (stop_threads)
        return;
    
    ThreadTimer t;
    for (;;)
    {
	Maybe<CmIpEvent> e = connection->get_new_event();
	if (!e)
        {
            cms_ns_if_print("cmip", 4, "recv_do_notify: No new event");
	    break;
        }
	cms_ns2_if_print("cmip", MyLibCM_name, 4, "Got new event: id=%u, just_connected=%d, just_disconnected=%d, msgcount=%zu",
			e->connection_id,
			e->just_connected,
			e->just_disconnected,
			e->messages.size());

        if (e->just_connected)
        {
            nowConnected(e->connection_id, e->address_string);
        }

        CM_for (std::string const &s, e->messages)
            OnRecvFromPartner(s.c_str(), s.size(), e->connection_id);

        if (e->just_disconnected)
        {
            OnPartnerDisconnected(e->connection_id);
        }
    }
    cms_ns_if_print("profile", 4, "recv_do_notify: %.3lf", t.elapsed_msec_double());
}

void CmCore::Start()
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    
    connection = create_connection(ip_descriptor,
                                   cm::bind_front(&CmCore::recv_do_notify, this),
                                   MyLibCM_name);

    if (!msv_control_ip_descriptor.ip_descriptors.empty())
    {
        msv_connection = create_connection(msv_control_ip_descriptor,
                                           cm::bind_front(&CmCore::master_slave_do_notify, this),
                                           MyLibCM_name);
    }

    if (m_ext_hook)
    {
        m_ext_hook->Start();
        PollCondExt->Wake();
    }

    has_started = true;

    // Tohle je potreba, abychom dostali callback od promennych z
    // konfiguraku.
    generate_callback();
}

unsigned CmCore::numConnectedPartners(void)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    return Partners.size();
}

std::string CmCore::getPartnersPrefixes(void)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    std::string RetVal = std::string("");
    for (std::list <CCmPartner>::iterator it = Partners.begin(); it != Partners.end(); it++)
    {
        if (it->PartnerVarPrefix.size()>0)
        {
            if (RetVal.size()>0)
            {
                RetVal += std::string(",") + it->PartnerVarPrefix;
            } else {
                RetVal += it->PartnerVarPrefix;
            }
        }
    }
    return RetVal;
}

// ---------------------------------------------------------

std::list<CCmPartner>::iterator CmCore::findVariableOwner(std::string name, CCmVariable *var)
{
    // std::list<CCmPartner>::iterator ret_it = Partners.end();
    for (std::list<CCmPartner>::iterator it = Partners.begin(); it != Partners.end(); it++)
    {
	if (!contains(it->variables_state.his_colliding_variables, name))
	{
	    VarMap::iterator it_v = it->variables_state.hisVariables.find(name);
	    if (it_v != it->variables_state.hisVariables.end())
	    {
		*var = it_v->second;
		return it;
	    }
	}
    }
    return Partners.end();
}

// 
// void CmCore::internalChangePartnerVariable(std::list<CCmPartner>::iterator it_p, std::string name, CCmVariable const *var_arg, CCmVariable const *orig_var, uint8_t newPriority)
// {
//     CCmVariable var = *var_arg;
//     double ActTime = GetActualTimeDouble();
//     var.setModifTime(ActTime);
//     if (it_p->ChangePartnerVariable(name, &var, newPriority))
//     {
//         if (orig_var != NULL)
//         {
//             var.diffAllFields(*orig_var);
//             // var.EventWhat = 2;
//             VariableUpdated_CB(&var,NULL,it_p->ID,true,false);
//         }
//         else
//         {
//             VariableUpdated_No_CB(&var, 0);
//         }
//         PollCondSend->Wake(); //// neprobouzet pokazde, ale chytreji
//     }
//     if (m_ext_hook)
//     {
// 	WORKER_DEBUG("libcm", 22, "ExtHook: internalChangePartnerVariable: %s", name.c_str());
// 	m_ext_hook->internalChangePartnerVariable(&var);
//     }
// }

// Koukneme, jestli nebyla v kolizi s partnerovymi
// promennymi. Pokud ano, tak vezememe prvniho partnera, ktereho
// najdeme, a jeho promennou nadefinujeme.
void CmCore::tryDefineSomeCollidingVariable(std::string const &name)
{
    if (contains(myVariables, name))
    {
	WORKER_DEBUG("libcm", 22, "trydefinesomecollidingvariable %s: Already in my variables", name.c_str());
	return;
    }
    CCmPartner *new_owner = 0;
    CM_for (CCmPartner &p, Partners)
    {
	if (use_master_slave && master_partner != &p)
	    continue;
	if (contains(p.variables_state.hisVariables, name))
        {
            if (!contains(p.variables_state.his_colliding_variables, name))
            {
                // Uz ji nekdo ma nadefinovaou
                WORKER_DEBUG("libcm", 2, "trydefinesomecollidingvariable: %s already owned by %s",
                             name.c_str(), p.address_string.c_str());
                return;
            }
            else
            {
                new_owner = &p;
            }
        }
    }
    if (new_owner)
    {
        WORKER_DEBUG("libcm", 2, "Defining previously colliding variable %s of partner %s",
                     name.c_str(), new_owner->address_string.c_str());
        new_owner->variables_state.his_colliding_variables.erase(name);
        CCmVariable &v = new_owner->variables_state.hisVariables.at(name);
        VariableCreated_CB(&v,new_owner->ID, true);
    }
    else
    {
        WORKER_DEBUG("libcm", 22, "trydefinesomecollidingvariable %s: Nothing to define", name.c_str());
    }
}

void CmCore::internalUndefineMyVariable(CCmVariable &v, bool generate_cb)
{
    // Tohle nevim, jestli je porteba. (nepouziva to treba SaveFile?)
    v.setModifTime(GetActualTimeDouble());

    // Upravime callback
    if (generate_cb)
	VariableDeleted_CB(&v, 0, true);
    else
	VariableDeleted_No_CB(&v,0);

    if (v.haveFlagExternal())
    {
	// Oddefinujeme ji u partneru
	bool new_data_to_send = false;
	for (std::list<CCmPartner>::iterator it = Partners.begin(); it != Partners.end(); it++)
	    if (it->UndefineMyVariable(v.getName()))
		new_data_to_send = true;
	if (new_data_to_send)
	{
	    WORKER_DEBUG("libcm", 22, "New data to send after undefine, PollCondSend.Wake()");
	    PollCondSend->Wake();
	}

	// Zmenime Savefile
	if (SaveFile_Type != 0 && will_be_saved(v))
	    SaveFileItemChangedNow();
    }
    // Zavolame Ext
    if (m_ext_hook)
    {
	WORKER_DEBUG("libcm", 22, "ExtHook: internalChangeMyVariable: undefine %s", v.getName().c_str());
	bool changed = m_ext_hook->internalChangeMyVariable(&v, 0 /* EventWhat */);
        if (changed)
            PollCondExt->Wake();
    }

    // Tohle musi jit az po Ext (asi)
    tryDefineSomeCollidingVariable(v.getName());
}

bool CmCore::will_be_saved(CCmVariable const &v)
{
    return v.haveFlagSaveFile() &&
        (savefile_policy_save_all || contains(savefile_enabled_variables, v.getName()));
}

void CmCore::internalUpdateExistingVariable(VarMap::iterator it_my, CCmVariable &var, bool gen_cb, uint8_t newPriority)
{
//        if ((it_my->second) != var_copy) // check ALL filters???? --- but check will be privided by each partner
//        {
    // var.EventWhat = 2; // UPDATE --- but check will be privided by each partner
    // EventWhat = 2;
    // send_to_partners = true;
    if (gen_cb) // GenerateCB
    {
	CCmVariable var_copy2 = it_my->second;
	it_my->second = var;
        it_my->second.EventFields.clear();
	if (newPriority != CMLIB_VALUE_PRIORITY_UNDEF)
	{
	    var.setFlagPriorityChanged(true);
	    var.setPriority(newPriority);
	}
	VariableUpdated_CB(&var, &var_copy2, 0, true, false);
    }
    else
    {
	VariableUpdated_No_CB(&var, 0);
	it_my->second = var;
        it_my->second.EventFields.clear();
    }
}
//        }
    
void CmCore::internalRedefineExistingVariable(VarMap::iterator it_my, CCmVariable &var, bool gen_cb, uint8_t newPriority)	
{
    // var.EventWhat = 1; // DEF // redefine fully --- but check will be privided by each partner
    // EventWhat = 1;
    it_my->second = var;
    var.EventFields.clear();
    // send_to_partners = true;
    if (gen_cb)
    {
	if (newPriority != CMLIB_VALUE_PRIORITY_UNDEF)
	{
	    var.setFlagPriorityChanged(true);
	    var.setPriority(newPriority);
	}
	VariableCreated_CB(&var,0,true);
    } else {
	VariableCreated_No_CB(&var,0);
    }
}

void CmCore::internalDefineNewVariable(CCmVariable &var, bool gen_cb, uint8_t newPriority)
{
    // var.EventWhat = 1; // DEF // new variable --- but check will be provided by each partner
    // EventWhat = 1;
    myVariables[var.getName()] = var;
    var.EventFields.clear();
    if (gen_cb)
    {
	if (newPriority != CMLIB_VALUE_PRIORITY_UNDEF)
	{
	    var.setFlagPriorityChanged(true);
	    var.setPriority(newPriority);
	}
	VariableCreated_CB(&var,0,true);
    } else {
	VariableCreated_No_CB(&var,0);
    }
}

// Pokud promennou menim ja, tak je partner_it == NUL.
// Pokud ji meni partner, tak je to iterator na neho.
void CmCore::internalChangeMyVariable(CCmVariable const *var_arg, CCmPartner *partner_it, bool gen_cb, uint8_t newPriority)
{
    AccumulatingTimer at3(debug_time_accum3);
    WORKER_DEBUG("libcm_change", 22, "internalChangeMyVariable (%s): %s", gen_cb ? "CB" : "NoCB", print_var(*var_arg).c_str());
    double ActTime = GetActualTimeDouble();
    std::string name = var_arg->getName();
    // unsigned EventWhat = var_arg->EventWhat; // var.EventWhat = 1; // 0=undefine , 1=define , 2=update
    CCmVariable var = *var_arg;
    var.setModifTime(ActTime);
    at3.stop();

    AccumulatingTimer at4(debug_time_accum4);
    VarMap::iterator it_my = myVariables.find(name);
    int EventWhat = 1; // Define
    // Tady pozor, tyhle funkce nastavi do var napriklad prioritu, atd.
    if (it_my != myVariables.end())
    {
	// TODO: Proc tady neni type and limits?
	if (it_my->second.compareType(var))
	{
	    internalUpdateExistingVariable(it_my, var, gen_cb, newPriority);
	    EventWhat = 2; // Update
	}
	else
	{
	    internalRedefineExistingVariable(it_my, var, gen_cb, newPriority);
	}
    }
    else
    {
	// cms_ns_if_print("libcm", 3, "internalDefinenewvariable");
	internalDefineNewVariable(var, gen_cb, newPriority);
    }
    at4.stop();

    AccumulatingTimer at5(debug_time_accum5);
    bool new_data_to_send = false;
    if (var_arg->haveFlagExternal())
    {
	// cms_ns_if_print("libcm", 3, "internalDefinenewvariable have flag external");
        for (std::list <CCmPartner>::iterator it = Partners.begin(); it != Partners.end(); it++)
        {
	    bool dont_send = partner_it && partner_it == &*it;
            bool changed = false;
            if (dont_send)
                changed = it->AddMyVariableModification(var);
            else
                changed = it->ChangeMyVariable(var, newPriority);
	    // cms_ns_if_print("libcm", 3, "internalDefinenewvariable changed = %d", changed);
	    if (changed && !dont_send)
		new_data_to_send = true;
	}
    }
    // cms_ns_if_print("libcm", 3, "PollCondSend doing Wake: new_data_to_send=%d", new_data_to_se
		    // nd);
    if (new_data_to_send)
    {
        PollCondSend->Wake(); //// neprobouzet pokazde, ale chytreji
    }
    // WORKER_DEBUG("libcm", 3, "Changed %s: SaveFileFlags=%d, ForeignFlag=%d",
    //              var_arg->getName().c_str(), var_arg->haveFlagSaveFile(), var_arg->haveFlagForeign());
    if ((SaveFile_Type!=0) && will_be_saved(*var_arg))
    {
        SaveFileItemChangedNow();
    }
    // Tady pozor, zmena jde do Extu pouze v pripade, ze je ode
    // me. Pokud je od partnera, tak do Ext dostane z callbacku.
    if (m_ext_hook && !partner_it)
    {
	WORKER_DEBUG("libcm", 22, "ExtHook: internalChangeMyVariable %s", var.getName().c_str());
	bool changed = m_ext_hook->internalChangeMyVariable(&var, EventWhat);
        if (changed) PollCondExt->Wake();
    }
    at5.stop();
}

void CmCore::markPartnersCollidingVariables(std::string const &varname)
{
    for (std::list<CCmPartner>::iterator itp = Partners.begin(); itp != Partners.end(); itp++)
    {
	CCmPartner &p = *itp;
	VarMap::iterator var_it = p.variables_state.hisVariables.find(varname);

	if (var_it != p.variables_state.hisVariables.end())
	{
	    WORKER_DEBUG("libcm", 1, "DefineMyVariable %s: Collision with %s",
			 varname.c_str(),
			 p.address_string.c_str());

	    p.variables_state.his_colliding_variables.insert(varname);
	}
    }
}

void CmCore::DefineMyVariable(CCmVariable const &var, bool _GenerateCB)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    internalDefineMyVariable(var, _GenerateCB);
}

void CmCore::DefineMyVariable(std::string str, bool _GenerateCB )
{
    CCmVariable var;
    std::string error_str = var.defineFromString(str);
    if (error_str.empty())
    {
//      std::string transformed = V.toStringDefine();
	// var.EventWhat = 1;
	DefineMyVariable(var, _GenerateCB);
    }
    else
    {
	WORKER_DEBUG("libcm", 1, "libcm(%s):DefineMyVariable variable error (%s)", MyLibCM_name.c_str(),error_str.c_str());
    }
}

void CmCore::DefineMyVariable(char **str, bool _GenerateCB )
{
  CCmVariable var;
  std::string error_str = var.defineFromArray(str,"");
  if (error_str.empty())
  {
      DefineMyVariable(var, _GenerateCB);
  }
  else
  {
      WORKER_DEBUG("libcm", 1, "libcm(%s):load variable error (%s)", MyLibCM_name.c_str(),error_str.c_str());
  }
}

void CmCore::DefineMyVariable(CCmVariable const *var, bool _GenerateCB )
{
    DefineMyVariable(*var, _GenerateCB);
}

bool CmCore::UndefineVariable(std::string name, bool _GenerateCB )
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    VarMap::iterator it_my = myVariables.find(name);
    if (it_my == myVariables.end())
    {
	WORKER_DEBUG("libcm", 1, "UndefineMyVariable: Variable %s does not exist", name.c_str());
	return false;
    }
    WORKER_DEBUG("libcm_change", 4, "UndefineVariable %s", name.c_str());
    CCmVariable v = it_my->second; // copy
    myVariables.erase(it_my);
    internalUndefineMyVariable(v, _GenerateCB);
    return true;
}

void CmCore::ClearMyVariables(bool _GenerateCB)
{
    {
	DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
	while (myVariables.size()>0)
	{
	    VarMap::iterator it = myVariables.begin();
	    internalChangeMyVariable(&(it->second),NULL, _GenerateCB, CMLIB_VALUE_PRIORITY_UNDEF);
	}
    }    
    // if (need_call_notify_callback)
    // 	NotifyCallback();
}

void CmCore::DefineMyVariables(VarMap const *newVariables, bool _GenerateCB )
{
    if (!newVariables)
        return;
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    debug_time_accum1.reset();
    debug_time_accum2.reset();
    debug_time_accum3.reset();
    debug_time_accum4.reset();
    debug_time_accum5.reset();
    debug_time_accum6.reset();

    ThreadTimer tt;
    for (VarMap::const_iterator it = newVariables->begin(); it != newVariables->end(); it++)
    {
        internalDefineMyVariable(it->second, _GenerateCB);
    }
    PROFILE(debug_time_accum1, "%s accum1 (%zu vars)", MyLibCM_name.c_str(), newVariables->size());
    PROFILE(debug_time_accum2, "%s accum2 (%zu vars)", MyLibCM_name.c_str(), newVariables->size());
    PROFILE(debug_time_accum3, "%s accum3 (%zu vars)", MyLibCM_name.c_str(), newVariables->size());
    PROFILE(debug_time_accum4, "%s accum4 (%zu vars)", MyLibCM_name.c_str(), newVariables->size());
    PROFILE(debug_time_accum5, "%s accum5 (%zu vars)", MyLibCM_name.c_str(), newVariables->size());
    PROFILE(debug_time_accum6, "%s accum6 (%zu vars)", MyLibCM_name.c_str(), newVariables->size());

    PROFILE(tt, "%s total (%zu vars)", MyLibCM_name.c_str(), newVariables->size());
}

void CmCore::LoadConfigVariables(CMSConfig *config, VarMap *LoadedVariables)
{
  libCM_LoadConfigVariables(config, LoadedVariables, &MyLibCM_name);
  // Nastav, ze je muzem modifikovat
  for (auto &p : *LoadedVariables)
      p.second.setFlagOwned(true);
}

void CmCore::LoadConfigMyVariables(CMSConfig *config)
{
    VarMap LoadedVariables;
    LoadConfigVariables(config, &LoadedVariables);
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    WORKER_DEBUG("libcm_change", 4, "Loading variables from config: %zu variables", LoadedVariables.size());
    for (VarMap::iterator it = LoadedVariables.begin(); it != LoadedVariables.end(); it++)
    {
	// it->second.EventWhat = 1;
	WORKER_DEBUG("libcm_change", 4, "Loading from config: Variable %s", print_var(it->second).c_str());
	internalChangeMyVariable(&it->second,NULL, true, CMLIB_VALUE_PRIORITY_UNDEF);
        savefile_enabled_variables.insert(it->first);
    }
//  WORKER_DEBUG("libcm", 3,"\n %s",ListAllVariables().c_str());
}

template <typename Change>
bool CmCore::ChangePartnerVariableMasterSlave(std::string name, uint8_t newPriority, bool add_to_cb, Change change)
{
    bool found = false;
    double ActTime = GetActualTimeDouble();
    for (std::list<CCmPartner>::iterator it_p = Partners.begin(); it_p != Partners.end(); ++it_p)
    {
	VarMap::iterator it = it_p->variables_state.hisVariables.find(name);
	if (it != it_p->variables_state.hisVariables.end())
	{
	    if (&*it_p == master_partner || msv_send_mod_to_all)
	    {
		// Pozn: Pokud neni nastaven master, ale slave
		// tuto promennou ma, tak se stejne bere jako
		// nenalezena.
		found = true;
		CCmVariable var = it->second; // copy current value
		bool changed = change(var);
		WORKER_DEBUG("libcm_change",4, "ChangePartnerVariable (changed=%d) %s", changed, print_var(var).c_str());
		if (changed)
		{
		    var.diffAllFields(it->second);
		    var.setModifTime(ActTime);
		    bool changed = it_p->ChangePartnerVariable(name, &var, newPriority);
		    if (changed && &*it_p == master_partner)
		    {
			PollCondSend->Wake();
			// Generate CB
			if (add_to_cb)
			    VariableUpdated_CB(&var,NULL,it_p->ID,true,false);
			else
			    VariableUpdated_No_CB(&var, 0 /* conn_id */);
			// Pass changes to Ext
			if (m_ext_hook)
                        {
			    bool changed = m_ext_hook->internalChangePartnerVariable(&var);
                            if (changed)
                                PollCondExt->Wake();

                        }
		    }
		}
	    }
	}
    }
    return found;
}

template <typename Change>
bool CmCore::ChangePartnerVariableNoMasterSlave(std::string name, uint8_t newPriority, bool add_to_cb, Change change)
{
    // Nemusime to prochazet cele, ale delame to kvuli assertum...
    bool found = false;
    bool found_name = false;
    double ActTime = GetActualTimeDouble();
    for (std::list<CCmPartner>::iterator it_p = Partners.begin(); it_p != Partners.end(); ++it_p)
    {
	VarMap::iterator it = it_p->variables_state.hisVariables.find(name);
	if (it != it_p->variables_state.hisVariables.end())
	{
	    found_name = true;
	    // Pozn: Vime, ze neni v myVariables. Takze pokud
	    // je v colliding variables, tak to znamena, ze
	    // dva partneri maji promennou stejneho jmena.
	    if (!contains(it_p->variables_state.his_colliding_variables, name))
	    {
		assert(!found); // Dva partneri maji stejnou nekolidujici promennou
		found = true;
		CCmVariable var = it->second; // copy current value
		bool changed = change(var);
		WORKER_DEBUG("libcm_change",4, "ChangePartnerVariable (changed=%d) %s", changed, print_var(var).c_str());
		if (changed)
		{
		    var.setModifTime(ActTime);
		    bool changed = it_p->ChangePartnerVariable(name, &var, newPriority);
		    if (changed)
		    {
			PollCondSend->Wake();
			// Generate CB
			if (add_to_cb)
			    VariableUpdated_CB(&var,NULL,it_p->ID,true,false);
			else
			    VariableUpdated_No_CB(&var, 0 /* conn_id */);
			// Pass changes to Ext
			if (m_ext_hook)
                        {
			    bool changed = m_ext_hook->internalChangePartnerVariable(&var);
                            if (changed)
                                PollCondExt->Wake();

                        }
		    }
		}
	    }
	}
    }
    if (!found)
	if (found_name) // Promenna existuje, ale vsichni ji maj v koliznich
            assert(false);
    return found;
}

// Change: (CCmVariable &) -> Bool
// Change muze zmenit hodnotu, pokud tak ucini, musi vratit true
template <typename Change>
bool CmCore::ChangeVariableTempl(std::string name, uint8_t newPriority, bool add_to_cb, Change change)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    VarMap::iterator it = myVariables.find(name);
    if (it != myVariables.end())
    {
	CCmVariable var = it->second; // copy current value
	bool changed = change(var);
	var.diffAllFields(it->second);
	WORKER_DEBUG("libcm_change",4, "ChangeMyVariable (changed=%d) %s", changed, print_var(var).c_str());
	if (changed)
	{
	    internalChangeMyVariable(&var,NULL,add_to_cb,newPriority);
	}
	return true;
    }
    else
    {
	bool found = false;
	if (use_master_slave)
	    found = ChangePartnerVariableMasterSlave(name, newPriority, add_to_cb, change);
	else
	    found = ChangePartnerVariableNoMasterSlave(name, newPriority, add_to_cb, change);
	if (!found)
	{
	    WORKER_DEBUG("libcm", 1, "Cannot change variable %s, not found", name.c_str());
	}
	return found;
    }
}

struct ChangeVariableIfNeeded
{
    CCmVariable const *new_var;
    ChangeVariableIfNeeded(CCmVariable const *v) : new_var(v) {}
    bool operator()(CCmVariable &orig)
    {
        if (orig != *new_var)
        {
            orig = *new_var;
            return true;
        }
        return false;
    }
};

void CmCore::ChangeVariable(CCmVariable const *new_var_arg, uint8_t newPriority, bool _GenerateCB)
{
    if (!new_var_arg)
	throw std::logic_error("ChangeVariable: got NULL");
    CCmVariable new_var = *new_var_arg;
    if (new_var.getType() == '?')
	throw std::logic_error("ChangeVariable: got type '?'");
    new_var.EventFields.clear();
    double ActTime = GetActualTimeDouble();
    new_var.setModifTime(ActTime);
    ChangeVariableTempl(new_var.getName(), newPriority, _GenerateCB, ChangeVariableIfNeeded(&new_var));
}

template <typename T>
struct ChangeSimpleVariable
{
    typedef bool (CCmVariable::*Fun)(T, unsigned, unsigned);
    Fun f;
    T *value;
    unsigned idx_y;
    unsigned idx_x;
    
    ChangeSimpleVariable(Fun a_f, T *v, unsigned y, unsigned x) : f(a_f), value(v), idx_y(y), idx_x(x) {}
        
    bool operator()(CCmVariable &var)
    {
        return (var.*f)(*value, idx_y, idx_x);
    }
};

struct ChangeStrVariable
{
    std::string params;    
    ChangeStrVariable(std::string p) : params(p) {}
    bool operator()(CCmVariable &var)
    {
        return var.setVALUES(params, NULL);
    }
};
    
void CmCore::ChangeVariable(std::string name, std::string StrParams, uint8_t newPriority)
{
    ChangeVariableTempl(name, newPriority, false, ChangeStrVariable(StrParams));
}

void CmCore::ChangeVariableBool(std::string name, bool value, unsigned idx_y, unsigned idx_x, uint8_t newPriority)
{
    bool (CCmVariable::*f)(bool, unsigned, unsigned) = &CCmVariable::setVariableBool;
    ChangeVariableTempl(name, newPriority, false, ChangeSimpleVariable<bool>(f, &value, idx_y, idx_x));
}

void CmCore::ChangeVariableInt(std::string name, libcm_integer value, unsigned idx_y, unsigned idx_x, uint8_t newPriority)
{
    bool (CCmVariable::*f)(libcm_integer, unsigned, unsigned) = &CCmVariable::setVariableInt;
    ChangeVariableTempl(name, newPriority, false, ChangeSimpleVariable<int64_t>(f, &value, idx_y, idx_x));
}

void CmCore::ChangeVariableFloat(std::string name, double value, unsigned idx_y, unsigned idx_x, uint8_t newPriority)
{
    bool (CCmVariable::*f)(double, unsigned, unsigned) = &CCmVariable::setVariableFloat;
    ChangeVariableTempl(name, newPriority, false, ChangeSimpleVariable<double>(f, &value, idx_y, idx_x));
}

void CmCore::ChangeVariableString(std::string name, std::string value, unsigned idx_y, unsigned idx_x, uint8_t newPriority)
{
    bool (CCmVariable::*f)(std::string, unsigned, unsigned) = &CCmVariable::setVariableString;
    ChangeVariableTempl(name, newPriority, false, ChangeSimpleVariable<std::string>(f, &value, idx_y, idx_x));
}

void CmCore::ChangeVariableEnum(std::string name, int value, unsigned idx_y, unsigned idx_x, uint8_t newPriority)
{
    bool (CCmVariable::*f)(int64_t, unsigned, unsigned) = &CCmVariable::setVariableEnum;
    int64_t val = value;
    ChangeVariableTempl(name, newPriority, false, ChangeSimpleVariable<int64_t>(f, &val, idx_y, idx_x));
}

void CmCore::ChangeVariableEnum(std::string name, std::string value, unsigned idx_y, unsigned idx_x, uint8_t newPriority)
{
    bool (CCmVariable::*f)(std::string, unsigned, unsigned) = &CCmVariable::setVariableEnum;
    ChangeVariableTempl(name, newPriority, false, ChangeSimpleVariable<std::string>(f, &value, idx_y, idx_x));
}

// Pri kolizi vrati true, jinak false.
//
// Pokud uz ja nebo nekdo jiny ma tuto promennou nadefinovanou, tak
// toho src_it, kdo ji chce definovat, odpojime.
// 
bool CmCore::doesPartnerVariableCollide(std::string name, CCmPartner &src_it)
{
    if (contains(myVariables, name))
    {
	WORKER_DEBUG("libcm", 1, "Received DEF %s from %s, but there is a variable collision with my variables",
		     name.c_str(),
		     src_it.address_string.c_str());
	return true;
    }
    
    if (!use_master_slave)
    {
	CM_for (CCmPartner &p, Partners)
	{
	    if (&p != &src_it && contains(p.variables_state.hisVariables, name) && !contains(p.variables_state.his_colliding_variables, name))
	    {
		WORKER_DEBUG("libcm", 1, "Received DEF %s from %s, but there is a variable collision with partner %s",
			     name.c_str(),
			     src_it.address_string.c_str(),
			     p.address_string.c_str());
		return true;
	    }
	}
    }
    return false;
}

//     bool retval = false;
//     VarMap::iterator it = myVariables.find(name);
//     if (it != myVariables.end())
//     {
//         WORKER_DEBUG("libcm", 1,"libcm(%s):received DEF %s, but variable collision is here - disconnect ME(%d)",
//                         MyLibCM_name.c_str(),name.c_str(),src_it->ID);
//         if (LiveDebugFlags & DEBUG_FLAG_PRINT_COMUN_CONN)
//         {
//             char xstr[64000];
//             sprintf(xstr,"info:DISConnect (%d), variable collision \"%s\" MY_VARIABLES and",
//                     src_it->ID, name.c_str());
//             LiveDebugPrintANY(DEBUG_FLAG_PRINT_COMUN_CONN, src_it->ID, "S:", xstr);
//         }
//         src_it->DisconnectMe = true;
//         retval = true;
//     }
//     else
//     {
//         if (!use_master_slave)
//         {
//             for (std::list <CCmPartner>::iterator itp = Partners.begin(); itp != Partners.end(); itp++)
//             {
//                 if (itp != src_it)
//                 {
//                     VarMap::iterator itv = itp->hisVariables.find(name);
//                     if (itv != itp->hisVariables.end())
//                     {
//                         WORKER_DEBUG("libcm", 1,"libcm(%s):received DEF but variable collision is here - disconnect HIM(%d)",MyLibCM_name.c_str(),itp->ID);
//                         if (LiveDebugFlags & DEBUG_FLAG_PRINT_COMUN_CONN)
//                         {
//                             char xstr[64000];
//                             sprintf(xstr,"info:DISConnect (%d), variable collision \"%s  .. partner ISs (%d,%d) \n ", itp->ID, name.c_str(), itp->ID, src_it->ID);
//                             LiveDebugPrintANY(DEBUG_FLAG_PRINT_COMUN_CONN, itp->ID, "S:", xstr);
//                         }
// 			src_it->DisconnectMe = true;
//                         // itp->DisconnectMe = true;
//                         retval = true;
//                     }
//                 }
//             }
//         }
//     }
//     return retval;
// }

class CCmVariable CmCore::GetVariable( std::string name, bool *found_and_filled)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    *found_and_filled = false;
    VarMap::iterator it = myVariables.find(name);
    if (it != myVariables.end())
    {
        *found_and_filled = true;
	return it->second;
    }
    else
    {
        for (std::list <CCmPartner>::iterator itp = Partners.begin(); itp != Partners.end(); itp++)
        {
            if (use_master_slave && &*itp != master_partner)
                continue;
	    if (contains(itp->variables_state.his_colliding_variables, name))
		continue;
            VarMap::iterator itv = itp->variables_state.hisVariables.find(name);
            if (itv != itp->variables_state.hisVariables.end())
            {
		*found_and_filled = true;
		return itv->second;
            }
        }
    }
    return CCmVariable();
}

void CmCore::AddAllVariables(VarMap &retMap, uint8_t FlagMaskTest, uint8_t FlagMaskVal)
{
  AddMyVariables(retMap, FlagMaskTest, FlagMaskVal);
  AddPartnersVariables(retMap, FlagMaskTest, FlagMaskVal);
}

void CmCore::GetAllVariables(VarMap &retMap, uint8_t FlagMaskTest, uint8_t FlagMaskVal)
{
  retMap.clear();
  GetMyVariables(retMap, FlagMaskTest, FlagMaskVal);
  AddPartnersVariables(retMap, FlagMaskTest, FlagMaskVal);
}



void CmCore::AddPartnersVariables(VarMap &retMap, uint8_t FlagMaskTest, uint8_t FlagMaskVal)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
//  retMap = myVariables;

  for (std::list <CCmPartner>::iterator itp = Partners.begin(); itp != Partners.end(); itp++)
  {
      if (use_master_slave && &*itp != master_partner)
          continue;
      for (VarMap::iterator itv = itp->variables_state.hisVariables.begin(); itv != itp->variables_state.hisVariables.end(); itv++)
      {
	  if (!contains(itp->variables_state.his_colliding_variables, itv->first))
	  {
	      if ((FlagMaskTest==0) || ((itv->second.getFlags() & FlagMaskTest) == FlagMaskVal))
	      {
		  retMap[itv->first] = itv->second;
	      }
	  }
      }
  }
}

void CmCore::GetPartnersVariables(VarMap &retMap, uint8_t FlagMaskTest, uint8_t FlagMaskVal)
{
  retMap.clear();
  AddPartnersVariables(retMap, FlagMaskTest, FlagMaskVal);
}

void CmCore::AddMyVariables(VarMap &retMap, uint8_t FlagMaskTest, uint8_t FlagMaskVal)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    for (VarMap::iterator itv = myVariables.begin(); itv != myVariables.end(); itv++)
    {
      if ((FlagMaskTest==0) || ((itv->second.getFlags() & FlagMaskTest) == FlagMaskVal))
      {
        retMap[itv->first] = itv->second;
      }
    }
}

void CmCore::GetMyVariables(VarMap &retMap, uint8_t FlagMaskTest, uint8_t FlagMaskVal)
{
    retMap.clear();
    if (FlagMaskTest==0)
    {
        DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
        retMap = myVariables;
    }
    else
    {
        AddMyVariables(retMap, FlagMaskTest, FlagMaskVal);
    }
}


std::string CmCore::ListAllVariables(bool printAge)
{
  std::string RetVal = std::string("");
  char WStr[50];
  RetVal += std::string("==============ListAllVariables begin=====================\n");
  DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
  double ActTime = 0.0;
  if (printAge) ActTime = GetActualTimeDouble();
  for (VarMap::iterator it = myVariables.begin(); it != myVariables.end(); it++)
  {
    RetVal += std::string("  @@my@@ ") + it->second.fullPrint(ActTime) + std::string("\n");
  }
  for (std::list <CCmPartner>::iterator itp = Partners.begin(); itp != Partners.end(); itp++)
  {
      if (use_master_slave && &*itp != master_partner)
          continue;
      for (VarMap::iterator itv = itp->variables_state.hisVariables.begin(); itv != itp->variables_state.hisVariables.end(); itv++)
      {
          sprintf(WStr,"  @@%02d@@ ",itp->ID);
          RetVal += std::string(WStr) + itv->second.fullPrint(ActTime) + std::string("\n");
      }
  }
  RetVal += std::string("==============ListAllVariables end=====================\n");
  return RetVal;
}


// -----------------  savefile ---------------------------------

void CmCore::SaveFileItemChangedNow(void)
{
  double ActTime = GetActualTimeDouble();
  bool wake_up = false;
  if (SaveFile_FirstUnsavedChange == 0.0) wake_up = true;
  if (SaveFile_FirstUnsavedChange == 0.0) SaveFile_FirstUnsavedChange = ActTime;
  SaveFile_LastUnsavedChange = ActTime;
  if (wake_up) PollCondSave->Wake();
// WORKER_DEBUG("libcm", 22, "SIGNAL TO SAVEFILE  change at %f wake_up=%s",ActTime, (wake_up ? "Yes" : "No"));
}


void CmCore::SaveFile_LoadVariable(std::string const &name, CCmVariable &var)
{
    VarMap::iterator it_my = myVariables.find(name);
    if (it_my != myVariables.end())
    {
        if (it_my->second.compareType(var))
//          if (it_my->second.compareTypeAndLimits(var))
        {
//            if ((it_my->second) != var) // check ALL filters???? --- but check will be privided by each partner
            {
                // var.EventWhat = 2;
                if (!it_my->second.compareTypeAndLimits(var))
                {
                    var.copy_limits(it_my->second);
                }
                internalChangeMyVariable(&var,NULL,true, CMLIB_VALUE_PRIORITY_UNDEF);
            }
        }
        else
        {
            WORKER_DEBUG("libcm", 1,"libcm(%s):SaveFile_Load: cannot use loaded variable \"%s\", because type is different",MyLibCM_name.c_str(),name.c_str());
        }
    }
    else
    {
        WORKER_DEBUG("libcm", 1,"libcm(%s):SaveFile_Load: cannot define new variable \"%s\"",MyLibCM_name.c_str(),name.c_str());
    }
}

void CmCore::SaveFile_Load()
{
    cms_ns_if_print("libcm", 3, "Loading SaveFile %s", SaveFile_Params.c_str());
    std::string SaveFile_Name = SaveFile_Params;
    if (SaveFile_Type == 1)
    {
        std::vector<std::string> file_content;

        // Nacti soubor
        FILE *F = fopen(SaveFile_Name.c_str(),"rt");
        if (F != NULL)
        {
            char *Line  = new char[MSGMAX];
            while (fgets(Line,MSGMAX,F)!=NULL)
            {
                if ((strlen(Line)>0) && (Line[strlen(Line)-1]=='\n'))
                    Line[strlen(Line)-1] = 0;
                if (strlen(Line)>0)
                {
                    file_content.push_back(std::string(Line));
                }
            }
            fclose(F);
            delete [] Line;

            if (file_content.size()>0)
            {
                for (std::vector<std::string>::iterator it = file_content.begin(); it != file_content.end(); it++)
                {
                    CCmVariable var;
                    var.defineFromString((*it));
                    std::string name = var.getName();
                    SaveFile_LoadVariable(name, var);
                }
            }
            else
            {
                WORKER_DEBUG("libcm", 3, "SaveFile %s exists, but it is empty", SaveFile_Name.c_str());
            }
        }
        else
        {
            int err = errno;
            if (err == ENOENT)
            {
                WORKER_DEBUG("libcm", 3, "SaveFile %s does not exist", SaveFile_Name.c_str());
            }
            else
            {
                WORKER_DEBUG("libcm", 1, "Failed to open SaveFile %s: %s", SaveFile_Name.c_str(), strerror(err));
            }
        }
    }
    else
    {
        WORKER_DEBUG("libcm", 1,"SaveFile_Load: with type %d - unimplemented now", SaveFile_Type);
    }
    SaveFile_LastUnsavedChange = 0.0;
    SaveFile_FirstUnsavedChange = 0.0;
    SaveFile_LOAD = false;
}


void CmCore::SaveFile_Save()
{
    cms_ns_if_print("libcm", 3, "SaveFile_Save");

  std::string SaveFile_Name = SaveFile_Params;
  while (SaveFile_LOAD) usleep(10000);

  SaveFile_LastUnsavedChange = 0.0;
  SaveFile_FirstUnsavedChange = 0.0;
  // create path directories, if not exist
  if (SaveFile_Type == 1)
  {
    std::list<std::string> file_content;
    for (VarMap::iterator it = myVariables.begin(); it != myVariables.end(); it++)
    {
      if (will_be_saved(it->second))
      {
        file_content.push_back(it->second.toStringDefine());
      }
    }
    std::string file_content_1 = std::string("");
    file_content.sort();
    for (std::list<std::string>::iterator it = file_content.begin(); it != file_content.end(); it++)
    {
     file_content_1 += (*it) + std::string("\n");
    }
    std::string SaveFile_tmp = SaveFile_Name + std::string(".tmp");

    // mutex->Unlock();
    // SaveFileMutex->Lock();
    FILE *F = fopen(SaveFile_tmp.c_str(),"wt");
    if (F!=NULL)
    {
      fprintf(F,"%s",file_content_1.c_str());
      fclose(F);
      int result= rename( SaveFile_tmp.c_str(), SaveFile_Name.c_str() );
      if ( result == 0 )
      {
        WORKER_DEBUG("libcm", 6,"libcm(%s):SAVEFILE \"%s\" successfully saved",MyLibCM_name.c_str(),SaveFile_Name.c_str());
      } else {
        WORKER_DEBUG("libcm", 1,"libcm(%s):SAV Error renaming file \"%s\" -> \"%s\"",MyLibCM_name.c_str(),SaveFile_tmp.c_str(), SaveFile_Name.c_str());
      }
    } else WORKER_DEBUG("libcm", 1,"libcm(%s):Cannot Save SaveFile ... cannot create temporary file \"%s\"",MyLibCM_name.c_str(),SaveFile_tmp.c_str());
    // SaveFileMutex->Unlock();
    // mutex->Lock();

  } else {
    WORKER_DEBUG("libcm", 1,"libcm(%s):Cannot Save SaveFile with type : %d",MyLibCM_name.c_str(),SaveFile_Type);
  }
  // zavest SaveFileMutex a tam delat operace s HDD mimo hlavni zamek a pak updatovat mapu z mapy v global locku
}

void CmCore::StartUseSaveFile(bool _Blocked)
{
    cms_ns_if_print("libcm", 3, "StarUseSaveFile");
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    if (SaveFile_Type != 0)
    {
        if (_Blocked)
        {
            SaveFile_Load();
            SaveFile_USE = true;
            SaveFile_firstLoop = true;
            PollCondSave->Wake();
        }
        else
        {
            SaveFile_LOAD = true;
            SaveFile_USE = true;
            SaveFile_firstLoop = true;
            PollCondSave->Wake();
        }
    }
}

void CmCore::StopUseSaveFile(bool _EnableSaveNow)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    SaveFile_LOAD = false;
    SaveFile_USE = false;
    PollCondSave->Wake();
}
















// ========================================================================================================================================================

void CmCore::LiveDebugInit(void)
{

  LiveDebug.Init();
  LiveDebug.SetAllFlagsPTR(&LiveDebugFlags);
  std::map<std::string, std::string> ConstantsMap;
  ConstantsMap["$CMNAME"] = MyLibCM_name;
  // ConstantsMap["$CONN"] = IpDesc_Changed;
  ConstantsMap["$PNAME"] = MyPrgName;
  ConstantsMap["$PID"] = MyPidStr;
  LiveDebug.SetConstants(ConstantsMap);
}
void CmCore::LiveDebugDone(bool NeedLock)
{
  LiveDebug.Done(NeedLock);
}
void CmCore::LiveDebugCheckChanges(void)
{
  LiveDebug.CheckChanges();
}
void CmCore::LiveDebugSetControlFiles(std::string Files)
{
  LiveDebug.SetControlFiles(Files);
}
void CmCore::LiveDebugPrintANY(uint64_t TestFlags, unsigned conn_id, const char *BeforeData, const char *Data)
{
  LiveDebug.PrintANY(TestFlags, conn_id, BeforeData, Data);
}
void CmCore::LiveDebugPrintANY(uint64_t TestFlags, unsigned conn_id, std::string &BeforeData, std::string &Data)
{
  LiveDebug.PrintANY(TestFlags, conn_id, BeforeData, Data);
}
void CmCore::LiveDebugPrintFilters(uint64_t TestFlags, unsigned conn_id, const char *BeforeData, class CCmFiltersSet *Filters)
{
  LiveDebug.PrintFilters(TestFlags, conn_id, BeforeData, Filters);
}
void CmCore::LiveDebugPrintVar(uint64_t TestFlags, unsigned conn_id, const char *BeforeData, class CCmVariable const *var)
{
  LiveDebug.PrintVar(TestFlags, conn_id, BeforeData, var);
}
void CmCore::LiveDebugPrintVars(uint64_t TestFlags, unsigned conn_id, const char *BeforeData, VarMap *mapVar)
{
  LiveDebug.PrintVars(TestFlags, conn_id, BeforeData, mapVar);
}

//////////////////////////////////////////////////////////////////////

CCmPartner *CmCore::find_master_partner(std::list<CCmPartner> &list)
{
    CM_for (CCmPartner &p, list)
    {
        // Podle control variable
        if (msv_control_variable && p.master_slave_control_variable_value == true)
            return &p;
        // Podle toho, co prislo z CMS
        if (!msv_master_name.empty() && p.partner_name == msv_master_name)
            return &p;
    }
    return 0;
}

std::string maybe_partner_name(CCmPartner const *p)
{
    return p ? p->address_string : "(none)";
}

// new_master je novy master nebo NULL
// CmCore::master_partner je aktualni master nebo NULL
void CmCore::switch_master_variables(CCmPartner *new_master)
{
    if (master_partner == new_master)
	return;

    WORKER_DEBUG("msv", 3, "switch_master_variables from %s to %s",
		 maybe_partner_name(master_partner).c_str(),
		 maybe_partner_name(new_master).c_str());
    // Definuj/Predefinuj/Updatni promenne noveho mastera
    if (new_master)
    {
        CM_for (VarMap::value_type &p, new_master->variables_state.hisVariables)
        {
            std::string const &name = p.first;
            CCmVariable &new_var = p.second;
            bool need_create = true;
            if (master_partner)
            {
                if (CCmVariable const *orig_var = map_find(master_partner->variables_state.hisVariables, name))
                {    
                    if (new_var != *orig_var)
                    {
                        if (new_var.compareTypeAndLimits(*orig_var))
                        {
                            CCmVariable copy = new_var;
                            copy.diffAllFields(*orig_var);
                            need_create = false;
                            WORKER_DEBUG("msv", 4, "switch updated %s", name.c_str());
                            VariableUpdated_CB(&copy, orig_var, new_master->ID, false, false);
                        }
                    }
                    else
                    {
                        // nemusime nic delat, hodnoty jsou stejne
                        need_create = false;
                    }
                }
                else
                {
                    // musime ji vytvorit, nechame need_create=true
                }
            }
            if (need_create)
            {
                WORKER_DEBUG("msv", 4, "switch created %s", name.c_str());
                VariableCreated_CB(&new_var, new_master->ID, false);
            }
        }
    }
    // Oddefinuj promenne predchoziho mastera
    if (master_partner)
    {
        CM_for (VarMap::value_type &p, master_partner->variables_state.hisVariables)
        {
            bool need_delete = true;
            if (new_master)
            {
                VarMap::iterator it = new_master->variables_state.hisVariables.find(p.first);
                if (it != new_master->variables_state.hisVariables.end())
                    need_delete = false;
            }
            if (need_delete)
            {
                WORKER_DEBUG("msv", 4, "switch deleted %s", p.first.c_str());
                VariableDeleted_CB(&p.second, master_partner->ID, false);
            }
        }
    }

    msv_changed = true;
}

void CmCore::msv_refresh_internal()
{
    std::stringstream ss;
    CM_for (CCmPartner &p, Partners)
	ss << p.ID << ":'" << p.partner_name << "' ";
	
    WORKER_DEBUG("msv", 4, "msv_refresh_internal: group=%s, master_name=%s, available_partners=%s",
		   msv_group_name.c_str(), msv_master_name.c_str(), ss.str().c_str());

    CCmPartner *new_master = find_master_partner(Partners); // or nullptr
    if (master_partner && new_master == master_partner)
    {
        // nechavame puvodniho
        return;
    }
    
    if (!new_master && msv_always_connect)
    {
        if (master_partner)
	{
	    WORKER_DEBUG("msv", 4, "No client matches, doing nothing");
            return; // Nechame tam toho puvodniho
	}
        if (!Partners.empty())
	{
	    WORKER_DEBUG("msv", 4, "No client matches, choosing random client");
            new_master = &Partners.front(); // Vezmeme libovolneho
	}
    }
    if (new_master)
        db_state->set_master(new_master->ID);
    else
        db_state->unset_master();
    
    switch_master_variables(new_master);

    // Nastav noveho mastera (muze byt i NULL)
    master_partner = new_master;
    generate_callback();
}

void CmCore::SetMaster(std::string const &name)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    if (!use_master_slave)
    {
        WORKER_DEBUG("libcm", 1, "cannot set master, because MasterSlave is off");
        return;
    }
    msv_master_name = name;
    msv_refresh_internal();
}

// Zprava je tvaru:
// SSR:SSR1:SSR1+SSR2 TIME:TIME1:TIME1+TIME2 CMS:LCMS2:LCMS1+LCMS2+RMM2
//
// Mame zamknuto
void CmCore::msv_receive_control_message(std::string const &msg)
{
    WORKER_DEBUG("msv", 4, "Control: received string `%s'\n", msg.c_str());
    std::vector<std::string> items = cm_split_by_delim(msg, ' ');

    // NOTE: vezme prvni rovnajici se msv_group_name
    CM_for (std::string &s, items)
    {
        std::vector<std::string> stations = cm_split_by_delim(s, ':');
        if (stations.size() < 2)
        {
            WORKER_DEBUG("msv", 1, "Control: received invalid control string: `%s'\n", msg.c_str());
            continue;
        }
        std::string &group_name = stations[0];
        std::string &master_name = stations[1];
        if (group_name == msv_group_name)
        {
            if (msv_master_name != master_name)
            {
                msv_master_name = master_name;
                msv_refresh_internal();
                break;
            }
        }
    }
}

void CmCore::EvalMasterSlave()
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    if (stop_threads)
        return;

    while (Maybe<CmIpEvent> e = msv_connection->get_new_event())
    {
        for (std::string const &msg : e->messages)
            msv_receive_control_message(msg);
    }
}

void CmCore::GetSomeChanges(std::string const &pattern, VarMap &c, VarMap &u, VarMap &d)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    m_var_changes.move_some_changes(pattern, c, u, d);
    if (m_ext_hook)
    {
	bool changed = m_ext_hook->VariableGoCB(&c, &u, &d);
        if (changed)
            PollCondExt->Wake();

    }
}

CmChanges CmCore::GetRawChanges(bool adjust_created_updated, CCmFiltersSet &PartnerFiltersCreated, CCmFiltersSet &PartnerFiltersDeleted)
{
    cms_ns_if_print("libcm_change", 5, "GetChanges locking ...");
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    cms_ns_if_print("libcm_change", 5, "GetChanges locked");
    CmChanges changes;
    GetVariableChanges(changes.vars_created, changes.vars_updated, changes.vars_deleted);
    if (adjust_created_updated)
    {
        // Udelame tohle:
        // created: pouze cizi
        // updated: pouze moje
        // deleted: pouze cizi
        std::set<std::string> moved;
        for (auto &p : changes.vars_updated)
        {
            if (!contains(myVariables, p.first))
            {
                moved.insert(p.first);
                assert(!contains(changes.vars_created, p.first));
                changes.vars_created.insert(p);
            }
        }
        for (std::string const &s : moved)
        {
            changes.vars_updated.erase(s);
        }

        for (auto &p : changes.vars_created)
            p.second.EventFields.clear();
        for (auto &p : changes.vars_updated)
            p.second.EventFields.clear();
        for (auto &p : changes.vars_deleted)
            p.second.EventFields.clear();
    }
    changes.filters_changed = GetFilterChanges(changes.filters_active, PartnerFiltersCreated, PartnerFiltersDeleted);
    changes.received_queries = db_state->get_received_queries();
    changes.received_responses = db_state->get_received_responses();
    changes.partners_accepted_services = db_state->get_accepted_services_change();
    changes.queries_to_forward = db_state->get_forwarded_queries();
    changes.responses_to_forward = db_state->get_forwarded_responses();
    changes.cancelled_queries = db_state->get_cancelled_queries();
    
    cms_ns_if_print("libcm_change", 5, "GetChanges end");
    return changes;
}

CmChanges CmCore::GetChanges()
{
    CCmFiltersSet created;
    CCmFiltersSet deleted;
    return GetRawChanges(true, created, deleted);
}

void CmCore::GetVariableChanges(VarMap &c, VarMap &u, VarMap &d)
{
    m_var_changes.move_all_changes(c, u, d);
    if (m_ext_hook)
    {
	bool changed = m_ext_hook->VariableGoCB(&c, &u, &d);
        if (changed)
            PollCondExt->Wake();
    }
}

bool CmCore::GetFilterChanges(CCmFiltersSet &PartnerFiltersActive, CCmFiltersSet &PartnerFiltersCreated, CCmFiltersSet &PartnerFiltersDeleted)
{
    if (!PartnerFiltersChanged)
	return false;
    PartnerFiltersActive.clear();
    PartnerFiltersCreated.clear();
    PartnerFiltersDeleted.clear();

    if (LiveDebugFlags & DEBUG_FLAG_DUMP_FILTERS_PARTNER_EACH)
    {
	for (std::list <CCmPartner>::iterator it_partner = Partners.begin(); it_partner != Partners.end(); it_partner++)
	{
	    if (it_partner->PartnerFiltersChanged)
	    {
		it_partner->PartnerFiltersChanged = false;
		LiveDebugPrintFilters(DEBUG_FLAG_DUMP_FILTERS_PARTNER_EACH, it_partner->ID, "R:",&it_partner->PartnerFilters);
	    }
	}
    }
    // Tohle sezbira filtry od vsech partneru, nic jineho to nemeni
    // Navic to zavola Ext Hook
    RegenaratePartnerFilters(PartnerFiltersActive);

    // Tohle diffne sezbirane filtry oproti tem poslednim, co sly do callbacku a nastavi created a deleted.
    PartnerFiltersDiffForCB(PartnerFiltersActive,&PartnerFiltersCreated,&PartnerFiltersDeleted);
    return true;
}

// Vola se pri zamcenem mutexu
void CmCore::generate_callback()
{
    if (has_started && (!m_var_changes.empty() || PartnerFiltersChanged || DbChanged))
    {
	cms_ns_if_print("libcm_change", 5, "generate_callback");
	if (!stop_threads)
	    PollCondCB->Wake();
    }
}

// CCmWorker je odemknuty
void CmCore::cb_thread_iter()
{
    cms_ns_if_print("libcm_change", 5, "cb_thread_iter begin");
    if (m_use_notify_cb)
    {
	if (m_notify_cb)
        {
            CCmNotification notif;
            {
                DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
                if (msv_changed)
                {
                    msv_changed = false;
                    notif.master_slave_changed = true;
                }
            }
	    m_notify_cb(notif);
        }
    }
    else
    {
        CCmFiltersSet created;
        CCmFiltersSet deleted;
	CmChanges changes = GetRawChanges(false, created, deleted);
	if (changes.filters_changed)
	    if (FilterCB_function)
		FilterCB_function(&changes.filters_active, &created, &deleted, UserPtr);
	VarMap &c = changes.vars_created;
	VarMap &u = changes.vars_updated;
	VarMap &d = changes.vars_deleted;
	cms_ns_if_print("libcm_change", 5, "got %zu,%zu,%zu variable changes", c.size(), u.size(), d.size());
	if (!c.empty() || !u.empty() || !d.empty())
	    if (VariableCB_function)
		VariableCB_function(&c, &u, &d, UserPtr);
    }
    cms_ns_if_print("libcm_change", 5, "cb_thread_iter end");
}

std::string CmCore::GetMasterName()
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    if (master_partner)
        return master_partner->partner_name;
    return "";
}

void CmCore::SetExtHook(CCmExtHook *hook)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    WORKER_DEBUG("libcm", 22, "ExtHook: SetExtHook");
    m_ext_hook = cm::move(hook);
    // Tohle jsem presunul ze cteni ext configu...
    PollCondExt->Wake();
    RefreshFilters(false);
    PollCondExt->Wake();
}

CCmExtHook *CmCore::GetExtHook()
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    return m_ext_hook.get();
}

std::set<std::string> CmCore::getCollidingVariables()
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    std::set<std::string> result;
    CM_for (CCmPartner &p, Partners)
    {
	CM_for (std::string const &s, p.variables_state.his_colliding_variables)
	    if (contains(myVariables, s))
		result.insert(s);
    }
    return result;
}

// Kdyz jmeno koliduje s partnerovou promennou: Pokud chceme callback,
// tak neni treba generovat zadnej delete CB te stare, proste to
// prijde do CB jako redefinice. Pokud nechceme callback, tak ani neni
// mozne delat delete, a vse je korektni. Musime ale vyhodit create a
// update zmeny z Callbacku v obou pripadech.
void CmCore::internalDefineMyVariable(CCmVariable const &var, bool _GenerateCB)
{
    AccumulatingTimer at1(debug_time_accum1);
    WORKER_DEBUG("libcm_change",4, "DefineMyVariable: %s", print_var(var).c_str());
    // Pokud promnena existovala a vlastnil ji partner, tak mu ji
    // oznacime za kolizni. Potrebujeme ji vymazat z callbacku (napr z
    // updated), tak to klidne udelame tak, aby to vypadalo, ze ji on
    // sam oddefinoval.  Ten delete stejne bude prepsan novou
    // definici.
    markPartnersCollidingVariables(var.getName());
        
    CCmVariable v;
    v.setName(var.getName());
    if (_GenerateCB)
        m_var_changes.add_foreign_changes(VarMap(), VarMap(), make_single_change(v));
    else
        m_var_changes.add_my_changes(VarMap(), VarMap(), make_single_change(v));
    at1.stop();

    AccumulatingTimer at2(debug_time_accum2);
    CCmVariable var_copy = var;
    internalChangeMyVariable(&var_copy, NULL, _GenerateCB, CMLIB_VALUE_PRIORITY_UNDEF);
}

void CmCore::DbAcceptService(DbService service_name, DbServiceParams params)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    db_state->accept_service(service_name, params);
    PollCondSend->Wake();
    DbChanged = true;
    generate_callback();
}

void CmCore::DbSetAcceptedServices(std::map<DbService, DbServiceParams> const &services)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    db_state->set_accepted_services(services);
    PollCondSend->Wake();
    DbChanged = true;
    generate_callback();
}

Maybe<QueryId> CmCore::DbSendQueryMaybe(DbService service_name, CmDbQuery const &query)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    // if (query_compression_enabled)
    //     query.use_compression = true;

    WORKER_DEBUG("libcm", 3, "DbSendQuery %s %s", service_name.c_str(), to_short_string(query, 50).c_str());
    Maybe<QueryId> id = db_state->send_query(service_name, query, get_current_time_usec());
    if (id)
    {
        Maybe<int64_t> wake_time = db_state->get_next_wakeup_time();
        if (wake_time)
            PollCondDb->ChangeRqTimeUsec(*wake_time);
        PollCondSend->Wake();
        DbChanged = true; // Pouze pri chybe, tohle chce vylepsit...
        generate_callback(); // A tohle taky
    }
    return id;
}

QueryId CmCore::DbSendQuery(DbService service_name, CmDbQuery const &query)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    // if (query_compression_enabled)
    //     query.use_compression = true;
    
    // WORKER_DEBUG("libcm", 3, "DbSendQuery %s %s", service_name.c_str(), to_short_string(query, 50).c_str());
    QueryId id = db_state->send_query(service_name, query, get_current_time_usec());
    Maybe<int64_t> wake_time = db_state->get_next_wakeup_time();
    // WORKER_DEBUG("libcm", 1, "DbSendQuery wake_time=%" PRId64, wake_time ? *wake_time : -1);
    if (wake_time)
        PollCondDb->ChangeRqTimeUsec(*wake_time);
    PollCondSend->Wake();
    DbChanged = true; // Pouze pri chybe, tohle chce vylepsit...
    generate_callback(); // A tohle taky
    return id;
}

void CmCore::DbSendResponse(CmDbResponse response)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    db_state->send_response(response);
    PollCondSend->Wake();
    DbChanged = true;
    generate_callback();
}

void CmCore::DbForwardResponse(DbMessage msg)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    db_state->add_forwarded_response(msg);
    PollCondSend->Wake();
    DbChanged = true;
    generate_callback();
}

Maybe<QueryId> CmCore::DbBeginForwardedQuery(DbService service)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    Maybe<QueryId> id = db_state->begin_forwarded_query(service);
    PollCondSend->Wake();
    DbChanged = true;
    generate_callback();
    return id;
}

void CmCore::DbAddToForwardedQuery(DbMessage msg)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    db_state->add_to_forwarded_query(msg);
    PollCondSend->Wake();
    DbChanged = true;
    generate_callback();
}

void CmCore::DbCancelQuery(QueryId query_id, WeakString reason, bool gen_callback)
{
    DebugScopedLock lock(*CM_Mutex, MyLibCM_name);
    db_state->cancel_query(query_id, reason, gen_callback);
    PollCondSend->Wake();
    DbChanged = true;
    generate_callback();
}

std::string CmCore::print_partner(CCmPartner const &p)
{
    if (use_master_slave)
        return format("(%s master=%d name=%s)", p.address_string.c_str(), master_partner == &p, p.partner_name.c_str());
    else
        return format("(%s)", p.address_string.c_str());
        
}

void CmCore::ApplyVariableChanges(VariableChangesConstRef changes)
{
    for (auto const &p : changes.deleted)
    {
        UndefineVariable(p.first, false);
    }
    DefineMyVariables(&changes.created, false);
    for (auto const &p : changes.updated)
    {
        ChangeVariable(&p.second, CMLIB_VALUE_PRIORITY_UNDEF, false);
    }
}

void CmCore::SaveFileSetPolicy(bool save_all)
{
    CMSScopedLock lock(*CM_Mutex);
    savefile_policy_save_all = save_all;
    PollCondSave->Wake();
}

void CmCore::SaveFileAddVariables(std::set<std::string> const &vars)
{
    CMSScopedLock lock(*CM_Mutex);
    savefile_enabled_variables.insert(vars.begin(), vars.end());
    PollCondSave->Wake();
}

CmStatistics CmCore::getStatistics()
{
    CMSScopedLock lock(*CM_Mutex);
    CmStatistics st;
    for (CCmPartner &p : Partners)
    {
        st.variables_to_send += p.variables_state.myVariablesToSend.size();
        st.variables_to_send += p.variables_state.his_variables_to_send.size();
        if (p.variables_state.fragments_to_send)
            st.variables_to_send += 1;
    }
    return st;
}

