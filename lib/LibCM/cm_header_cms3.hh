#ifndef CM_HEADER_CMS3_HH_
#define CM_HEADER_CMS3_HH_

#include "CMSConfig.hh"
#include "CMSDebug.hh"
#include "CMSClient.hh"
#include "CMSServer2.hh"
#include "CMSIP.hh"
#include "CMSMisc.hh"
#include "CMSSynch.hh"

#define CMSServer2CB(funct, user_arg) &funct, user_arg

inline void CMSClientInit(CMSClient &c) { c.Init(); }

inline std::string CMSConfigGetLine2(CMSConfig &cfg)
{ return cfg.GetLine2();}

inline bool CMSConfigFindNextItem2(CMSConfig &cfg, std::string &name)
{ return cfg.FindNextItem2(name); }

inline bool CMSConfigFindNextSection2(CMSConfig &cfg, std::string &name)
{ return cfg.FindNextSection2(name);}

inline std::string CMS_client_get_local_addr(cms_client_t *client) { return cms_client_get_local_addr(client); }
inline unsigned short CMS_client_get_local_port(cms_client_t *client) { return cms_client_get_local_port(client); }
inline std::string CMS_client_get_remote_addr(cms_client_t *client) { return cms_client_get_remote_addr(client); }
inline unsigned short CMS_client_get_remote_port(cms_client_t *client) { return cms_client_get_remote_port(client); }

inline std::string CMSClient_GetLocalAddr(CMSClient& client) { return client.getLocalAddr(); }
inline unsigned short CMSClient_GetLocalPort(CMSClient& client) { return client.getLocalPort(); }
inline std::string CMSClient_GetRemoteAddr(CMSClient& client) { return client.getRemoteAddr(); }
inline unsigned short CMSClient_GetRemotePort(CMSClient& client) { return client.getRemotePort(); }

template <typename T> void CMSShutdown(T &obj) { obj.ShutDown(); }

inline unsigned cms_client_conenction_id(cms_client_t *client)
{return client->connection_id;}

inline int CMSServer2Recv(CMSServer2 &server, void *buffer, unsigned short maxsize, unsigned *connection_id)
{return server.Recv(buffer, maxsize, connection_id);}

inline bool CMSConfigFindValueDouble2(CMSConfig &cfg, double &val, std::string item_name) { return cfg.FindValueDouble2(val, item_name); }
inline bool CMSConfigFindValueInt2   (CMSConfig &cfg, long &val, std::string item_name)   { int x; bool ok = cfg.FindValueInt2(x, item_name); if (ok) val = x; return ok; }
inline bool CMSConfigFindValueString2(CMSConfig &cfg, std::string &val, std::string item_name) { return cfg.FindValueString2(val, item_name); }
inline bool CMSConfigFindValueBool2  (CMSConfig &cfg, bool &val, std::string item_name)   { return cfg.FindValueBool2  (val, item_name); }

#endif // CM_HEADER_CMS3_HH_
