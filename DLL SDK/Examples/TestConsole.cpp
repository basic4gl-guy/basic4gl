/*	TestConsole.cpp

	Created 12-Mar-06: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

	Test text console. For testing object sharing between plugin DLLs.
*/

#include "TestConsole.h"

TestConsole::TestConsole() {

	// Create and display a standard text console
	consoleBuffer = NULL;
}

TestConsole::~TestConsole() {

	// Close text console
	Hide();
}

void TestConsole::Show() {
	if (consoleBuffer == NULL) {
		AllocConsole();
		consoleBuffer = CreateConsoleScreenBuffer(
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			CONSOLE_TEXTMODE_BUFFER,
			NULL);
		SetConsoleActiveScreenBuffer(consoleBuffer);
	}
}

void TestConsole::Hide() {
	if (consoleBuffer != NULL) {
		CloseHandle(consoleBuffer);
		consoleBuffer = NULL;
		FreeConsole();
	}
}

void DLLFUNC TestConsole::Cls() {
	// CODE HERE!!!
}

void DLLFUNC TestConsole::Print(const char *text) {
	unsigned long written;
	WriteConsole(
		consoleBuffer,
		text,
		strlen(text),
		&written,
		NULL);
}

void DLLFUNC TestConsole::NewLine() {
	Print("\n");
}

void DLLFUNC TestConsole::Locate(int x, int y) {
	COORD pos;
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(consoleBuffer, pos);
}
