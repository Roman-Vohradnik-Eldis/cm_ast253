#include "cm_header_internal.hh"
#include "cm_ip_udp.hh"
#include "cm_ip_udp_message.hh"
#include "cm_ip_udp_partner.hh"
#include "cm_ip_udp_partner_list.hh"
#include "cm_string.hh"
#include "cm_base_functs.hh"
#include "cm_utils.hh"
#include "weak_string.hh"
#include <cstdint>
#include <fcntl.h>
#include <string.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>

// #define cmip_debug_print(...) (void)0
#define cmip_debug_print(...) cms_ns_if_print(__VA_ARGS__)

// int32_t lcg_next(int32_t state)
// {
//     return (((state * 1103515245) + 12345) & 0x7fffffff) >> 16;
// }

const int64_t ack_delay = 10000;


int64_t compute_resend_delay(Partner const &p)
{
    int64_t avg_ack_delay = p.ack_delay_history.get_median(1000*1000);
    // cms_ns_if_print("cmip", 3, "delay_avg=%ldusec history=%s",
    //                 avg_ack_delay,
    //                 cm_concat(p.ack_delay_history.begin(),
    //                           p.ack_delay_history.end(),
    //                     ", ").c_str());
    return std::max(2 * avg_ack_delay, p.udp_params.resend_delay);
}

int64_t next_event_time(Partner const &p)
{
    int64_t min_time = INT64_MAX;
    min_time = std::min(min_time, p.last_sent_time + p.udp_params.ping_period);
    min_time = std::min(min_time, p.last_recv_time + p.udp_params.timeout);

    // If we need to send acknownledge
    if (p.last_recv_data_time && p.last_sent_time < *p.last_recv_data_time) 
        min_time = std::min(min_time, *p.last_recv_data_time + ack_delay);

    int64_t resend_delay = compute_resend_delay(p);
    CM_for (SentMessage const &m, p.messages_sent)
    {
	// cmip_debug_print("cmip", 5, "Check Resend %s", m.msg.c_str());
	min_time = std::min(min_time, m.time_sent + resend_delay);
    }
    return min_time;
}

void set_address(sockaddr_in *sa, uint32_t addr, uint16_t port)
{
    std::memset(sa, 0, sizeof(sockaddr_in));
    sa->sin_family = AF_INET; // IPv4 
    sa->sin_addr.s_addr = addr; 
    sa->sin_port = htons(port);
}

std::string addr_string(uint32_t addr)
{
    char buf[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET, &addr, buf, INET_ADDRSTRLEN))
	return "Unknown adddress";
    return buf;
}

uint32_t address_to_int(std::string const &addr)
{
    in_addr a;
    if (inet_aton(addr.c_str(), &a) == 0)
	throw std::runtime_error("Invalid address: '" + addr + "'");
    return a.s_addr;
}

CmIP_UDP::~CmIP_UDP()
{
    CMSScopedLock lock(mutex);
    stopped = true;
    poll_obj.interrupt();
}

BootId get_or_create_boot_file_and_increment(unsigned port, std::string const &debug_name)
{
    std::string fname = format("/tmp/CM/cm_udp_bind_port_0x%04x", port);
    libCM_CreateDirectoryFromPath(fname.c_str());

    // Read previous value, if present, or choose random one
    BootId id;
    {
        
        std::ifstream f(fname.c_str());
        uint64_t x = 0;
        if (f >> x)
        {
            ++x;
            x %= BootId::max();
            id = BootId::parse(x);
        }
        else
        {
            id = BootId::random();
        }
    }

    // Write new value
    std::ofstream f(fname.c_str());
    if (!(f << id.value << std::endl))
    {
        cms_ns2_if_print("cmip", debug_name, 3, "Failed to create boot file %s, value=%u", fname.c_str(), id.value);
    }
    else
    {
        cms_ns2_if_print("cmip", debug_name, 3, "Writen boot file %s, value=%u", fname.c_str(), id.value);
    }
    return id;
}

CmIP_UDP::CmIP_UDP(cm::function<void()> cb,
                   CmIpAddress addr,
                   Maybe<CmIpAddress> client_addr,
                   CmIPParams params,
                   std::string debug_name)
    : debug_name(debug_name),
      notify_cb(cb),
      stopped(false),
      partners(new PartnerList()),
      events(debug_name),
      have_new_event(false),
      udp_params(params)
{
    // Set client address
    if (client_addr)
    {
	connect_addr = sockaddr_in();
	set_address(&*connect_addr, address_to_int(client_addr->addr), client_addr->port);
    }

    // Bind socket
    sockaddr_in servaddr;
    set_address(&servaddr, address_to_int(addr.addr), addr.port);
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); // UDP
    if (sockfd < 0)
	throw std::runtime_error("socket() failed");

    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0)
    {
	close(sockfd);
	throw std::runtime_error("fcntl() O_NONBLOCK failed");
    }

    if (fcntl(sockfd, F_SETFD, FD_CLOEXEC) < 0)
    {
	close(sockfd);
	throw std::runtime_error("fcntl() FD_CLOEXEC failed");
    }

    if (bind(sockfd, (sockaddr const *)&servaddr, sizeof(servaddr)) < 0)
    {
        int err = errno;
	close(sockfd);
	throw std::runtime_error(format("bind() failed: %s", strerror(err)));
    }

    if (params.debug_delay_datagrams > 0 || params.debug_udp_probability > 0)
    {
        use_delay_simulator = true;
        delay_simulator.set_delay(params.debug_delay_datagrams);
        delay_simulator.set_debug_probability(params.debug_udp_probability);
    }
    debug_print_address();

    // Pokud jsme server a povedl se bind(), takze jsme jediny
    // program, kdo na tomhle portu posloucha.
    if (!client_addr)
    {
        server_boot_id = get_or_create_boot_file_and_increment(local_port, debug_name);
    }

    recv_thread = new Thread(cm::bind_front(&CmIP_UDP::recv_main, this), "udpsrecv");
}

void CmIP_UDP::debug_print_address()
{
    // Tohle je jenom pro debugovani
    sockaddr_in addr;
    socklen_t addrlen = sizeof addr;
    bzero(&addr, sizeof addr);
    if (getsockname(sockfd, (sockaddr *)&addr, &addrlen) == 0)
    {
	local_addr = addr.sin_addr.s_addr;
	local_port = ntohs(addr.sin_port);
	cms_ns2_if_print("cmip",debug_name,3,"UDP listening at %s %u", addr_string(local_addr).c_str(), local_port);
    }
    else
    {
	cms_ns2_if_print("cmip",debug_name,3,"UDP listening, but getsockname() failed");
    }
}

SentMessage &prepare_message_to_send(Partner &p, WeakString body)
{
    SentMessage msg;
    msg.msg_id = p.my_msg_id_counter;
    msg.time_sent = 0; // Tohle neni pouzite, pokud jsme nic neposlali
    msg.msg = body; // Make a copy
    msg.marked_as_missing = false;
    msg.sent_times = 0;
    p.messages_sent.push_back(cm::move(msg));
    ++p.my_msg_id_counter;
    return p.messages_sent.back();
}

// Socket ma nastaveno non-blocking, protoze chceme neblokuji write
// (ktere se provadi primo z uzivatelova threadu). Zaroven ale chceme
// cekat na recv_from, abychom neblokovali.
void wait_for_event(int sockfd, bool need_write, int64_t timeout_usec)
{
}

void CmIP_UDP::recv_main()
{
    CMSScopedLock lock(mutex);
    connect_session_id = rand();
    int64_t init_last_sent_time = 0;
    int64_t init_period = 300*1000;

    while (!stopped)
    {
	bool wait_for_write = false;
	int64_t time_now = get_current_time_usec();
	
	cmip_debug_print("cmip", 5, "recv_main iteration: #partners=%zu", partners->size());
	// Pokud jsme v rezimu klient, a nejsme pripojeni, tak
	// posilame ping.
	if (partners->empty() && connect_addr && init_last_sent_time + init_period < time_now)
	{
	    cmip_debug_print("cmip", 5, "recv_main: sending init message");
            if (!my_conn_request)
            {
                my_conn_request = ConnectionRequest{
                    gen_random_hash_str(),
                    (int64_t)(cm_get_random_value<uint64_t>() % msg_id_end),
                };
            }
	    ReceivedMessage rm('i');
            rm.data = my_conn_request->hash;
            rm.sender_msg_id = my_conn_request->sender_msg_id;
	    if (send_message_internal(*connect_addr, rm))
		init_last_sent_time = time_now;
	    else
		wait_for_write = true;
	}

        int64_t recv_until_now = total_recv_messages;
	// Precti vsechny prijate zpravy 
	while (do_receive())
	{
	}
        cms_ns2_if_print("cmip",debug_name, 3, "recv_main: receive done: total_recv=%lld new=%lld",
                        (long long)total_recv_messages,
                        (long long)total_recv_messages - recv_until_now);

	cmip_debug_print("cmip", 5, "recv_main recv done: #partners=%zu", partners->size());

	// Vyhod partnery, kterym vyprsel timeout
	check_partners_timeout();

	// Odesli zpravy, pokud to jde a je to nutne
	CM_for (Partner &p, *partners)
	{
	    cmip_debug_print("cmip", 5, "recv_main send_partners_messages");
            // Pridej zpravy, ktere jsou ve fronte, protoze nesly odeslat
            while (!p.queue_to_send.empty() && can_send_messages(p))
            {
                std::string msg;
                std::swap(p.queue_to_send.front(), msg);
                p.queue_to_send.pop_front();
                prepare_message_to_send(p, msg);
            }

            // Zkus odeslat nove zpravy, nebo zopakuj zpravu, nebo posli ping
	    if (!send_partners_messages(p))
		wait_for_write = true;
	}

        if (use_delay_simulator)
            delay_simulator.process_events();

	// Spocti sleep time
	time_now = get_current_time_usec(); // Asi neni potreba...
	int64_t next_time = time_now + 100*1000*1000; // infinity
        cmip_debug_print("cmip", 5, "recv_main sleep_time_usec1 = %" PRId64"", next_time - time_now);
	CM_for (Partner &p, *partners)
	    next_time = std::min(next_time, next_event_time(p));
        cmip_debug_print("cmip", 5, "recv_main sleep_time_usec2 = %" PRId64"", next_time - time_now);
	if (partners->empty() && connect_addr)
	    next_time = std::min(next_time, init_last_sent_time + init_period);
        cmip_debug_print("cmip", 5, "recv_main sleep_time_usec3 = %" PRId64"", next_time - time_now);

        if (use_delay_simulator)
            next_time = std::min(next_time, delay_simulator.get_next_event_time(time_now));
        
	int64_t sleep_time_usec = next_time - time_now;
        cmip_debug_print("cmip", 5, "recv_main sleep_time_usec = %" PRId64"", sleep_time_usec);

	bool notify = have_new_event;
	have_new_event = false;

	// Pozor: Cekame odemceny
	CMSScopedUnlock unlock(mutex);
	if (notify)
	{
	    // cms_ns2_if_print("cmip",debug_name, 3, "recv_main calling notify");
	    notify_cb();
	}
	if (sleep_time_usec > 0)
	{
	    poll_obj.wait_for_event(sockfd, wait_for_write, sleep_time_usec);
	}
    }
    cmip_debug_print("cmip", 4, "recv_main terminating, closing socket");
    if (close(sockfd) < 0)
    {
        cms_ns2_if_print("cmip",debug_name, 1, "shutdown() failed, errno=%d: %s", errno, strerror(errno));
    }
}

struct IsConfirmed
{
    uint16_t val;
    IsConfirmed(uint16_t v) : val(v) {}
    bool operator()(SentMessage const &m)
    {
        return m.msg_id < val;
    }
};

// messages_sent jsou serazeny podle msg_id.
void erase_acknowledged_messages(Partner &p, int missing_count, std::string const &debug_name)
{
    Maybe<int64_t> min_send_time = 0;
    for (SentMessage const &m : p.messages_sent)
    {
        if (m.msg_id < p.my_msg_id_received)
        {
            if (m.sent_times == 1)
                if (!min_send_time || *min_send_time < m.time_sent)
                    min_send_time = m.time_sent;
        }
    }
    if (min_send_time)
    {
        int64_t time_now = get_current_time_usec();
        int64_t delay = time_now - *min_send_time;
        p.ack_delay_history.push({time_now, delay});
    }
    
    auto it = std::find_if(p.messages_sent.begin(), p.messages_sent.end(),
                           [=](SentMessage const &m){return m.msg_id >= p.my_msg_id_received;});
    cms_ns2_if_print("cmip",debug_name, 3, "Pop acknowledged:"
                    " total=%lld"
                    " sent=%zu"
                    " msg_id_received=%lld"
                    " acknowledged=%zu"
                    " missing=%d"
                    " window_size=%u"
                    " sent_without_loss=%u"
                    " queue=%zu",
                    (long long)p.my_msg_id_counter,
                    p.messages_sent.size(),
                    (long long)p.my_msg_id_received,
                    it - p.messages_sent.begin(),
                    missing_count,
                    p.ack_window_size,
                    p.number_of_datagrams_without_loss,
                    p.queue_to_send.size());
    p.messages_sent.erase(p.messages_sent.begin(), it);
}

void CmIP_UDP::handle_message_acknowledge(Partner &p, int64_t receiver_msg_id_rel, uint8_t missing_count)
{
    int64_t receiver_msg_id_abs = convert_msg_id_to_abs(receiver_msg_id_rel, p.my_msg_id_counter, "recver_msg_id");

    if (receiver_msg_id_abs > p.my_msg_id_counter)
    {
        cms_ns2_if_print("cmip",debug_name, 1, "Message with invalid receiver_msg_id");
        return;
    }
    
    // Update window size
    // if (missing_count > 0) // || p.last_ack_delay > p.udp_params.resend_delay / 2)
    // {
    //     p.number_of_datagrams_without_loss = 0;
    // }
    // else
    // {
    int64_t new_ack_count = receiver_msg_id_abs - p.my_msg_id_received;
    p.number_of_datagrams_without_loss += new_ack_count;
    // }

    // Koukneme, jestli nam potvrdil neco noveho
    if (receiver_msg_id_abs > p.my_msg_id_received)
    {
        p.my_msg_id_received = receiver_msg_id_abs;
        erase_acknowledged_messages(p, missing_count, debug_name);
    }

    if (missing_count > 0)
    {
        cms_ns2_if_print("cmip",debug_name, 3, "missing_count=%d", missing_count);
        int64_t missing_begin = p.my_msg_id_received;
        int64_t missing_end = missing_begin + missing_count;
        for (SentMessage &msg : p.messages_sent)
        {
            if (missing_begin <= msg.msg_id && msg.msg_id < missing_end)
            {
                cms_ns2_if_print("cmip",debug_name, 3, "marked as missing: %s", msg.msg.c_str());
                msg.marked_as_missing = true;
            }
        }
    }
}

// vrati true, pokud mame novou zpravu pro uzivatele
//
bool CmIP_UDP::handle_data_message(Partner &p, int64_t sender_msg_id, WeakString data, int64_t time_now)
{
    int64_t sender_msg_id_abs = convert_msg_id_to_abs(sender_msg_id, p.his_msg_id_received, "sender_msg_id");

    if (std::abs(p.his_msg_id_received - sender_msg_id_abs) >= (3 * msg_id_end) / 4)
    {
        // To je hodne divne
        cms_ns2_if_print("cmip",debug_name, 1, "Message with invalid sender_msg_id");
        return false;
    }
    
    if (sender_msg_id_abs < p.his_msg_id_received)
    {
        cmip_debug_print("cmip", 2, "handle_message: Duplicate message, discarding.");
        return false;
    }

    if (sender_msg_id_abs > p.his_msg_id_received)
    {
        // Nejaka predchozi zprava nam chybi, tak si ji zatim jenom zapamatujeme
        cmip_debug_print("cmip", 2, "handle_message: Some previous message is missing");
        // if (p.messages_received.size() >= 10)
        // {
        // 	cmip_debug_print("cmip", 5, "handle_message: Discarding: %s", buffer.str().c_str());
        // }
        // else
        // {
        p.messages_received.insert(std::make_pair(sender_msg_id_abs, data));
        return false;
        // }
    }

    cmip_debug_print("cmip", 5, "handle_message: OK: %s", data.str().c_str());
    assert(sender_msg_id_abs == p.his_msg_id_received);

    p.last_recv_data_time = time_now;

    // Pridame tuto zpravu
    events.add_message(p.cmip_id, data.begin(), data.size());
    ++p.his_msg_id_received;

    // Pridame nasledne zpravy ve fronte
    while (!p.messages_received.empty())
    {
        std::map<int64_t, std::string>::iterator front_it = p.messages_received.begin();
        if (front_it->first != p.his_msg_id_received)
            break;
        ++p.his_msg_id_received;
        cmip_debug_print("cmip", 5, "handle_message: Add queued message: %s", front_it->second.c_str());
        events.add_message(p.cmip_id, front_it->second.c_str(), front_it->second.size());
        p.messages_received.erase(front_it);
    }
	    
    // Notifikujeme uzivatele
    return true;
}

// Bud PING nebo MESSAGE
void CmIP_UDP::handle_valid_message(Partner &p, ReceivedMessage rm, WeakString buffer)
{
    //cmip_debug_print("cmip", 5, "handle_valid_message: rm.sender_msg_id set=%d", rm.sender_msg_id ? 1 : 0);
    int64_t time_now = get_current_time_usec();

    p.last_recv_time = time_now;

    // Koukneme se, jestli nam potvrdil nejake nase zpravy
    if (rm.receiver_msg_id && rm.missing_count)
    {
        handle_message_acknowledge(p, *rm.receiver_msg_id, *rm.missing_count);
    }

    // Koukneme, jestli nam neposila novou zpravu
    if (rm.sender_msg_id)
    {
        bool ok = handle_data_message(p, *rm.sender_msg_id, rm.data, time_now);
        if (ok)
        {
            have_new_event = true;
        }
    }
}

std::string CmIP_UDP::make_debug_addr_string(unsigned id, uint32_t address, uint16_t port)
{
    char buffer[128];
    snprintf(buffer, sizeof buffer, "(id=%u, udp-local=%s:0x%x, udp-dest=%s:0x%x)",
	     id,
	     addr_string(local_addr).c_str(),
	     local_port,
	     addr_string(address).c_str(),
	     port);
    return buffer;
};

void CmIP_UDP::handle_received_message(uint32_t address, uint16_t port, ReceivedMessage rm, WeakString buffer_for_debugging)
{
    if (rm.msg_kind == 'i')
    {
        // Client se chce k nam pripojit, hned mu odpovime
        if (connect_addr)
        {
            cms_ns2_if_print("cmip",debug_name, 1, "Received message of type 'i', but I am not a server");
        }
        else
        {
            AcceptedSession ses = partners->get_or_create_session_id(
                rm.data,
                rm.sender_msg_id.get_or_throw("sender_msg_id not present"));
            ReceivedMessage response('a');
            response.server_boot_id = *server_boot_id;
            response.session_id = ses.session_id;
            response.sender_msg_id = ses.my_initial_msg_id;
            response.data = rm.data;
            sockaddr_in cliaddr;
            set_address(&cliaddr, address, port);
            bool ok = send_message_internal(cliaddr, response);
            if (!ok)
            {
                // Nevadi, tak se musi klient zeptat jeste jednou
                cms_ns2_if_print("cmip",debug_name, 1, "Failed to send message of type 'a'");
            }
        }
    }
    else if (rm.msg_kind == 'a')
    {
        // Server nam potvrdil spojeni
        if (!connect_addr)
        {
            cms_ns2_if_print("cmip",debug_name, 1, "Received message of type 'a', but I am not a client");
        }
        else if (!my_conn_request)
        {
            cms_ns2_if_print("cmip",debug_name, 1, "Received message of type 'a' which we did not request");
        }
        else if (rm.data != my_conn_request->hash)
        {
            cms_ns2_if_print("cmip",debug_name, 1, "Received message of type 'a' with different hash string");
        }
        else
        {
            // Server nam potvrdil, ze se k nemu muzeme pripojit
            Partner &p = partners->add_partner(address, port, udp_params, *rm.session_id);
            p.last_recv_time = get_current_time_usec();
            p.server_boot_id = *rm.server_boot_id;

            p.my_msg_id_counter = my_conn_request->sender_msg_id + 1;
            p.my_msg_id_received = p.my_msg_id_counter;

            p.his_msg_id_received = *rm.sender_msg_id + 1;
            
            cms_ns2_if_print("cmip",debug_name, 2, "New partner: %s", partner_info(p).c_str());
            events.connect(p.cmip_id, make_debug_addr_string(p.cmip_id, address, port));
            have_new_event = true;
            my_conn_request = Nothing();
        }
    }
    else if (rm.msg_kind == 'p' || rm.msg_kind == 'm' || rm.msg_kind == 'd')
    {
        Partner *p = partners->find_by_session_id(*rm.session_id);
        if (!p)
        {
            if (!connect_addr)
            {
                // Jsem server
                if (*rm.server_boot_id != *server_boot_id)
                {
                    cms_ns2_if_print("cmip",debug_name, 1, "Partner not found and different server_boot_id");
                }
                else
                {
                    // Zprava je adresovana mne.  Podivame se, jestli
                    // to neni novy partner, kteremu jsme uz potvrdili
                    // spojeni.
                    if (AcceptedSession const *ses = partners->find_session_id(*rm.session_id))
                    {
                        p = &partners->add_partner(address, port, udp_params, *rm.session_id);
                        p->last_recv_time = get_current_time_usec();
                        p->server_boot_id = *server_boot_id;

                        p->my_msg_id_counter = ses->my_initial_msg_id + 1;
                        p->my_msg_id_received = p->my_msg_id_counter;

                        p->his_msg_id_received = ses->his_initial_msg_id + 1;
                        
                        cms_ns2_if_print("cmip",debug_name, 2, "New partner: %s", partner_info(*p).c_str());
                        events.connect(p->cmip_id, make_debug_addr_string(p->cmip_id, address, port));
                        have_new_event = true;
                    }
                    else
                    {
                        cms_ns2_if_print("cmip",debug_name, 1, "Received message with unknown session id");                        
                    }
                }
            }
            else
            {
                // Jsem client
                cms_ns2_if_print("cmip",debug_name, 1, "Received message from unknown server");
            }
        }
        if (p)
        {
            if (*rm.server_boot_id != p->server_boot_id)
            {
                cms_ns2_if_print("cmip",debug_name, 1, "Partner found, but received different server_boot_id");
            }
            else
            {
                handle_valid_message(*p, rm, buffer_for_debugging);
            }
        }
        else
        {
            cms_ns2_if_print("cmip",debug_name, 1, "Received message with unknown session id");
        }
    }
    else
    {
        assert(false);
    }
}

void CmIP_UDP::check_partners_timeout()
{
    std::vector<unsigned> partners_to_disconnect;
    int64_t time_now = get_current_time_usec();
    CM_for (Partner &p, *partners)
    {
	// Zkontroluj timeout
	if (p.last_recv_time + p.udp_params.timeout < time_now)
	{
	    cms_ns2_if_print("cmip",debug_name, 3, "partner timeout");
	    partners_to_disconnect.push_back(p.cmip_id);
	}
    }
    CM_for (unsigned id, partners_to_disconnect)
    {
	partners->erase_partner_by_cmip_id(id);
	events.disconnect(id);
	have_new_event = true;
    }
}

// Vrati true, pokud bylo neco prijato
bool CmIP_UDP::do_receive()
{
    char buffer[MSGMAX];
    sockaddr_in cliaddr;
    socklen_t addrlen = sizeof(cliaddr);
    std::memset(&cliaddr, 0, sizeof cliaddr);
    cmip_debug_print("cmip", 5, "Waiting on recv_from ...");

    ssize_t n = recvfrom(sockfd, buffer, MSGMAX, 0, (sockaddr *)&cliaddr, &addrlen);
    if (n < 0)
    {
	if (errno == EWOULDBLOCK || errno == EAGAIN)
	{
	    cmip_debug_print("cmip", 5, "recvfrom() EAGAIN");
	}
	else
	{
	    cms_ns2_if_print("cmip",debug_name, 1, "recvfrom() error");
	}
	return false;
    }
    else
    {
        WeakString buf_ws(buffer, buffer + n);
	cmip_debug_print("cmip", 4, "recvfrom() received %s", buf_ws.str().c_str());
        ++total_recv_messages;
        try
        {
            ReceivedMessage rm = parse_message(buf_ws);
            cmip_debug_print("cmip", 4, "recvfrom() parsed %s", print_message(rm).c_str());
            try
            {
                handle_received_message(cliaddr.sin_addr.s_addr, ntohs(cliaddr.sin_port), rm, buffer);
            }
            catch (std::runtime_error &e)
            {
                cms_ns2_if_print("cmip",debug_name, 1, "Received invalid message: %s: %s", e.what(), print_message(rm).c_str());
            }
        }
        catch (std::runtime_error &e)
        {
            cms_ns2_if_print("cmip",debug_name, 1, "Failed to parse message: %s: >%s<", e.what(), buf_ws.str().c_str());
        }
	return true;
    }
}

// Tohle vola uzivatel
void CmIP_UDP::set_params(unsigned conn_id, CmIPParams params)
{
    CMSScopedLock lock(mutex);
    if (Partner *p = partners->find_by_cmip_id(conn_id))
    {
	p->udp_params = params;
	cms_ns2_if_print("cmip",debug_name, 3, "Set ping_period=%" PRId64" msec", params.ping_period / 1000);
	cms_ns2_if_print("cmip",debug_name, 3, "Set timeout=%" PRId64" msec", params.timeout / 1000);
	cms_ns2_if_print("cmip",debug_name, 3, "Set resend_delay=%" PRId64" msec", params.resend_delay / 1000);
	// Recv thread pravdepodobne ceka na recvfrom(), tak az mu
	// skonci timeout, tak bude tyhle nove parametry pouzivat.
    }
    else
    {
	cms_ns2_if_print("cmip",debug_name, 1, "Cannot set_params, partner has disconnected");
    } 
}

bool CmIP_UDP::is_ready_to_send(unsigned int conn_id)
{
    CMSScopedLock lock(mutex);
    if (Partner *p = partners->find_by_cmip_id(conn_id))
    {
        return can_send_messages(*p);
    }
    else
    {
     	cms_ns2_if_print("cmip",debug_name, 1, "Cannot send, partner not found");
        return true;
    }
}

// Tohle volá uživatel
void CmIP_UDP::send(char const *buffer, int size, unsigned conn_id)
{
    cmip_debug_print("cmip", 3, "CmIP_UDP::send size=%d >%s<", size, escape_special_chars(WeakString(buffer, buffer + size)).c_str());
    CMSScopedLock lock(mutex);
    if (Partner *p = partners->find_by_cmip_id(conn_id))
    {
        if (can_send_messages(*p))
        {
            SentMessage &msg = prepare_message_to_send(*p, WeakString(buffer, buffer + size));
            bool ok = send_regular_message(*p, msg.msg_id, msg.msg);
            if (ok)
            {
                ++msg.sent_times;
                msg.time_sent = get_current_time_usec();
            }
            else
                poll_obj.interrupt(); // Naplanuje se znovuodeslani
        }
        else
        {
            cmip_debug_print("cmip", 2, "Queuing message, too many unacknowledged messages: %zu, window_size=%u",
                             count_unacknowledged_messages(*p),
                             p->ack_window_size);
            p->queue_to_send.push_back(std::string(buffer, buffer + size));
            // Neprobouzime, cekame na potvrzeni zprav.
        }
    }
    else
    {
	cms_ns2_if_print("cmip",debug_name, 1, "Cannot send, partner has disconnected");
    }    
}

void CmIP_UDP::disconnect(unsigned conn_id)
{
    CMSScopedLock lock(mutex);
    if (partners->find_by_cmip_id(conn_id))
    {
	events.erase(conn_id);
	partners->erase_partner_by_cmip_id(conn_id);
    }
    else
    {
	cms_ns2_if_print("cmip",debug_name, 1, "Cannot disconnected, partner has disconnected");
    }
}

ssize_t CmIP_UDP::sendto(int sock, char const *data, size_t len, sockaddr_in dest)
{
    if (use_delay_simulator)
    {
        return delay_simulator.sendto(sock, data, len, dest);
    }
    else
    {
        cmip_debug_print("cmip", 4, "sendto() Sending: %s", std::string(data, len).c_str());
        return ::sendto(sockfd, data, len, 0, (sockaddr const *)&dest, sizeof dest);
    }
}

bool CmIP_UDP::send_message_internal(sockaddr_in dest, ReceivedMessage rm)
{
    cms_ns2_if_print("cmip",debug_name, 2, "Sending message: %s", print_message(rm).c_str());
    std::string msg_ss = write_message(rm);
    ssize_t len = sendto(sockfd, msg_ss.c_str(), msg_ss.size(), dest);
    if (len < 0)
    {
	if (errno == EWOULDBLOCK || errno == EAGAIN)
	{
	    cmip_debug_print("cmip", 5, "sendto() EAGAIN");
	    // Nic, zkusime to priste
	}
	else
	{
	    cmip_debug_print("cmip", 5, "sendto() error, errno=%d", errno);
	}
    }
    else
    {
	if ((size_t)len != msg_ss.size())
	{
	    cms_ns2_if_print("cmip",debug_name, 1, "sendto(): Data was sent partially, should not happen in UDP!");
	}
	else
	{
	    cmip_debug_print("cmip", 5, "Message sent successfully");
	    return true;
	}
    }
    return false;
}

uint8_t compute_missing_count(std::map<int64_t, std::string> const &messages_received, int64_t msg_id_end)
{
    if (messages_received.empty())
        return 0;
    int64_t msg_id = messages_received.begin()->first;
    assert(msg_id > msg_id_end);
    int64_t diff = msg_id - msg_id_end;
    // Pokud jich chybi moc, tak mu posleme 0. To je korektni.
    if (diff > 255)
        return 0;
    return diff;
}

void CmIP_UDP::shutdown()
{
    // TODO: Z destruktoru dat sem. Ale nemam to ozkoseny, tak to sem radeji nedavam.
}

bool CmIP_UDP::send_regular_message(Partner &p, int64_t msg_id, WeakString msg)
{
    sockaddr_in cliaddr;
    set_address(&cliaddr, p.address, p.port);

    cmip_debug_print("cmip", 5, "Sending to %s %u: msg_id=%04" PRIX64 ": %s ...",
		    addr_string(p.address).c_str(),
		    p.port,
                    msg_id,
		    msg.str().c_str());

    ReceivedMessage rm('m');
    rm.server_boot_id = p.server_boot_id;
    rm.session_id = p.session_id;
    rm.sender_msg_id = convert_msg_to_rem(msg_id);
    rm.receiver_msg_id = convert_msg_to_rem(p.his_msg_id_received);
    rm.missing_count = compute_missing_count(p.messages_received, p.his_msg_id_received);
    rm.data = msg;
    
    bool ok = send_message_internal(cliaddr, rm);
    if (ok)
    {
	p.last_sent_time = get_current_time_usec();
        if (*rm.missing_count > 0)
            cms_ns2_if_print("cmip",debug_name, 3, "Sent missing count = %d", *rm.missing_count);
    }
    return ok;
}

bool CmIP_UDP::send_ping_message(Partner &p)
{
    ReceivedMessage rm('p');
    rm.server_boot_id = p.server_boot_id;
    rm.session_id = p.session_id;
    rm.receiver_msg_id = convert_msg_to_rem(p.his_msg_id_received);
    rm.missing_count = compute_missing_count(p.messages_received, p.his_msg_id_received);

    sockaddr_in cliaddr;
    set_address(&cliaddr, p.address, p.port);
    bool ok = send_message_internal(cliaddr, rm);
    if (ok)
    {
        int64_t time_now = get_current_time_usec();
        int64_t last_ack_before = time_now - (p.last_sent_time ?: time_now);
        cms_ns2_if_print("cmip",debug_name, 5, "sending ping: n_recv=%lld, last_ack_before=%.3lf msec", (long long)p.his_msg_id_received, last_ack_before / 1000.0);
	p.last_sent_time = time_now;
        if (*rm.missing_count > 0)
            cms_ns2_if_print("cmip",debug_name, 3, "Sent missing count = %d", *rm.missing_count);
    }
    return ok;
}

// Vrati true, pokud se vse povedlo
// Vrati false, pokud se sendto() nepovedlo.
bool CmIP_UDP::send_partners_messages(Partner &p)
{
    cmip_debug_print("cmip", 5, "send_partners_messages: messages_sent=%zu", p.messages_sent.size());
    int64_t time_now = get_current_time_usec();
    
    // Nejdrive se podivej, jestli je potreba neco znovuposlat
    size_t resend_count = 0;
    size_t resend_marked_count = 0;
    size_t resend_first_send_count = 0;

    // bool no_response = false;
    // if (!p.messages_sent.empty())
    // {
    //     SentMessage &msg = p.messages_sent.front();
    //     if (msg.time_sent + p.udp_params.resend_delay < time_now)
    //         no_response = true;
    // }

    int64_t resend_delay = compute_resend_delay(p);
    CM_for (SentMessage &msg, p.messages_sent)
    {
	if (msg.time_sent + resend_delay < time_now || msg.sent_times == 0 || (msg.sent_times == 1 && msg.marked_as_missing))
	{
            if (msg.sent_times > 0)
                cms_ns2_if_print("cmip",debug_name, 2, "Resending msgid=%04" PRId64 ", sent_times=%u, marked_as_missing=%d sent_before=%.3lf msec, resend_delay=%.3lf msec: %s",
                                msg.msg_id,
                                msg.sent_times,
                                msg.marked_as_missing,
                                (time_now - msg.time_sent) / 1000.0,
                                resend_delay / 1000.0,
                                msg.msg.c_str()
                    );
	    if (!send_regular_message(p, msg.msg_id, msg.msg))
		return false;
            if (msg.marked_as_missing)
                ++resend_marked_count;
            if (msg.sent_times == 0)
                ++resend_first_send_count;
            ++resend_count;
            ++msg.sent_times;
            msg.marked_as_missing = false;
	    msg.time_sent = time_now;
	}
    }

    if (resend_count <= resend_first_send_count)
    {
        if (p.number_of_datagrams_without_loss > p.ack_window_size)
        {
            // Na linuxu je typicky fronta k odeslani dlouha max 1000,
            // dalsi datagramy jsou zahazovany. Kdyz jsem u zprav
            // simuloval zpozdeni, tak to vzdy pretekalo. Tak zde mam
            // limit 512.
            p.ack_window_size = std::min(p.number_of_datagrams_without_loss,
                                         (unsigned)std::max(512, (int)max_sent_messages));
            // if (p.ack_window_size * 2 < max_sent_messages && p.ack_window_size < 512)
            //     p.ack_window_size += 2;
        }
    }
    else
    {
        p.number_of_datagrams_without_loss = 0;
        if (p.ack_window_size > 2)
        {
            p.ack_window_size /= 2;
        }
    }
    
    cmip_debug_print("cmip", 5, "Resend Message: resend_count=%zu, marked=%zu, first_send=%zu", resend_count, resend_marked_count, resend_first_send_count);
    // cmip_debug_print("cmip", 5, "send_partners_messages: something_was_send=%d", something_was_send);
    // cmip_debug_print("cmip", 5, "send_partners_messages: last_sent_time=%" PRId64"", p.last_sent_time / 1000);

    if (resend_count == 0)
    {
	// Pokud jsme nic neposlali, musime zkontrolovat, jestli neni
	// treba poslat ack nebo ping
	bool need_send_msg = false;
	if (p.last_recv_data_time && p.last_sent_time < *p.last_recv_data_time && p.last_sent_time + ack_delay < time_now)
	{
	    cmip_debug_print("cmip", 5, "send_partners_messages: Need to send ACK");
	    need_send_msg = true;
	}
	else if (p.last_sent_time + p.udp_params.ping_period < time_now)
	{
	    cmip_debug_print("cmip", 5, "send_partners_messages: Need to send PING");
	    need_send_msg = true;
	}
	if (need_send_msg)
	{
	    if (!send_ping_message(p))
		return false;
	}
    }
    return true;
};
