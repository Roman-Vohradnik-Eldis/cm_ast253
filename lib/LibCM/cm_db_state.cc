
#include "cm_db_state.hh"
#include "cm_base_partner.hh"
#include "cm_db_query_common.hh"
#include "cm_db_types.hh"
#include "cm_debug.hh"
#include "cm_utils.hh"
#include "cm_write_binary.hh"
#include "cm_send_buffer_simple.hh"
#include <algorithm>
#include <cstdio>
#include <iterator>
#include <limits>

#define DB_DEBUG(level, fmt, ...) cms_ns2_if_print("cmdb", debug_name, level, fmt, ##__VA_ARGS__)

static void debug_invalid_message(std::string const &debug_name,
                                  Maybe<unsigned> partner_id,
                                  View<uint8_t> data,
                                  char const *msg)
{    
    DB_DEBUG(1, "%s: partner_id=%u, %zu bytes, str='%s', hex='%s'",
             msg,
             partner_id.get_or(-1),
             data.size(),
             replace_nonprintable(data).c_str(),
             str_to_hex(data).c_str());
}

std::vector<uint8_t> pop_output_buffer(std::string const &debug_name, unsigned partner_id, SendBufferSimple &buffer)
{
    std::vector<uint8_t> result;
    ByteBuffer const &b = buffer.get();
    assert(b.size() != 1);
    result.insert(result.end(), b.begin(), b.end());
    buffer.reset(); // Neni potreba
    DB_DEBUG(4, "generate_data_to_send for %u: size=%zu: %s", partner_id, result.size(), replace_nonprintable(result).c_str());
    return result;
}

template <typename Container, typename It>
void append_moved(Container &container, It b, It e)
{
    container.insert(container.end(), std::make_move_iterator(b), std::make_move_iterator(e));
}

template <typename Map>
typename Map::key_type const *map_find_value(Map const &m, typename Map::mapped_type const &val)
{
    for (auto &p : m)
    {
        if (p.second == val)
            return &p.first;
    }
    return 0;
}

void erase_finished_partners(std::list<CmDbPartner> &partners)
{
    auto it = std::remove_if(partners.begin(), partners.end(), [](CmDbPartner &p)
    {
        return p.is_disconnected && p.my_query_list.size() == 0 && p.my_response_list.size() == 0;
    });
    partners.erase(it, partners.end());
}

CmDbState::CmDbState(WeakString dbg_name)
    : my_query_counter{rand() % 1000}
    , debug_name(dbg_name)
{
    partners.push_back(CmDbPartner(Nothing(), dbg_name, true, make_default_simple_buffer_settings()));
}

void CmDbState::accept_service(DbService service, DbServiceParams params)
{
    bool have_service = contains(my_services, service);
    if (!have_service && my_services.size() >= ServiceId::max_value)
    {
        DB_DEBUG(1, "accept_service: Too many accepted services!");
        return;
    }
    if (!have_service || !(my_services.at(service) == params))
    {
        DB_DEBUG(3, "Accepting service %s (forward=%d)", service.c_str(), params.forward);
        insert_or_replace(my_services, service, params);
    }
    else
    {
        DB_DEBUG(3, "Service %s already accepted (forward=%d)", service.c_str(), params.forward);
    }
    process_self_messages();
}


CmDbPartner &find_self_partner(std::list<CmDbPartner> &partners)
{
    for (CmDbPartner &p : partners)
    {
        if (!p.id)
            return p;
    }
    assert(false);
}

void CmDbState::process_self_messages()
{
    DB_DEBUG(4, "Processing self_messages begin");
    CmDbPartner &self = find_self_partner(partners);

    self.send_buffer.set_max_size(std::numeric_limits<size_t>::max());
    {
        self.send_buffer.clear();
        generate_accepted_services_to_send(self, self.send_buffer);
        std::vector<uint8_t> msg = pop_output_buffer(debug_name, 9999, self.send_buffer);
        if (!msg.empty())
        {
            DB_DEBUG(4, "Process self messages: accepted services: %zu bytes", msg.size());
            receive_partner_data(self, msg);
        }
    }
    {
        self.send_buffer.clear();
        self.my_query_list.generate_data_to_send(self.send_buffer);
        std::vector<uint8_t> msg = pop_output_buffer(debug_name, 9999, self.send_buffer);
        if (!msg.empty())
        {
            DB_DEBUG(4, "Process self messages: query: %zu bytes", msg.size());
            receive_partner_data(self, msg);
        }
    }
    {
        self.send_buffer.clear();
        self.my_response_list.generate_data_to_send(self.send_buffer);
        std::vector<uint8_t> msg = pop_output_buffer(debug_name, 9999, self.send_buffer);
        if (!msg.empty())
        {
            DB_DEBUG(4, "Process self messages: response: %zu bytes", msg.size());
            receive_partner_data(self, msg);
        }
    }
}

void CmDbState::set_accepted_services(std::map<DbService, DbServiceParams> const &services)
{
    std::map<DbService, DbServiceParams> new_services;
    if (services.size() >= ServiceId::max_value)
    {
        DB_DEBUG(1, "accept_service: Too many accepted services!");
        return;
    }
    for (auto const &p : services)
    {
        if (!contains(my_services, p.first) || !(my_services.at(p.first) == p.second))
            DB_DEBUG(3, "Accepting service %s (forward=%d)", p.first.c_str(), p.second.forward);
        new_services.insert({p.first, {p.second}});
    }
    for (auto const &p : my_services)
    {
        if (!contains(new_services, p.first))
            DB_DEBUG(3, "Refusing service %s", p.first.c_str());
    }
    my_services = new_services;
    process_self_messages();
}

CmDbPartner *CmDbState::find_partner(unsigned id)
{
    for (CmDbPartner &p : partners)
    {
        if (p.id && *p.id == id)
            return &p;
    }
    return nullptr;
}

void CmDbState::connect_partner(unsigned partner_id, PartnerParams const &params)
{
    if (find_partner(partner_id))
    {
        DB_DEBUG(1, "connect_partner: Partner %u already connected", partner_id);
        return;
    }
    DB_DEBUG(3, "New connected partner %u: compression=%s",
             partner_id,
             params.send_settings.compression_level ? "yes" : "no");
    partners.push_back(CmDbPartner(partner_id,
                                   debug_name,
                                   params.recv_use_control_character,
                                   params.send_settings));
}

void CmDbState::disconnect_partner(unsigned partner_id)
{
    auto it = std::find_if(partners.begin(), partners.end(), [=](CmDbPartner const &p) {return p.id && *p.id == partner_id;});
    if (it == partners.end())
    {
        DB_DEBUG(1, "connect_partner: Partner %u does not exist", partner_id);
        return;
    }

    DB_DEBUG(3, "Disconnecting partner %u", partner_id);
    it->is_disconnected = true;

    it->my_query_list.disconnect();
    it->my_response_list.disconnect();
    
    if (master_partner && *master_partner == partner_id)
        master_partner = Nothing();

    erase_finished_partners(partners);
}


template <typename It, typename F>
typename It::value_type *cm_find_if(It b, It e, F f)
{
    auto it = std::find_if(b, e, f);
    return it == e ? nullptr : &*it;
}

void CmDbState::set_master(unsigned int partner_id)
{
    CmDbPartner *p = cm_find_if(partners.begin(), partners.end(), [=](CmDbPartner const &p){return p.id && *p.id == partner_id;});
    if (!p)
    {
        DB_DEBUG(1, "set_master: Partner %u does not exist", partner_id);
        return;
    }
    unset_master();
    DB_DEBUG(3, "Partner %u is a master", partner_id);
    master_partner = partner_id;
}

void CmDbState::unset_master()
{
    if (master_partner)
    {
        DB_DEBUG(3, "Partner %u is not master anymore", *master_partner);
    }
    master_partner = Nothing();
}

CmDbPartner *CmDbState::find_destination_partner(DbService service)
{
    if (master_partner)
    {
        CmDbPartner *p = cm_find_if(partners.begin(), partners.end(),
                                    [=](CmDbPartner const &p){return p.id && *p.id == *master_partner;});
        assert(p);
        if (!p->is_disconnected)
            if (contains(p->partners_accepted_services, service))
                return p;
    }

    // Pokud jsme se dostali sem, tak master neni nikdo, nebo master nepodporuje zadany service
    if (master_slave_policy == CM_DB_SEND_TO_MASTER)
    {
        return nullptr;
    }
    else
    {
        // Vezmeme libovolneho, pokud nekdo service podporuje
        for (CmDbPartner &p : partners)
        {
            if (!p.is_disconnected && contains(p.partners_accepted_services, service))
                return &p;
        }
        return nullptr;
    }
}

Maybe<QueryId> CmDbState::send_query_maybe(DbService service, CmDbQuery const &q, int64_t time_now)
{
    if (service == "_TIMER")
    {
        QueryId query_id = my_query_counter;
        ++my_query_counter.value;
        timer_list.add_query(query_id, service, q, time_now);
        process_self_messages(); // Je to tady potreba?
        return query_id;
    }
    else if (CmDbPartner *p = find_destination_partner(service))
    {
        QueryId query_id = my_query_counter;
        ++my_query_counter.value;

        p->my_query_list.add_new_query(query_id, p->partners_accepted_services.at(service), service, q, time_now);

        process_self_messages();
        return query_id;
    }
    else
    {
        DB_DEBUG(3, "send_query: No partner accepts %s", service.c_str());
        return Nothing();
    }
}

QueryId CmDbState::send_query(DbService service, CmDbQuery const &q, int64_t time_now)
{
    DB_DEBUG(3, "send_query %s: %s", to_string(service).c_str(), to_string(q).c_str());
    if (Maybe<QueryId> id = send_query_maybe(service, q, time_now))
    {        
        return *id;
    }
    else
    {
        QueryId query_id = my_query_counter;
        ++my_query_counter.value;
        std::string error_message = format("Service %s is not available", service.c_str());
        for (size_t i = 0; i < q.commands.size(); ++i)
        {
            DB_DEBUG(3, "Adding response error message cmd_id=%zu", i);
            bool last = i+1 == q.commands.size();
            responses_from_nonexistant_services.push_back(
                {query_id, i, service, CM_DB_ERROR, {}, error_message, last});
        }
        return query_id;
    }
}

Maybe<QueryId> CmDbState::begin_forwarded_query(DbService service)
{
    CmDbPartner *pp = find_destination_partner(service);
    if (!pp)
    {
        DB_DEBUG(3, "send_query: No partner accepts %s", service.c_str());
        return Nothing();
    }

    CmDbPartner &p = *pp;

    QueryId query_id = my_query_counter;
    ++my_query_counter.value;
    p.my_query_list.begin_forwarded_query(query_id, p.partners_accepted_services.at(service), service);
    DB_DEBUG(4, "begin_forwarded_query service=%s, creating user_query_id=%llx",
             service.c_str(), query_id.value);

    process_self_messages();
    return query_id;
}

void CmDbState::add_to_forwarded_query(DbMessage msg)
{
    DB_DEBUG(4, "add_to_forwarded_query: %s", to_string(msg).c_str());
    assert(msg.kind == 'q' || msg.kind == 'Q');
    for (CmDbPartner &p : partners)
    {
        if (p.is_disconnected)
            continue;
        
        if (p.my_query_list.has_query_id(QueryId{msg.query_id}))
        {
            p.my_query_list.add_forwarded_query_data(QueryId({msg.query_id}),
                                                     msg.kind == 'Q',
                                                     msg.data);
            process_self_messages();
            return;
        }
    }
    DB_DEBUG(1, "add_to_forwarded_query: Unknown query_id %llx", msg.query_id.value);
}

void CmDbState::add_forwarded_response(DbMessage msg)
{
    DB_DEBUG(4, "add_forwarded_response: %s", to_string(msg).c_str());
    assert(msg.kind == 'r' || msg.kind == 'R' || msg.kind == 'D' || msg.kind == 'E');
    for (CmDbPartner &p : partners)
    {
        if (p.is_disconnected)
            continue;
        
        if (p.my_response_list.has_query_id(QueryId{msg.query_id}))
        {
            p.my_response_list.send_forwarded_response(QueryId({msg.query_id}),
                                                      msg.data,
                                                      msg.kind);
            process_self_messages();
            return;
        }
    }
    DB_DEBUG(1, "add_forwarded_response: Unknown query_id %llx", msg.query_id.value);
}

void CmDbState::send_response(CmDbResponse response)
{
    cms_ns2_lines("dbstate", debug_name, 3, get_debug());
    for (CmDbPartner &p : partners)
    {
        if (p.my_response_list.has_query_id(QueryId{response.query_id}))
        {
            p.my_response_list.send_response(QueryId({response.query_id}),
                                                     response.cmd_id,
                                                     response.cmd_status,
                                                     std::move(response.rows),
                                                     std::move(response.error_message));
            process_self_messages();
            return;
        }
    }
    DB_DEBUG(1, "send_response: No partner has query id %llx", (long long)response.query_id.value);
}

std::vector<CmDbReceivedQuery> CmDbState::get_received_queries()
{
    std::vector<CmDbReceivedQuery> result;
    for (CmDbPartner &p : partners)
    {
        std::vector<CmDbReceivedQuery> r = p.my_response_list.get_new_queries(0);
        append_moved(result, r.begin(), r.end());
    }
    erase_finished_partners(partners);
    return result;
}

size_t CmDbState::query_count() const
{
    size_t n = 0;
    for (auto const &p : partners)
        n += p.my_query_list.size();
    return n;
}

size_t CmDbState::response_count() const
{
    size_t n = 0;
    for (auto const &p : partners)
        n += p.my_response_list.size();
    return n;    
}

std::map<QueryId, std::string> CmDbState::get_cancelled_queries()
{
    std::map<QueryId, std::string> result;
    for (CmDbPartner &p : partners)
    {
        auto r = p.my_response_list.get_cancelled_queries();
        result.insert(r.begin(), r.end());
    }
    erase_finished_partners(partners);
    return result;
}

std::vector<CmDbReceivedResponse> CmDbState::get_received_responses()
{
    std::vector<CmDbReceivedResponse> result;

    // Queries where service did not not existed
    append_moved(result,
                 responses_from_nonexistant_services.begin(),
                 responses_from_nonexistant_services.end());
    responses_from_nonexistant_services.clear();

    {
        auto qs = timer_list.get_received_responses();
        append_moved(result, qs.begin(), qs.end());
    }

    for (CmDbPartner &p : partners)
    {
        std::vector<CmDbReceivedResponse> r = p.my_query_list.get_new_responses(0);
        append_moved(result, r.begin(), r.end());
    }
    // cms_ns_if_print("debug", 3, "get_received_responses: %zu", result.size());
    erase_finished_partners(partners);
    return result;
}

std::vector<DbMessage> CmDbState::get_forwarded_responses()
{
    std::vector<DbMessage> result;

    for (CmDbPartner &p : partners)
    {
        auto r = p.my_query_list.get_forwarded_responses();
        append_moved(result, r.begin(), r.end());
    }
    erase_finished_partners(partners);
    return result;
}

std::vector<DbMessage> CmDbState::get_forwarded_queries()
{
    std::vector<DbMessage> result;
    for (CmDbPartner &p : partners)
    {
        auto r = p.my_response_list.get_queries_to_forward();
        append_moved(result, r.begin(), r.end());
    }
    return result;
}

Maybe<std::map<DbService, DbAvailableServiceParams>> CmDbState::get_accepted_services_change()
{
    // Spocteme sjednoceni services od vsech partneru
    std::map<DbService, DbAvailableServiceParams> result;
    for (CmDbPartner const &p : partners)
    {
        if (p.is_disconnected)
            continue;
        if (p.id)
        {
            for (auto const &as : p.partners_accepted_services)
            {
                DbAvailableServiceParams &params = result[as.first];
                params.is_provided_by_partner = true;
            }
        }
    }
    for (auto const &p : my_services)
    {
        DbAvailableServiceParams &params = result[p.first];
        if (p.second.forward)
            params.is_forwarded_locally = true;
        else
            params.is_provided_locally = true;
    }
    
    // Porovname s poslednim callbackem
    if (result != all_partners_accepted_services)
    {
        all_partners_accepted_services = result;
        return result;
    }
    else
    {
        return Nothing();
    }
}

// data: Orizly kind.
bool CmDbState::receive_msg_accept(CmDbPartner &p, View<uint8_t> orig_data, View<uint8_t> data)
{
    if (data.size() <= 4)
    {
        debug_invalid_message(debug_name, p.id, orig_data, "receive_msg_accept: Message invalid length");
        return false;
    }
    uint64_t service_id_num = read_hex_number(view(data.begin(), data.begin() + 4));
    std::string t(data.begin() + 4, data.end());
    if (service_id_num > INT16_MAX)
    {
        DB_DEBUG(1, "Invalid service id: %s = %llx", t.c_str(), (long long)service_id_num);
        return false;
    }
    ServiceId service_id{(int16_t)service_id_num};
    if (contains(p.partners_accepted_services, t))
    {
        DB_DEBUG(1, "Service already known: %s", t.c_str());
        return false;
    }
    else if (map_find_value(p.partners_accepted_services, service_id))
    {
        DB_DEBUG(1, "Service ID already used, ignoring");
        return false;
    }
    DB_DEBUG(3, "Partner %d now accepts service '%s' id=%llx", p.id ? *p.id : -1, t.c_str(), (long long)service_id_num);
    p.partners_accepted_services.insert({t, service_id});
    return true;
}

// data: Orizly kind.
bool CmDbState::receive_msg_reject(CmDbPartner &p, View<uint8_t> orig_data, View<uint8_t> data)
{
    if (data.size() != 4)
    {
        debug_invalid_message(debug_name, p.id, orig_data, "receive_msg_reject: Message invalid length");
        return false;
    }
    uint64_t service_id_num = read_hex_number(view(data.begin(), data.begin() + 4));
    if (service_id_num > INT16_MAX)
    {
        DB_DEBUG(1, "Invalid service id: %" PRIu64, service_id_num);
        return false;            
    }
    ServiceId service_id{(int16_t)service_id_num};
    DbService const *service = map_find_value(p.partners_accepted_services, service_id);
    if (!service)
    {
        DB_DEBUG(1, "Service id not known: %llx", (long long)service_id_num);
        return false;
    }
    DB_DEBUG(3, "Partner %u now refuses service '%s' id=%llx", p.id ? *p.id : -1, service->c_str(), (long long)service_id_num);
    p.partners_accepted_services.erase(*service);
    return true;
}

// data: Orizly kind.
bool CmDbState::receive_msg_full(CmDbPartner &p, char kind, View<uint8_t> orig_data, View<uint8_t> data)
{
    if (orig_data.size() < query_header_length())
    {
        debug_invalid_message(debug_name, p.id, orig_data, "receive_msg_full: Message too short");
        return false;
    }
    uint64_t service_id_num = read_hex_number(view(data.begin(), data.begin() + 4));
    if (service_id_num > INT16_MAX)
    {
        DB_DEBUG(1, "Invalid session id %llx", (long long)service_id_num);
        return false;        
    }
            
    uint64_t query_id_num = read_hex_number(view(data.begin() + 4, data.begin() + 4 + 16));
    if (query_id_num > INT64_MAX)
    {
        DB_DEBUG(1, "Invalid query id %llx", (long long)query_id_num);
        return false;        
    }
    ServiceId service_id{(int64_t)service_id_num};
    PartnerQueryId query_id{(int64_t)query_id_num};
    View<uint8_t> body(data.begin() + 4 + 16, data.end());

    if (kind == 'q' || kind == 'Q' || kind == 'C')
    {
        DbService const *service_name = map_find_value(p.my_accepted_services, service_id);
        if (!service_name)
        {
            // TODO: Tohle se muze stat, pokud jsme poskytovali
            // službu, pak jsme ji přestali poskytovat, ale když
            // Client posilal dotaz, tak o tom jeste nevedel
            DB_DEBUG(1, "Unknown service id %llx in receive_msg: kind=%c partner_id=%u query_id=%llx",
                     (long long)service_id_num,
                     kind,
                     p.id ? *p.id : -1,
                     (long long)query_id_num);
            return false;
        }
        DbServiceParams const *params = map_find(my_services, *service_name);
        if (!params)
        {
            DB_DEBUG(1, "Service %s was supported, but partner was not told yet", service_name->c_str());
            return false;        
        }
        if (kind == 'q' || kind == 'Q')
        {
            if (!p.my_response_list.has_query(query_id))
            {
                QueryId id{my_query_counter.value++};
                p.my_response_list.create_new_query(id, query_id, service_id, *service_name, params->forward, false);
            }
            return p.my_response_list.receive_data(query_id, service_id, kind == 'Q', body);
        }
        else
        {
            assert(kind == 'C');
            return p.my_response_list.receive_cancel(query_id, service_id, body);
        }
    }
    else if (kind == 'r' || kind == 'R' || kind == 'E')
    {
        DbService const *service_name = map_find_value(p.partners_accepted_services, service_id);
        if (!service_name)
        {
            DB_DEBUG(1, "Unknown service id %llx in receive_msg: kind=%c partner_id=%d query_id=%llx",
                     (long long)service_id_num,
                     kind,
                     p.id ? *p.id : -1,
                     (long long)query_id_num);
            return false;        
        }
        return p.my_query_list.receive_data(query_id, service_id, body, kind);
    }
    else
    {
        DB_DEBUG(1, "Partner %d: Invalid DB message kind %c", p.id ? *p.id : -1, kind);
        return false;
    }
}

bool CmDbState::receive_partner_data(CmDbPartner &p, View<uint8_t> input_data)
{
    if (input_data.empty())
        return false;

    char kind = input_data[0];
    input_data.advance(1);

    if (p.recv_use_control_character)
    {
        // TODO: May throw
        p.recv_buffer.reset();
        p.recv_buffer.add_data(input_data);
    }
    else
    {
        // TODO: Zbytecne kopirovani
        std::vector<uint8_t> &out = p.recv_buffer.get();
        out.clear();
        out.insert(out.end(), input_data.begin(), input_data.end());
    }

    std::vector<uint8_t> &out = p.recv_buffer.get();
    View<uint8_t> orig_data(out);

    // View<uint8_t> data(orig_data);
    // char kind = data[0];
    // data.advance(1);

    if (kind == 'a')
    {
        return receive_msg_accept(p, orig_data, orig_data);
    }
    else if (kind == 'A')
    {
        return receive_msg_reject(p, orig_data, orig_data);
    }
    else
    {
        return receive_msg_full(p, kind, orig_data, orig_data);
    }    
}

// Prijde textova zprava od partnera
bool CmDbState::receive_text_data(unsigned partner_id, View<uint8_t> orig_data)
{
    DB_DEBUG(4, "receive_text_data from %u", partner_id);
    CmDbPartner *p = find_partner(partner_id);
    if (!p)
    {
        DB_DEBUG(1, "Partner %u does not exist", partner_id);
        return false;
    }
    if (p->is_disconnected)
    {
        DB_DEBUG(1, "Partner %u was disconnected", partner_id);
        return false;
    }
    
    return receive_partner_data(*p, orig_data);
}

Maybe<ServiceId> get_fresh_service_id(std::map<DbService, ServiceId> const &services)
{
    if (services.size() >= ServiceId::max_value)
        return Nothing();
    ServiceId id{rand() % ServiceId::max_value};
    while (map_find_value(services, id))
        ++id.value;
    return id;
}

// Posleme partnerovi nase sluzby
void CmDbState::generate_accepted_services_to_send(CmDbPartner &p, OutputBufferSimple &out)
{
    for (auto const &sp : my_services)
    {
        if (p.id && !sp.second.external)
            continue;
        
        DbService const &s = sp.first;
        if (!contains(p.my_accepted_services, s))
        {
            Maybe<ServiceId> serviceid = get_fresh_service_id(p.my_accepted_services);
            assert(serviceid);
            if (serviceid)
            {
                std::vector<uint8_t> msg;
                push_hex_number(msg, serviceid->value, 4);
                msg.insert(msg.end(), s.name.begin(), s.name.end());

                ssize_t len = out.try_add_message('a', 'a', msg, {}, 0);
                if (len >= 0)
                {
                    assert(len == 0);
                    DB_DEBUG(4, "Sending accepted service (partner %d):%s", p.id ? *p.id : -1, s.c_str());
                    p.my_accepted_services.insert({s, *serviceid});
                    return;
                }
            }
        }
    }
    for (auto const &sp : p.my_accepted_services)
    {
        DbService const &s = sp.first;
        if (!contains(my_services, s))
        {
            std::vector<uint8_t> msg;
            push_hex_number(msg, sp.second.value, 4);

            ssize_t len = out.try_add_message('A', 'A', msg, {}, 0);
            if (len >= 0)
            {
                assert(len == 0);
                DB_DEBUG(4, "Removing accepted service (partner %d):%s", p.id ? *p.id : -1, s.c_str());
                p.my_accepted_services.erase(s);
                return;
            }
        }
    }
}

template <typename T>
Maybe<T> maybe_min(Maybe<T> const &x, Maybe<T> const &y)
{
    if (x)
    {
        return y ? std::min(*x, *y) : x;
    }
    else
    {
        return y;
    }
}

Maybe<int64_t> CmDbState::get_next_wakeup_time()
{
    Maybe<int64_t> min;
    for (CmDbPartner &p : partners)
    {
        min = maybe_min(min, p.my_query_list.get_next_wakeup_time());
    }
    min = maybe_min(min, timer_list.get_next_wakeup_time());
    return min;
}

bool CmDbState::wakeup(int64_t time_now)
{
    bool was_changed = false;
    for (CmDbPartner &p : partners)
    {
        bool ch = p.my_query_list.check_timeout(time_now);
        was_changed = was_changed || ch;
    }
    if (timer_list.wakeup(time_now))
        was_changed = true;

    process_self_messages();
    return was_changed;
}

std::vector<uint8_t> CmDbState::generate_data_to_send(unsigned partner_id, Maybe<size_t> max_length)
{
    CmDbPartner *p = find_partner(partner_id);
    if (!p)
    {
        DB_DEBUG(1, "generate_data_to_send: Partner %u not found", partner_id);
        return {};
    }
    if (p->is_disconnected)
    {
        DB_DEBUG(1, "generate_data_to_send: Partner %u was disconnected", partner_id);
        return {};        
    }

    p->send_buffer.set_max_size(max_length ? *max_length : 0xffff);

    // if (rand() % 5 == 0)
    // p->send_buffer.set_max_size(32 + rand() % 100);

    p->send_buffer.clear();
    generate_accepted_services_to_send(*p, p->send_buffer);
    if (!p->send_buffer.get().empty())
        return pop_output_buffer(debug_name, partner_id, p->send_buffer);

    p->send_buffer.clear();
    p->my_query_list.generate_data_to_send(p->send_buffer);
    if (!p->send_buffer.get().empty())
        return pop_output_buffer(debug_name, partner_id, p->send_buffer);

    p->send_buffer.clear();
    p->my_response_list.generate_data_to_send(p->send_buffer);
    if (!p->send_buffer.get().empty())
        return pop_output_buffer(debug_name, partner_id, p->send_buffer);

    return {};
}

CmDbPartner *find_partner_by_my_query_id(std::list<CmDbPartner> &partners, QueryId query_id)
{
    for (CmDbPartner &p : partners)
    {
        if (p.my_query_list.has_query_id(QueryId{query_id}))
            return &p;
    }
    return nullptr;
}

void CmDbState::cancel_query(QueryId query_id, WeakString reason, bool gen_callback)
{
    DB_DEBUG(1, "Cancel query %llx", query_id.value);
    if (CmDbPartner *p = find_partner_by_my_query_id(partners, query_id))
    {
        p->my_query_list.cancel_query(query_id, reason, gen_callback);
        process_self_messages();
    }
    else if (timer_list.has_query_id(query_id))
    {
        timer_list.cancel_query(query_id, reason, gen_callback);
        process_self_messages(); // Je to potreba?
    }
    else
    {
        DB_DEBUG(1, "Query id not found: %llx", query_id.value);
    }
}

void CmDbState::set_master_slave_policy(CmDbMasterSlavePolicy msv_policy)
{
    master_slave_policy = msv_policy;
}

template <typename Range, typename What>
void insert_to_end(Range &r, What const &w)
{
    r.insert(r.end(), w.begin(), w.end());
}

std::vector<std::string> CmDbState::get_debug() const
{
    std::vector<std::string> lines;
    for (CmDbPartner const &p : partners)
    {
        lines.push_back(format("Partner %d, connected=%d:", p.id ? *p.id : -1, !p.is_disconnected));
        for (std::string const &l : p.my_response_list.get_debug())
            lines.push_back("+  " + l);
    }
    return lines;
}
