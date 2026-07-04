#ifndef CM_STRING_BUFFER_
#define CM_STRING_BUFFER_

#include "weak_string.hh"
#include "cm_view.hh"
#include "assert.h"
#include <vector>

// Immutable string with a 'window'.
template <typename T>
class BasicStringBuffer
{
    std::vector<T> buffer;
    size_t offset;

public:
    typedef T* iterator;
    typedef T const* const_iterator;
    typedef T value_type;

    iterator begin() { return buffer.data() + offset; }
    iterator end() { return buffer.data() + buffer.size(); }
    const_iterator begin() const { return buffer.data() + offset; }
    const_iterator end() const { return buffer.data() + buffer.size(); }

    BasicStringBuffer(View<T> s)
        : buffer(s.begin(), s.end()),
          offset(0)
    {
    }

    BasicStringBuffer()
        : offset(0)
    {
    }

    std::string str()
    {
        return std::string(buffer.begin(), buffer.end());
    }

    size_t size() const { return buffer.size() - offset; }

    void append(View<T> what)
    {
        if (buffer.size() + what.size() > buffer.capacity())
        {
            buffer.erase(buffer.begin(), buffer.begin() + offset);
            offset = 0;
        }
        buffer.insert(buffer.end(), what.begin(), what.end());
    }

    // template <typename Iter>
    // void insert(T *where, Iter b, Iter e)
    // {
    //     assert(begin() <= where && where <= end());
    //     size_t pos = where - begin();
    //     assert(pos >= offset);
    //     buffer.insert(buffer.begin() + pos, b, e);
    // }
    
    void advance(size_t n)
    {
        size_t new_offset = offset + n;
        if (new_offset > buffer.size())
            throw std::out_of_range("Advanced past the buffer");
        offset = new_offset;
    }

    void push_back(T const &x)
    {
        buffer.push_back(x);
    }

    T *data()
    {
        return begin();
    }

    T const *data() const
    {
        return begin();
    }
    
    void resize(size_t new_size)
    {
        buffer.resize(offset + new_size);
    }

    void clear()
    {
        buffer.clear();
        offset = 0;
    }

    T const &operator[](size_t n) const { return buffer[n+offset]; }

    T &operator[](size_t n) { return buffer[n+offset]; }
    
    // May throw std::out_of_range
    T const &at(size_t n) const { return buffer.at(n+offset); } 

    // std::string str() const { return std::string(begin(), end()); }

    bool empty() const { return offset >= buffer.size(); }
};

typedef BasicStringBuffer<char> StringBuffer;
typedef BasicStringBuffer<uint8_t> ByteBuffer;

#endif // CM_STRING_BUFFER_
