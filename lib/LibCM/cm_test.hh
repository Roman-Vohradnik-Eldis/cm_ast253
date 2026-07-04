#ifndef CM_TEST_HH_
#define CM_TEST_HH_

#include <cstdio>
#include <iostream>
#include <string>

#include "cm_utils.hh"

void inc_n_tests(bool passed);
void print_test_result(bool passed,
                       char const *label,
                       char const *a,
                       char const *b,
                       char const *file,
                       int line,
                       char const *msg);
int print_test_result();
void cm_test_set_abort_on_fail(bool val);


template <typename T>
std::string cm_test_to_string(T const &what)
{
    return to_string(what);
}


template <typename T>
std::string cm_test_to_string(std::set<T> const &what)
{
    return cm_concat(what.begin(), what.end(), ",");
}

template <typename T>
std::string cm_test_to_string(std::vector<T> const &what)
{
    return cm_concat(what.begin(), what.end(), ",");
}

struct Test_eq {template <typename T, typename U> bool operator()(T const &x, U const &y) { return x == y; }};
struct Test_le {template <typename T, typename U> bool operator()(T const &x, U const &y) { return x < y; } };
struct Test_ge {template <typename T, typename U> bool operator()(T const &x, U const &y) { return x > y; } };
struct Test_leq {template <typename T, typename U> bool operator()(T const &x, U const &y) { return x <= y; } };
struct Test_geq {template <typename T, typename U> bool operator()(T const &x, U const &y) { return x >= y; } };


template <typename T, typename U, typename Op>
void check(Op op, T const &value, U const &expected_value, char const *label, char const *file, int line, char const *msg)
{
    bool passed = op(value, expected_value);
    inc_n_tests(passed);
    print_test_result(passed,
                      label,
                      cm_test_to_string(value).c_str(),
                      cm_test_to_string(expected_value).c_str(),
                      file,
                      line,
                      msg);
}

template <typename T>
void check_true(T const &value, char const *label, char const *file, int line, char const *msg)
{
    bool passed = (bool)value;
    inc_n_tests(passed);
    print_test_result(passed,
                      label,
                      cm_test_to_string(value).c_str(),
                      cm_test_to_string(true).c_str(),
                      file,
                      line,
                      msg);
}

template <typename U>
void handle_test_exception(std::runtime_error &e,
                           U const &expected_value, char const *label, char const *file, int line, char const *msg)
{
    inc_n_tests(false);
    print_test_result(false,
                      label,
                      "<exception>",
                      cm_test_to_string(expected_value).c_str(),
                      file,
                      line,
                      msg);
}

template <typename X1, typename X2>
void cm_assert_eq(X1 const &x1, X2 const &x2, char const *x1_str, char const *x2_str,
                  char const *file, int line)
{
    if (!(x1 == x2))                                                 
    {                                                                   
        std::cerr << file << ":" << line << ": Assertion failed: "
                  << x1_str << " == " << x2_str << ":\n" 
                  << x1 << "\n"                                         
                  << x2 << std::endl;
        abort();
    }                                                                   
}

#define CM_ASSERT_EQ(x1, x2) cm_assert_eq(x1, x2, #x1, #x2, __FILE__, __LINE__)

#define CM_TEST_EQ(expr, expected)  check(Test_eq(),  expr, expected, #expr " == " #expected, __FILE__, __LINE__, nullptr)
#define CM_TEST_GEQ(expr, expected) check(Test_geq(), expr, expected, #expr " >= " #expected, __FILE__, __LINE__, nullptr)
#define CM_TEST_LEQ(expr, expected) check(Test_leq(), expr, expected, #expr " <= " #expected, __FILE__, __LINE__, nullptr)
#define CM_TEST_LE(expr, expected)  check(Test_le(),  expr, expected, #expr " < " #expected, __FILE__, __LINE__, nullptr)
#define CM_TEST_GE(expr, expected)  check(Test_ge(),  expr, expected, #expr " > " #expected, __FILE__, __LINE__, nullptr)

#define CM_TEST_APPLY(f, X...)  check_true(f(X), #f "(" #X ")", __FILE__, __LINE__, nullptr)

#define CM_TEST_NOCATCH(expr, expected) check(expr, expected, #expr " == " #expected, __FILE__, __LINE__, nullptr)

#define CM_TEST CM_TEST_EQ


#define CM_TEST_CATCH(expr, expected)                                   \
    do {                                                                \
         auto expected_value = expected;                                 \
         try {                                                           \
             auto expr_value = expr;                                     \
             check(Test_eq(), expr_value, expected_value, #expr " == " #expected, __FILE__, __LINE__, nullptr); \
         }                                                               \
         catch (std::runtime_error &e)                                   \
         {                                                               \
             handle_test_exception(e, expected_value, #expr " == " #expected, __FILE__, __LINE__, e.what()); \
         }                                                               \
     } while (0)

// #define CM_TEST CM_TEST_NOCATCH

#endif // CM_TEST_HH_
