#ifndef CM_THREAD_HH_
#define CM_THREAD_HH_

#include "cm_header_internal.hh"
#include <functional>
#include <deque>

class Thread
{
    static gpointer thread_func(gpointer data);

    GThread *gthread;
    cm::function<void ()> user_funct;
    std::string thread_name;

public:
    Thread(cm::function<void ()> f, std::string name = "thread");
    ~Thread();

    bool is_current_thread();

private:
    Thread(Thread const &);
    Thread &operator=(Thread const &);
};

template <typename Mutex>
struct CmBasicScopedLock
{
    CmBasicScopedLock(Mutex &m) : mut(m)
    {
        mut.Lock();
    }
    ~CmBasicScopedLock()
    {
        mut.Unlock();
    }

private:
    Mutex &mut;
    CmBasicScopedLock(const Mutex &m);
    CmBasicScopedLock &operator=(const Mutex &m);
};

template <typename Mutex>
struct CmBasicScopedUnlock
{
    CmBasicScopedUnlock(Mutex &m) : mut(m)
    {
        mut.Unlock();
    }
    ~CmBasicScopedUnlock()
    {
        mut.Lock();
    }

private:
    Mutex &mut;
    CmBasicScopedUnlock(const Mutex &m);
    CmBasicScopedUnlock &operator=(const Mutex &m);
};

template <typename Mutex>
struct BasicScopedTryLock
{
    BasicScopedTryLock(Mutex &m) : mut(m)
    {
        is_locked_flag = mut.TryLock();
    }
    ~BasicScopedTryLock()
    {
        if (is_locked_flag)
            mut.Unlock();
    }
    bool is_locked() const
    {
        return is_locked_flag;
    }

private:
    Mutex &mut;
    bool is_locked_flag;
    BasicScopedTryLock(const Mutex &m);
    BasicScopedTryLock &operator=(const Mutex &m);
};

typedef CmBasicScopedLock<CMSMutex> CMSScopedLock;
typedef CmBasicScopedUnlock<CMSMutex> CMSScopedUnlock;
typedef BasicScopedTryLock<CMSMutex> CMSScopedTryLock;


template <typename T>
class ThreadWorker
{
    std::deque<T> m_queue;
    bool m_was_stopped;
    std::string m_name;
    CMSMutex m_mutex;
    CMSCondition m_cond;
    cm::function<void(T)> m_worker_funct;
    Thread m_thread;

public:
    void push(T x)
    {
        CMSScopedLock lock(m_mutex);
        m_queue.push_back(cm::move(x));
        m_cond.Signal();
    }

    ~ThreadWorker()
    {
        CMSScopedLock lock(m_mutex);
        m_was_stopped = true;
        m_cond.Signal();
        // Now, we wait for the m_thread to join
    }

    ThreadWorker(cm::function<void(T)> funct, std::string name)
        : m_was_stopped(false),
          m_name(name),
          m_worker_funct(funct),
          m_thread(cm::bind_front(&ThreadWorker::worker_main, this), name)
    {}

private:
    void worker_main()
    {
	CMSScopedLock lock(m_mutex);
        while (!m_was_stopped)
        {
	    if (!m_queue.empty())
	    {
		T x = cm::move(m_queue.front());
                m_queue.pop_front();
		CMSScopedUnlock unlock(m_mutex);
		m_worker_funct(x);
	    }
	    else
	    {
		m_cond.Wait(&m_mutex);
	    }
	}
    }
};

#endif // CM_THREAD_HH_
