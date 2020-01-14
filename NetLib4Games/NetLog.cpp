#include "NetLog.h"

#ifdef NETLOG
#include <windows.h>
#include "NetMiscRoutines.h"

namespace NetLib4Games {
NetLogCallback netLogCallback = NULL;
}
using namespace NetLib4Games;

void DebugLogger (std::string text) {
	OutputDebugString (("Net event (" + IntToString (GetTickCount ()) + "): " + text + "\r\n").c_str ());
}
void SetNetLogger (NetLogCallback callback) {
    netLogCallback = callback;
}
void DebugNetLogger () {
    netLogCallback = DebugLogger;
}

#endif

