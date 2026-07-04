#ifndef CM_MAYBE_HH_
#define CM_MAYBE_HH_

#include <vector>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <assert.h>

struct Nothing
{};

#if __cplusplus >= 201103L

template <typename T>
class Maybe
{
    union
    {
        T value;
    };
    bool present;

public:
    Maybe(Nothing) : present(false) {}
    Maybe() : present(false) {}

    Maybe(T const &x) : present(true)
    {
        new (&value) T(x);
    }

    Maybe(T &&other)
        : present(true)
    {
        new (&value) T(std::move(other));
    }

    // TODO:  noexcept (std::is_nothrow_move_constructible<T>::value)
    Maybe(Maybe<T> &&other)
        : present(other.present)
    {
        if (other.present)
	{
            new (&value) T(std::move(other.value));
	    other.present = false;
	}
    }

    Maybe(Maybe<T> const &other)
        : present(other.present)
    {
        if (other.present)
            new (&value) T(other.value);
    }

    Maybe<T> &operator=(T &&other)
    {
        return *this = Maybe<T>(std::move(other));
    }

    Maybe<T> &operator=(Maybe<T> &&other)
    {
        if (present)
        {
            if (other.present)
                value = std::move(other.value); // may throw
            else
            {
                present = false;
                value.~T();
            }
        }
        else
        {
            if (other.present)
            {
                new (&value) T(std::move(other.value)); // may throw
                present = true;
            }
            else
            {
                // nothing
            }
        }
        return *this;
    }

    ~Maybe()
    {
        if (present)
            value.~T();
    }
    Maybe<T> &operator=(Maybe<T> const &other)
    {
        if (present)
        {
            if (other.present)
                value = other.value; // may throw
            else
            {
                present = false;
                value.~T();
            }
        }
        else
        {
            if (other.present)
            {
                new (&value) T(other.value); // may throw
                present = true;
            }
            else
            {
                // nothing
            }
        }
        return *this;
    }

    T get_or(T x) const
    {
	if (present)
	    return value;
	return x;
    }

    T get_or_throw(std::string const &msg)
    {
        if (present)
            return value;
        throw std::runtime_error(msg);
    }
    
    T pop()
    {
        assert_present();
        T x = std::move(value);
        present = false;
        value.~T();
        return x;
    }

    Maybe<T> move()
    {
        if (!present)
            return Nothing();
        T x = std::move(value);
        present = false;
        value.~T();
        return x;
    }

    template <typename F, typename R = typename std::result_of<F(T const &)>::type>
    Maybe<R> map(F f)
    {
        if (this->has_value())
            return Maybe<R>(f(this->operator*()));
        else
            return Nothing();
    }

    explicit operator bool() const { return present; }
    bool has_value() const { return present; }
    T &operator*()
    {
        assert_present();
        return value;
    }
    const T &operator*() const
    {
        assert_present();
        return value;
    }
    T *operator->()
    {
        assert_present();
        return &value;
    }
    const T *operator->() const
    {
        assert_present();
        return &value;
    }
    void assert_present() const
    {
        if (!present)
        {
            assert(false);
            throw std::out_of_range("Maybe is empty");
        }
    }
};

#else

template <typename T>
class Maybe
{
    std::vector<T> singleton;

public:
    Maybe(Nothing) {}
    Maybe() {}
    Maybe(T const &x) { singleton.push_back(x); }

    operator bool() const { return !singleton.empty(); }
    T &operator*() { return singleton.at(0); }
    T const &operator*() const { return singleton.at(0); }
    T *operator->() { return &singleton.at(0); }
    T const *operator->() const { return &singleton.at(0); }
};

#endif

// #else

// template <typename T>
// struct Maybe
// {
//     Maybe() {}
//     Maybe(const T &x) { vec.push_back(x); }
//     Maybe(Nothing) {}

//     CV_EXPLICIT operator bool() const { return has_value(); }

//     T pop()
//     {
//         T x = value();
//         vec.clear();
//         return x;
//     }

//     T &operator*() { return value(); }
//     const T &operator*() const { return value(); }
//     T *operator->() { return &value(); }
//     const T *operator->() const { return &value(); }

//     bool has_value() const { return !vec.empty(); }

// private:
//     std::vector<T> vec;
//     T &value()
//     {
//         if (vec.empty())
//         {
//             std::cerr << "dereferencing Nothing\n";
//             abort();
//         }
//         return vec.at(0);
//     }
//     const T &value() const
//     {
//         if (vec.empty())
//         {
//             std::cerr << "dereferencing Nothing\n";
//             abort();
//         }
//         return vec.at(0);
//     }
// };

// #endif // CXXNEW

// template <typename T, typename ErrorT>
// struct Maybe2
// {
//     Maybe2() {}
//     Maybe2(const T &x) { vec.push_back(x); }
//     Maybe2(Nothing, ErrorT e) : error(e) {}
//     CV_EXPLICIT operator bool() const { return has_value(); }

//     T pop()
//     {
//         T x = value();
//         vec.clear();
//         return x;
//     }

//     ErrorT get_error()
//     {
//         if (has_value())
//             throw std::runtime_error("not an error");
//         return error;
//     }

//     T &operator*() { return value(); }
//     const T &operator*() const { return value(); }
//     T *operator->() { return &value(); }
//     const T *operator->() const { return &value(); }

// private:
//     ErrorT error;
//     std::vector<T> vec;
//     T &value()
//     {
//         if (vec.empty())
//         {
//             std::cerr << "dereferencing Nothing\n";
//             abort();
//         }
//         return vec.at(0);

//     }
//     const T &value() const
//     {
//         if (vec.empty())
//         {
//             std::cerr << "dereferencing Nothing\n";
//             abort();
//         }
//         return vec.at(0);
//     }
//     bool has_value() const { return !vec.empty(); }
// };

// template <typename T, typename ErrorT>
// Maybe2<T, ErrorT> gen_error(ErrorT e)
// {
//     return Maybe2<T, ErrorT>(Nothing(), e);
// }

template <typename T>
inline bool operator==(Maybe<T> const &a, Maybe<T> const &b)
{
    return (!a && !b) || (a && b && *a == *b);
}

#endif // CM_MAYBE_HH_
