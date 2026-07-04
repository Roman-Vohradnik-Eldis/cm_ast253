#ifndef EXPR_PARSER_HH_
#define EXPR_PARSER_HH_

#include "weak_string.hh"
#include "expr.hh"

enum class ExprParserState
{
    Normal,
    InsideString,
    InsideBlob,
    InsideNumber,
    InsideSymbol,
    InsideComment,
};

// ExprParser reads Exprs (delimited by whitespace) from a string.
class ExprParser
{
    // Successfully parsed expressions
    std::vector<Expr> result;

    // Current state (partially parsed expression)
    ExprParserState parser_state = ExprParserState::Normal;

    std::vector<std::vector<Expr>> stack;

    // Used when insinde string, blob, or number
    bool last_char_was_escape = false;
    std::string current_token;

public:

    // Continue parsing by consuming given string. On error,
    // std::runtime_error is thrown and the state of the `ExprParser`
    // is reset.
    void add_string(WeakString str);

    // Continue parsing by consuming given string. On error,
    // std::runtime_error is thrown and the state of the `ExprParser`
    // is reset.
    //
    // Stop parsing after `n_exprs` expressions.
    size_t add_string_with_limit(WeakString str, size_t n_exprs);

    // Tell the parser that the end-of-input was reached, and so the
    // current expression shall end.
    //
    // After `add_delimiter()`, the state of parser is the same as if
    // it was `reset()`.  can be used again.
    //
    // On error, std::runtime_error is thrown and the state of the
    // `ExprParser` is reset.
    void add_eof();

    // Parser is /clean/ when it was reset (or just created), or it
    // has completed an expression and has not begun parsing a new one
    // yet.
    bool is_clean() const;

    // Discard all input data and state. To completely reinitialize
    // ExprParser, you should also call `get_result()`.
    void reset();

    // Pop all parsed expressions. If there are no expressions, empty
    // vector is returned.
    std::vector<Expr> get_result();

private:
    void parse_number(WeakString &s);
    void parse_string(WeakString &s);
    void parse_symbol(WeakString &s);
    void parse_comment(WeakString &s);
    void parse_blob(WeakString &s);
    void parse_normal(WeakString &s);
    void parse_main(WeakString &s);

    void parse_number_eof();
    void parse_string_eof();
    void parse_symbol_eof();
    void parse_comment_eof();
    void parse_blob_eof();
    void parse_normal_eof();
    void parse_main_eof();
    
    void yield_expr(Expr e);
};

// Parse single expression. There may be spaces before and after the
// expression. On error, it throws `std::runtime_error`.
//
// If you plan to parse many expression, you should create and reuse
// `ExprParser` to save allocations.
Expr parse_one(WeakString w);

// Parse expressions separated by white space.
// On error, it throws `std::runtime_error`.

// If you plan to parse many expression, you should create and reuse
// `ExprParser` to save allocations.
std::vector<Expr> parse_all(WeakString w);

#endif // EXPR_PARSER_HH_
