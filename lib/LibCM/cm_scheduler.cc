#include "cm_header_internal.hh"
#include "cm_thread.hh"
#include "cm_scheduler.hh"
#include "cm_debug.hh"

#if GLIB_CHECK_VERSION (2, 32, 0)
#define MUTEX_TYPE GMutex
#define DEFINE_MUTEX(mutex) static GMutex mutex; // zero initialized: OK
#define LOCK_MUTEX g_mutex_lock
#define UNLOCK_MUTEX g_mutex_unlock
#else
#define MUTEX_TYPE GStaticMutex
#define DEFINE_MUTEX(mutex) static GStaticMutex mutex = G_STATIC_MUTEX_INIT;
#define LOCK_MUTEX g_static_mutex_lock
#define UNLOCK_MUTEX g_static_mutex_unlock
#endif

class GScopedLock
{
    MUTEX_TYPE &mutex;
public:
    GScopedLock(MUTEX_TYPE &m) : mutex(m) { LOCK_MUTEX(&mutex); }
    ~GScopedLock() { UNLOCK_MUTEX(&mutex); }
};

template <typename T>
class Reference
{
    T *ptr;
    MUTEX_TYPE *mutex;
    int *ref_count;

public:
    Reference(T *p, MUTEX_TYPE *m, int *rc)
	: ptr(p), mutex(m), ref_count(rc)
    {
	// We are locked by get_or_create()
	++*ref_count;
    }

    ~Reference()
    {
	GScopedLock lock(*mutex);
	if (--*ref_count == 0)
	    delete ptr;
    }

    Reference(Reference<T> const &other)
	: ptr(other.ptr),
	  mutex(other.mutex),
	  ref_count(other.ref_count)
    {
	GScopedLock lock(*mutex);
	++*ref_count;
    }

    Reference<T> &operator=(Reference<T> const &other)
    {
        GScopedLock lock(*mutex);
	ptr = other.ptr;
	mutex = other.mutex;
	ref_count = other.ref_count;
        ++ref_count;
	return *this;
    }

    T &operator*() { return *ptr; }
    T const &operator*() const { return *ptr; }
    T *operator->() { return ptr; }
    T const *operator->() const { return ptr; }
};

// This function is thread-safe even in C++98
template <typename T>
Reference<T> get_or_create()
{
    static T *singleton;
    DEFINE_MUTEX(mutex);
    static int ref_count; // zero initialized
    GScopedLock lock(mutex);
    if (ref_count == 0)
	singleton = new T();
    return Reference<T>(singleton, &mutex, &ref_count);
}

// static int64_t get_current_time_usec()
// {
//     struct timeval tv;
//     gettimeofday(&tv, NULL);
//     return (tv.tv_sec * 1000L * 1000L) + (uint64_t)tv.tv_usec;
// }

class Scheduler;

template <typename Map, typename K, typename T>
typename Map::mapped_type map_get_or(Map const &m, K const &key, T const &default_val)
{
    auto it = m.find(key);
    if (it == m.end())
        return default_val;
    else
        return it->second;
}

int64_t get_time_diff(int64_t time)
{
    int64_t time_now = get_current_time_usec();
    if (time < time_now)
        return 0;
    else
        return time - time_now;
}

class SchedulerThread
{
    struct ScheduledTask
    {
	void *source;
	int64_t when;
	bool is_primary_task;
	cm::function<void()> func;

        ScheduledTask(void *s, int64_t w, bool p, cm::function<void()> f)
            : source(s), when(w), is_primary_task(p), func(f) {}
    };

    struct HasSource
    {
        void *source;
        HasSource(void *s) : source(s) {}
        bool operator()(ScheduledTask const &st)
        {
            return st.source == source;
        }
    };
    
    std::list<ScheduledTask> scheduled_tasks;
    bool stopped;
    void *current_source;
    std::map<void *, std::string> source_names;
    CMSMutex mutex;
    CMSCondition cond;
    
    Thread t;

public:
    
    SchedulerThread()
        : stopped(false),
          current_source(0),
          t(cm::bind_front(&SchedulerThread::recv_main, this), "scheduler")
    {}

    ~SchedulerThread()
    {
	cms_ns_if_print("scheduler", 5, "SchedulerThread::~SchedulerThread() begin");
	CMSScopedLock lock(mutex);
	cms_ns_if_print("scheduler", 5, "SchedulerThread::~SchedulerThread() locked");
	stopped = true;
	scheduled_tasks.clear();
	cond.Broadcast();	    
    }

    void add_source(void *s, std::string name)
    {
        CMSScopedLock lock(mutex);
        source_names[s] = name;
    }

    void schedule(void *s, cm::function<void()> f, int64_t when, bool is_primary_task)
    {
	CMSScopedLock lock(mutex);
        cms_ns_if_print("scheduler", 4, "scheduling %s (%p) after %.1f msec",
                        map_get_or(source_names, s, "?").c_str(),
                        s,
                        get_time_diff(when) / 1000.0);
	// cms_ns_if_print("scheduler", 5, "schedule %p locked", s);
	if (!stopped)
	{
	    if (is_primary_task)
	    {
		CM_for (ScheduledTask &st, scheduled_tasks)
		{
		    if (st.source == s && st.is_primary_task)
		    {
			st.func = f;
			// Reschedule
			if (when < st.when)
			{
			    st.when = when;
			    cond.Broadcast();
			}
			return;
		    }
		}
	    }
	    scheduled_tasks.push_back(ScheduledTask(s,when,is_primary_task,f));
	    cond.Broadcast();
	}
    }

    void erase_by_source(void *s)
    {
	cms_ns_if_print("scheduler", 5, "erase_by_souce %p begin", s);
	CMSScopedLock lock(mutex);
	cms_ns_if_print("scheduler", 5, "erase_by_souce %p locked", s);
	if (!stopped)
	{
	    while (current_source == s)
		cond.Wait(&mutex);
	    std::list<ScheduledTask>::iterator p = std::remove_if(scheduled_tasks.begin(), scheduled_tasks.end(), HasSource(s));
	    scheduled_tasks.erase(p, scheduled_tasks.end());
	}
	cms_ns_if_print("scheduler", 5, "erase_by_souce %p end", s);
    }

private:
    std::list<ScheduledTask>::iterator get_first_task()
    {
	assert(!scheduled_tasks.empty());
	std::list<ScheduledTask>::iterator min = scheduled_tasks.begin();
	std::list<ScheduledTask>::iterator it = min;
	++it;
	for (; it != scheduled_tasks.end(); ++it)
	{
	    if (it->when < min->when)
		min = it;
	}
	return min;
    }

    void recv_main()
    {
	cms_ns_if_print("scheduler", 5, "SchedulerThread::main()");
	CMSScopedLock lock(mutex);
	for (;;)
	{
	    cms_ns_if_print("scheduler", 5, "Iteration: size=%zu", scheduled_tasks.size());

	    current_source = 0;
	    cond.Broadcast();

	    if (stopped)
		break;

	    if (scheduled_tasks.empty())
	    {
		cond.Wait(&mutex);
	    }
	    else
	    {
		std::list<ScheduledTask>::iterator it = get_first_task();
		int64_t time_now = get_current_time_usec();
		if (it->when <= time_now)
		{
		    ScheduledTask t = *it;
		    scheduled_tasks.erase(it);
		    current_source = t.source;
                    std::string *maybe_source_name = map_find(source_names, t.source);
                    std::string source_name = maybe_source_name ? *maybe_source_name : "";
                    cms_ns_if_print("scheduler", 4, "running %s (%p)", source_name.c_str(), t.source);
		    CMSScopedUnlock unlock(mutex);
                    ThreadTimer tt;
		    t.func();
                    int64_t elapsed = tt.elapsed_usec();
                    if (elapsed > 1000)
                    {
                        cms_ns_if_print("scheduler", 3, "Task %s completed in %.3lf msec",
                                        source_name.c_str(),
                                        elapsed / 1000.0);
                    }
		}
		else
		{
		    cond.WaitTime(&mutex, it->when - time_now);
		}
		
	    }
	}
	cms_ns_if_print("scheduler", 5, "SchedulerThread::main() ended");
    }
};

struct SchedulerImpl
{
    cm::function<void()> primary_func;
    std::string debug_name;
    Reference<SchedulerThread> t;
    
    SchedulerImpl(cm::function<void()> f, std::string name)
	: primary_func(f),
	  debug_name(name),
	  t(get_or_create<SchedulerThread>())
    {
    }
};

PollCond::~PollCond()
{
    impl->t->erase_by_source(this);
}

void PollCond::run_async(cm::function<void()> f)
{
    impl->t->schedule(this, f, 0, false);
}

void PollCond::ChangeRqTime(double when_seconds)
{
    ChangeRqTimeUsec(when_seconds * 1000*1000);
}
void PollCond::ChangeRqTimeUsec(int64_t when)
{
    impl->t->schedule(this, impl->primary_func, when, true);
}

void PollCond::Wake()
{
    impl->t->schedule(this, impl->primary_func, get_current_time_usec(), true);
}

PollCond::PollCond(cm::function<void()> f, std::string name)
    : impl(new SchedulerImpl(f, name))
{
    impl->t->add_source(this, name);
}
