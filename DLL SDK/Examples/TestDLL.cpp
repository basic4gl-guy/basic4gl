//kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Basic4GLDLLInterface.h"
#include "Basic4GLStandardObjects.h"
#include <math.h>
#include "TestConsole.h"

///////////////////////////////////////////////////////////////////////////////
//	TestPlugin
//
///	Main object representing the plugin to Basic4GL.
/// Basic4GL will access this through the IDLL_Plugin interface.

// Global variables.
// Necessary, because the runtime functions are called at a global scope.
TestConsole *console = NULL;
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
		char *detailSrc = "Basic4GL plugin example #1";
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

/// Display a messagebox
void DLLFUNC Func_MessageBox(IDLL_Basic4GL_Runtime &basic4gl) {

	// Extract string parameter
	const char *msg = basic4gl.GetStringParam(1);

	// Display the message box
	MessageBox(NULL, msg, "TestDLL", MB_OK);
}

void DLLFUNC Func_LogXY(IDLL_Basic4GL_Runtime &basic4gl) {

	// Read X parameter
	float x = basic4gl.GetFloatParam(2);

	// Read Y parameter
	float y = basic4gl.GetFloatParam(1);

	// Calculate logx(y)
	float result = (float) (log(y) / log(x));

	// Return result to Basic4GL program
	basic4gl.SetFloatResult(result);
}

void DLLFUNC Func_Time(IDLL_Basic4GL_Runtime &basic4gl) {

	// Get the time
	SYSTEMTIME time;
	GetSystemTime(&time);

	// Format into a text string
	// Note: To keep the size down, we aren't including <string> or <stdio.h> etc, so
	// we need to do a bit of number->text logic ourselves
	char result[13];
	result[0]	= time.wHour / 10 + '0';
	result[1]	= time.wHour % 10 + '0';
	result[2]	= ':';
	result[3]	= time.wMinute / 10 + '0';
	result[4]	= time.wMinute % 10 + '0';
	result[5]	= ':';
	result[6]	= time.wSecond / 10 + '0';
	result[7]	= time.wSecond % 10 + '0';
	result[8]	= '.';
	result[9]	= time.wMilliseconds / 100 + '0';
	result[10]	= (time.wMilliseconds / 10) % 10 + '0';
	result[11]	= time.wMilliseconds % 10 + '0';
	result[12]	= 0;

	// Return result to Basic4GL
	// Note: Basic4GL will COPY the string into its own storage, so we don't
	// need to worry that the string will dissapear as soon as this function exits
	basic4gl.SetStringResult(result);
}

int buffer1[10000], buffer2[10000];

void DLLFUNC Func_RotateArray(IDLL_Basic4GL_Runtime &basic4gl) {

	// Get array dimensions
	int xSize = basic4gl.GetArrayParamDimension(1, 0);
	int ySize = basic4gl.GetArrayParamDimension(1, 1);
	if (xSize > 100)
		xSize = 100;
	if (ySize > 100)
		ySize = 100;

	// Read array
	basic4gl.GetIntArrayParam(1, buffer1, 2, 100, 100);

	// Rotate array
	for (int y = 0; y < ySize; y++)
		for (int x = 0; x < xSize; x++)
			buffer2[y * xSize + x] = buffer1[x * 100 + (ySize - y - 1)];

	// Return rotated array
	basic4gl.SetIntArrayResult(buffer2, 2, ySize, xSize);
}

void DLLFUNC Func_ArraySize1(IDLL_Basic4GL_Runtime &basic4gl) {
	int dimension = basic4gl.GetIntParam(1);
	int size = 0;
	if (dimension >= 0 && dimension < 1)
		size = basic4gl.GetArrayParamDimension(2, dimension);
	basic4gl.SetIntResult(size);
}

void DLLFUNC Func_ArraySize2(IDLL_Basic4GL_Runtime &basic4gl) {
	int dimension = basic4gl.GetIntParam(1);
	int size = 0;
	if (dimension >= 0 && dimension < 2)
		size = basic4gl.GetArrayParamDimension(2, dimension);
	basic4gl.SetIntResult(size);
}

void DLLFUNC Func_ArraySize3(IDLL_Basic4GL_Runtime &basic4gl) {
	int dimension = basic4gl.GetIntParam(1);
	int size = 0;
	if (dimension >= 0 && dimension < 3)
		size = basic4gl.GetArrayParamDimension(2, dimension);
	basic4gl.SetIntResult(size);
}

void DLLFUNC Func_ConPrint(IDLL_Basic4GL_Runtime &basic4gl) {
	console->Print(basic4gl.GetStringParam(1));
}

void DLLFUNC Func_ConPrintr(IDLL_Basic4GL_Runtime &basic4gl) {
	console->Print(basic4gl.GetStringParam(1));
	console->NewLine();
}

void DLLFUNC Func_ConPrintr2(IDLL_Basic4GL_Runtime &basic4gl) {
	console->NewLine();
}

void DLLFUNC Func_ConLocate(IDLL_Basic4GL_Runtime &basic4gl) {
	console->Locate(basic4gl.GetIntParam(2), basic4gl.GetIntParam(1));
}

///////////////////////////////////////////////////////////////////////////////
//	TestPlugin

bool DLLFUNC TestPlugin::Load(IDLL_Basic4GL_FunctionRegistry &registry, bool isStandaloneExe) {

    // Fetch text interface

	// Initialise DLL stuff
	console = new TestConsole();

	// Register shared interfaces
	registry.RegisterInterface(console, "TextConsole", 1, 0);

    // Register constants
    registry.RegisterStringConstant("TESTPLUGIN_AUTHOR", "Tom");
    registry.RegisterIntConstant("TESTPLUGIN_DLL_VERSION", BASIC4GL_DLL_VERSION);

	// Register runtime functions

	// Message box function
	registry.RegisterVoidFunction("MessageBox", Func_MessageBox);
	registry.ModTimeshare();					// This function is slow, so Basic4GL better perform a timeshare break after it finishes
	registry.AddParam(DLL_BASIC4GL_STRING);

	// Logx(y) function
	registry.RegisterFunction("LogXY", Func_LogXY, DLL_BASIC4GL_FLOAT);
	registry.AddParam(DLL_BASIC4GL_FLOAT);
	registry.AddParam(DLL_BASIC4GL_FLOAT);

	// Time$ function
	registry.RegisterFunction("Time$", Func_Time, DLL_BASIC4GL_STRING);

	// RotateArray function
	registry.RegisterArrayFunction("RotateArray", Func_RotateArray, DLL_BASIC4GL_INT, 2);
	registry.AddArrayParam(DLL_BASIC4GL_INT, 2);

	// Array size function
	registry.RegisterFunction("ArraySize", Func_ArraySize1, DLL_BASIC4GL_INT);
	registry.AddArrayParam(DLL_BASIC4GL_INT, 1);
	registry.AddParam(DLL_BASIC4GL_INT);

	registry.RegisterFunction("ArraySize", Func_ArraySize2, DLL_BASIC4GL_INT);
	registry.AddArrayParam(DLL_BASIC4GL_INT, 2);
	registry.AddParam(DLL_BASIC4GL_INT);

	registry.RegisterFunction("ArraySize", Func_ArraySize3, DLL_BASIC4GL_INT);
	registry.AddArrayParam(DLL_BASIC4GL_INT, 3);
	registry.AddParam(DLL_BASIC4GL_INT);

	registry.RegisterFunction("ArraySize", Func_ArraySize1, DLL_BASIC4GL_INT);
	registry.AddArrayParam(DLL_BASIC4GL_FLOAT, 1);
	registry.AddParam(DLL_BASIC4GL_INT);

	registry.RegisterFunction("ArraySize", Func_ArraySize2, DLL_BASIC4GL_INT);
	registry.AddArrayParam(DLL_BASIC4GL_FLOAT, 2);
	registry.AddParam(DLL_BASIC4GL_INT);

	registry.RegisterFunction("ArraySize", Func_ArraySize3, DLL_BASIC4GL_INT);
	registry.AddArrayParam(DLL_BASIC4GL_FLOAT, 3);
	registry.AddParam(DLL_BASIC4GL_INT);

	registry.RegisterVoidFunction("ConPrint", Func_ConPrint);
	registry.AddParam(DLL_BASIC4GL_STRING);
	registry.ModNoBrackets();

	registry.RegisterVoidFunction("ConPrintr", Func_ConPrintr);
	registry.AddParam(DLL_BASIC4GL_STRING);
	registry.ModNoBrackets();

	registry.RegisterVoidFunction("ConPrintr", Func_ConPrintr2);
	registry.ModNoBrackets();

	registry.RegisterVoidFunction("ConLocate", Func_ConLocate);
	registry.AddParam(DLL_BASIC4GL_INT);
	registry.AddParam(DLL_BASIC4GL_INT);
	registry.ModNoBrackets();

	return true;
}

void DLLFUNC TestPlugin::Unload() {

	// DLL is about to unload.
	// Final cleanup code
	if (console != NULL)
        delete console;

	// Delete this object. This is safe because Basic4GL will not access it again.
	delete this;
}

bool DLLFUNC TestPlugin::Start() {

	// Program is about to start.
	// Any pre-start init code would go here.
	console->Show();

	return true;
}

void DLLFUNC TestPlugin::End() {

	// Program is about to end
	// Any cleanup code would go here
	console->Hide();
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

