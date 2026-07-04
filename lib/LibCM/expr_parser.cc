
#include <cctype>
#include <stdlib.h>
#include "expr_parser.hh"
#include "expr.hh"
#include "cm_string.hh"
#include "blob.hh"
#include <stdexcept>


std::string truncate(WeakString s, size_t max_len)
{
    if (s.size() < max_len+3)
        return s;
    else
        return std::string(s.begin(), s.begin() + max_len) + "...";
}

void print_debug_msg(char const *label, WeakString s, WeakString current_token)
{
    // printf("Parse %s: input='%s', current_token='%s'\n", label,
    //        truncate(s, 20).c_str(), current_token.str().c_str());
}

bool is_delimiter(char c)
{
    char delim[] = " \n;,\t()[]\"";
    return strchr(delim, c);
};

// Na vstupu je posloupnost znaku [-+][0-9]+
inline Expr parse_int(WeakString s)
{
    assert(!s.empty());
    bool negative = false;
    if (s[0] == '-')
    {
        negative = true;
        s.advance(1);
    }
    else if (s[0] == '+')
    {
        s.advance(1);
    }
    else
    {
        assert(isdigit(s[0]));
    }
    int64_t x = 0;
    for (size_t i = 0; i < s.size(); ++i)
    {
        x *= 10;
        x += s[i] - '0';
    }
    return negative ? -x : x;
}

// Vstup muze byt i spatne. Bereme ho jako validni, prave kdyz projde
// pres `strtod`.
Expr parse_double(WeakString s)
{
    std::string str(s.begin(), s.end());
    char *endptr;
    double x = strtod(str.c_str(), &endptr);
    if (endptr != str.c_str() + s.size())
        throw std::runtime_error(format("Parse double failed: on string %s", str.c_str()));        
    return x;
}

void ExprParser::yield_expr(Expr e)
{
    if (stack.empty())
    {
        result.push_back(std::move(e));
    }
    else
    {
        stack.back().push_back(std::move(e));
    }
}

bool is_valid_integer(std::string const &s)
{
    if (s.empty())
        return false;
    if (!(s[0] == '+' || s[0] == '-' || isdigit(s[0])))
        return false;
    for (size_t i = 1; i < s.size(); ++i)
        if (!isdigit(s[i]))
            return false;
    return true;
}

// `s` je neprazdny, zacina znakem [-+.0-9]. Dale muze obsahovat
// cokoliv.
//
Expr make_number(std::string const &s)
{
    if (is_valid_integer(s))
        return parse_int(s);
    else
        return parse_double(s);
}

// Jsme za prvnim znakem cisla
//
// Cteme az po delimiter
void ExprParser::parse_number(WeakString &s)
{
    print_debug_msg(__FUNCTION__, s, current_token);
    assert(parser_state == ExprParserState::InsideNumber);    
    while (!s.empty())
    {
        if (is_delimiter(s[0]))
        {
            yield_expr(make_number(current_token));
            current_token.clear();
            parser_state = ExprParserState::Normal;
            break;
        }
        else
        {
            current_token.push_back(s[0]);
            s.advance(1);
        }
    }
}

// Jsme za strednikem
void ExprParser::parse_comment(WeakString &s)
{
    print_debug_msg(__FUNCTION__, s, current_token);
    assert(parser_state == ExprParserState::InsideComment);
    assert(current_token.empty());
    while (!s.empty())
    {
        if (s[0] == '\n')
        {
            if (!s.empty())
                s.advance(1);
            parser_state = ExprParserState::Normal;
            break;
        }
        else
        {
            s.advance(1);
        }
    }
}

// Jsme za otviraci zavorkou
void ExprParser::parse_blob(WeakString &s)
{
    print_debug_msg(__FUNCTION__, s, current_token);
    assert(parser_state == ExprParserState::InsideBlob);
    while (!s.empty())
    {
        if (s[0] == ']')
        {
            yield_expr(make_blob(binary_deserialize(current_token)));
            current_token.clear();
            s.advance(1);
            parser_state = ExprParserState::Normal;
            return;
        }
        else
        {
            current_token.push_back(s[0]);
            s.advance(1);
        }
    }
}

// // Jsme za otviraci uvozovkou
// // Tehnhle kod neumi escapovani
// void ExprParser::parse_string(WeakString &s)
// {
//     for (;;)
//     {
//         char const *pos = (char const *)memchr(s.begin(), '"', s.size());
//         if (pos == NULL)
//         {
//             current_token.append(s.begin(), s.end());
//             s.advance(s.size());
//             break;
//         }
//         if (pos != s.begin() && pos[-1] == '\\')
//         {
//             ++pos;
//             size_t len = pos - s.begin();
//             s.advance(len);
//         }
//         else
//         {
//             ++pos;
//             size_t len = pos - s.begin();
//             current_token.append(s.begin(), pos);
//             yield_expr(make_string(unescape_special_chars(current_token, '"')));
//             s.advance(len);
//             current_token.clear();
//             parser_state = ExprParserState::Normal;
//             break;      
//         }
//     }
// }

// Jsme za otviraci uvozovkou
void ExprParser::parse_string(WeakString &s)
{
    print_debug_msg(__FUNCTION__, s, current_token);
    assert(parser_state == ExprParserState::InsideString);
    size_t len = 0;
    while (len != s.size())
    {
        char c = s[len];
        if (last_char_was_escape)
        {
            last_char_was_escape = false;
        }
        else
        {
            if (c == '"')
            {
                ++len;  // include closing quote
                current_token.append(s.begin(), s.begin() + len);
                yield_expr(make_string(unescape_special_chars(current_token, '"')));

                s.advance(len);
                current_token.clear();
                parser_state = ExprParserState::Normal;
                return;
            }
            else if (c == '\\')
            {
                last_char_was_escape = true;
            }
        }
        ++len;
    }
    current_token.append(s.begin(), s.begin() + len);
    s.advance(len);
}

char to_printable(char c)
{
    return isgraph(c) ? c : '?';
}

// Jsme za prvnim znakem symbolu
void ExprParser::parse_symbol(WeakString &s)
{
    print_debug_msg(__FUNCTION__, s, current_token);
    assert(parser_state == ExprParserState::InsideSymbol);
    size_t len = 0;
    while (len < s.size())
    {
        if (is_delimiter(s[len]))
        {
            current_token.append(s.begin(), s.begin() + len);
            s.advance(len);
            yield_expr(make_string(current_token));
            current_token.clear();
            parser_state = ExprParserState::Normal;
            return;
        }
        else
        {
            if (!is_inner_char_of_a_symbol(s[len]))
                throw std::runtime_error(format("Invalid character %c (ascii %u) in a symbol",
                                                to_printable(s[len]),
                                                (unsigned)s[len]));
            ++len;
        }
    }
    current_token.append(s.begin(), s.begin() + len);
    s.advance(len);
}

void ExprParser::parse_normal(WeakString &s)
{
    print_debug_msg(__FUNCTION__, s, current_token);
    assert(parser_state == ExprParserState::Normal);
    assert(current_token.empty());
    while (!s.empty())
    {
        if (s[0] == ' ' || s[0] == '\t' || s[0] == '\n')
        {
            s.advance(1);
        }
        else
        {
            break;
            
        }
    }
    if (!s.empty())
    {
        if (s[0] == ';')
        {
            parser_state = ExprParserState::InsideComment;
            s.advance(1);
        }
        else if (s[0] == '(')
        {
            stack.push_back({});
            s.advance(1);
        }
        else if (s[0] == ')')
        {
            s.advance(1);
            if (stack.empty())
                throw std::runtime_error("Unexpected end of list ')'");
            std::vector<Expr> l = std::move(stack.back());
            stack.pop_back();
            yield_expr(std::move(l));
        }
        else if ((s[0] >= '0' && s[0] <= '9') || s[0] == '-' || s[0] == '+')
        {
            parser_state = ExprParserState::InsideNumber;
            // current_token.push_back(s[0]);
            // s.advance(1);
            //parse_number(s, eof);
        }
        else if (s[0] == '[')
        {
            parser_state = ExprParserState::InsideBlob;
            s.advance(1);
            //parse_blob(s, eof);
        }
        else if (s[0] == '"')
        {
            parser_state = ExprParserState::InsideString;
            s.advance(1);
            //parse_string(s, eof);
        }
        else if (is_first_char_of_a_symbol(s[0]))
        {
            parser_state = ExprParserState::InsideSymbol;
            // current_token.push_back(s[0]);
            // s.advance(1);
            //parse_symbol(s, eof);
        }
        else
        {
            throw std::runtime_error(format("Unknown symbol '%c' (ascii %d)",
                                            to_printable(s[0]),
                                            (unsigned)s[0]));
        }
    }
}

void ExprParser::parse_normal_eof()
{

}

void ExprParser::parse_comment_eof()
{
    parser_state = ExprParserState::Normal;
}

void ExprParser::parse_number_eof()
{
    yield_expr(make_number(current_token));
    current_token.clear();
    parser_state = ExprParserState::Normal;
}

void ExprParser::parse_string_eof()
{
    throw std::runtime_error("Unterminated string");
}

void ExprParser::parse_blob_eof()
{
    throw std::runtime_error("Unterminated blob");
}

void ExprParser::parse_symbol_eof()
{
    yield_expr(make_string(current_token));
    current_token.clear();
    parser_state = ExprParserState::Normal;
}

void ExprParser::add_eof()
{
    try
    {
        parse_main_eof();
        assert(parser_state == ExprParserState::Normal);
        if (!stack.empty())
            throw std::runtime_error("Unterminated list");
    }
    catch (std::exception &e)
    {
        reset();
        throw;
    }
}

size_t ExprParser::add_string_with_limit(WeakString str, size_t limit)
{
    size_t prev_size = result.size();
    size_t input_size = str.size();
    try
    {
        while (!str.empty() && (result.size() - prev_size) < limit)
            parse_main(str);
        return input_size - str.size();
    }
    catch (std::exception &e)
    {
        reset();
        throw;
    }
}

void ExprParser::add_string(WeakString str)
{
    try
    {
        while (!str.empty())
            parse_main(str);
    }
    catch (std::exception &e)
    {
        reset();
        throw;
    }
}

void ExprParser::reset()
{
    stack.clear();
    parser_state = ExprParserState::Normal;
    current_token.clear();
}

// In each case, we read at most one Expression.
void ExprParser::parse_main(WeakString &str)
{
    print_debug_msg(__FUNCTION__, str, current_token);
    switch (parser_state)
    {
    case ExprParserState::Normal: parse_normal(str); break;
    case ExprParserState::InsideString: parse_string(str); break;
    case ExprParserState::InsideBlob: parse_blob(str); break;
    case ExprParserState::InsideNumber: parse_number(str); break;
    case ExprParserState::InsideSymbol: parse_symbol(str); break;
    case ExprParserState::InsideComment: parse_comment(str); break;
    }
}

// In each case, we read at most one Expression.
void ExprParser::parse_main_eof()
{
    switch (parser_state)
    {
    case ExprParserState::Normal: parse_normal_eof(); break;
    case ExprParserState::InsideString: parse_string_eof(); break;
    case ExprParserState::InsideBlob: parse_blob_eof(); break;
    case ExprParserState::InsideNumber: parse_number_eof(); break;
    case ExprParserState::InsideSymbol: parse_symbol_eof(); break;
    case ExprParserState::InsideComment: parse_comment_eof(); break;
    }
}

std::vector<Expr> ExprParser::get_result()
{
    std::vector<Expr> v;
    std::swap(v, result);
    return v;
}

bool ExprParser::is_clean() const
{
    return parser_state == ExprParserState::Normal && stack.empty();
}

Expr parse_one(WeakString s)
{
    ExprParser parser;
    parser.add_string(s);
    parser.add_eof();
    std::vector<Expr> v = parser.get_result();
    if (v.size() == 0)
        throw std::runtime_error("parse_one: Expected expression");
    return v[0];
}

std::vector<Expr> parse_all(WeakString s)
{
    ExprParser parser;
    parser.add_string(s);
    parser.add_eof();
    return parser.get_result();
}
