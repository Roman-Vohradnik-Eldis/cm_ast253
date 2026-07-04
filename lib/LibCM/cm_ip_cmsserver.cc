#include "cm_ip_cmsserver.hh"

gboolean CmIP_CMSServer2::onServerConnect(cms_client_t *client, void * data)
{
    ((CmIP_CMSServer2*)data)->on_connect(client);
    return true;
}

gboolean CmIP_CMSServer2::onServerDisconnect(cms_client_t *client, void * data)
{
    ((CmIP_CMSServer2*)data)->on_disconnect(client);
    return true;
}
