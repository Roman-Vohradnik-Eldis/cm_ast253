#ifndef VIEW_HH_
#define VIEW_HH_

#include <vector>
#include <stdexcept>
#include <string>
#include <cstdint>

template <typename T>
class View
{
    T const *begin_ptr;
    T const *end_ptr;

public:
    typedef T const value_type;
    typedef T const *iterator;
    typedef T const *const_iterator;
    
    // Create empty view
    View()
        : begin_ptr(0), end_ptr(0)
    {}
    
    View(T const *b, T const *e)
        : begin_ptr(b), end_ptr(e)
    {
        if (end_ptr < begin_ptr)
            throw std::logic_error("View::View: invalid range");
    }

    View(std::vector<T> const &v)
        : begin_ptr(v.data()), end_ptr(begin_ptr + v.size())
    {
    }

    View(std::basic_string<T> const &s)
        : begin_ptr(s.c_str()), end_ptr(begin_ptr + s.size())
    {
    }
    
    bool empty() const { return begin_ptr == end_ptr; }
    size_t size() const { return end_ptr - begin_ptr; }
    T const *data() const { return begin_ptr; }
    T const *begin() const { return begin_ptr; }
    T const *end() const { return end_ptr; }
    T const &operator[](size_t i) const { return begin_ptr[i]; }

    void advance(size_t i)
    {
        if (begin_ptr + i > end_ptr)
            throw std::logic_error("View::advance: out of range");
        begin_ptr += i;
    }
    
    T const &at(size_t i)
    {
        if (begin_ptr + i >= end_ptr)
            throw std::logic_error("View::at: out of range");
        return begin_ptr[i];
    }
};

template <typename T>
bool operator==(View<T> a, View<T> b)
{
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

template <typename T>
bool operator!=(View<T> a, View<T> b)
{
    return a.size() != b.size() || !std::equal(a.begin(), a.end(), b.begin());
}

// Creation

template <typename T>
View<T> view(T const *begin, T const *end)
{
    return View<T>(begin, end);
}

template <typename T>
View<T> view(std::vector<T> const &v)
{
    return View<T>(v);
}

template <typename T>
View<T> view(std::basic_string<T> const &v)
{
    return View<T>(v);
}

template <typename T>
std::vector<T> make_vector(View<T> view)
{
    return std::vector<T>(view.begin(), view.end());
}

// Bytes

template <typename T>
View<uint8_t> view_bytes(View<T> view)
{
    return View<uint8_t>((uint8_t const *)view.begin(), (uint8_t const *)view.end());
}

template <typename T>
View<uint8_t> view_bytes(T const *begin, T const *end)
{
    return View<uint8_t>((uint8_t const *)begin, (uint8_t const *)end);
}

template <typename T>
View<uint8_t> view_bytes(std::basic_string<T> const &str)
{
    return view_bytes(str.c_str(), str.c_str() + str.size());
}

template <typename T>
View<uint8_t> view_bytes(std::vector<T> const &str)
{
    return view_bytes(str.data(), str.data() + str.size());
}

#endif // VIEW_HH_
