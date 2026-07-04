#ifndef CM_UTILS_HH_
#define CM_UTILS_HH_

#include "cm_header_internal.hh"
#include <assert.h>
#include <sstream>
#include "cm_view.hh"

#define CHECK(cond, fmt, ...) do \
    { if (!(cond)) { fprintf(stderr, fmt "\n", ##__VA_ARGS__); throw std::runtime_error("Runtime error"); } } while (0)

#define ASSERT(cond, fmt, ...) do \
    { if (!(cond)) { fprintf(stderr, fmt "\n", ##__VA_ARGS__); throw std::logic_error("Logic error"); } } while (0)

template <typename It>
It find_first_by_name(It b, It e, std::string const &name)
{
    It it;
    for (it = b; it != e; ++it)
        if (it->getName() == name)
            break;
    return it;
}

template <typename It>
It find_first_by_variable_name(It b, It e, std::string const &name)
{
    It it;
    for (it = b; it != e; ++it)
        if (it->variable.getName() == name)
            break;
    return it;
}

template <typename List>
void find_and_erase_by_name(List &l, std::string const &name)
{
    for (typename List::iterator it = l.begin(); it != l.end(); ++it)
    {
        if (it->getName() == name)
        {
            l.erase(it);
            break;
        }
    }
}

template <typename T>
std::string to_short_string(T const &x, size_t limit)
{
    std::stringstream ss;
    ss << x;
    std::string s = ss.str();
    if (s.size() > limit)
    {
        s.resize(limit);
        s += "...";
    }
    return s;
}

template <typename T>
std::string to_string(const T &x)
{
    std::stringstream ss;
    ss << x;
    return ss.str();
}

template <typename Map, typename T>
bool contains(Map const &m, T const &x)
{
    return m.find(x) != m.end();
}

template <typename Map>
inline std::set<typename Map::key_type> map_to_set(Map const &map)
{
    std::set<typename Map::key_type> result;
    for (typename Map::value_type const &p : map)
        result.insert(p.first);
    return result;
}

template <typename Map, typename T>
typename Map::mapped_type const *map_find(Map const &m, T const &key)
{
    typename Map::const_iterator it = m.find(key);
    return it == m.end() ? NULL : &it->second;
}

template <typename Map, typename T>
typename Map::mapped_type *map_find(Map &m, T const &key)
{
    typename Map::iterator it = m.find(key);
    return it == m.end() ? NULL : &it->second;
}

template <typename Map, typename T>
bool map_contains_value(Map &m, T const &value)
{
    for (typename Map::iterator it = m.begin(); it != m.end(); ++it)
    {
        if (it->second == value)
            return true;
    }
    return false;
}

// inline bool contains(std::string const &s, char c)
// {
//     return s.find(c) != std::string::npos;
// }

template <typename Map, typename Key>
typename Map::mapped_type &insert_or_replace(Map &m, Key const &k, typename Map::mapped_type const &val)
{
    std::pair<typename Map::iterator, bool> result = m.insert(std::make_pair(k, val));
    if (result.second)
    {
        // Was inserted
        return result.first->second;
    }
    else
    {
        // Already exists
	return result.first->second = val;
    }
}

template <typename T>
std::string cm_tostring(T const &x)
{
    std::stringstream ss;
    ss << x;
    assert(ss);
    return ss.str();
}

template <typename Iterator>
std::string cm_concat(Iterator begin, Iterator end, std::string separ = "")
{
    std::stringstream ss;
    bool first = true;
    for (Iterator it = begin; it != end; ++it)
    {
	if (first)
	    first = false;
	else
	    ss << separ;
	ss << *it;
    }
    return ss.str();
}

template <typename Container>
std::string cm_concat(Container const &c, std::string separ = "")
{
    std::stringstream ss;
    bool first = true;
    for (typename Container::iterator it = c.begin(); it != c.end(); ++it)
    {
	if (first)
	    first = false;
	else
	    ss << separ;
	ss << *it;
    }
    return ss.str();
}

template <typename Iterator>
std::string cm_concat_first(Iterator begin, Iterator end, std::string separ = "")
{
    std::stringstream ss;
    bool first = true;
    for (Iterator it = begin; it != end; ++it)
    {
	if (first)
	    first = false;
	else
	    ss << separ;
	ss << it->first;
    }
    return ss.str();
}

//////////////////////////////////////////////////////////////////////

template <typename Iterator>
struct Concat
{
    Iterator begin, end;
    std::string separ;
};

template <typename Iterator>
std::ostream &operator<<(std::ostream &o, Concat<Iterator> const &c)
{
    bool first = true;
    for (Iterator i = c.begin; i != c.end; ++i)
    {
        if (first)
            first = false;
        else
            o << c.separ;
        o << *i;
        
    }
    return o;
}

template <typename Container>
Concat<typename Container::const_iterator> concat(Container const &c, std::string separ)
{
    return Concat<typename Container::const_iterator>({c.begin(), c.end(), separ});
}

template <typename It>
Concat<It> concat(It b, It e, std::string separ)
{
    return Concat<It>({b, e, separ});
}

inline std::string cm_trim_spaces(std::string const &str)
{
    auto b = str.begin();
    while (b != str.end() && std::isspace(*b))
        ++b;

    auto e = str.end();
    while (e != b && isspace(*(e-1)))
        --e;
    return std::string(b, e);
}

// Pozor: delims nesmi obsahovat mezery
// inline std::vector<std::string> cm_split_by_any_delim_trim(std::string const &input, std::string delims)
// {
//     std::vector<std::string> result;
//     std::string::const_iterator start = input.begin();
//     for (std::string::const_iterator it = input.begin(); it != input.end(); ++it)
//     {
//         if (delims.find(*it) != std::string::npos)
//         {
//             result.push_back(cm_trim_spaces(std::string(start, it)));
//             start = it + 1;
//         }
//     }
//     result.push_back(cm_trim_spaces(std::string(start, input.end())));
//     return result;
// }

// inline std::vector<std::string> cm_split_by_space(std::string const &input)
// {
//     while (start != input.end() && isspace(*start))
//         ++start;

//     if (start != input.end())
//     {
//         for (;;)
//         {
//             while (start != input.end() && isspace(*start))
//                 ++start;

//             auto it = start;
//             while (it != input.end() && !isspace(*it) && delims.find(*it) == std::string::npos)
//                 ++it;

//             if (it == input.end())
//             {
//                 // Precetli jsme slovo (mozna prazdne) a jsme na konci vstupu.
//                 result.push_back(std::string(start, it));
//                 break;
//             }    
// }

inline std::vector<std::string> cm_split_trim_by_spaces(std::string const &input)
{
    std::vector<std::string> result;
    std::string::const_iterator start = input.begin();

    for (;;)
    {
        while (start != input.end() && isspace(*start))
            ++start;

        if (start == input.end())
            break;
            
        // We are at a non-space character
        auto it = std::find_if(start, input.end(), [](char c){return std::isspace(c);});
        result.push_back(std::string(start, it));

        if (it == input.end())
            break;
        
        start = it;
        ++start;
    }
    return result;
}

// F: T -> std::vector<R>
template <typename T, typename F, typename R = decltype(std::declval<F>()(std::declval<T>()))>
R cm_append_map(F f, std::vector<T> const &input)
{
    R result;
    for (T const &x : input)
    {
        auto v = f(x);
        result.insert(result.end(), v.begin(), v.end());
    }
    return result;
}

// F: T -> R
template <typename T, typename F, typename R = decltype(std::declval<F>()(std::declval<T>()))>
std::vector<R> cm_map(F f, std::vector<T> const &input)
{
    std::vector<R> result;
    result.reserve(input.size());
    for (T const &x : input)
    {
        result.push_back(f(x));
    }
    return result;
}

template <typename Pred>
inline std::vector<std::string> cm_split_trim_by_pred(std::string const &input, Pred pred)
{
    std::vector<std::string> result;
    std::string::const_iterator start = input.begin();

    while (start != input.end() && isspace(*start))
        ++start;

    if (start != input.end())
    {
        for (;;)
        {
            // We are at the end of a non-space character
            auto it = std::find_if(start, input.end(), pred);
            result.push_back(cm_trim_spaces(std::string(start, it)));

            if (it == input.end())
                break;

            start = it;
            ++start;

            // Jsme za delimiterem
            while (start != input.end() && isspace(*start))
                ++start;
        }
    }
    return result;
}

inline std::vector<std::string> cm_split_trim_by_any_delim(std::string const &input, std::string const &delims)
{
    auto p = [&](char c){return delims.find(c) != std::string::npos;};
    return cm_split_trim_by_pred(input, p);
}

inline std::vector<std::string> cm_split_trim_by_delim(std::string const &input, char delim)
{
    auto p = [&](char c){return c == delim;};
    return cm_split_trim_by_pred(input, p);
}

// inline std::vector<std::string> cm_split_by_delim_trim(std::string const &input, char delim_ch)
// {
//     std::string delim;
//     delim.push_back(delim_ch);
//     return cm_split_by_any_delim_trim(input, delim);
// }

inline std::vector<std::string> cm_split_by_delim(std::string const &input, char delim)
{
    std::vector<std::string> result;
    if (input.empty())
        return result;
    
    std::string::const_iterator start = input.begin();
    for (std::string::const_iterator it = input.begin(); it != input.end(); ++it)
    {
        if (*it == delim)
        {
            result.push_back(std::string(start, it));
            start = it + 1;
        }
    }

    // Protoze na vstupu neco je, je `start` bud na zacatku vstupu
    // nebo za delimiterem.
    result.push_back(std::string(start, input.end()));
    return result;
}

// Append file content to given `buffer`. On error, throw std::runtime
// error.
void read_whole_file_to_buffer(std::string const &name, std::vector<uint8_t> &buffer);

// Read file content. On error, throw std::runtime error.
std::vector<uint8_t> read_whole_file(std::string const &name);

// return 16 bytes
std::vector<uint8_t> compute_md5(View<uint8_t> bytes);

// Return base64 representation of md5sum of `bytes`. Since md5sum has
// 16 bytes, the resulting string has 20 characters
std::string compute_md5_string(View<uint8_t> bytes);

// Replace nonprinatable by '?'
std::string replace_nonprintable(View<uint8_t> str);

// Return string of the form of "0D 45 68"
std::string str_to_hex(View<uint8_t> bytes, std::string const &separator = " ");

// For example, value 0x12AB with length 6 is written as "0012AB"
// If length is too small, throw std::runtime_error
std::string number_to_hex_string(unsigned long long value, size_t length);

unsigned long long read_hex_number(View<uint8_t> bytes);

void push_hex_number(std::vector<uint8_t> &v, unsigned long long value, size_t length);

// Read "0012AB" to 0x12AB
// On error, throw std::runtime_error
unsigned long long read_hex_number(View<uint8_t> bytes);

void test_cm_utils();

#endif // CM_UTILS_HH_
