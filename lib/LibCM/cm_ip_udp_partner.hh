#ifndef CM_IP_UDP_PARTNER_HH_
#define CM_IP_UDP_PARTNER_HH_

#include "cm_random.hh"
#include "cm_value_history.hh"
#include "weak_string.hh"
#include "cm_ip_udp_message.hh"
#include "cm_debug.hh"

#include "cm_ip.hh"
#include <cstdint>
#include <deque>
#include <map>

struct SentMessage
{
    int64_t msg_id;
    int64_t time_sent; // Menime pri znovuposlani.

    // Tohle nam typicky posle partner, pokud mu chybi tato zprava.
    // Co my s tim: Pokud se prehodilo poradi paketu, tak je nejlepsi
    // chcili pockat.  Pokud se packet ztratil, tak je nejlepsi ho co
    // nejrychleji znovu poslat.
    bool marked_as_missing;

    // Kolikrat jsme znovuposilali tuto zpravu.
    unsigned sent_times;

    // Tohle je telo zpravy, kere se posila.
    // K nemu se pri posilani prida session_id, jeho msg_id, apod.
    std::string msg;
};

struct Partner
{
    std::string rand_str;
    
    // ID pro externi rozhrani CmIP 
    unsigned cmip_id;

    // Kam mu posilame zpravy.
    // Pozor: port se pred odeslanim musi prevest pres htons()
    uint32_t address;
    uint16_t port;

    BootId server_boot_id;
    SessionId session_id;

    // Dostali jsem od nej vsechny zpravy s id < his_msg_id_received.
    int64_t his_msg_id_received;

    // Potvrdil vsechny moje zpravy < my_msg_id_received.
    int64_t my_msg_id_received;

    // Vsechny zpravy < my_msg_id_last_sent jsou odeslany nebo cekaji
    // na potvrzeni. Kdyz posilame novou zpravu, tak pouzijeme toto id
    // a inkrementujeme ho.
    int64_t my_msg_id_counter;

    // Pozn: Nikde si nepamatujeme, jake msg_id jsme mu naposled
    // potvrzovali. Ale pokud mu neco potvrdíme, tak si pamatujeme
    // aspon cas posledniho potvrzeni.
    int64_t last_sent_time;

    // Pouzito pro timeout. Pocitame do toho vsechny druhy zprav.
    int64_t last_recv_time;

    // int64_t last_ack_delay = 0;

    unsigned ack_window_size = 2;
    unsigned number_of_datagrams_without_loss = 0;

    // Kdy jsme dostali zpravu typu MESSAGE, zadna predchozi
    // nechybela, a kterou musime potvrdit.
    Maybe<int64_t> last_recv_data_time;

    // Config
    CmIPParams udp_params;
    
    // Odeslane zpravy, ke kterym zatim neprislo potvrzeni.
    std::deque<SentMessage> messages_sent;

    ValueHistory<int64_t> ack_delay_history;

    // Zpravy, ktere jsme nemohli poslat ani ocislovat, protoze se nevesly do `window_size`.
    std::deque<std::string> queue_to_send;

    // Prijate zpravy, ktere ale cekaji, protoze neprisla nejaka
    // predchozi zprava.
    std::map<int64_t, std::string> messages_received;

    Partner()
        : his_msg_id_received(0)
        , my_msg_id_received(0)
        , my_msg_id_counter(0)
        , last_sent_time(0)
        , last_recv_time(0)
    {}
};

size_t count_unacknowledged_messages(Partner const &p);

bool can_send_messages(Partner const &p);

std::string partner_info(Partner const &p);

#endif // CM_IP_UDP_PARTNER_HH_
