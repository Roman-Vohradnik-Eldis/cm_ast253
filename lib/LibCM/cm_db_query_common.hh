#ifndef CM_QUERY_COMMON_HH_
#define CM_QUERY_COMMON_HH_

#include "cm_db_types.hh"
#include "cm_utils.hh"

inline size_t query_header_length()
{
    // kind + service_id + query_id
    return 1 + 4 + 16;
}

inline std::string debug_print(CmDbQuery const &st)
{
    std::stringstream ss;
    ss << "{";
    for (CmDbCommand const &c : st.commands)
        ss << c.cmd << ": " << Expr(c.args);
    ss << "}";
    return ss.str();
}

// Interni
//////////////////////////////////////////////////////////////////////

struct PartnerQueryId { long long value; };

inline bool operator==(PartnerQueryId a, PartnerQueryId b) { return a.value == b.value; }
inline bool operator!=(PartnerQueryId a, PartnerQueryId b) { return a.value != b.value; }

struct ServiceId
{
    int64_t value;
    static constexpr int64_t max_value{256};
};

inline bool operator==(ServiceId a, ServiceId b) { return a.value == b.value; }
inline bool operator!=(ServiceId a, ServiceId b) { return a.value != b.value; }

struct MyReceivedResponse
{
    // Kdyz sem neco pridame, dame na false. Az si to uzivatel vyzvedne, da sem true.
    bool was_read = false;
    
    CmDbCommandStatus cmd_status = CM_DB_ROWS; // Not set unless was_read == true
    std::vector<Expr> rows;
    std::string error_message;
};

std::ostream &operator<<(std::ostream &o, MyReceivedResponse const &r);

std::ostream &operator<<(std::ostream &o, std::vector<MyReceivedResponse> const &responses);

void add_to_query_state(std::vector<MyReceivedResponse> &responses,
                        size_t cmd_id,
                        CmDbCommandStatus status,
                        std::vector<Expr> const &rows,
                        std::string const &err_msg);

#endif // CM_QUERY_COMMON_HH_
