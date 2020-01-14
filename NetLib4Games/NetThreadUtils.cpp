/*	ThreadUtils.cpp

	Created 14-Feb-2005: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
*/

#include "NetThreadUtils.h"

namespace NetLib4Games {

// Thread entry point
DWORD WINAPI ThreadProc (LPVOID lpParameter) {

	// Extract threaded object pointer
	Threaded *threaded = (Threaded *) lpParameter;

	// Call its execute method
	threaded->ThreadExecute ();
	
	return true;
}

}