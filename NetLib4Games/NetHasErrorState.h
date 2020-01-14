/*  Created 27-Jun-2003: Thomas Mulgrew

    Simple base class for object error handling.

*/

#ifndef NetHasErrorStateH
#define NetHasErrorStateH
#include <string>
#include <assert.h>
#include "NetThreadUtils.h"

namespace NetLib4Games {

/// Abstract base class for very simple error handling mechanism
class AbstractHasErrorState {
protected:
	virtual void SetError (std::string text) = 0;

public:

	/// True if object is in error state
	virtual bool Error ()			= 0;

    /// Return error text
	virtual std::string GetError () = 0;
	virtual void ClearError ()		= 0;
};

/// Base class for very simple error handling mechanism.
class HasErrorState : public AbstractHasErrorState {

    // Error status
    bool                        m_error;
    std::string                 m_errorString;

protected:
    virtual void SetError (std::string text);

public:
    HasErrorState () : m_error (false) { ; }

    virtual bool Error ();
    virtual std::string GetError ();
    virtual void ClearError ();
};

/// Threadsafe version of HasErrorState
class HasErrorStateThreadsafe : public HasErrorState {
	
	ThreadLock *m_lock;			// Lock object.
	bool		m_ownLock;		// True if we own it (and will free it on destruction)

public:
	HasErrorStateThreadsafe (ThreadLock& lock) : m_lock (&lock), m_ownLock (false) { ; }
	HasErrorStateThreadsafe ();
	virtual ~HasErrorStateThreadsafe ();

	// Note: Locking is automatic around the standard HasErrorState methods, however
	// calling code may want to explicitly lock around a set of calls to make them 
	// transactional, e.g:
	//
	//		o.LockError ();
	//			if (o.Error ()) {
	//				DoSomethingWith (o.GetError ());
	//				o.ClearError ();
	//			}
	//		o.UnlockError ();
	//
	// This would ensure that another thread didn't clear the error between the test
	// and reading the string with ::GetError.
	//
	void LockError ()	{ m_lock->Lock (); }
	void UnlockError () { m_lock->Unlock (); }

	// HasErrorState methods
    virtual void SetError (std::string text);
    virtual bool Error ();
    virtual std::string GetError ();
    virtual void ClearError ();
};

}

#endif
