/*	NetLog.h

	Created 10-Jan-2005: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

	Simple network logging.

*/

#ifndef _NetLog_h
#define _NetLog_h

#ifdef NETLOG

#include <string>

typedef void (*NetLogCallback) (std::string text);

namespace NetLib4Games {
extern NetLogCallback netLogCallback;
}

#define NetLog(str) {if(NetLib4Games::netLogCallback!=NULL)NetLib4Games::netLogCallback((std::string)(str));}
void SetNetLogger (NetLogCallback callback);
void DebugNetLogger ();

#else
#define NetLog(x)
#define SetNetLogger(x)
#define DebugNetLogger()
#endif

#endif
