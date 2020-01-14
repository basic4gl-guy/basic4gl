/*	TextConsole.h

	Created 12-Mar-06: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
*/

#ifndef _TextConsole_h
#define _TextConsole_h

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "DLLMisc.h"

///////////////////////////////////////////////////////////////////////////////
//	ITextConsole 
//
///	A standard text console window
class ITextConsole {
public:
	virtual void DLLFUNC Cls() = 0;
	virtual void DLLFUNC Print(const char *text) = 0;
	virtual void DLLFUNC NewLine() = 0;
	virtual void DLLFUNC Locate(int x, int y) = 0;
};

#endif
