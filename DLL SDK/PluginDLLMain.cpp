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

#include "windows.h"

///////////////////////////////////////////////////////////////////////////////
// Constants
//
// Update these to describe your DLL.
const char *DESCRIPTION = "Miscellaneous Win32 routines";			// This is displayed in the "Plug-in Chooser" in Basic4GL
const int VERSION_MAJOR = 1;									    // Major version number
const int VERSION_MINOR = 0;									    // Minor version number

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

void DLLFUNC Func_ShellExecute(IDLL_Basic4GL_Runtime &basic4gl) {
    basic4gl.SetIntResult(
		(int)ShellExecute(
			NULL,
			basic4gl.GetStringParam(5),
			basic4gl.GetStringParam(4),
			basic4gl.GetStringParam(3),
			basic4gl.GetStringParam(2),
			basic4gl.GetIntParam(1)));
}

///////////////////////////////////////////////////////////////////////////////
//	Plugin

bool DLLFUNC Plugin::Load(IDLL_Basic4GL_FunctionRegistry &registry, bool isStandaloneExe) {

	// Initialise DLL stuff

	// Register shared interfaces

	// Register constants
	registry.RegisterIntConstant("ERROR_FILE_NOT_FOUND", ERROR_FILE_NOT_FOUND);
	registry.RegisterIntConstant("ERROR_PATH_NOT_FOUND", ERROR_PATH_NOT_FOUND);
	registry.RegisterIntConstant("ERROR_BAD_FORMAT", ERROR_BAD_FORMAT);
	registry.RegisterIntConstant("SE_ERR_ACCESSDENIED", SE_ERR_ACCESSDENIED);
	registry.RegisterIntConstant("SE_ERR_ASSOCINCOMPLETE", SE_ERR_ASSOCINCOMPLETE);
	registry.RegisterIntConstant("SE_ERR_DDEBUSY", SE_ERR_DDEBUSY);
	registry.RegisterIntConstant("SE_ERR_DDEFAIL", SE_ERR_DDEFAIL);
	registry.RegisterIntConstant("SE_ERR_DDETIMEOUT", SE_ERR_DDETIMEOUT);
	registry.RegisterIntConstant("SE_ERR_DLLNOTFOUND", SE_ERR_DLLNOTFOUND);
	registry.RegisterIntConstant("SE_ERR_FNF", SE_ERR_FNF);
	registry.RegisterIntConstant("SE_ERR_NOASSOC", SE_ERR_NOASSOC);
	registry.RegisterIntConstant("SE_ERR_OOM", SE_ERR_OOM);
	registry.RegisterIntConstant("SE_ERR_PNF", SE_ERR_PNF);
	registry.RegisterIntConstant("SE_ERR_SHARE", SE_ERR_SHARE);

	registry.RegisterIntConstant("SW_HIDE", SW_HIDE);
	registry.RegisterIntConstant("SW_MAXIMIZE", SW_MAXIMIZE);
	registry.RegisterIntConstant("SW_MINIMIZE", SW_MINIMIZE);
	registry.RegisterIntConstant("SW_RESTORE", SW_RESTORE);
	registry.RegisterIntConstant("SW_SHOW", SW_SHOW);
	registry.RegisterIntConstant("SW_SHOWDEFAULT", SW_SHOWDEFAULT);
	registry.RegisterIntConstant("SW_SHOWMAXIMIZED", SW_SHOWMAXIMIZED);
	registry.RegisterIntConstant("SW_SHOWMINIMIZED", SW_SHOWMINIMIZED);
	registry.RegisterIntConstant("SW_SHOWMINNOACTIVE", SW_SHOWMINNOACTIVE);
	registry.RegisterIntConstant("SW_SHOWNA", SW_SHOWNA);
	registry.RegisterIntConstant("SW_SHOWNOACTIVATE", SW_SHOWNOACTIVATE);
	registry.RegisterIntConstant("SW_SHOWNORMAL", SW_SHOWNORMAL);

	// Register runtime functions
	registry.RegisterFunction("ShellExecute", Func_ShellExecute, DLL_BASIC4GL_INT);
		registry.AddParam(DLL_BASIC4GL_STRING);
		registry.AddParam(DLL_BASIC4GL_STRING);
		registry.AddParam(DLL_BASIC4GL_STRING);
		registry.AddParam(DLL_BASIC4GL_STRING);
		registry.AddParam(DLL_BASIC4GL_INT);

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
