#ifndef CM_COMPAT_FOREACH
#define CM_COMPAT_FOREACH


#if __cplusplus >= 201103L

#define CM_for(decl, container) for (decl : container)

#else

// Omezeni: Kontejner musi byt lvalue

struct AnyIterBase {};

template <typename Iter>
struct AnyIter : AnyIterBase
{
    mutable Iter iter;
    AnyIter(Iter it) : iter(it) {}
    Iter &get() const { return iter; }
};

template <typename Cont>
AnyIter<typename Cont::iterator> make_iter(Cont &c)
{
    return AnyIter<typename Cont::iterator>(c.begin());
}

template <typename Cont>
AnyIter<typename Cont::const_iterator> make_iter(Cont const &c)
{
    return AnyIter<typename Cont::const_iterator>(c.begin());
}

template <typename Cont>
typename Cont::iterator &get_iter(AnyIterBase const &ai, Cont &)
{
    return static_cast<AnyIter<typename Cont::iterator> const&>(ai).get();
}

template <typename Cont>
typename Cont::const_iterator &get_iter(AnyIterBase const &ai, Cont const &)
{
    return static_cast<AnyIter<typename Cont::const_iterator> const&>(ai).get();
}

#define CM_for(decl, container)					\
    for (AnyIterBase const &CM_foreach_ai = make_iter(container);	\
         get_iter(CM_foreach_ai, container) != (container).end();       \
	 ++get_iter(CM_foreach_ai, container))				\
	for (bool CM_foreach_test = true; CM_foreach_test;)		\
	    for (decl = *get_iter(CM_foreach_ai, container); CM_foreach_test; CM_foreach_test = false)

#endif // __cplusplus >= 201103L

#endif // CM_COMPAT_FOREACH
