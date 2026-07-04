#ifndef CM_SCHEDULER_HH_
#define CM_SCHEDULER_HH_

#include "cm_header_internal.hh"
#include "cm_utils.hh"
#include "cm_maybe.hh"
#include "cm_thread.hh"

struct SchedulerImpl;

// Scheduler si drzi frontu funkci, ktere si uzivatel naplanuje pomoci
// schedule(), a postupne je ve svem vnitrnim threadu provadi.
//
// Kdyz Scheduler zanika, tak blokuje, dokud neskonci prave provadena
// uzivatelova funkce.
class PollCond
{
    cm::scoped_ptr<SchedulerImpl> impl;

public:
    PollCond(cm::function<void()> default_task, std::string name);
    ~PollCond();
    
    // Lze volat i z uzivatelovy naplanovane funkce, tj. v predanem
    // callbacku se znovu naplanovat.
    void run_async(cm::function<void()> f);

    void ChangeRqTime(double when_seconds);
    void ChangeRqTimeUsec(int64_t when);
    void Wake();

private:
    PollCond(PollCond const &);
    PollCond &operator=(PollCond);
};

// class PollCond
// {
//     cm::function<void()> user_fun;
//     CMSMutex mutex;
//     bool was_stopped = false;
//     Maybe<int64_t> next_time;
//     std::string m_name;

//     Scheduler scheduler;

// public:
//     PollCond(cm::function<void()> f, std::string name)
// 	: user_fun(f), m_name(name)
//     {
// 	cms_ns_if_print("scheduler", 3, "PollCond created");
//     }
//     ~PollCond()
//     {
// 	cms_ns_if_print("scheduler", 3, "PollCond destroy begin");
// 	CMSScopedLock lock(mutex);
// 	cms_ns_if_print("scheduler", 3, "PollCond destroy locked");
// 	was_stopped = true;
//     }
//     void Wake()
//     {
// 	{
// 	    CMSScopedLock lock(mutex);
// 	    if (was_stopped)
// 		return;
// 	    next_time = 0;
// 	    cms_ns_if_print("scheduler", 3, "PollCond %s: Wake now", m_name.c_str());
// 	}
// 	scheduler.schedule(cm::bind_front(&PollCond::poll_funct, this));
//     }
//     void ChangeRqTime(double when_seconds)
//     {
// 	Maybe<int64_t> local_next_time;
// 	{
// 	    CMSScopedLock lock(mutex);
// 	    if (was_stopped)
// 		return;
// 	    int64_t req_time = static_cast<int64_t>(when_seconds * 1E6);
// 	    cms_ns_if_print("scheduler", 3, "PollCond %s: Wakeup at %ld", m_name.c_str(), req_time);
// 	    if (!next_time || req_time < *next_time)
// 	    {
// 		next_time = req_time;
// 		local_next_time = next_time;
// 	    }
// 	}
// 	if (local_next_time)
// 	    scheduler.schedule(cm::bind_front(&PollCond::poll_funct, this), *local_next_time);
//     }

// private:
//     void poll_funct()
//     {
// 	{
// 	    CMSScopedLock lock(mutex);
// 	    if (was_stopped || !next_time || *next_time > get_current_time_usec())
// 		return;
// 	    next_time = Nothing();
// 	}
// 	cms_ns_if_print("scheduler", 3, "PollCond %s: Userfun begin", m_name.c_str());
// 	user_fun();
// 	cms_ns_if_print("scheduler", 3, "PollCond %s: Userfun end", m_name.c_str());
//     }
// };

#endif // CM_SCHEDULER_HH_
