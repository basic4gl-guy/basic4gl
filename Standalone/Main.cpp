// kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib

/*	Basic4GL standalone stub.
	Copyright (C) Tom Mulgrew, 2003-2005 (tmulgrew@slingshot.co.nz)

	Contains:
		* Virtual machine
		* Compiled Basic4GL program as op-codes (or will do once it is
		  externally patched in by Basic4GL.)

    The compiled Basic4GL program is stored in the executable's resource section.
*/

#include "TomVM.h"
#include "glTextGrid.h"
#include "TomOpenGLBasicLib.h"
#include "glBasicLib_gl.h"
#include "glbasiclib_glu.h"
#include "TomStdBasicLib.h"
#include "TomWindowsBasicLib.h"
#include "TomTextBasicLib.h"
#include "TomJoystickBasicLib.h"
#include "TomTrigBasicLib.h"
#include "TomFileIOBasicLib.h"
#include "TomNetBasicLib.h"
#include "TomCompilerBasicLib.h"
#include "glSpriteEngine.h"
#include "EmbeddedFiles.h"
#include "Standalone.h"
#include "ResourceIDs.h"
#include "TomSoundBasicLib.h"
#include "PluginDLL.h"
#include "DebuggerInterfaces.h"
#include "AppSettings.h"
#include "glheaders.h"
#include "GlfwMouse.h"

#include <stdlib.h>

using namespace std;

// Virtual machine requires a debugger object.
// We don't do any actual debugging, so can pass it a dummy object.
class DummyDebugger : public IVMDebugger {
    virtual int UserBreakPtCount();
    virtual int UserBreakPtLine(int index);
};
int DummyDebugger::UserBreakPtCount()           { return 0; }
int DummyDebugger::UserBreakPtLine(int index)   { return 0; }

// AppSettings
class StandaloneAppSettings : public IAppSettings {
public:
    virtual bool IsSandboxMode();
	virtual int Syntax();
} standaloneAppSettings;

// Standalone applications are NOT sandboxed.
bool StandaloneAppSettings::IsSandboxMode() { return false; }
int StandaloneAppSettings::Syntax() { return 1; }		// TODO: Should this be passed through in header?

IAppSettings* GetAppSettings() {
	return &standaloneAppSettings;
}

// A VMHostApplication implementation is required for executing runtime compiled code (in TomCompilerBasicLib).
// This may be cleaned up later, in which case this class will not be necessary.
class GlfwVMHostApplication : public VMHostApplication {
	GlfwWindowManager& windowManager;
    bool escapeKeyQuits;
public:
	GlfwVMHostApplication(GlfwWindowManager& windowManager, bool _escapeKeyQuits)
        : windowManager(windowManager), escapeKeyQuits(_escapeKeyQuits) 
	{ }

    bool ProcessMessages() override;
};

bool GlfwVMHostApplication::ProcessMessages() {
	if (!windowManager.IsWindowCreated()) return false;

	// Poll events
	glfwPollEvents();

	// Check for close
	return glfwWindowShouldClose(windowManager.GetGlfwWindow()) && escapeKeyQuits;
}

bool DoExecuteVMSteps(TomVM& vm, int numSteps)
{
	__try {
		vm.Continue(numSteps);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		switch (GetExceptionCode()) {

			// Skip mathematics errors (overflows, divide by 0 etc).
			// This is quite important!, as some OpenGL drivers will trigger
			// divide-by-zero and other conditions if geometry happens to
			// be aligned in certain ways. The appropriate behaviour is to
			// ignore these errors, and keep running, and NOT to stop the
			// program!
		case EXCEPTION_FLT_DENORMAL_OPERAND:
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		case EXCEPTION_FLT_INEXACT_RESULT:
		case EXCEPTION_FLT_INVALID_OPERATION:
		case EXCEPTION_FLT_OVERFLOW:
		case EXCEPTION_FLT_STACK_CHECK:
		case EXCEPTION_FLT_UNDERFLOW:
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
		case EXCEPTION_INT_OVERFLOW:
			vm.SkipInstruction();
			break;

			// All other exceptions will stop the program.
		default:
			return false;
		}
	}

	return true;
}

void ExecuteVMSteps(TomVM& vm, int numSteps)
{
	if (!DoExecuteVMSteps(vm, numSteps))
	{
		vm.MiscError("An exception occurred");
	}
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow) {

    // Get the current directory
	char currentDir[1024];
	GetCurrentDirectory(1024, currentDir);

	// Allocate objects
    PluginDLLManager    dlls(currentDir, true);
    DummyDebugger       debugger;
    TomVM               vm(dlls, debugger);					// Virtual machine
	TomBasicCompiler    comp(vm, dlls);
	FileOpener			files;

    ////////////////////////////////////////////////////////////////////////////
    // Unpack program data

	// Extract the Basic4GL data from resource
	HRSRC resInfo = FindResource (NULL, MAKEINTRESOURCE (IDR_BASIC4GLDATA), "BIN");
	if (resInfo == NULL) {
		MessageBox (NULL, "Missing program data", "Program integrity error", MB_OK);
		return 0;
	}
	HGLOBAL resHandle = LoadResource (NULL, resInfo);
	if (resHandle == NULL) {
		MessageBox (NULL, "Missing program data", "Program integrity error", MB_OK);
		return 0;
	}
	void *basic4GLData = LockResource (resHandle);
	if (basic4GLData == NULL) {
		MessageBox (NULL, "Missing program data", "Program integrity error", MB_OK);
		return 0;
	}

    // Copy data into header
    char* ptr = (char*)basic4GLData;
    StandaloneHeader header;
    memcpy(&header, ptr, sizeof(header));
    ptr += sizeof(header);

    // Copy compiler and vm data into stream
    int len;
    memcpy(&len, ptr, sizeof(len));
    ptr += sizeof(len);
//#ifdef _MSC_VER
//  	strstream stream(ptr, len);
//#else
	stringstream stream;
	stream.write(ptr, len);
//#endif
    ptr += len;

    // Remaining data is embedded files
    files.AddFiles(ptr);

    ////////////////////////////////////////////////////////////////////////////
    // Startup
    //
    // The order is important:
    // Files must be initialized first (so that charset.png is available to
    // OpenGL window).
    // OpenGL window must be created next, so that it is available to built-in
    // libraries.
    // Built-in libraries must be initialized next as they create objects that
    // plugin DLLs use.
    // Then the compiler/virtual machine state can be streamed in (which also
    // automatically loads the plugin DLLs).

    // Verify version
    if (header.version != STANDALONE_VERSION) {
        MessageBox(NULL, "Incorrect version in program header", "Program integrity error", MB_OK);
        return 0;
    }

	// Create internal components
	GlfwWindowManager windowManager;
	if (windowManager.Error())
	{
		MessageBox(
			nullptr,
			windowManager.GetError().c_str(),
			"ERROR!",
			MB_OK);
		return 0;
	}
	Content2DManager contentManager(windowManager);
	GlfwKeyboard keyboard(windowManager);
	GlfwMouse mouse(windowManager);
	glTextGrid glText("charset.png", &files, 25, 40, 16, 16);	// Setup a text grid for the window
	if (glText.Error()) {
		MessageBox(
			nullptr,
			glText.GetError().c_str(),
			"ERROR!",
			MB_OK);
		return 0;
	}

	// Setup OpenGL window
	OpenGLWindowParams windowParams;
	if (header.useDesktopRes)
	{
		windowParams.width = 0;
		windowParams.height = 0;
	}
	else
	{
		windowParams.width = header.screenWidth;
		windowParams.height = header.screenHeight;
	}

	windowParams.isFullscreen = header.fullScreen;

	if (header.fullScreen && header.colourDepth == 1)
	{
		windowParams.bpp = 16;
	}
	else if (header.fullScreen && header.colourDepth == 2)
	{
		windowParams.bpp = 32;
	}
	else
	{
		windowParams.bpp = 0;
	}

	windowParams.isBordered = header.border;
	windowParams.isResizable = header.allowResizing;
	windowParams.isStencilBufferRequired = header.stencil;
	windowParams.title = header.title;
	
	windowManager.pendingParams = windowParams;

    // Create object to represent application
    GlfwVMHostApplication app(windowManager, header.escKeyQuits);

	// Initialise function and constant libraries
    InitTomStdBasicLib(comp);							// Standard library
    InitTomWindowsBasicLib(comp, &files, &windowManager);		// Windows specific library
    InitTomOpenGLBasicLib(comp, &files, &windowManager);		// OpenGL
    InitTomTextBasicLib(comp, &glText, &windowManager, &contentManager, &keyboard, &mouse);
    InitGLBasicLib_gl(comp);
    InitGLBasicLib_glu(comp);
	InitTomJoystickBasicLib(comp, &keyboard);
    InitTomTrigBasicLib(comp);							// Trigonometry library
    InitTomFileIOBasicLib(comp, &files);				// File I/O library
	InitTomNetBasicLib(comp);
    InitTomSoundBasicLib(comp, &files);                 // Sound engine
    InitTomCompilerBasicLib(comp, &app, &files);

    // Load in compiler/vm state
    if (!comp.StreamIn(stream)) {
        MessageBox(
                NULL,
                ((string)"Error loading program state: " + comp.GetError()).c_str(),
                "Program integrity error",
                MB_OK);
        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Initialize program

	// Header determines whether we create the actual window now or wait until
	// UpdateWindow() is executed in the Basic4GL code.
	if (header.startupWindowOption == 0)
	{
		windowManager.RecreateWindow();
	}

	if (windowManager.Error()) {
		MessageBox(
			nullptr,
			windowManager.GetError().c_str(),
			"ERROR!",
			MB_OK);
		return 0;
	}

	// Reset virtual machine
    vm.Reset();
	if (!dlls.ProgramStart()) {
		windowManager.DestroyWindow();
		MessageBox(nullptr, dlls.Error().c_str(), "Error", MB_OK);
		return 0;
	}

	// Load command line arguments
	vector<string> arguments;
	for (int i = 1; i < __argc; i++) 
		arguments.push_back(__argv[i]);
    SetProgramArguments(arguments);	

    ////////////////////////////////////////////////////////////////////////////
    // Run program
    do {

		// Execute maximum of 1000 virtual machine Op-codes
		ExecuteVMSteps(vm, 1000);

        // Process windows messages (to keep application responsive)
		if (vm.CanWaitForEvents())
			glfwWaitEvents();
		glfwPollEvents();
        dlls.ProcessMessages();

    } while (!vm.Error () && !vm.Done () && !(windowManager.IsCloseRequested() && header.escKeyQuits));
    dlls.ProgramEnd();

	if (vm.Error ()) {

		// Runtime error encountered.
		// Perform whatever error logic was requested in header file.
		if (header.fullScreen)
			windowManager.DestroyWindow();

		if (header.errorOption == 1)
	
			// Show generic error message
			MessageBox (nullptr, "An error has occured.", "ERROR!", MB_OK);
		else if (header.errorOption == 2) {

			// Show specific error message
			int line, col;
			vm.GetIPInSourceCode (line, col);
			MessageBox (nullptr, ((string) "Runtime error: " + vm.GetError () + ", line " + IntToString (line + 1) + ", col " + IntToString (col + 1)).c_str (), "ERROR!", MB_OK);
		}
		// Otherwise just exit
	}
	else if (vm.Done ()) {
		
		// Program completed normally
		// Perform whatever close down logic was requested in header file.
		// (Wait for key, just close e.t.c)

		if (windowManager.IsWindowCreated()) {
			// Invalid case, fullscreen + wait for window to be closed.		
			if (header.fullScreen && header.finishedOption == 2)
				header.finishedOption = 1;			// Replace with wait for keypress

			if (header.finishedOption == 1) {
				// Wait for keypress or close window
				glfwPollEvents();
				while (keyboard.GetNextKey() != 0 && !windowManager.IsCloseRequested()) { glfwWaitEvents(); glfwPollEvents(); }
				while (keyboard.GetNextKey() == 0 && !windowManager.IsCloseRequested()) { glfwWaitEvents(); glfwPollEvents(); }
			}
			else if (header.finishedOption == 2) {
				// Wait for window to be closed
				while (!windowManager.IsCloseRequested()) {
					glfwWaitEvents();
					glfwPollEvents();
				}
			}
			// Otherwise just exit
		}
	}	

//	CleanupTomNetBasicLib ();
    CleanupTomSoundBasicLib();
	return 0;
}

