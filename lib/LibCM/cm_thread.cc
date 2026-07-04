#include "cm_thread.hh"
#include "cm_debug.hh"

gpointer Thread::thread_func(gpointer data)
{
    Thread *t = static_cast<Thread*>(data);
    add_debug_thread(t->thread_name);
    t->user_funct();
    return NULL;
}

Thread::Thread(cm::function<void ()> f, std::string name)
    : user_funct(f), thread_name(name)
{
#if GLIB_CHECK_VERSION (2, 32, 0)
    gthread = g_thread_new(thread_name.c_str(), &Thread::thread_func, this);
#else
    gthread = g_thread_create(&Thread::thread_func, this, TRUE, NULL);
#endif
}

Thread::~Thread()
{
    cms_ns_if_print("thread", 3, "Joining thread %s ...", thread_name.c_str());
    //  println("Joining thread: %...") % thread_name;
    g_thread_join(gthread);
    cms_ns_if_print("thread", 3, "Thread %s joined", thread_name.c_str());
    //  println("Joined: %...") % thread_name;
}

bool Thread::is_current_thread()
{
    return g_thread_self() == gthread;
}


