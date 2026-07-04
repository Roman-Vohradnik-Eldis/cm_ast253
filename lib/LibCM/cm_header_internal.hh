#ifndef CM_HEADER_INTERNAL_H
#define CM_HEADER_INTERNAL_H

// #include ".config.h"

// #ifndef false
// //#define false ((bool)0)
// #endif

// #ifndef true
// //#define true ((bool)1)
// #endif

// #ifndef USE_LIB_MUPARSERX
// #define USE_LIB_MUPARSERX 0
// #endif // USE_LIB_MUPARSERX

// V C++11 jsou stdint a inttypes obsazeny, a vyhazovalo by to warning
#if __cplusplus < 201103L
#define __STDC_FORMAT_MACROS 1
#define __STDC_LIMIT_MACROS 1
#endif

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>

#include <stdint.h>
#include <inttypes.h>

#include <string>
#include <map>
#include <set>

#include <memory>
#include <functional>

#include "cm_compat_function.hh"

namespace cm
{
    
    template <typename T>
    class scoped_ptr
    {
        T *ptr;
    public:
        ~scoped_ptr() { delete ptr; }
        scoped_ptr(T *p) : ptr(p) {}
        scoped_ptr() : ptr(0) {}
        scoped_ptr &operator=(T *p)
        {
            delete ptr;
            ptr = p;
            return *this;
        }
        operator bool() { return ptr; }
        T *get() { return ptr; }
        T &operator*() { return *ptr; }
        T const &operator*() const { return *ptr; }
        T *operator->() { return ptr; }
        T const *operator->() const { return ptr; }

    private:
        scoped_ptr(scoped_ptr const &other);
        scoped_ptr &operator=(scoped_ptr const &other);
    };
}

#include "cm_compat_foreach.hh"

// #if __cplusplus < 201103L

namespace cm
{
    
    template <typename T>
    T move(T const &other)
    {
        return other;
    }
}

// #endif

namespace cm
{

    // template <typename T>
    // function
    
    template <typename Class, typename R, typename F>
    struct bind_front_impl
    {
        F f;
        Class *c;

        bind_front_impl(F a_f, Class *a_c) : f(a_f), c(a_c) {}
        R operator()() { return (c->*f)(); }
        template <typename A0>
        R operator()(A0 a0) { return (c->*f)(a0); }

    };

    template <typename Class, typename R>
    bind_front_impl<Class, R, R (Class::*)()> bind_front(R (Class::*f)(), Class *c)
    {
        return bind_front_impl<Class, R, R (Class::*)()>(f, c);
    }

    template <typename Class, typename R, typename A0>
    bind_front_impl<Class, R, R (Class::*)(A0)> bind_front(R (Class::*f)(A0 a0), Class *c)
    {
        return bind_front_impl<Class, R, R (Class::*)(A0)>(f, c);
    }
};

#include "cm_header_cms.hh"

#endif // CM_HEADER_INTERNAL_H
