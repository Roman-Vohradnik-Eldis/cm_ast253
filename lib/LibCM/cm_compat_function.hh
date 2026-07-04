#ifndef CM_COMPAT_FUNCTION_HH_
#define CM_COMPAT_FUNCTION_HH_

// Tohle je implementace std::function pro C++98

namespace cm
{

    template <typename T>
    struct copy_ptr
    {
        T *ptr;
        copy_ptr(T *p) : ptr(p) {}
        ~copy_ptr() { delete ptr; }
        copy_ptr(copy_ptr const &other) : ptr(other.ptr ? other.ptr->clone() : 0) {}
        copy_ptr &operator=(copy_ptr const &other)
        {
            delete ptr;
            if (other.ptr)
                ptr = other.ptr->clone();
            return *this;
        }
        operator bool() { return ptr; }
        T &operator*() { return *ptr; }
        T const &operator*() const { return *ptr; }
        T *operator->() { return ptr; }
        T const *operator->() const { return ptr; }
    };

//////////////////////////////////////////////////////////////////////

    template <typename F>
    struct function_trait {};

    template <typename R>
    struct function_trait<R ()>
    {
        typedef R return_type;
        enum { arity = 0 };
        static R call(R (*f)()) { return f(); };
    };

    template <typename R, typename A0>
    struct function_trait<R (A0 a0)>
    {
        typedef R return_type;
        typedef A0 a0_type;
        enum { arity = 1 };
        static R call(R (*f)(A0 a0), a0_type a0) { return f(a0); };
    };

    template <typename R, typename A0, typename A1>
    struct function_trait<R (A0 a0, A1 a1)>
    {
        typedef R return_type;
        typedef A0 a0_type;
        typedef A1 a1_type;
        enum { arity = 2 };
        static R call(R (*f)(A0 a0, A1 a1), a0_type a0, a1_type a1) { return f(a0, a1); };
    };

    template <typename Class, typename R>
    struct function_trait<R (Class::*)()>
    {
        typedef R return_type;
        typedef Class *a0_type;
        enum { arity = 1 };
        static R call(R (Class::*f)(), a0_type a0) { return (a0->*f)(); };
    };

    template <typename Class, typename R, typename A0>
    struct function_trait<R (Class::*)(A0 a0)>
    {
        typedef R return_type;
        typedef Class *a0_type;
        typedef A0 a1_type;
        enum { arity = 2 };
        static R call(R (Class::*f)(a1_type a1), a0_type a0, a1_type a1) { return (a0->*f)(a1); };
    };

    template <typename R>
    struct function_trait<R (*)()> : function_trait<R()> {};

    template <typename R, typename A0>
    struct function_trait<R (*)(A0 a0)> : function_trait<R(A0 a0)> {};

    template <typename R, typename A0, typename A1>
    struct function_trait<R (*)(A0 a0, A1 a1)> : function_trait<R(A0 a0, A1 a1)> {};

//////////////////////////////////////////////////////////////////////

    template <typename F, int arity = function_trait<F>::arity>
    struct function_base {};

    template <typename F>
    struct function_base<F, 0>
    {
        virtual ~function_base() {}
        virtual function_base<F, 0> *clone() = 0;
        virtual typename function_trait<F>::return_type operator()() = 0;
    };

    template <typename F>
    struct function_base<F, 1>
    {
        virtual ~function_base() {}
        virtual function_base<F, 1> *clone() = 0;
        virtual typename function_trait<F>::return_type operator()(typename function_trait<F>::a0_type) = 0;
    };

    template <typename F>
    struct function_base<F, 2>
    {
        virtual ~function_base() {}
        virtual function_base<F, 2> *clone() = 0;
        virtual typename function_trait<F>::return_type operator()(typename function_trait<F>::a0_type, typename function_trait<F>::a1_type) = 0;
    };

//////////////////////////////////////////////////////////////////////

    template <typename T, typename F, int arity = function_trait<F>::arity>
    struct function_type {};

    template <typename T, typename F>
    struct function_type<T, F, 0> : function_base<F, 0>
    {
        T fun;
        function_type(T f) : fun(f) {}
        virtual ~function_type() {}
        function_base<F,0> *clone() { return new function_type<T,F,0>(*this); };
        typename function_trait<F>::return_type operator()()
        { return fun(); }
    };

    template <typename T, typename F>
    struct function_type<T, F, 1> : function_base<F, 1>
    {
        T fun;
        function_type(T f) : fun(f) {}
        virtual ~function_type() {}        
        function_base<F, 1> *clone() { return new function_type<T,F,1>(*this); };
        typename function_trait<F>::return_type operator()(typename function_trait<F>::a0_type a0)
        { return fun(a0); }    
    };

    template <typename T, typename F>
    struct function_type<T, F, 2> : function_base<F, 2>
    {
        T fun;
        function_type(T f) : fun(f) {}
        virtual ~function_type() {}
        function_base<F, 2> *clone() { return new function_type<T,F,2>(*this); };
        typename function_trait<F>::return_type operator()(typename function_trait<F>::a0_type a0, typename function_trait<F>::a1_type a1)
        { return fun(a0, a1); }
    };

//////////////////////////////////////////////////////////////////////

    template <typename F, int arity = function_trait<F>::arity>
    struct function {};

    template <typename F>
    struct function_trait<function<F> > : function_trait<F> {};

    struct nothing {};
    
    template <typename F>
    struct function<F,0>
    {
        copy_ptr<function_base<F> > base;
        function() : base(0) {}
        function(nothing) : base(0) {}
        operator bool() { return base; }
        template <typename T> function(T f) : base(new function_type<T, F>(f)) {}
        typename function_trait<F>::return_type operator()() { return base->operator()(); }
    };

    template <typename F>
    struct function<F,1>
    {
        copy_ptr<function_base<F> > base;
        function() : base(0) {}
        function(nothing) : base(0) {}
        operator bool() { return base; }
        template <typename T> function(T f) : base(new function_type<T, F>(f)) {}
        typename function_trait<F>::return_type operator()(typename function_trait<F>::a0_type a0) { return base->operator()(a0); }
    };

    template <typename F>
    struct function<F,2>
    {
        copy_ptr<function_base<F> > base;
        function() : base(0) {}
        function(nothing) : base(0) {}
        operator bool() { return base; }
        template <typename T> function(T f) : base(new function_type<T, F>(f)) {}
        typename function_trait<F>::return_type operator()(typename function_trait<F>::a0_type a0, typename function_trait<F>::a1_type a1) { return base->operator()(a0, a1); }
    };
    
} // namespace cm

#endif // CM_COMPAT_FUNCTION_HH_
