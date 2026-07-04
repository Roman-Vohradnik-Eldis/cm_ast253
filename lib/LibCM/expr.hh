#ifndef EXPR_HH_H_
#define EXPR_HH_H_

#include "weak_string.hh"
#include "cm_maybe.hh"
#include "cm_debug.hh"

#include <math.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <deque>
#include <atomic>
#include <cctype>
#include <assert.h>

enum BasicExprType
{
    TNil = 0,
    TInt = 1,
    TPair = 2,
    TExprString = 3,
    TBlob = 4,
    TDouble = 6,
    TVector = 7,
};

inline std::string to_str(BasicExprType t)
{
    return
        ( t    == TNil) ?  "Nil" :
        ( t    == TInt) ?  "Int" :
        ( t    == TPair ||  t    == TVector) ?  "NonEmptyList" :
        ( t    == TExprString) ?  "String" :
        ( t    == TBlob) ?  "Blob" :
        ( t    == TDouble) ?  "Double" : "Unknown";
}

struct Pair;

template <typename T>
struct ReferenceCounted
{
    ReferenceCounted(T v) : value(std::move(v)), ref_count(1) {}
    T value;
    std::atomic<int> ref_count;
};

typedef std::vector<uint8_t> Blob;

class Expr
{
    BasicExprType _type;
    size_t vector_from = 0;
    union ExprValue
    {
        int64_t int_val;
        double double_val;
        ReferenceCounted<Pair> *pair_val;
        ReferenceCounted<std::string> *string_val;
        ReferenceCounted<Blob> *blob_val;
        ReferenceCounted<std::vector<Expr>> *vector_val;
    } value;

public:

    // Konvence: Iterator, ktery ukazuje na konec Expr, musi mit head == nullptr.
    // Pokud Expr ma typ TNil nebo je to prazdny vektor, tak nastavime head na nullptr.
    // To se dela funkci iterator_normalize.
    // class iterator
    // {
    //     // Either nullptr or a an Expr of type TPair or an non-empty TVector.
    //     Expr *head;

    //     // Used if and only if head is an Expr of type TVector.
    //     // vector_pos < head->vector_value->size()
    //     size_t vector_pos = 0;

    // public:
    //     explicit iterator() : head(nullptr) {}
    //     explicit iterator(Expr *h) : head(h) { iterator_normalize(head, vector_pos); }

    //     using iterator_category = std::forward_iterator_tag;
    //     using value_type = Expr;
    //     using difference_type = void;
    //     using pointer = Expr *;
    //     using reference = Expr &;

    //     iterator &operator++();
    //     bool operator==(iterator const &other) const { return head == other.head; }
    //     bool operator!=(iterator const &other) const { return head != other.head; }
    //     Expr &operator*();
    //     Expr *operator->();
    // };

    class const_iterator
    {
        Expr const *head;
        size_t vector_pos;

    public:
        explicit const_iterator() : head(nullptr), vector_pos(0) {}
        explicit const_iterator(Expr const *h) : head(h), vector_pos(0) { iterator_normalize(head, vector_pos); }

	using iterator_category = std::forward_iterator_tag;
	using value_type = Expr const;
	using difference_type = void;
	using pointer = Expr const *;
	using reference = Expr const &;

        const_iterator &operator++();
        bool operator==(const_iterator const &other) const { return head == other.head && vector_pos == other.vector_pos; }
        bool operator!=(const_iterator const &other) const { return ! (*this == other); }
        Expr const &operator*() const;
        Expr const *operator->() const;
    };

    using iterator = const_iterator;

    template <typename Iter, typename E>
    static Iter begin_impl(E self_ptr)
    {
        switch (self_ptr->_type)
        {
        case TNil: return Iter();
        case TVector: return Iter(self_ptr);
        case TPair: return Iter(self_ptr);
        default: throw std::runtime_error("Expr::begin(): not a list");
        }
    }

    template <typename Iter, typename E>
    static Iter end_impl(E self_ptr)
    {
        if (!self_ptr->is_list())
            throw std::runtime_error("Expr::end(): not a list");
        return Iter();
    }

    iterator begin() { return begin_impl<iterator>(this); }
    const_iterator begin() const { return begin_impl<const_iterator>(this); }

    iterator end() { return end_impl<iterator>(this); }
    const_iterator end() const { return end_impl<const_iterator>(this); }

    std::string type_string() const
    {
        return to_str(_type);
    }
   
    // S implicitníma konverzema pozor. Například nejde napsat `Expr e = 10;',
    // protože překladač neví, jestli to je double nebo int64. Je třeba psát
    // `Expr e = 10L;'.

    // U inicializaci typem TNil neni potreba nastavovat. Zajimavy je,
    // ze gcc hazi warningy, kdyz je value neinicializovany...
    Expr() : _type(TNil) { value.int_val = 0; }
    Expr(int64_t x) : _type(TInt) { value.int_val = x; }
    Expr(double d) : _type(TDouble) { value.double_val = d; }
    Expr(std::string s) : _type(TExprString) { value.string_val = new ReferenceCounted<std::string>(std::move(s)); }
    Expr(const char *s) : _type(TExprString) { value.string_val = new ReferenceCounted<std::string>(s); }
    Expr(std::vector<Expr> vec) : _type(TVector) { value.vector_val = new ReferenceCounted<std::vector<Expr>>(std::move(vec)); }

    Expr(const Expr &);
    Expr &operator=(Expr);

    friend void swap(Expr &first, Expr &second);

    template <typename F>
    auto switch_type(F &&f) const -> decltype(f(0.0))
    {
        switch (_type)
        {
        case TInt: return f(value.int_val);
        case TExprString: return f(value.string_val->value);
        case TBlob: return f(value.blob_val->value);
        case TDouble: return f(value.double_val);
        case TNil:
        case TVector:
        case TPair: return f(*this);
        default: assert(false);
        }
    }
    
    ~Expr();

    Expr(Expr &&arg)
        : _type(arg._type),
          vector_from(arg.vector_from),
          value(arg.value)
    {
        arg._type = TNil;
    }

    // Použití pro seznamy
    Expr &operator[](size_t i);
    Expr const &operator[](size_t i) const;
    Expr &at(size_t pos);
    Expr const &at(size_t pos) const;

    Expr const &car() const;
    Expr cdr() const;

    int64_t to_int() const;
    double to_double() const;
    const std::string &to_string() const;
    Blob const &to_blob() const;
    void to_nil() const;

    size_t size() const;

    bool is_nil() const { return _type == TNil || (_type == TVector && value.vector_val->value.empty()); }
    bool is_int() const { return _type == TInt; }
    bool is_list() const { return _type == TPair || _type == TNil || _type == TVector; }
    bool is_string() const { return _type == TExprString; }
    bool is_blob() const { return _type == TBlob; }
    bool is_double() const { return _type == TDouble; }
    bool is_pair() const { return _type == TPair || (_type == TVector && !value.vector_val->value.empty()); }

    friend Expr nil();
    friend bool is_equal(Expr const &a, Expr const &b, bool handle_nan);
    friend bool operator!=(Expr const &a, Expr const &b);
    friend Expr cons(Expr a, Expr b);
    friend Expr make_string(std::string);
    friend Expr make_blob(const Blob &s);

    size_t ref_count() const;

private:
    void assert_type(WeakString fname, BasicExprType t) const;

    template <typename E>
    static E &expr_at(E &self, size_t i)
    {
        E *head = &self;
        while (head->_type == TPair)
        {
            if (i == 0)
                return head->value.pair_val->value.car;
            --i;
            head = &head->value.pair_val->value.cdr;
        }
        if (head->_type == TVector)
        {
            return head->value.vector_val->value.at(self.vector_from + i);
        }
        else
        {
            throw std::runtime_error("iterator::at: Out of range");
        }
    }

    template <typename E>
    static E &iterator_dereference(E *head, size_t vector_pos)
    {
        // std::cerr << "iterator_dereference\n";
        if (head)
        {
            if (head->_type == TPair)
            {
                return head->value.pair_val->value.car;
            }
            else
            {
                assert(head->_type == TVector);
                return head->value.vector_val->value.at(head->vector_from + vector_pos);
            }
        }
        else
        {
            throw std::runtime_error("iterator::operator*: List is empty");
        }
    }

    template <typename E>
    static void iterator_normalize(E *&head, size_t &vector_pos)
    {
        if (head->_type == TNil)
        {
            // std::cerr << "iterator_normalize nullptr from Nil\n";
            head = nullptr;
            vector_pos = 0;
        }
        else if (head->_type == TVector &&
                 vector_pos + head->vector_from >= head->value.vector_val->value.size())
        {
            // std::cerr << "iterator_normalize nullptr from empty_vector"
            //           << " vector_pos=" << vector_pos
            //           << " vector_from=" << head->vector_from
            //           << " vector_size=" << head->value.vector_val->value.size()
            //           << "\n";
            head = nullptr;
            vector_pos = 0;
        }
    }
    
    template <typename E>
    static void iterator_advance(E *&head, size_t &vector_pos)
    {
        // std::cerr << "iterator_advance\n";
        if (head)
        {
            if (head->_type == TPair)
                head = &head->value.pair_val->value.cdr;
            else
            {
                assert(head->_type == TVector && vector_pos + head->vector_from < head->value.vector_val->value.size());
                ++vector_pos;
            }
            iterator_normalize(head, vector_pos);
        }
        else
        {
            throw std::runtime_error("iterator::operator++: List is empty");
        }
    }
};

struct Pair
{
    Pair(Expr a, Expr b) : car(a), cdr(b) {}
    Expr car;
    Expr cdr; // Must be a TPair, TNil, or TVector
};

inline Expr const &Expr::at(size_t i) const { return expr_at<Expr const>(*this, i); }
inline Expr &Expr::at(size_t i) { return expr_at<Expr>(*this, i); }
inline Expr const &Expr::operator[](size_t i) const { return expr_at<Expr const>(*this, i); }
inline Expr &Expr::operator[](size_t i) { return expr_at<Expr>(*this, i); }

inline size_t Expr::size() const
{
    Expr const *head = this;
    size_t i = 0;
    while (head->_type == TPair)
    {
        ++i;
        head = &head->value.pair_val->value.cdr;
    }
    if (head->_type == TVector)
    {
        assert(vector_from <= head->value.vector_val->value.size());
        return i + head->value.vector_val->value.size() - vector_from;
    }
    else if (head->_type != TNil)
        throw std::runtime_error("Expr::size: Not a list");
    return 0;
}
   
// inline Expr &Expr::iterator::operator*() { return Expr::iterator_dereference(head, vector_pos); }
// inline Expr *Expr::iterator::operator->() { return &Expr::iterator_dereference(head, vector_pos); }
// inline Expr::iterator &Expr::iterator::operator++() { Expr::iterator_advance(head, vector_pos); return *this; }

inline Expr const &Expr::const_iterator::operator*() const { return Expr::iterator_dereference(head, vector_pos); }
inline Expr const *Expr::const_iterator::operator->() const { return &Expr::iterator_dereference(head, vector_pos); }
inline Expr::const_iterator &Expr::const_iterator::operator++() { Expr::iterator_advance(head, vector_pos); return *this; }

inline void swap(Expr &first, Expr &second)
{
    std::swap(first._type, second._type);
    std::swap(first.vector_from, second.vector_from);
    std::swap(first.value, second.value);
}

inline Expr::Expr(const Expr &other)
{
    _type = other._type;
    vector_from = other.vector_from;
    if (_type == TPair)
    {
        value.pair_val = other.value.pair_val;
        ++value.pair_val->ref_count;
    }
    else if (_type == TExprString)
    {
        value.string_val = other.value.string_val;
        ++value.string_val->ref_count;
    }
    else if (_type == TVector)
    {
        value.vector_val = other.value.vector_val;
        ++value.vector_val->ref_count;
    }
    else if (_type == TBlob)
    {
        value.blob_val = other.value.blob_val;
        ++value.blob_val->ref_count;
    }
    else if (_type == TInt)
    {
        value.int_val = other.value.int_val;
    }
    else if (_type == TDouble)
    {
        value.double_val = other.value.double_val;
    }
}

inline Expr::~Expr()
{
    if (_type == TPair)
    {
        if (--value.pair_val->ref_count == 0)
            delete value.pair_val;
    }
    else if (_type == TExprString)
    {
        if (--value.string_val->ref_count == 0)
            delete value.string_val;
    }
    else if (_type == TVector)
    {
        if (--value.vector_val->ref_count == 0)
            delete value.vector_val;
    }
    else if (_type == TBlob)
    {
        if (--value.blob_val->ref_count == 0)
            delete value.blob_val;
    }
}

// Plati, ze pro kazde `x`, `!(NaN == x)`.
//
// Specialne `Nan != Nan`. V nekterch pripadech chceme, aby kdyz
// udelame kopii Expr, aby byly `stejne`. Tak pak pri porovnavani
// pouzivame `handle_nan=true`, kde se toto chovani specialne
// osetruje.
//
inline bool is_double_equal(double a, double b, bool handle_nan)
{
    #ifndef isnan
    // Tady je nakej bug asi v GCC < 6, zalezi, jestli se pouzije cmath nebo math.h
    using std::isnan; 
    using std::signbit;
    #endif
    return (a == b) || (handle_nan && isnan(a) && isnan(b) && signbit(a) == signbit(b));
}

inline bool is_equal(Expr const &a, Expr const &b, bool handle_nan)
{
    switch (a._type)
    {
    case TInt : return b._type == TInt && a.to_int() == b.to_int();
    case TExprString : return b._type == TExprString && a.to_string() == b.to_string();
    case TBlob : return b._type == TBlob && a.to_blob() == b.to_blob();
    case TDouble : return b._type == TDouble && is_double_equal(a.to_double(), b.to_double(), handle_nan);
    case TPair:
    case TVector:
    case TNil:
    {
        if (!b.is_list())
            return false;
        if (a.size() != b.size())
            return false;
        auto b_it = b.begin();
        for (auto a_it = a.begin(); a_it != a.end(); ++a_it)
        {
            if (!(is_equal(*a_it, *b_it, handle_nan)))
                return false;
            ++b_it;
        }
        return true;
    }
    default: assert(false);
    }    
}

inline bool operator==(Expr const &a, Expr const &b)
{
    return is_equal(a, b, false);
}

inline bool operator!=(Expr const &a, Expr const &b)
{
    return !(a == b);
}

inline Expr &Expr::operator=(Expr other)
{
    swap(*this, other);
    return *this;
}

inline Expr nil()
{
    Expr e;
    e._type = TNil;
    return e;
}

inline Expr const &Expr::car() const
{
    if (_type == TPair)
        return value.pair_val->value.car;
    else if (_type == TVector && vector_from < value.vector_val->value.size())
        return value.vector_val->value.at(vector_from);
    throw std::runtime_error("Expr::car: Expected non-empty list");
}

inline Expr Expr::cdr() const
{
    if (_type == TPair)
        return value.pair_val->value.cdr;
    else if (_type == TVector && vector_from < value.vector_val->value.size())
    {
        Expr copy = *this;
        ++copy.vector_from;
        return copy;
    }
    throw std::runtime_error("Expr::cdr: Expected non-empty list");
}

inline int64_t Expr::to_int() const { assert_type("to_int", TInt); return value.int_val; }
inline void Expr::to_nil() const { if (!is_nil()) throw std::runtime_error("Expected nil"); }
inline double Expr::to_double() const { assert_type("to_double", TDouble); return value.double_val; }
inline std::string const &Expr::to_string() const { assert_type("to_string", TExprString); return value.string_val->value; }
inline Blob const &Expr::to_blob() const { assert_type("to_blob", TBlob); return value.blob_val->value; }

inline Expr cons(Expr a, Expr b)
{
    if (b.is_list())
    {
        Expr e;
        e._type = TPair;
        e.value.pair_val = new ReferenceCounted<Pair>(Pair(std::move(a), std::move(b)));
        return e;
    }
    throw std::runtime_error("cons: Second argument is not a list");
}

inline bool is_first_char_of_a_symbol(char c)
{
    return isalpha(c) || c == '_';
}

inline bool is_inner_char_of_a_symbol(char c)
{
    return isalpha(c) || isdigit(c) || c == '_' || c == '-' || c == '?';
}

inline Expr make_string(std::string s)
{
    Expr e;
    e._type = TExprString;
    e.value.string_val = new ReferenceCounted<std::string>(std::move(s));
    return e;
}

inline Expr make_blob(const Blob &s)
{
    Expr e;
    e._type = TBlob;
    e.value.blob_val = new ReferenceCounted<Blob>(s);
    return e;
}

// inline size_t list_length(Expr e)
// {
//     size_t len = 0;
//     for (Expr::iterator it = e.begin(); it != e.end(); ++it)
//         ++len;
//     return len;
// }

// inline Expr deep_copy(Expr const &e)
// {
//     switch (e.type())
//     {
//     case TNil: return e;
//     case TInt: return e;
//     case TPair: return cons(deep_copy(e.car()), deep_copy(e.cdr()));
//     case TExprString: return make_string(e.to_string());
//     case TBlob: return make_blob(e.to_blob());
//     case TDouble: return e;
//     default: abort();
//     }
// }

// Bez C++11 to jinak nejde, a přes operátory to vypadaá ošklivě

template <typename ...Args>
Expr cm_make_list(Args &&...args)
{
    std::vector<Expr> v{std::forward<Args>(args)...};
    return v;
}

// inline Expr list()
// { return nil(); }

// inline Expr list(Expr e0)
// { return cons(e0, nil()); }

// inline Expr list(Expr e0, Expr e1)
// { return cons(e0, list(e1)); }

// inline Expr list(Expr e0, Expr e1, Expr e2)
// { return cons(e0, list(e1, e2)); }

// inline Expr list(Expr e0, Expr e1, Expr e2, Expr e3)
// { return cons(e0, list(e1, e2, e3)); }

// inline Expr list(Expr e0, Expr e1, Expr e2, Expr e3, Expr e4)
// { return cons(e0, list(e1, e2, e3, e4)); }

// inline Expr list(Expr e0, Expr e1, Expr e2, Expr e3, Expr e4, Expr e5)
// { return cons(e0, list(e1, e2, e3, e4, e5)); }

// inline Expr list(Expr e0, Expr e1, Expr e2, Expr e3, Expr e4, Expr e5, Expr e6)
// { return cons(e0, list(e1, e2, e3, e4, e5, e6)); }

// inline Expr list(Expr e0, Expr e1, Expr e2, Expr e3, Expr e4, Expr e5, Expr e6, Expr e7)
// { return cons(e0, list(e1, e2, e3, e4, e5, e6, e7)); }

// inline Expr list(Expr e0, Expr e1, Expr e2, Expr e3, Expr e4, Expr e5, Expr e6, Expr e7, Expr e8)
// { return cons(e0, list(e1, e2, e3, e4, e5, e6, e7, e8)); }

// inline Expr list(Expr e0, Expr e1, Expr e2, Expr e3, Expr e4, Expr e5, Expr e6, Expr e7, Expr e8, Expr e9)
// { return cons(e0, list(e1, e2, e3, e4, e5, e6, e7, e8, e9)); }

// BUILDING EXPRESSIONS
//////////////////////////////////////////////////////////////////////

// struct ExprList
// {
//     std::vector<Expr> list;

//     // ExprList(const Expr &_head, const Expr &_tail)
//     //     : head(_head), tail(_tail), list_size(0)
//     // {}

//     void push_back(Expr e);
//     bool empty() const;
//     size_t size() const { return list_size; }
//     operator Expr() { return Expr(std::move(list)); }
// };

// inline bool ExprList::empty() const
// {
//     return list.empty();
// }

// inline void ExprList::push_back(const Expr &e)
// {
//     list.push_back(e);
//     if (tail.type() == TNil)
//     {
//         head = tail = cons(e, nil());
//         ++list_size;
//     }
//     else if (tail.type() == TPair)
//     {
//         tail.cdr() = cons(e, nil());
//         tail = tail.cdr();
//         ++list_size;
//     }
//     else
//     {
//         throw std::runtime_error("push_back: expected list");
//     }
// }

// WRITING
//////////////////////////////////////////////////////////////////////

// expand_level:
//
// 0: ((x 1) (y 2))
// 1: ((x)
//     (y))
// 2: ((x
//      1)
//     (y
//      2))
// ...
std::ostream &write(std::ostream &o, Expr const &e, int expand_level = 0, bool use_hexfloat = false);

std::ostream &write_list(std::ostream &o, Expr const &e);

inline std::ostream &operator<<(std::ostream &o, Expr const &e)
{
    return write(o, e);
}

std::ostream &pretty_print(std::ostream &o, Expr const &e);

// inline size_t list_length(Expr const &e)
// {
//     return e.size();
// }

// READING
//////////////////////////////////////////////////////////////////////

// inline Expr get_car_and_advance(Expr &e)
// {
//     Expr car = e.car();
//     e = e.cdr();
//     return car;
// }

// inline Expr &operator>>(Expr &e, int64_t &x)
// {
//     x = get_car_and_advance(e).to_int();
//     return e;
// }

// inline Expr &operator>>(Expr &e, bool &x)
// {
//     const std::string &s = get_car_and_advance(e).to_symbol();
//     if (s == "true")
//         x = true;
//     else if (s == "false")
//         x = false;
//     else
//         throw std::runtime_error("not a boolean");
//     return e;
// }

inline Expr to_expr(Expr e)
{
    return e;
}

template <typename It>
inline Expr to_expr(It begin, It end)
{
    std::vector<Expr> v;
    for (It it = begin; it != end; ++it)
        v.push_back(to_expr(*it));
    return v;
}

// template <typename T>
// inline void from_expr(std::vector<T> &dest, const Expr &src)
// {
//     for (Expr i = src; i.type() == TPair; i = i.cdr())
//     {
//         T t;
//         from_expr(t, i.car());
//         dest.push_back(t);
//     }
// }

template <typename T>
T read_from_expr(Expr const &e)
{
    T x;
    from_expr(x, e);
    return x;
}

inline double cast_to_double(Expr const &e)
{
    if (e.is_int())
        return e.to_int();
    if (e.is_double())
        return e.to_double();
    throw std::runtime_error("cast_to_double: expected int or double");
}

inline bool is_numeric(Expr const &e)
{
    return e.is_int() || e.is_double();
}

template <typename T>
T read_expr(Expr const &e);


// Strings-are non-quoted
std::ostream &display(std::ostream &o, Expr const &e);

// Strings-are non-quoted
std::string display_to_string(Expr const &e);

inline void Expr::assert_type(WeakString fname, BasicExprType t) const
{
    if (_type != t)
        throw std::runtime_error(format("%s: expected %s, got %s", fname.str().c_str(), to_str(t).c_str(), to_str(_type).c_str()));
}

inline std::string to_string(Expr const &e)
{
    std::stringstream ss;
    ss << e;
    return ss.str();
}

class ExprVector
{
    Expr value;
    
public:
    explicit ExprVector(Expr v) : value(std::move(v))
    {
        if (!value.is_list())
            throw std::runtime_error("ExprVector: Not a list");
    }
    
    using iterator = Expr::iterator;
    using const_iterator = Expr::iterator;

    iterator begin() { return value.begin(); }
    iterator end() { return value.end(); }
    const_iterator begin() const { return value.begin(); }
    const_iterator end() const { return value.end(); }

    size_t size() const { return value.size(); }
    bool empty() const { return value.is_nil(); }

    Expr const &operator[](size_t i) const { return value[i]; }
    Expr const &at(size_t i) const { return value.at(i); }
};

#endif // EXPR_HH_H_
