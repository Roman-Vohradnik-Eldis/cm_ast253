#ifndef CM_IP_EVENTSTATE_HH_
#define CM_IP_EVENTSTATE_HH_

#include "cm_header_internal.hh"
#include "cm_debug.hh"
#include "cm_ip.hh"
#include "cm_thread.hh"
#include <map>

struct atomic_bool
{
    CMSMutex mutex;
    bool val;

public:
    atomic_bool(bool init = false) : val(init) {}
    void operator=(bool b) { CMSScopedLock lock(mutex); val = b; }
    operator bool() { CMSScopedLock lock(mutex); return val; }
};

// Postup:
//
// Nejdrive se zavola connect(id)
// Pak se mohou pridavat zpravy pomoci add_message(id)
// Pak se zavola disconnect(id)
//
// V prubehu se muze pouzivat is_connected() a mohou se posilat
// zpravy.
//
// Pod odpojeni se toho id nesmi pouzivat.
//
// K cemu to je:
//
// Recv thread sem dava zpravy, uzivatel si je zde vyzvedava.
//
class EventsState
{
    std::string debug_name;
    typedef std::map<unsigned, CmIpEvent> EventMap;

    // Ty pripojeni, ktere zname, tady jsou.
    std::map<unsigned, CmIpEvent> events;
    CMSMutex mutex;

public:
    EventsState(std::string debug_name)
        : debug_name(debug_name)
    {}
    
    // Kdyz CmIp posila zpravu, tak se timhle pta, jestli to ma cenu.
    bool is_connected(unsigned id)
    {
        CMSScopedLock lock(mutex);
        return contains(events, id) && !events.at(id).just_disconnected;
    }

    void add_message(unsigned connection_id, char const *buffer, int length)
    {
	CMSScopedLock lock(mutex);
        // cms_ns_if_print("ipevent", 5, "add_message %u %s", connection_id, std::string(buffer, buffer + std::min(length, 30)).c_str());
        if (!contains(events, connection_id))
            cms_ns2_if_print("cmip", debug_name, 1, "add_message: Connection Id %u does not exist", connection_id);
        else
        {
            CmIpEvent &e = events.at(connection_id);
            // cms_ns_if_print("cmip", 5, "Add message: %s", std::string(buffer, buffer + length).c_str());
            e.messages.push_back(std::string(buffer, buffer + length));
        }
    }

    void connect(unsigned id, std::string address_string)
    {
	CMSScopedLock lock(mutex);
        cms_ns2_if_print("ipevent", debug_name, 5, "connect %u %s", id, address_string.c_str());

        CmIpEvent e;
        e.connection_id = id;
        e.just_connected = true;
        e.just_disconnected = false;
        std::swap(e.address_string, address_string);

        events.insert(std::make_pair(id, e));
    }

    void disconnect(unsigned id)
    {
	CMSScopedLock lock(mutex);
        cms_ns2_if_print("ipevent", debug_name, 5, "disconnect %u", id);
        if (!contains(events, id))
        {
            cms_ns2_if_print("ipevent", debug_name, 1, "disconnect: Connection Id %u does not exit", id);
            return;
        }
	CmIpEvent &e = events.at(id);
        if (e.just_connected)
        {
            // Partner se pripojil a hned zase odpojil, tak uzivateli
            // o tom ani nebudeme rikat.
            events.erase(id);
        }
        else
        {
            e.just_disconnected = true;
        }
    }

    // Tohle vola uzivatel, kdyz chce spojeni prerusit.  Znamena to,
    // ze CmIpEvent od neho uz neprijde.
    void erase(unsigned id)
    {
        CMSScopedLock lock(mutex);
        cms_ns2_if_print("ipevent", debug_name, 5, "erase %u", id);
        events.erase(id);
    }

    // Tohle vola uzivatel. Muze byt potreba, aby to zavolal vicekrat.
    Maybe<CmIpEvent> get_new_event()
    {
	CMSScopedLock lock(mutex);
	for (EventMap::iterator it = events.begin(); it != events.end(); ++it)
	{
            unsigned id = it->first;
            CmIpEvent &e = it->second;
            if (e.just_connected || e.just_disconnected || !e.messages.empty())
            {
                // Prekopirujeme zmeny
                CmIpEvent u;
                u.connection_id = id;
                u.just_connected = e.just_connected;
                u.just_disconnected = e.just_disconnected;
                if (e.just_connected)
                    u.address_string = e.address_string;
                std::swap(u.messages, e.messages);

                // Ozstranime je z pameti
                if (u.just_disconnected)
                    events.erase(it);
                else
                {
                    e.just_disconnected = false;
                    e.just_connected = false;
                }
                return u;
            }
        }
	return Nothing();
    }
};

#endif
