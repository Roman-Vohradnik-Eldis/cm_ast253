#ifndef WEAK_STRING_HH_
#define WEAK_STRING_HH_

#include <string>
#include <iterator>
#include <cstring>
#include <iostream>
#include <cstdint>

// WeakString je ukazatel na začatek pole charů plus jeho délka. Paměť, na
// kterou tento ukazatel ukazuje, samotný WeakString NEvlastní. Typicky
// WeakString ukazuje na nějakou část std::stringu. Je zamýšlen tak, aby se
// předával hodnotou.

// WeakString nemusí být ukončen nulovým znakem. Jde převést na std::string
// pomocí metody str(), která provede kopii paměti, na kterou WeakString
// ukazuje.

// TODO: Asi by zde klidne mohl byt i operator na implcitini konverzi na std::string.

class WeakString
{
    const char *str_begin;

    // Delka. Bez nuloveho znaku, pokud je s nim vubec string
    // ukoncenej, coz nemusi.
    size_t len;

public:
    typedef char const *iterator;
    typedef char const *const_iterator;

    // Nevim, jestli tady nema byt jeste reference a const_reference,
    // zatim mi to nikde nevyhazovalo error.
    typedef char value_type;
    
    WeakString()
        : str_begin(0), len(0)
    {}
    WeakString(const std::string &s)
        : str_begin(s.c_str()), len(s.size())
    {}

    WeakString(const char *s, const char *e)
        : str_begin(s), len(e - s)
    {}

    WeakString(const uint8_t *s, const uint8_t *e)
        : str_begin((const char *)s), len(e - s)
    {}

    WeakString(std::string::const_iterator begin, std::string::const_iterator end)
        : str_begin (begin == end ? 0 : &*begin),
          len(end-begin)
    {
    }

    WeakString(const char *s)
        : str_begin(s), len(strlen(s))
    {}

    char operator[](size_t i) const
    {
        return str_begin[i];
    }

    char at(size_t i) const
    {
        if (i >= len)
            throw std::out_of_range("Out of bounds");
        return str_begin[i];
    }

    size_t size() const
    {
        return len;
    }

    WeakString advance(size_t n)
    {
        if (n > len)
            throw std::out_of_range("Out of bounds");
        str_begin += n;
        len -= n;
        return *this;
    }

    char front()
    {
	return str_begin[0];
    }

    bool empty() const
    {
        return len == 0;
    }

    std::string str() const
    {
        return std::string(str_begin, str_begin+len);
    }

    operator std::string() const
    {
	return str();
    }

    const char *begin() const { return str_begin; }
    const char *end() const { return str_begin + len; }

    friend bool operator==(WeakString a, WeakString b)
    {
        return a.len == b.len && memcmp(a.str_begin, b.str_begin, a.len) == 0;
    }

    friend bool operator!=(WeakString a, WeakString b)
    {
        return !(a == b);
    }
};

inline std::ostream &operator<<(std::ostream &o, WeakString w)
{
    std::copy(w.begin(), w.begin() + w.size(), std::ostream_iterator<char>(o));
    return o;
}

inline bool string_contains(WeakString s, char c)
{
    for (size_t i = 0; i < s.size(); ++i)
        if (c == s[i])
            return true;
    return false;
}

inline std::string operator+(std::string a, WeakString b)
{
    a.append(b.begin(), b.end());
    return a;
}

inline std::string operator+(WeakString w, std::string const &b)
{
    std::string a(w.begin(), w.end());
    a.append(b.begin(), b.end());
    return a;
}

struct WeakStringInput
{
    WeakString input;

    explicit WeakStringInput(WeakString i) : input(i) {}
    
    char operator[](size_t n) { return input[n]; }
    size_t request(size_t n) { return input.size(); }
    void advance(size_t n) { input.advance(n); }
    WeakString access_range(size_t n) { return WeakString(input.begin(), input.begin() + n); }
    WeakString debug_string() { return input; }
};

class IOStreamInput
{
    std::istream &in;
    std::string buffer;

public:
    explicit IOStreamInput(std::istream &i) : in(i) {}
    
    char operator[](size_t n)
    {
        return buffer.at(n);
    }
    size_t request(size_t n)
    {
        while (buffer.size() < n && in)
        {
            char c;
            if (in.get(c))
                buffer.push_back(c);
        }
        return buffer.size();
    }
    size_t size() const
    {
        return buffer.size();
    }
    void advance(size_t n)
    {
        buffer.erase(0, n);
    }
    WeakString access_range(size_t n)
    {
        return WeakString(buffer.begin(), buffer.begin() + n);
    }

    WeakString debug_string()
    {
        return buffer;
    }
};

#endif // WEAK_STRING_HH_
