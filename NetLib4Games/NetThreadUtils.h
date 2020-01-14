/*	ThreadUtils.h

	Created 14-Feb-2005: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

	Simple thread object wrappers for Win32.
	Also lumps all the relevant code into a single unit, which hopefully
	would simplify porting.
	(You only need to replace this unit.)
*/

#ifndef _NetThreadUtils_h
#define _NetThreadUtils_h

#include <windows.h>
#include <assert.h>

namespace NetLib4Games {

/// Thread entry point
extern DWORD WINAPI ThreadProc (LPVOID lpParameter);

///////////////////////////////////////////////////////////////////////////////
//	ThreadEvent
//
///	Simple event wrapper for thread synchronsiation event
class ThreadEvent {
	HANDLE	m_event;
public:
	ThreadEvent (bool manualReset = false, bool signalled = false) { m_event = CreateEvent (NULL, manualReset, signalled, NULL); }
	~ThreadEvent ()					{ CloseHandle (m_event); }

	// Member access
	HANDLE EventHandle ()			{ return m_event; }

	// Methods
	void Set ()						{ SetEvent (m_event); }
	void Reset ()					{ ResetEvent (m_event); }
	bool WaitFor (DWORD timeout)	{ return WaitForSingleObject (m_event, timeout) == WAIT_OBJECT_0; }
	bool WaitFor ()					{ return WaitFor (INFINITE); }
	void Pulse ()					{ PulseEvent (m_event); }
};

///////////////////////////////////////////////////////////////////////////////
//	ThreadLock
//
///	Basic wrapper for thread critical section or mutex object
class ThreadLock {
	int		m_lockCount;	
	HANDLE	m_lock;
public:
	ThreadLock () : m_lockCount (0)	{ m_lock = CreateMutex (NULL, false, NULL); }
	~ThreadLock ()					{ CloseHandle (m_lock); }

	// Member access
	HANDLE LockHandle ()			{ return m_lock; }

	// Methods
	bool Lock (DWORD timeout)		{ 
		bool result = WaitForSingleObject (m_lock, timeout) == WAIT_OBJECT_0; 
		if (result)
			m_lockCount++;
		return result;	
	}
	bool Lock ()					{ return Lock (INFINITE); }
	void Unlock ()					{ m_lockCount--; ReleaseMutex (m_lock);}
};

///////////////////////////////////////////////////////////////////////////////
//	Thread
//
///	Simple wrapper class for win32 thread.

class Threaded;

class Thread {
	HANDLE		m_thread;
	DWORD		m_threadID;
	ThreadEvent	m_terminateEvent;
    bool        m_failed;
public:
	Thread () : m_thread (NULL), m_terminateEvent (true) { ; }
	~Thread () {
		Terminate ();
	};

	//	Member access (windows specific. cross-platform code should avoid)
	HANDLE	ThreadHandle () { return m_thread; }
	DWORD	ThreadID ()		{ return m_threadID; }

	/// Start the thread. Thread does not start until this method is called.
	void Start (Threaded *threaded) {
		assert (m_thread == NULL);
		assert (threaded != NULL);
		m_terminateEvent.Reset ();
		m_thread = CreateThread (NULL, 0, ThreadProc, (LPVOID) threaded, 0, &m_threadID);
	}

	bool Running () {
		return m_thread != NULL;
	}

	/// Signal to the thread to terminate.
	/// Threaded::ThreadExecute should call Terminating() to check if the thread wants to
	/// terminate
	void Terminate (bool waitFor = true) { 
		m_terminateEvent.Set (); 
		if (waitFor) 
			WaitFor ();
	}

	/// Handle of the "terminating" event.
	/// (Incase calling code wants to add it as a waitobject.)
	ThreadEvent& TerminateEvent () { return m_terminateEvent; }

	/// Returns true if Terminate() has been called.
	bool Terminating () { return TerminateEvent ().WaitFor (0); }

	/// Wait for the thread to terminate
	bool WaitFor (DWORD timeout) { 
		if (m_thread != NULL) {
			bool signalled = WaitForSingleObject (m_thread, timeout) == WAIT_OBJECT_0; 
			if (signalled)
				m_thread = NULL;		
			return signalled;
		}
		else
			return true;
	}
	bool WaitFor ()	{ 
		return WaitFor (INFINITE); 
	}

    void RaisePriority () {
        assert (m_thread != NULL);
        SetThreadPriority (m_thread, THREAD_PRIORITY_ABOVE_NORMAL);
    }
};

///////////////////////////////////////////////////////////////////////////////
//	Threaded
//
///	Abstract base class for objects that support threading.
///	Actual thread is mainted in an internal "Thread" object, which after 
/// construction will call Threaded::ThreadExecute (from the new thread).
class Threaded {
protected:
	virtual void ThreadExecute () = 0;

	friend DWORD WINAPI ThreadProc (LPVOID lpParameter);
};

///////////////////////////////////////////////////////////////////////////////
//	Helper functions

inline bool WaitForEvents (ThreadEvent **events, int count, bool all = false, DWORD timeout = INFINITE) {

	// Build handle array
	assert (count < 256);
	HANDLE handles [256];
	for (int i = 0; i < count; i++) 
		handles [i] = events [i]->EventHandle ();

	// Perform wait
	DWORD waitResult = WaitForMultipleObjects (count, handles, all, timeout);
	return waitResult >= WAIT_OBJECT_0 && waitResult < WAIT_OBJECT_0 + count;
}

}

#endif
