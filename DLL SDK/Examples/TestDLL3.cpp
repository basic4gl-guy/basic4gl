//kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Basic4GLDLLInterface.h"
#include "Basic4GLStandardObjects.h"
#include <string.h>

///////////////////////////////////////////////////////////////////////////////
//	TestPlugin
//
///	Main object representing the plugin to Basic4GL.
/// Basic4GL will access this through the IDLL_Plugin interface.

// Global variables.
// Necessary, because the runtime functions are called at a global scope.
IB4GLText *textScreen = NULL;
char *pluginError = NULL;

class TestPlugin : public IDLL_Basic4GL_Plugin {
public:
	TestPlugin() { ; }
    virtual bool DLLFUNC Load(IDLL_Basic4GL_FunctionRegistry &registry, bool isStandaloneExe);
    virtual bool DLLFUNC Start();
    virtual void DLLFUNC End();
    virtual void DLLFUNC Unload();
    virtual void DLLFUNC GetError(char *error);
	virtual void DLLFUNC Pause();
	virtual void DLLFUNC Resume();
	virtual void DLLFUNC DelayedResume();
    virtual void DLLFUNC ProcessMessages();
};

///////////////////////////////////////////////////////////////////////////////
// DLL exported functions
extern "C" {

	/// Query function
	DLLExport int DLLFUNC Basic4GL_Query(char *details, int *major, int *minor) {
		char *detailSrc = "Basic4GL plugin example #3";
		int i;
		for (i = 0; detailSrc[i] != 0; i++)
			details[i] = detailSrc[i];
		*major = 1;
		*minor = 0;

		// MUST return BASIC4GL_DLL_VERSION here!
		return BASIC4GL_DLL_VERSION;
	}

	/// Main initialisation function.
	/// Here we will construct and return our plugin object
	DLLExport IDLL_Basic4GL_Plugin *DLLFUNC Basic4GL_Init() {

		// Construct and return our plugin object
		return new TestPlugin();
	}
}

///////////////////////////////////////////////////////////////////////////////
// Runtime functions
// These are the functions that Basic4GL calls

/// Display a scrolling message
inline int Min(int a, int b) {
    return a < b ? a : b;
}

inline int Max(int a, int b) {
    return a > b ? a : b;
}

void DoScrollMessage(const char *msg, int y) {

    // Allocate an array of characters.
    char* line = new char[textScreen->TextCols() + 1];

    // Disable text scrolling
    bool saveTextScroll = textScreen->TextScroll();     // Save previous setting
    textScreen->SetTextScroll(false);

    int cols = textScreen->TextCols();
    int len = strlen(msg);

    // Track the leftmost character of message.
    // It will move all the way from the right side of the screen, past the
    // left hand side and far enough that the right most character moves off
    // the screen also.

    int x1 = cols - 1, x2 = -len;
    for (int leftX = x1; leftX >= x2; leftX--) {

        // Create a line of text.
        int x;
        for (x = 0; x < Min(leftX, cols); x++)
            line[x] = ' ';

        for (x = Max(leftX, 0); x < Min(leftX + len, cols); x++)
            line[x] = msg[x - leftX];

        for (x = Max(leftX + len, 0); x < cols; x++)
            line[x] = ' ';

        line[cols] = 0;

        // Display it at the given location
        textScreen->Locate(0, y);
        textScreen->Print(line, false);
        textScreen->DrawText();

        // Delay
        Sleep(100);
    }

    delete[] line;
    textScreen->SetTextScroll(saveTextScroll);              // Restore text scroll setting
}
void DLLFUNC Func_ScrollMessage(IDLL_Basic4GL_Runtime &basic4gl) {
    DoScrollMessage(basic4gl.GetStringParam(2), basic4gl.GetIntParam(1));
}
void DLLFUNC Func_ScrollMessage2(IDLL_Basic4GL_Runtime &basic4gl) {
    DoScrollMessage(basic4gl.GetStringParam(1), textScreen->TextRows() - 1);
}

///////////////////////////////////////////////////////////////////////////////
//	TestPlugin

bool DLLFUNC TestPlugin::Load(IDLL_Basic4GL_FunctionRegistry &registry, bool isStandaloneExe) {

    // Fetch text interface.
    //
    // Note: We will grab the general text screen interface IB4GLText, rather than
    // the OpenGL specific one IB4GLOpenGLText.
    // This makes our plugin more general, and means it could be used with any
    // type of text display such as a console window, which may be added to
    // later versions of Basic4GL.

    textScreen = (IB4GLText *) registry.FetchInterface("IB4GLText", 1, 0);
    if (textScreen == NULL) {
        pluginError = "Could not get IB4GLText interface to text screen. Is this the right Basic4GL version?";
        return false;
    }

	// Initialise DLL stuff

	// Register shared interfaces

    // Register constants

	// Register runtime functions

	// Scrolling message function
	registry.RegisterVoidFunction("ScrollMessage", Func_ScrollMessage);
	registry.ModTimeshare();                    // This function is slow, so Basic4GL better perform a timeshare break after it finishes
	registry.AddParam(DLL_BASIC4GL_STRING);
	registry.AddParam(DLL_BASIC4GL_INT);

    // Second version of same function, but with just one paramter.
    // The Y position defaults to the bottom of the screen.
    // Note that we can give it the same name in Basic4GL. Basic4GL will
    // determine which one to call based on the parameters given.
	registry.RegisterVoidFunction("ScrollMessage", Func_ScrollMessage2);
	registry.ModTimeshare();
	registry.AddParam(DLL_BASIC4GL_STRING);
	return true;
}

void DLLFUNC TestPlugin::Unload() {

	// DLL is about to unload.
	// Final cleanup code

	// Delete this object. This is safe because Basic4GL will not access it again.
	delete this;
}

bool DLLFUNC TestPlugin::Start() {

	// Program is about to start.
	// Any pre-start init code would go here.

	return true;
}

void DLLFUNC TestPlugin::End() {

	// Program is about to end
	// Any cleanup code would go here
}

void DLLFUNC TestPlugin::Pause() {

	// Called when the program is paused (i.e. during debugging).
	// A full screen window might hide itself here so that the user can see the
	// Basic4GL edit window (and debug their program)
}

void DLLFUNC TestPlugin::Resume() {

	// Called when the program is resumed (i.e. during debugging).
	// This is also called when the user "steps"
}

void DLLFUNC TestPlugin::DelayedResume() {

	// Called when the program is resumed for more than a short "step".
	// This is where a fullscreen window would re-show itself (if it did so in ::Resume()
	// we would get flickering when the user steps through the program)
}

void DLLFUNC TestPlugin::GetError(char *error) {
	if (pluginError != NULL)
		for (int i = 0; pluginError[i] != 0; i++)
			error[i] = pluginError[i];
}

void DLLFUNC TestPlugin::ProcessMessages() {

    // Called periodically (in practice, quite frequently).
    // This would be a good place to process windows messages etc.
}

