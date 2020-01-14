//kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Basic4GLDLLInterface.h"
#include "Basic4GLOpenGLObjects.h"
#include <gl/gl.h>
#include <gl/glu.h>

///////////////////////////////////////////////////////////////////////////////
//	TestPlugin
//
///	Main object representing the plugin to Basic4GL.
/// Basic4GL will access this through the IDLL_Plugin interface.

// Global variables.
// Necessary, because the runtime functions are called at a global scope.
IB4GLOpenGLWindow *openGLWindow = NULL;
char *pluginError = NULL;

int penX = 0, penY = 0;

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
		char *detailSrc = "Basic4GL plugin example #4";
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

void Set2D() {

    // Save all OpenGL attributes
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // Setup 2D projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();                     // Save previous projection matrix
    glLoadIdentity();
    glOrtho(0, openGLWindow->Width(), openGLWindow->Height(), 0, 0, 1);

    // Clear any transformations
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();                     // Save previous model view matrix
    glLoadIdentity();
}
void RestoreOpenGLState() {

    // Restore projection matrix
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    // Restore transformation matrix
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    // Restore OpenGL attributes
    glPopAttrib();
}

/// Draw a line, using pixel coordinates
void DLLFUNC Func_MoveTo(IDLL_Basic4GL_Runtime &basic4gl) {
    penX = basic4gl.GetIntParam(2);
    penY = basic4gl.GetIntParam(1);
}
void DLLFUNC Func_LineTo(IDLL_Basic4GL_Runtime &basic4gl) {
    int newX = basic4gl.GetIntParam(2);
    int newY = basic4gl.GetIntParam(1);

    // Set 2D mode first
    Set2D();

    // Draw the line from old pen position to new pen position
    glBegin(GL_LINES);
        glVertex2i(penX, penY);
        glVertex2i(newX, newY);
    glEnd();

    // Restore OpenGL state
    RestoreOpenGLState();

    // Move pen
    penX = newX;
    penY = newY;
}

///////////////////////////////////////////////////////////////////////////////
//	TestPlugin

bool DLLFUNC TestPlugin::Load(IDLL_Basic4GL_FunctionRegistry &registry, bool isStandaloneExe) {

    // Fetch OpenGL window interface
    openGLWindow = (IB4GLOpenGLWindow *) registry.FetchInterface("IB4GLOpenGLWindow", 1, 0);
    if (openGLWindow == NULL) {
        pluginError = "Could not get IB4GLOpenGLWindow interface to text screen. Is this the right Basic4GL version?";
        return false;
    }

	// Initialise DLL stuff

	// Register shared interfaces

    // Register constants

	// Register runtime functions

	// Move pen position
	registry.RegisterVoidFunction("MoveTo", Func_MoveTo);
	registry.AddParam(DLL_BASIC4GL_INT);
	registry.AddParam(DLL_BASIC4GL_INT);

	registry.RegisterVoidFunction("LineTo", Func_LineTo);
	registry.AddParam(DLL_BASIC4GL_INT);
	registry.AddParam(DLL_BASIC4GL_INT);
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

	// Reset pen position
	penX = 0;
	penY = 0;
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

