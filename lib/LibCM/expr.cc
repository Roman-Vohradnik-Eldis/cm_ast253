#include "expr.hh"
#include "blob.hh"
#include "cm_string.hh"
#include <assert.h>
#include <math.h>

// WRITING
//////////////////////////////////////////////////////////////////////

bool is_symbol_string(std::string const &s)
{
    if (s.size() > 0 && s.size() <= 100 && is_first_char_of_a_symbol(s[0]))
    {
	for (size_t i = 1; i < s.size(); ++i)
	    if (!is_inner_char_of_a_symbol(s[i]))
		return false;
	return true;
    }
    return false;
}

std::ostream &write_string(std::ostream &o, const std::string &s)
{
    // printf("write_string: input >>%s<<\n", s.c_str());
    // printf("write_string: escaped >>%s<<\n", escape_special_chars(s).c_str());
    if (is_symbol_string(s))
    {
	o << s;
    }
    else
    {
	o << "\"";
	o << escape_special_chars(s);
	o << "\"";
    }
    return o;
}

// char unsigned_byte_to_hexchar(unsigned c)
// {
//     static char table[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
//     return table[c];
// }

void write_blob_value(std::ostream &o, Blob const &b)
{
    o << binary_serialize(b);
}

void write_hex_double(std::ostream &o, double d)
{
    char buffer[128];
    int len = snprintf(buffer, sizeof buffer, "%a", d);
    assert(len >= 0 && (size_t)len < sizeof buffer - 1);
    o << buffer;
}

std::ostream &write_impl(std::ostream &o, const Expr &e, int expand_level, int indent, bool first, bool use_hexfloat);

// Specialnost:
//
// 1. Specialni hodnoty `nan`, `inf` musime vypsat se znamenkem, jinak
// by se interpretovaly jako string.
//
// 2. U NaN ignorujeme NaN hodnotu.
//
// 3. I kdyz je cislo cele, musime vypsat desetinnou carku, jinak se
// interpretuje jako integer.
//
// Poznamka:
//
// Presnost se pouzije podle nastaveni ostreamu.
//
void expr_write_double(std::ostream &o, double x, bool use_hexfloat)
{
    if (isnan(x))
    {
        if (signbit(x))
            o << "-nan";
        else
            o << "+nan";
    }
    else if (x == INFINITY)
    {
        o << "+inf";
    }
    else if (x == -INFINITY)
    {
        o << "-inf";
    }
    else
    {
        if (use_hexfloat)
        {
            write_hex_double(o, x);
        }
        else
        {
            double int_part;
            double frac = modf(x, &int_part);
            if (frac == 0.0L)
                o << format("%.0lf.0", x);
            else
                o << x;
        }
    }
}

struct ExprWrite
{
    std::ostream &o;
    int expand_level;
    int indent;
    bool use_hexfloat;
    
    void operator()(int64_t x) { o << x; }
    void operator()(double x)
    {
        expr_write_double(o, x, use_hexfloat);
    }
    void operator()(std::string const &s) { write_string(o, s); }
    void operator()(Blob const &b)
    {
        o.put('[');
        write_blob_value(o, b);
        o.put(']');
    }
    void operator()(Expr const &l)
    {
        o << "(";
        bool first = true;
        for (Expr const &x : l)
        {
            if (first)
            {
                first = false;
                write_impl(o, x, expand_level - 1, indent + 1, true, use_hexfloat);
            }
            else
            {
                if (expand_level > 0)
                    o << "\n";
                else
                    o << " ";
                write_impl(o, x, expand_level - 1, indent + 1, false, use_hexfloat);
            }
        }
        o << ")";
    }
    template <typename T> void operator()(T const &) = delete;
};

std::ostream &write_impl(std::ostream &o, const Expr &e, int expand_level, int indent, bool first, bool use_hexfloat)
{
    if (expand_level >= 0 && !first)
        for (int i = 0; i < indent; ++i)
            o << " ";
    e.switch_type(ExprWrite{o, expand_level, indent, use_hexfloat});
    return o;
}

std::ostream &write(std::ostream &o, const Expr &e, int expand_level, bool use_hexfloat)
{
    return write_impl(o, e, expand_level, 0, false, use_hexfloat);
}

// std::ostream &write_list(std::ostream &o, const Expr &e)
// {
//     if (e.type() == TPair)
//     {
//         o << "(";
//         write(o, e.car());
//         for (Expr it = e.cdr(); it.type() == TPair; it = it.cdr())
//         {
//             o << "\n ";
//             write(o, it.car());
//         }
//         o << ")";
//     }
//     else
//     {
//         write(o, e);
//     }
//     return o;
// }

std::ostream &pretty_print(std::ostream &o, Expr const &e)
{
    if (e.is_list())
    {
        o << "(";
        bool first = true;
        for (Expr const &x : e)
        {
            write(o, x);
            if (first)
                first = false;
            else
                o << "\n ";
        }
        o << ")";
    }
    else
    {
        write(o, e);
    }
    return o;
}


std::ostream &display(std::ostream &o, Expr const &e)
{
    if (e.is_list())
    {
        o << "(";
        bool first = true;
        for (Expr const &x : e)
        {
            if (first)
                first = false;
            else
                o << " ";
            display(o, x);
        }
        o << ")";
    }
    else if (e.is_string())
    {
        o << e.to_string();
    }
    else
    {
        o << e;
    }
    return o;
}

std::string display_to_string(Expr const &e)
{
    std::stringstream ss;
    if (!display(ss, e))
        throw std::runtime_error("Cannot display Expr");
    return ss.str();
}

