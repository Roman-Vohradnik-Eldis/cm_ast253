#include "cm_debug.hh"
#include "cm_base_functs.hh"
#include "cm_thread.hh"
#include "cm_header_internal.hh"
#include "cm_utils.hh"

#include <fstream>
#include <memory>
#include <mutex>
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>

std::string vformat(char const *fmt, va_list myargs)
{
    char *ptr = NULL;
    int len = vasprintf(&ptr, fmt, myargs);
    std::string result;
    if (len != -1)
    {
	result.assign(ptr, ptr + len);
	free(ptr);
    }
    else
    {
	cms_ns_if_print("global", 1, "Failed to format string: %s", fmt);
    }
    return result;
}

std::string format(char const *fmt, ...)
{
    va_list myargs;
    va_start(myargs, fmt);
    std::string result = vformat(fmt, myargs);
    va_end(myargs);
    return result;
}

void cm_fprint(std::ostream &out, char const *fmt, ...)
{
    va_list myargs;
    va_start(myargs, fmt);
    std::string str = vformat(fmt, myargs);
    va_end(myargs);
    if (!str.empty())
        out << str;
}


static int ref_count;
static CMSMutex *mutex;
static std::map<GThread*, std::string> *thread_names;

// cm_debug_enabled > 0 znamena, ze se nebude pouzivat k vypisovani
// CMSDebug, ale specialni funkce z LibCM. Promenna cm_debug_enabled
// se nastavi pri startu programu pred volani main() pomoci getenv() z
// env promenne CM_DEBUG. Pak se na to nesmi sahat. To asi neni uplne
// prenostitelne, ale melo by to fungovat na kazde rozumne
// implementaci.
static int cm_debug_enabled;

void add_debug_thread(std::string name)
{
    name.resize(8, ' ');
    mutex->Lock();
    thread_names->insert(std::make_pair(g_thread_self(), name));
    mutex->Unlock();
}

// Tady predpokladame, ze getenv bude fungovat pred main
CmDebugInit::CmDebugInit()
{
    if (ref_count++ == 0)
    {
	if (getenv("CM_DEBUG"))
	    cm_debug_enabled = 1;
	mutex = new CMSMutex();
	thread_names = new std::map<GThread*, std::string>();
        // Jinak je potreba pockat, az uzivatel zavola g_thread_init().
        if (GLIB_CHECK_VERSION (2, 32, 0))
        {
            add_debug_thread("main");
        }
    }	
}

CmDebugInit::~CmDebugInit()
{
    if (--ref_count == 0)
    {
	delete mutex;
	delete thread_names;
    }
}

static char const *thread_self_name()
{
    std::map<GThread*, std::string>::iterator it = thread_names->find(g_thread_self());
    char const *name = "?     ";
    if (it != thread_names->end())
        name = it->second.c_str();
    return name;
}

static std::string time_to_str(int64_t time)
{
    struct timeval tv;
    tv.tv_sec = time / 1000000L;
    tv.tv_usec = time % 1000000L;
    struct tm *tm_result = localtime(&tv.tv_sec);
    char buffer[128];
    strftime(buffer, sizeof buffer, "%H:%M:%S", tm_result);
    char buffer2[256];
    snprintf(buffer2, sizeof buffer2, "%s.%03ld.%ld", buffer, tv.tv_usec / 1000, (tv.tv_usec % 1000) / 100);
    return buffer2;
}

void cm_debug_printf(char const *fmt, ...)
{
    va_list myargs;
    va_start(myargs, fmt);
    mutex->Lock();
    printf("%s %s: ", thread_self_name(), time_to_str(get_current_time_usec()).c_str());
    vprintf(fmt, myargs);
    printf("\n");
    fflush(stdout);
    mutex->Unlock();
    va_end(myargs);
}

size_t worker_debug_optimal_len = 10;

// mutex je zamceny
static std::string worker_debug_make_ns(std::string ns, std::string ns2)
{
    std::string s = ns;
    if (!ns2.empty())
    {
        s += "/";
        s += ns2;
    }
    if (s.size() < worker_debug_optimal_len)
	s.resize(worker_debug_optimal_len, ' ');
    if (s.size() > worker_debug_optimal_len)
        worker_debug_optimal_len += 5;
    return s;
}

int64_t libcm_get_thread_time_usec()
{
    struct timespec ts;
    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts) == -1)
	return -1;
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

int64_t get_current_time_usec()
{
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
	return -1;
    return ts.tv_sec * 1000000LL + ts.tv_nsec / 1000LL;
    // struct timeval tv;
    // gettimeofday(&tv, NULL);
    // return (tv.tv_sec * 1000L * 1000L) + (uint64_t)tv.tv_usec;
}


void cm_debug_print(char const *ns, std::string ns2, int level,
		     char const *filename, char const *function, int line,
		     char const *fmt, ...)
{
    char const *kind = "";
    switch (level)
    {
    case 1: kind = "ERROR: "; break;
    case 2: kind = "WARN:  "; break;
    case 3: kind = "INFO:  "; break;
    }
    if (cm_debug_enabled)
    {
	va_list myargs;
	va_start(myargs, fmt);
	mutex->Lock();
	printf("%s %s: %s: %s",
	       thread_self_name(),
	       time_to_str(get_current_time_usec()).c_str(),
	       worker_debug_make_ns(ns, ns2).c_str(),
	       kind);
	vprintf(fmt, myargs);
	printf("\n");
	fflush(stdout); // Je tohle vubec dobre delat?
	mutex->Unlock();
	va_end(myargs);
    }
    else
    {
	va_list myargs;
	va_start(myargs, fmt);
	std::string msg = vformat(fmt, myargs);
	va_end(myargs);
	cms_debug_print_text(NULL, 0, filename, function, line, ns, level, "%s", msg.c_str());
    }
}

unsigned cm_debug_max_get_level(char const *ns)
{
    return cms_debug_max_get_level(ns);
}

std::shared_ptr<SyncedFile> open_shared_file(std::string const &name)
{
    static std::mutex mutex;
    static std::map<std::string, std::shared_ptr<SyncedFile>> files;

    std::unique_lock<std::mutex> lock(mutex);
    auto it = files.find(name);
    if (it == files.end())
    {
        libCM_CreateDirectoryFromPath(name.c_str());
        std::unique_ptr<std::ofstream> ptr(new std::ofstream(name));
        auto f = std::make_shared<SyncedFile>(std::move(ptr));
        it = files.insert({name, f}).first;
    }
    return it->second;
}
