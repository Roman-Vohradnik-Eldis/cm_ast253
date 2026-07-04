#ifndef CM_IP_HH_
#define CM_IP_HH_

#include <functional>
#include "weak_string.hh"
#include "cm_maybe.hh"
#include "cm_utils.hh"

std::string replace_nonprintable(WeakString str);

struct CmIpAddress
{
    std::string addr;
    uint16_t port;
    unsigned ping;
};

struct CmIpEvent
{
    unsigned connection_id;

    // Kdyz uzivateli davame poprve toto connection_id, tak mu
    // nastavime just_connected.  Kdyz se partner odpoji, tak dame
    // just_disconnected, a od tohoto partnera uz zadny CmIpEvent
    // neprijde. Nikdy nejsou tyto flagy nastavene najednou.
    //
    // Nevim presne, jestli se mohou connection_id recyklovat, tyto ID
    // jsou prijimany z LibCMS, ke ktere to nikde dokumentovane neni.
    bool just_connected;
    bool just_disconnected;

    // Tohle je nastavene jenom kdyz je nastavene just_connected.
    std::string address_string;

    // I kdyz je just_disconnected, ze mohou byt zpravy, ktere jsme
    // dostali jeste pred tim, nez se odpojil.
    std::vector<std::string> messages;
};

struct CmIPParams
{
    int64_t ping_period;
    int64_t timeout;
    int64_t resend_delay;

    // Tohle je jenom pro debugovani UDP.  Pokud je > 0, tak se obcas
    // nejaky datagram zahodi, prehodi nebo zduplikuje s
    // pravdepodonosti 1/debug_udp_probability. Pri nasazeni by
    // samozrejme melo byt 0.
    int64_t debug_udp_probability;
    int64_t debug_delay_datagrams;

    CmIPParams()
        : ping_period(300*1000)
        , timeout(5*1000*1000)
        , resend_delay(200*1000)
        , debug_udp_probability(0)
        , debug_delay_datagrams(0)
    {
    }
};

struct CmIPBase
{
    virtual ~CmIPBase() {};

    // Posle zpravu. Pokud se mezitim partner odpojil, tak se zprava
    // zahodi. Prijde CmIpEvent, ze byl just_disconnected.
    virtual void send(char const *buffer, int size, unsigned conn_id) = 0;

    // Odpoji zadaneho partnera. Uz od neho nikdy neprijde zadny CmIPEvent.
    virtual void disconnect(unsigned conn_id) = 0;

    virtual bool is_ready_to_send(unsigned conn_id) = 0;

    // Tohle je jen pro optimalizaci. Muze se stat, ze destruktor
    // CmIPBase chvili trva. V tom pripade Aplikace muze udelat takto:
    
    // (1) Aplikace zavola CmIpBase::shutdown(), to neblokuje.
    //
    // (2) Aplikace si pak chvili neco muze delat. Mezitim se CmIP v
    // jinem threadu ukoncuje.
    //
    // (3) Pak zavola destruktor CmIPBase, ten uz neblokuje.
    //
    // Samorejme implentace muze mit shutdown prazdny a vse udelat v
    // destruktoru.
    //
    // ! Pozor: I po shutdown muze prijit notifikacni callback !
    //
    virtual void shutdown() = 0;

    // Tohle se musi pouzivat tak, ze se musi vyzvednout vsechny
    // dostupne CmIpEventy. Az kdyz teprve get_new_event vrati Nothing,
    // tak muze prijit dalsi callback:
    //
    // while (Maybe<CmIpEvent> e = conn->get_new_event())
    // {
    //    zpacuj_event();
    // }
    // cekej_na_dalsi_callback();
    //
    virtual Maybe<CmIpEvent> get_new_event() = 0;

    virtual void set_params(unsigned conn_id, CmIPParams params)
    {
	// cms_ns_if_print("cmip", 2, "This communication mode does not support CmIpParams");
    }
};

Maybe<uint16_t> parse_uint16_t(std::string str);
Maybe<int64_t> parse_long_int(std::string str);

struct MasterSlaveParams
{
    std::string ip_descriptor;
    std::string group;
    std::string my_name;
};

struct CmIpDescriptorSub
{
    bool use_single_connection = false;
    std::vector<std::string> ip_descriptors;
};

struct CmIpDescriptor
{
    Maybe<MasterSlaveParams> only_when_master;
    CmIpDescriptorSub descriptor;
};

MasterSlaveParams parse_master_slave_params(std::string const &line);
std::vector<std::string> parse_ip_descriptor_list(std::string const &line);

// (1) Z callbacku jdou volat libovolne funkce z CmIPBase
//
// (2) Pokud objekt zanika a zrovna je aktivni callback, tak se se v
// destruktoru ceka, az callback vrati.
//
// Jako mel by to bejt unique_ptr<CmIPBase>
//
// On error, throw std::runtime_error
//
CmIPBase* create_connection(std::string descriptor,
                            cm::function<void()> callback,
                            std::string name,
                            CmIPParams default_params = CmIPParams());

CmIPBase* create_connection(CmIpDescriptorSub descriptor,
                            cm::function<void()> callback,
                            std::string name,
                            CmIPParams default_params = CmIPParams());

CmIPBase* create_connection(CmIpDescriptor descriptor,
                            cm::function<void()> callback,
                            std::string name,
                            CmIPParams default_params = CmIPParams());

#endif // CM_IP_HH
