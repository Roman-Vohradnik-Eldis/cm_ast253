#ifndef CM_IP_UDPSERVER_H_
#define CM_IP_UDPSERVER_H_

#include "cm_header_internal.hh"
#include "cm_ip.hh"
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include "cm_ip_delay_simulator.hh"
#include "cm_thread.hh"
#include "cm_ip_eventstate.hh"
#include "cm_ip_udp_partner.hh"
#include "cm_ip_udp_message.hh"
#include "cm_ip_udp_poll.hh"
#include "weak_string.hh"

class PartnerList;

struct ConnectionRequest
{
    std::string hash;
    int64_t sender_msg_id;
};

struct DelayedMessage
{
    int64_t time_sent;
    std::string message;
};

class CmIP_UDP : public CmIPBase
{
    std::string debug_name;
    // Set on init, used for debugging.
    uint32_t local_addr;
    uint16_t local_port;

    // Tohle je promenna severu. Inkrementuje se, kdyz se server
    // restartuje, jeji hodnota je ulozena v /tmp
    Maybe<BootId> server_boot_id;
    
    cm::function<void()> notify_cb;
    int sockfd;
    bool stopped;
    cm::scoped_ptr<PartnerList> partners;
    EventsState events;
    Maybe<sockaddr_in> connect_addr;
    CMSMutex mutex;
    uint16_t connect_session_id;
    bool have_new_event;
    CmIPParams udp_params;

    Poll poll_obj;

    // For debugging
    int64_t total_recv_messages = 0;
    
    // Pro klienta
    Maybe<ConnectionRequest> my_conn_request;

    bool use_delay_simulator = false;
    DelaySimulator delay_simulator;

    cm::scoped_ptr<Thread> recv_thread;

    // connect_addr: Kam posilame zadost o spojeni (v pripade klienta)
public:
    CmIP_UDP(cm::function<void()> cb,
             CmIpAddress listen_addr,
             Maybe<CmIpAddress> connect_addr,
             CmIPParams params,
             std::string debug_name);
    ~CmIP_UDP();
    
    void send(char const *buffer, int size, unsigned conn_id);
    
    void disconnect(unsigned conn_id);

    void shutdown();

    void set_params(unsigned conn_id, CmIPParams params);

    bool is_ready_to_send(unsigned conn_id);

    Maybe<CmIpEvent> get_new_event()
    {
	return events.get_new_event();
    }

private:
    void recv_main();
    void debug_print_address();
    bool send_message(Partner &p, Maybe<int64_t> msg_id, WeakString data);
    bool send_partners_messages(Partner &p);
    bool do_receive();
    void handle_received_message(uint32_t address, uint16_t port, ReceivedMessage, WeakString);
    void handle_valid_message(Partner &p, ReceivedMessage, WeakString buffer);
    void check_partners_timeout();
    bool send_message_internal(sockaddr_in dest, ReceivedMessage);
    std::string make_debug_addr_string(unsigned id, uint32_t address, uint16_t port);
    ssize_t sendto(int sock, char const *data, size_t len, sockaddr_in dest);

    bool send_regular_message(Partner &p, int64_t msg_id, WeakString msg);
    bool send_ping_message(Partner &p);
    
    bool handle_data_message(Partner &p, int64_t sender_msg_id, WeakString data, int64_t time_now);
    void handle_message_acknowledge(Partner &p, int64_t receiver_msg_id, uint8_t missing_count);
};

#endif // CM_IP_UDPSERVER_H_
