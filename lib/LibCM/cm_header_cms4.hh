#ifndef CM_HEADER_CMS4_HH_
#define CM_HEADER_CMS4_HH_

#include "CMSConfig.hh"
#include "CMSDebug.hh"
#include "CMSClient.hh"
#include "CMSServer2.hh"
#include "CMSIP.hh"
#include "CMSMisc.hh"

#include <glib.h>

#define CMS_FLAG_BOTH CMS::FLAG_BOTH
#define CMS_FLAG_RECV CMS::FLAG_RECV
#define CMS_FLAG_SEND CMS::FLAG_SEND
#define CMS_TCP_MODE_TEXT CMS::TCP_MODE_TEXT
#define CMS_TCP_MODE_ELDIS CMS::TCP_MODE_ELDIS

typedef CMS::TCP_MODE CMS_TCP_MODE;
typedef CMS::Server2 CMSServer2;
typedef CMS::Client cms_client_t;
typedef CMS::Client CMSClient;
typedef CMS::IP CMSIP;
typedef CMS::Config CMSConfig;

using CMS::time2ymdhms;

inline std::string CMSConfigGetLine2(CMSConfig &cfg)
{
    std::string value;
    cfg.GetLine2(value);
    return value;
}

inline bool CMSConfigFindNextItem2(CMSConfig &cfg, std::string &name)
{ return cfg.GetItem2(name, false); }

inline bool CMSConfigFindNextSection2(CMSConfig &cfg, std::string &name)
{ return cfg.GetSection2(name, false); }

inline void CMSClientInit(CMSClient &c) {}

inline int CMSServer2Recv(CMSServer2 &server, void *buffer, unsigned short maxsize, unsigned *connection_id)
{return server.Recv(buffer, maxsize, *connection_id);}

inline unsigned cms_client_conenction_id(cms_client_t *client)
{return client->GetConnectionId();}

template <typename T> void CMSShutdown(T &obj) { obj.Shutdown(); }

inline std::string CMS_client_get_local_addr(cms_client_t *client) { return client->GetLocalAddr(); }
inline unsigned short CMS_client_get_local_port(cms_client_t *client) { return client->GetLocalPort(); }
inline std::string CMS_client_get_remote_addr(cms_client_t *client) { return client->GetRemoteAddr(); }
inline unsigned short CMS_client_get_remote_port(cms_client_t *client) { return client->GetRemotePort(); }

inline std::string CMSClient_GetLocalAddr(CMSClient& client) { return client.GetLocalAddr(); }
inline unsigned short CMSClient_GetLocalPort(CMSClient& client) { return client.GetLocalPort(); }
inline std::string CMSClient_GetRemoteAddr(CMSClient& client) { return client.GetRemoteAddr(); }
inline unsigned short CMSClient_GetRemotePort(CMSClient& client) { return client.GetRemotePort(); }

inline unsigned cms_debug_max_get_level(char const *ns)
{
    return CMS::Debug::GetMaxLevel(ns);
}

// Tohle je zkopirovane primo z LibCMS3

class CMSMutex
{
  friend class CMSCondition;

	GMutex * mutex;

  public:
	CMSMutex () { mutex = g_mutex_new (); }
	~CMSMutex () { g_mutex_free (mutex); }
	void Lock () { g_mutex_lock (mutex); }
	bool TryLock () { return g_mutex_trylock (mutex); }
	void Unlock () { return g_mutex_unlock (mutex); }
};


class CMSCondition
{
	GCond * cond;

  public:
	CMSCondition () { cond = g_cond_new (); }
	~CMSCondition () { g_cond_free (cond); }
	void Signal () { g_cond_signal (cond); }
	void Broadcast () { g_cond_broadcast (cond); }
	void Wait (CMSMutex * mutex) { g_cond_wait (cond, mutex->mutex); }
	int WaitTime (CMSMutex * mutex, long usec)
	{
	    GTimeVal time;
	    g_get_current_time(&time);
	    long long fulltime = (long long)time.tv_usec + ((long long)time.tv_sec * 1000000LL) + (long long)usec;
	    time.tv_sec = (long)(fulltime / 1000000LL);
	    time.tv_usec = (long)(fulltime % 1000000LL);
	    return g_cond_timed_wait (cond, mutex->mutex, &time);
	}
};

#define CMSServer2CB(funct, user_arg) [this](CMS::Client *c) { return funct(c, user_arg); }

#define cms_debug_print_text(ignore1, ignore2, filename, function, line, ns, level, fmt, ...) \
    CMS::Debug::Print(filename, function, line, false, ns, level, "", fmt, ##__VA_ARGS__)

inline void cms_debug_init(std::string const &init)
{
    CMS::Debug::Init(init);
}

inline void cms_debug_server_init(unsigned port)
{
    CMS::Debug::ServerInit(port);
}

inline void cms_debug_done()
{}

inline void cms_debug_set_project(const std::string & project)
{ CMS::Debug::SetProject(project); }

inline void cms_debug_set_program(const std::string & program)
{ CMS::Debug::SetProgram(program); }

using CMS::text2time;

inline bool CMSConfigFindValueDouble2(CMSConfig &cfg, double &val, std::string item_name)      { return cfg.GetValue2(val, item_name); }
inline bool CMSConfigFindValueInt2   (CMSConfig &cfg, long &val, std::string item_name)        { return cfg.GetValue2(val, item_name); }
inline bool CMSConfigFindValueString2(CMSConfig &cfg, std::string &val, std::string item_name) { return cfg.GetValue2(val, item_name); }
inline bool CMSConfigFindValueBool2  (CMSConfig &cfg, bool &val, std::string item_name)        { return cfg.GetValue2(val, item_name); }



#endif // CM_HEADER_CMS4_HH_
