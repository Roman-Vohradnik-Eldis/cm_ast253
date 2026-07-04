#include "cm_ip_udp_partner.hh"
#include "cm_utils.hh"
#include "cm_debug.hh"

size_t count_unacknowledged_messages(Partner const &p)
{
    assert(p.my_msg_id_received <= p.my_msg_id_counter);
    return p.my_msg_id_counter - p.my_msg_id_received;
}

bool can_send_messages(Partner const &p)
{
    size_t n = count_unacknowledged_messages(p);
    return n <= p.ack_window_size;
}

std::string partner_info(Partner const &p)
{
    return format("[boot_id=0x%lX, session_id=0x%lX]", (long)p.server_boot_id.value, (long)p.session_id.value);
}
