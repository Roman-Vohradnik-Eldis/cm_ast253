#ifndef _CMS_SYNCH_HH
#define _CMS_SYNCH_HH

#include "CMSHeader.hh"
#include "CMSDebug.hh"

class CMSMutex
{
  friend class CMSCondition;

	GMutex * mutex;

  public:
	CMSMutex () { mutex = g_mutex_new (); }
	~CMSMutex () { g_mutex_free (mutex); }
	void Lock () { g_mutex_lock (mutex); }
	bool TryLock () { return g_mutex_trylock (mutex); }
	void Unlock () { return g_mutex_unlock (mutex); }
};

class CMSMutexRecursive
{
	GStaticRecMutex mutex;

  public:
	CMSMutexRecursive () { g_static_rec_mutex_init (&mutex); }
	~CMSMutexRecursive () { }
	void Lock () { g_static_rec_mutex_lock (&mutex); }
	bool TryLock () { return g_static_rec_mutex_trylock (&mutex); }
	void Unlock () { g_static_rec_mutex_unlock (&mutex); }
};

class CMSLockRW
{
	GStaticRWLock lock;

  public:
	CMSLockRW () { g_static_rw_lock_init (&lock); }
	~CMSLockRW () { }
	void LockR () { g_static_rw_lock_reader_lock (&lock); }
	bool TryLockR () { return g_static_rw_lock_reader_trylock (&lock); }
	void UnlockR () { g_static_rw_lock_reader_unlock (&lock); }
	void LockW () { g_static_rw_lock_writer_lock (&lock); }
	bool TryLockW () { return g_static_rw_lock_writer_trylock (&lock); }
	void UnlockW () { g_static_rw_lock_writer_unlock (&lock); }
};

class CMSCondition
{
	GCond * cond;

  public:
	CMSCondition () { cond = g_cond_new (); }
	~CMSCondition () { g_cond_free (cond); }
	void Signal () { g_cond_signal (cond); }
	void Broadcast () { g_cond_broadcast (cond); }
	void Wait (CMSMutex * mutex) { g_cond_wait (cond, mutex->mutex); }
	int WaitTime (CMSMutex * mutex, long usec)
	{
	    GTimeVal time;
	    g_get_current_time(&time);
	    long long fulltime = (long long)time.tv_usec + ((long long)time.tv_sec * 1000000LL) + (long long)usec;
	    time.tv_sec = (long)(fulltime / 1000000LL);
	    time.tv_usec = (long)(fulltime % 1000000LL);
	    return g_cond_timed_wait (cond, mutex->mutex, &time);
	}
};

#endif /* _CMS_SYNCH_HH */
