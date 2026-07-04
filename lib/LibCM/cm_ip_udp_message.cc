#include "cm_ip_udp_message.hh"
#include "cm_debug.hh"
#include "cm_random.hh"
#include <assert.h>
#include <sstream>

static char to_hex(int c)
{
    static char s[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    return s[c];
}

std::string print_hex(uint64_t orig, unsigned n_chars)
{
    std::string chars;
    for (uint64_t n = orig; n > 0; n /= 16)
    {
        uint64_t x = n % 16;
        chars.push_back(to_hex(x));
    }
    if (chars.size() > n_chars)
    {
        assert(false);
        throw std::logic_error(format("number too large: 0x%llx, size=%zu, expected<=%u", (long long)orig, chars.size(), n_chars));
    }
    while (chars.size() < n_chars)
        chars.push_back('0');
    return std::string(chars.rbegin(), chars.rend());
}

uint64_t read_hex(WeakString w)
{
    uint64_t x = 0;
    for (size_t i = 0; i < w.size(); ++i)
    {
        char c = w[i];
        x *= 16;
        if (c >= '0' && c <= '9')
            x += c - '0';
        else if (c >= 'A' && c <= 'F')
            x += 10 + (c - 'A');
        else
            throw std::runtime_error(format("Failed to read hex number: >%s<", w.str().c_str()));
    }
    return x;
}

static bool is_valid_hash_char(char c)
{
    return c >= 'A' && c <= 'Z';
}

// 26^16 == cca 2^75 moznosti
bool is_valid_hash_str(WeakString w)
{
    if (w.size() != 16)
        return false;
    for (size_t i = 0; i < w.size(); ++i)
        if (!is_valid_hash_char(w[i]))
            return false;
    return true;
}

std::string gen_random_hash_str()
{
    size_t const len = 16;
    uint8_t buffer[len];
    cm_get_random(buffer, len);

    std::string s(len, 0);
    for (int i = 0; i < len; ++i)
        s[i] = 'A' + (buffer[i] % ('Z' - 'A'));
    return s;
}

static std::string change_nonprintable(WeakString in)
{
    std::string result(in.begin(), in.end());
    for (char &c : result)
        if (!isgraph(c) && c != ' ')
            c = '?';
    return result;
}


std::string print_message(ReceivedMessage rm)
{
    std::stringstream ss;
    ss << "[type=" << rm.msg_kind
       << " boot=" << (rm.server_boot_id ? print_hex(rm.server_boot_id->value, 2) : "?")
       << " session=" << (rm.session_id ? print_hex(rm.session_id->value, 4) : "?")
       << " sender_msg=" << (rm.sender_msg_id ? print_hex(*rm.sender_msg_id, 4) : "?")
       << " recver_msg=" << (rm.receiver_msg_id ? print_hex(*rm.receiver_msg_id, 4) : "?")
       << " data=" << change_nonprintable(rm.data)
       << "]";
    return ss.str();
}

ReceivedMessage parse_message(WeakString buffer)
{
    // Zprava obsahuje aspon "cm:t:"
    if (buffer.size() < 4 || buffer[0] != 'c' || buffer[1] != 'm' || buffer[2] != ':')
        throw std::runtime_error("Expected message beginning with 'cm:t'");
    char type = buffer[3];
    buffer.advance(4);

    // Jsme za znakem typu zpravy
    ReceivedMessage rm(type);
    if (type == 'i')
    {
        if (buffer.size() < 5)
            throw std::runtime_error("Header of type 'a' is too short");
        rm.sender_msg_id =   read_hex(WeakString(buffer.begin() + 0, buffer.begin() + 4));
        if (buffer[4] != ':')
            throw std::runtime_error("Expected colon after 'i'");
        rm.data = WeakString(buffer.begin() + 5, buffer.end());
        if (!is_valid_hash_str(rm.data))
            throw std::runtime_error(format("In message of type 'i': Invalid hash string >%s<", rm.data.str().c_str()));
    }
    else if (type == 'a')
    {
        if (buffer.size() < 10)
            throw std::runtime_error("Header of type 'a' is too short");
        rm.server_boot_id = BootId::parse(read_hex(WeakString(buffer.begin() + 0, buffer.begin() + 2)));
        rm.session_id = SessionId::parse(read_hex(WeakString(buffer.begin() + 2, buffer.begin() + 6)));
        rm.sender_msg_id =   read_hex(WeakString(buffer.begin() + 6, buffer.begin() + 10));
        if (buffer[10] != ':')
            throw std::runtime_error("Expected colon at the end of the header of type 'a'");
        rm.data = WeakString(buffer.begin() + 11, buffer.end());
        if (!is_valid_hash_str(rm.data))
            throw std::runtime_error(format("In message of type 'a': Invalid hash string >%s<", rm.data.str().c_str()));
    }
    else if (type == 'm')
    {
        if (buffer.size() < 16)
            throw std::runtime_error("Header of type 'm' is too short");
        rm.server_boot_id =  BootId::parse(read_hex(WeakString(buffer.begin() +  0, buffer.begin() +  2)));
        rm.session_id =      SessionId::parse(read_hex(WeakString(buffer.begin() +  2, buffer.begin() +  6)));
        rm.receiver_msg_id = read_hex(WeakString(buffer.begin() +  6, buffer.begin() + 10));
        rm.missing_count =   read_hex(WeakString(buffer.begin() + 10, buffer.begin() + 12));
        rm.sender_msg_id =   read_hex(WeakString(buffer.begin() + 12, buffer.begin() + 16));
        if (buffer[16] != ':')
            throw std::runtime_error("Expected colon at the end of the header of type 'm'");
        rm.data = WeakString(buffer.begin() + 17, buffer.end());
    }
    else if (type == 'p')
    {
        if (buffer.size() != 12)
            throw std::runtime_error("Header of type 'p' has bad length");
        rm.server_boot_id =  BootId::parse(read_hex(WeakString(buffer.begin() +  0, buffer.begin() +  2)));
        rm.session_id =      SessionId::parse(read_hex(WeakString(buffer.begin() +  2, buffer.begin() +  6)));
        rm.receiver_msg_id = read_hex(WeakString(buffer.begin() +  6, buffer.begin() + 10));
        rm.missing_count =   read_hex(WeakString(buffer.begin() + 10, buffer.begin() + 12));
    }
    else if (type == 'd')
    {
        if (buffer.size() != 10)
            throw std::runtime_error("Header of type 'd' has bad length");
        rm.server_boot_id = BootId::parse(read_hex(WeakString(buffer.begin() + 0, buffer.begin() + 2)));
        rm.session_id = SessionId::parse(read_hex(WeakString(buffer.begin() + 2, buffer.begin() + 6)));
        rm.receiver_msg_id = read_hex(WeakString(buffer.begin() + 6, buffer.begin() + 10));
    }
    else
    {
        throw std::runtime_error(format("Unknown message type '%c'", type));
    }
    return rm;
}

std::string write_message(ReceivedMessage rm)
{
    std::string msg_ss;
    msg_ss += "cm:";
    msg_ss.push_back(rm.msg_kind);
    switch (rm.msg_kind)
    {
    case 'i':
        msg_ss += print_hex(*rm.sender_msg_id, 4);
        msg_ss += ":";
        msg_ss += rm.data;
        break;
    case 'a':
        msg_ss += print_hex(rm.server_boot_id->value, 2);
        msg_ss += print_hex(rm.session_id->value, 4);
        msg_ss += print_hex(*rm.sender_msg_id, 4);
        msg_ss += ":";
        msg_ss += rm.data;
        break;
    case 'm':
        msg_ss += print_hex(rm.server_boot_id->value, 2);
        msg_ss += print_hex(rm.session_id->value, 4);
        msg_ss += print_hex(*rm.receiver_msg_id, 4);
        msg_ss += print_hex(*rm.missing_count, 2);
        msg_ss += print_hex(*rm.sender_msg_id, 4);
        msg_ss += ":";
        msg_ss += rm.data;
        break;
    case 'p':
        msg_ss += print_hex(rm.server_boot_id->value, 2);
        msg_ss += print_hex(rm.session_id->value, 4);
        msg_ss += print_hex(*rm.receiver_msg_id, 4);
        msg_ss += print_hex(*rm.missing_count, 2);
        break;
    case 'd':
        msg_ss += print_hex(rm.server_boot_id->value, 2);
        msg_ss += print_hex(rm.session_id->value, 4);
        break;
    default:
        assert(false);
    }
    return msg_ss;
}

//////////////////////////////////////////////////////////////////////

// static int64_t diff_abs(int64_t a, int64_t b)
// {
//     return a <= b ? (b - a) : (a - b);
// }

// Find `x >= 0` such that `x % m = y` and `|reference-x|` is minimal.
//
// Note that `0 <= |reference-x| <= m/2`.
//
int64_t find_nearest_mod(int64_t y, int64_t reference, int64_t m, char const *debug)
{
    // std::cout << "find_nearest_mod"
    //           << " " << debug
    //           << " y=" << y
    //           << " reference=" << reference
    //           << " m=" << msg_id_end
    //           << std::endl;
    assert(y < m);
    if (reference <= y)
        return y;
    int64_t x = ((reference - y) / m) * m + y;
    assert(x % m == y);
    assert(x <= reference);
    assert(reference - m < x);

    if (x < reference - m / 2)
        return x + m;
    else
        return x;
}

int64_t convert_msg_id_to_abs(int64_t msg_id_rem, int64_t my_id_reference, char const *debug)
{
    assert(msg_id_rem < msg_id_end);
    int64_t x = find_nearest_mod(msg_id_rem, my_id_reference, msg_id_end, debug);
    // std::cout << "find_nearest_mod"
    //           << " " << debug
    //           << " y=" << msg_id_rem
    //           << " reference=" << my_id_reference
    //           << " m=" << msg_id_end
    //           << " x=" << x
    //           << std::endl;
    return x;
    // int64_t base = (my_id_reference / msg_id_end) * msg_id_end - msg_id_end;
    // bool negative = false;
    // if (base < 0)
    // {
    //     negative = true;
    //     base += msg_id_end;
    // }
    // for (int i = 0; i <= 2; ++i)
    // {
    //     int64_t msg_id = base + msg_id_rem;
    //     // printf("Trying %d: diff = %d\n", msg_id, diff_abs(msg_id, my_id_reference));
    //     if (diff_abs(msg_id, my_id_reference) < msg_id_end / 2)
    //         return msg_id;
    //     base += msg_id_end;
    // }
    // assert(negative);
    // return msg_id_rem;
}

int64_t convert_msg_to_rem(int64_t msg_id)
{
    return msg_id % msg_id_end;
}
