#ifndef CM_DEBUG_HH_
#define CM_DEBUG_HH_

#include <memory>
#include <mutex>
#include <ostream>
#include <sys/time.h>
#include <string>

// TIME
//////////////////////////////////////////////////////////////////////

int64_t get_current_time_usec();
int64_t libcm_get_thread_time_usec();

// FORMAT
//////////////////////////////////////////////////////////////////////

// V pripade chyby vypise chybovou hlasku a vrati prazdny string
std::string format(char const *fmt, ...) __attribute__ ((format (printf, 1, 2)));
std::string vformat(char const *fmt, va_list args);

// V pripade chyby vypise chybovou hlasku a nic do out nezapise
void cm_fprint(std::ostream &out, char const *fmt, ...) __attribute__ ((format (printf, 2, 3)));

// DEBUG INITIALIZATION
//////////////////////////////////////////////////////////////////////

// staticky objekt CmDebugInit se konstruuje pred volani main(), pri
// prvni konstrukci to inicializuje vnitrni struktury.
struct CmDebugInit
{
    CmDebugInit();
    ~CmDebugInit();
};

static CmDebugInit cm_debug_init;

// Kdyz vytvorime novy thread, musime z neho zavolat
// add_debug_thread. Ve vypisech z tohoto threadu se bude tohle jmeno
// vypisovat. Pro nove glib je thread vytvarejici cm_debug_init
// pojmenovan jako "main". Thread z cm_thread.hh tohle vola sam.
void add_debug_thread(std::string name);

// DEBUG PRINT
//////////////////////////////////////////////////////////////////////

// Pro pouziti v makrech nize
void cm_debug_print(char const *ns, std::string ns2, int level,
		    char const *file, char const *function, int line,
		    char const *fmt, ...)
    __attribute__ ((format (printf, 7, 8)));

// Level z CMS
unsigned cm_debug_max_get_level(char const *ns);

// Tohle se pouziva jako v CMS, jenom to tomu jde navic pridat ns2,
// kam se typicky dava treba objektu.
#define cms_ns2_if_print(ns, ns2, level, fmt, ...)			\
    do {								\
	if (cm_debug_max_get_level(ns) >= level)			\
	    cm_debug_print(ns, ns2, level,				\
			   __FILE__, __FUNCTION__, __LINE__,		\
			   fmt, ##__VA_ARGS__);				\
    } while (0)

// Jako v CMS
#define cms_ns_if_print(ns,level,fmt,...) \
    cms_ns2_if_print(ns, "", level, fmt, ##__VA_ARGS__)

#define WORKER_DEBUG(ns, level, fmt, ...) \
    cms_ns2_if_print(ns, MyLibCM_name.c_str(), level, fmt, ##__VA_ARGS__)

#define cm_print(...) cms_ns_if_print("main", 3, __VA_ARGS__)
#define debug_printf(...) cms_ns_if_print("main", 4, __VA_ARGS__)

#define cms_ns2_lines(ns, ns2, level, lines)                            \
    do {                                                                \
        if (cm_debug_max_get_level(ns) >= level)                        \
        {                                                               \
            for (std::string const &debug_line : lines)                 \
            {                                                           \
                cm_debug_print(ns, ns2, level,                          \
                               __FILE__, __FUNCTION__, __LINE__,        \
                               "%s", debug_line.c_str());               \
            }                                                           \
        }                                                               \
    } while (0)

#define cms_ns_lines(ns, level, lines) cms_ns2_lines(ns, "", level, lines)

// UTILS
//////////////////////////////////////////////////////////////////////

template <typename F>
class BasicTimer
{
    int64_t time_start;

public:

    BasicTimer()
    {
        reset();
    }

    int64_t elapsed_usec()
    {
        int64_t time_now = F::get_time_usec();
	if (time_now == -1 || time_start == -1)
	    return -1;
        return time_now - time_start;
    }

    int64_t elapsed_msec()
    {
        return elapsed_usec() / 1000L;
    }

    double elapsed_msec_double()
    {
        return (double)elapsed_usec() / 1000.0;
    }

    void reset()
    {
        time_start = F::get_time_usec();
    }
};

struct WallClockTime { static int64_t get_time_usec() { return get_current_time_usec(); } };
struct ThreadTime { static int64_t get_time_usec() { return libcm_get_thread_time_usec(); } };

typedef BasicTimer<WallClockTime> DebugTimer;
typedef BasicTimer<ThreadTime> ThreadTimer;

class ScopedTimer
{
    std::string m_msg;
    DebugTimer t;

public:

    ScopedTimer(std::string msg)
        : m_msg(msg)
    {}

    ~ScopedTimer()
    {
        printf("%s: %.3lf msec\n", m_msg.c_str(), t.elapsed_msec_double());
    }
};

struct TimeAccumulator
{
    int64_t value;

    TimeAccumulator()
    {
        reset();
    }

    int64_t elapsed_usec()
    {
        return value;
    }

    int64_t elapsed_msec()
    {
        return elapsed_usec() / 1000L;
    }

    double elapsed_msec_double()
    {
        return (double)elapsed_usec() / 1000.0;
    }
    void reset()
    {
        value = 0;
    }
};

struct AccumulatingTimer
{
    TimeAccumulator &m_accum;
    ThreadTimer timer;
    bool stopped = false;
    
    AccumulatingTimer(TimeAccumulator &accum) : m_accum(accum) {}
    ~AccumulatingTimer() { if (!stopped) m_accum.value += timer.elapsed_usec(); }
    void stop() { stopped = true; m_accum.value += timer.elapsed_usec(); }
};

#define PROFILE(timer_obj, fmt, ...)                                    \
    do {                                                                \
        cms_ns_if_print("profile", 3, "(%.3lf msec) " fmt, timer_obj.elapsed_msec_double(), ##__VA_ARGS__); \
        timer_obj.reset();                                              \
} while(0)

class NsTimer
{
    ThreadTimer timer;
    char const *ns;
    std::string ns2;
    unsigned level;
    int64_t limit;
    std::string msg;

public:
    NsTimer(char const *ns, std::string ns2, unsigned level, std::string msg, int64_t limit = 1000)
        : ns(ns), ns2(ns2), level(level), limit(limit), msg(msg)
    {}

    ~NsTimer()
    {
        int64_t elapsed_usec = timer.elapsed_usec();
        if (elapsed_usec >= limit)
            cms_ns2_if_print(ns, ns2, level, "%s took %.3f msec", msg.c_str(), elapsed_usec/1000.0);
    }
};

struct SyncedFile
{
    std::mutex mutex;
    std::unique_ptr<std::ostream> out;

    SyncedFile(std::unique_ptr<std::ostream> out) : out(std::move(out)) {}
};

class SyncedWriter
{
    SyncedFile &f;

public:
    SyncedWriter(SyncedFile &f) : f(f)
    {
        f.mutex.lock();
    };

    ~SyncedWriter()
    {
        f.mutex.unlock();
    }

    template <typename T>
    friend SyncedWriter &operator<<(SyncedWriter &w, T const &x)
    {
        (*w.f.out) << x;
        return w;
    }

    void write(char const *data, size_t length)
    {
        f.out->write(data, length);
    }

    void flush()
    {
        f.out->flush();
    }
};

std::shared_ptr<SyncedFile> open_shared_file(std::string const &name);

#endif // CM_DEBUG_HH_
