#include "cm_db_timer_list.hh"
#include "cm_db_types.hh"
#include <exception>

template <typename Map, typename Pred>
void map_erase_if_second(Map &map, Pred &&pred)
{
    for (auto it = map.begin(); it != map.end(); )
    {
        if (pred(it->second))
            it = map.erase(it);
        else
            ++it;
    }
}

template <typename Map, typename Pred>
std::vector<typename Map::mapped_type> map_select(Map &map, Pred &&pred)
{
    std::vector<typename Map::mapped_type> result;
    for (auto it = map.begin(); it != map.end(); )
    {
        if (pred(*it))
        {
            result.push_back(std::move(it->second));
            it = map.erase(it);
        }
        else
            ++it;
    }
    return result;
}

template <typename Map, typename Pred>
std::vector<typename Map::mapped_type> map_select_by_second(Map &map, Pred &&pred)
{
    auto p = [pred](typename Map::value_type const &v){return pred(v.second); };
    return map_select(map, p);
}

template <typename Map, typename Pred>
std::vector<typename Map::mapped_type> map_select_by_first(Map &map, Pred &&pred)
{
    auto p = [pred](typename Map::value_type const &v){return pred(v.first); };
    return map_select(map, p);
}

bool CmDbTimerList::wakeup(int64_t time_now)
{
    auto p = [time_now](int64_t t){return t < time_now; };
    auto ex = map_select_by_first(waiting, p);
    expired.insert(expired.end(), ex.begin(), ex.end());
    return ex.size() > 0;
}

Maybe<int64_t> CmDbTimerList::get_next_wakeup_time()
{
    if (waiting.empty())
        return Nothing();
    else
        return waiting.begin()->first;
}

std::vector<CmDbReceivedResponse> CmDbTimerList::get_received_responses()
{
    return std::move(expired);
}

bool CmDbTimerList::has_query_id(QueryId q)
{
    for (auto &p : waiting)
        if (p.second.query_id == q)
            return true;
    for (auto &r : expired)
        if (r.query_id == q)
            return true;
    return false;
}

void CmDbTimerList::cancel_query(QueryId q, WeakString reason, bool gen_callback)
{
    auto p = [q](CmDbReceivedResponse const &r){return r.query_id == q;};
    if (gen_callback)
    {
        auto ex = map_select_by_second(waiting, p);
        expired.insert(expired.end(), ex.begin(), ex.end());

        bool found = false;
        for (CmDbReceivedResponse &r : expired)
        {
            if (r.query_id == q)
            {
                found = true;
                if (r.cmd_status == CM_DB_DONE)
                {
                    cms_ns_if_print("cmdb", 3, "Cancelling timer query");
                    r.error_message = "Cancelled: " + reason;
                    r.cmd_status = CM_DB_ERROR;
                }
                else
                {
                    assert(r.cmd_status == CM_DB_ERROR);
                    cms_ns_if_print("cmdb", 3, "Cannot cancel timer query: Query already cancelled");
                }
            }
        }
        if (!found)
            cms_ns_if_print("cmdb", 3, "Cannot cancel timer query: QueryId does not exist");

    }
    else
    {
        map_erase_if_second(waiting, p);
        auto it = std::remove_if(expired.begin(), expired.end(), p);
        expired.erase(it, expired.end());
    }
}

void CmDbTimerList::add_query(QueryId id, DbService service, CmDbQuery const &q, int64_t time_now)
{
    if (q.commands.size() == 1)
    {
        CmDbCommand const &cmd = q.commands.at(0);
        if (cmd.cmd == "wait")
        {
            if (cmd.args.size() == 1 && cmd.args.at(0).is_int())
            {
                int64_t wakeup_after = cmd.args.at(0).to_int();
                waiting.insert({time_now + wakeup_after,
                            CmDbReceivedResponse{
                            .query_id = id,
                                .cmd_id = 0,
                                .service_name = service,
                                .cmd_status = CM_DB_DONE,
                                .rows = {},
                                .error_message = "",
                                .is_query_completed = true,
                                }
                    });
                return;
            }
        }
    }

    // On error:

    for (size_t i = 0; i < q.commands.size(); ++i)
    {
        expired.push_back(CmDbReceivedResponse{
                .query_id = id,
                    .cmd_id = i,
                    .service_name = service,
                    .cmd_status = CM_DB_ERROR,
                    .rows = {},
                    .error_message = "Invalid command",
                    .is_query_completed = true,
                    });
    }
}
