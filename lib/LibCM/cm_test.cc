#include "cm_test.hh"

#include <atomic>

#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KNRM "\x1B[0m"

static std::atomic<int> n_passed_tests;
static std::atomic<int> n_failed_tests;
static std::atomic<bool> do_abort_on_fail{false};

void cm_test_set_abort_on_fail(bool val)
{
    do_abort_on_fail = val;
}

void inc_n_tests(bool passed)
{
    if (passed)
        ++n_passed_tests;
    else
        ++n_failed_tests;
}

void print_test_result(bool passed,
                       char const *label,
                       char const *a,
                       char const *b,
                       char const *file,
                       int line,
                       char const *msg)
{
    if (passed)
    {
        fprintf(stdout, "%s:%d: " KGRN "Passed:" KNRM " %s:"
                "\nResult: >>%s<<\n%s%s",
                file,
                line,
                label,
                a,
                msg ? msg : "",
                msg ? "\n" : "");
    }
    else
    {
        fprintf(stdout, "%s:%d: " KRED "Failed:" KNRM " %s:"
                "\nResult:   >>%s<<"
                "\nExpected: >>%s<<\n%s%s",
                file,
                line,
                label,
                a,
                b,
                msg ? msg : "",
                msg ? "\n" : "");
    }
    fflush(stdout);
    if (!passed && do_abort_on_fail)
        exit(1);
    // if (!passed && strcmp(getenv("CM_TEST"),  "fatal") == 0)
    //     abort();
}

int print_test_result()
{
    int n_passed = n_passed_tests;
    int n_failed = n_failed_tests;
    bool passed = n_failed_tests == 0;
    printf("%sTEST COMPLETED: %s:" KNRM " Passed=%d, Failed=%d\n",
           passed ? KGRN : KRED,
           passed ? "SUCCESS" : "FAILURE",
           n_passed,
           n_failed);
    return !passed;
}
