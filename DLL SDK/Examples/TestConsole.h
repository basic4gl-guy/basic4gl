/*	TestConsole.h

	Created 12-Mar-06: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

	Test text console. For testing object sharing between plugin DLLs.
*/

#ifndef _TestConsole_h
#define _TestConsole_h

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "TextConsole.h"

///////////////////////////////////////////////////////////////////////////////
//	TestConsole
//
///	Implementation of ITextConsole
class TestConsole : public ITextConsole {
private:
	HANDLE	consoleBuffer;
public:
	TestConsole();
	virtual ~TestConsole();

	void Show();
	void Hide();

	virtual void DLLFUNC Cls();
	virtual void DLLFUNC Print(const char *text);
	virtual void DLLFUNC NewLine();
	virtual void DLLFUNC Locate(int x, int y);
};

#endif