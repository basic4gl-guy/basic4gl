/*	Example DLL project.

	Created by: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

	This project will build a simple DLL that can be plugged into Basic4GL.
	It contains one library function for example purposes.

	I recommend using this as a template for creating Basic4GL plugin DLLs.
*/

///////////////////////////////////////////////////////////////////////////////
// Includes

// At the very least, Basic4GLDLLInterface.h must be included.
// Add any other include files here.
#include "Basic4GLDLLInterface.h"

// This example uses the windows MessageBox function, so we include the windows
// header.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

///////////////////////////////////////////////////////////////////////////////
// Constants
//
// Update these to describe your DLL.
const char *DESCRIPTION = "[Enter description of DLL here]";	// This is displayed in the "Plug-in Chooser" in Basic4GL
const int VERSION_MAJOR = 1;									// Major version number
const int VERSION_MINOR = 0;									// Minor version number

///////////////////////////////////////////////////////////////////////////////
//	Plugin
//
///	Main object representing the plugin to Basic4GL.
/// Basic4GL will access this through the IDLL_Plugin interface.

class Plugin : public IDLL_Basic4GL_Plugin {
public:
	Plugin() { ; }
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
		int i;
		for (i = 0; DESCRIPTION[i] != 0; i++)
			details[i] = DESCRIPTION[i];
		*major = VERSION_MAJOR;
		*minor = VERSION_MINOR;

		return BASIC4GL_DLL_VERSION;
	}

	/// Main initialisation function.
	/// Here we will construct and return our plugin object
	DLLExport IDLL_Basic4GL_Plugin *DLLFUNC Basic4GL_Init() {

		// Construct and return our plugin object
		return new Plugin();
	}
}

///////////////////////////////////////////////////////////////////////////////
// Runtime functions
// These are the functions that Basic4GL calls

/// Display a messagebox
void DLLFUNC Func_Max(IDLL_Basic4GL_Runtime &basic4gl) {

	// Extract parameters
	float firstValue = basic4gl.GetFloatParam(2);
	float secondValue = basic4gl.GetFloatParam(1);

	// Calculate maximum
	float maximum;
	if (firstValue > secondValue)
		maximum = firstValue;
	else
		maximum = secondValue;

	// Return value to Basic4GL
	basic4gl.SetFloatResult(maximum);
}

void DLLFUNC Func_MessageBox(IDLL_Basic4GL_Runtime &basic4gl) {

	// Extract string parameter
	const char *msg = basic4gl.GetStringParam(1);

	// Display the message box
	MessageBox(NULL, msg, "Plugin DLL", MB_OK);
}

///////////////////////////////////////////////////////////////////////////////
//	Plugin

bool DLLFUNC Plugin::Load(IDLL_Basic4GL_FunctionRegistry &registry, bool isStandaloneExe) {

	// Initialise DLL stuff

	// Register shared interfaces

	// Register runtime functions

	// Max function
	registry.RegisterFunction("Max", Func_Max, DLL_BASIC4GL_FLOAT);		// Function "max" returns a float (floating point number)
	registry.AddParam(DLL_BASIC4GL_FLOAT);									// First parameter is a float
	registry.AddParam(DLL_BASIC4GL_FLOAT);									// Second parameter is a float

	// Message box function
	registry.RegisterVoidFunction("MessageBox", Func_MessageBox);
	registry.ModTimeshare();					// This function is slow, so Basic4GL better perform a timeshare break after it finishes
	registry.AddParam(DLL_BASIC4GL_STRING);

	return true;
}

void DLLFUNC Plugin::Unload() {

	// DLL is about to unload.
	// Any final cleanup code (closing files, releasing resources etc) would go here

	// Delete this object. This is safe because Basic4GL will not access it again.
	delete this;
}

bool DLLFUNC Plugin::Start() {

	// Program is about to start.
	// Any pre-start init code would go here.

	return true;
}

void DLLFUNC Plugin::End() {

	// Program is about to end
	// Any cleanup code would go here

}

void DLLFUNC Plugin::Pause() {

	// Called when the program is paused (i.e. during debugging).
	// A full screen window might hide itself here so that the user can see the
	// Basic4GL edit window (and debug their program)
}

void DLLFUNC Plugin::Resume() {

	// Called when the program is resumed (i.e. during debugging).
	// This is also called when the user "steps"
}

void DLLFUNC Plugin::DelayedResume() {

	// Called when the program is resumed for more than a short "step".
	// This is where a fullscreen window would re-show itself (if it did so in ::Resume()
	// we would get flickering when the user steps through the program)
}

void DLLFUNC Plugin::GetError(char *error) {

	// We don't return errors, but if we did, we would strcpy them to *error here.
	;
}

void DLLFUNC Plugin::ProcessMessages() {

    // Called periodically (in practice, quite frequently).
    // This would be a good place to process windows messages etc.
}
