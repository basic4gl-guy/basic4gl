/*  Created 27-Jun-2003: Thomas Mulgrew

    Simple base class for object error handling.

*/
#include "NetHasErrorState.h"

namespace NetLib4Games {

///////////////////////////////////////////////////////////////////////////////
//	HasErrorState

void HasErrorState::SetError (std::string text) {
	m_error			= true;
	m_errorString	= text;
}

bool HasErrorState::Error () {
	return m_error;
}

std::string HasErrorState::GetError () {
	assert (Error ());
	return m_errorString;
}

void HasErrorState::ClearError () {
	m_error = false;
}

///////////////////////////////////////////////////////////////////////////////
//	HasErrorStateThreadsafe

HasErrorStateThreadsafe::HasErrorStateThreadsafe () {

	// Create own threadlock
	m_lock		= new ThreadLock ();
	m_ownLock	= true;
}

HasErrorStateThreadsafe::~HasErrorStateThreadsafe () {
	if (m_ownLock)
		delete m_lock;
}

void HasErrorStateThreadsafe::SetError (std::string text) {
	LockError ();
		HasErrorState::SetError (text);
	UnlockError ();
}

bool HasErrorStateThreadsafe::Error () {
	LockError ();
		bool result = HasErrorState::Error ();
	UnlockError ();
	return result;
}

std::string HasErrorStateThreadsafe::GetError () {
	LockError ();
		std::string result = HasErrorState::GetError ();
	UnlockError ();
	return result;
}

void HasErrorStateThreadsafe::ClearError () {
	LockError ();
		HasErrorState::ClearError ();
	UnlockError ();
}

}
