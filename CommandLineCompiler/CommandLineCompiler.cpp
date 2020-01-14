/*	Main.cpp

	Copyright (C) 2004, Tom Mulgrew (tmulgrew@slingshot.co.nz)

	Basic4GL command line compiler build.

	Usage:
		B4GL programname.gb

	Note:
		May be useful for those who wish to build the Basic4GL source on Windows
		but don't have access to Borland C++ Builder.

		This program should run all Basic4GL programs that the official released
		version of Basic4GL runs.

		There is no user interface in this version however.
*/

#include "TomComp.h"
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
#include "glSpriteEngine.h"
#include "EmbeddedFiles.h"
#include "Standalone.h"
#include "TomNetBasicLib.h"
#include "TomSoundBasicLib.h"
#include "TomCompilerBasicLib.h"
#include "compPreprocessor.h"
#include "DiskFile.h"
#include "AppSettings.h"

#ifdef _DEBUG
#define FULLSCREEN false
#else 
#define FULLSCREEN true
#endif

class DummyVMDebugger : public IVMDebugger {
public:
    virtual int UserBreakPtCount();
    virtual int UserBreakPtLine(int index);
};

int DummyVMDebugger::UserBreakPtCount() {
	return 0;
}

int DummyVMDebugger::UserBreakPtLine(int index) {
	return 0;
}

class DummyAppSettings : public IAppSettings {
public:
    virtual bool IsSandboxMode();
};

bool DummyAppSettings::IsSandboxMode() {
	return false;
}

DummyAppSettings dummyAppSettings;

IAppSettings* GetAppSettings() {
    return &dummyAppSettings;
}

std::string GetExeDirectory() {

	// Get module filename
	char buf[MAX_PATH + 1];
	GetModuleFileName(NULL, buf, MAX_PATH);
	std::string path = buf;
	
	// Trim filename
	int i = path.size() - 1;
	while (i >= 0 && path[i] != '\\' && path[i] != '/')
		i--;
	path = path.substr(0, i);	
	
	return path;
}

class Application : public VMHostApplication {
    glTextGridWindow *window;
    bool escapeKeyQuits;
public:
    Application(glTextGridWindow *_window, bool _escapeKeyQuits)
        : window(_window), escapeKeyQuits(_escapeKeyQuits) { ; }

    virtual bool ProcessMessages();
};

bool Application::ProcessMessages() {
    window->ProcessWindowsMessages();
    return !(window->Closing() && escapeKeyQuits);
}

void ReportProgramError(std::string text, int line, int col, compPreprocessor& preprocessor) {
	// Lookup file and row in file
	std::string filename;
	int fileLine;
	preprocessor.LineNumberMap().SourceFromMain(filename, fileLine, line);
	
	// Report error
	MessageBox(NULL, (text + ", line " + IntToString(fileLine + 1) + ", col " + IntToString(col + 1)).c_str(), "ERROR!", MB_OK);	
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	char buffer [4096];

	std::string directory = GetExeDirectory();

	// Allocate objects
	PluginDLLManager	plugins("", true);
	DummyVMDebugger		debugger;
    TomVM               vm(plugins, debugger);				// Virtual machine		
    TomBasicCompiler    comp(vm, plugins);					// Compiler
	FileOpener			files;

	// Load program into compiler
//	std::ifstream file (lpCmdLine);
//	if (file.fail ()) {
//		MessageBox (NULL, "Failed to open input file\r\nUsage: B4GL filename.gb", "Error opening file", MB_OK);
//		return 0;
//	}
	
	DiskFileServer fileServer;
	compPreprocessor preprocessor(1, &fileServer);
	ISourceFile* file = fileServer.OpenSourceFile(lpCmdLine);
	if (file == NULL) {
		MessageBox (NULL, "Failed to open input file\r\nUsage: B4GL filename.gb", "Error opening file", MB_OK);
		return 0;
	}
	bool result = preprocessor.Preprocess(file, comp.Parser());
	file->Release();	
	if (!result) {
		ReportProgramError((std::string)"Preprocess error: " + preprocessor.GetError(), comp.Parser().SourceCode().size() - 1, 0, preprocessor);
		return 0;
	}
	
	// Create OpenGL window	
    glTextGridWindow	glWin (	FULLSCREEN,					// Fullscreen mode
								true,						// Frame around window (windowed mode only)
								640,						// Width
								480,						// Height
								0,							// Bpp (fullscreen mode only, 0 = same as desktop, otherwise 16 or 32)
								false,						// No stencil
								lpCmdLine,					// Window title
								false,						// Allow resizing
								false,						// Fit to work area
								RGM_RECREATEWINDOW);		// Not used. How to REcreate OpenGL context. But we're only initialising OpenGL once before exiting, so not important.

	// Initialise text and sprite engine
    glSpriteEngine glText((char*)(directory + "\\charset.png").c_str(), &files, 25, 40, 16, 16);	// Setup a text grid for the window
    glWin.SetTextGrid (&glText);									
	if (glText.Error ()) {
		MessageBox (glWin.GetHWND (), glText.GetError ().c_str (), "ERROR!", MB_OK);
		return 0;
	}

    // Create object to represent application
    Application app(&glWin, true);

	// Initialise function libraries
    InitTomStdBasicLib(comp);							// Standard library
    InitTomWindowsBasicLib(comp, &files);				// Windows specific library
    InitTomOpenGLBasicLib(comp, &glWin, &files);		// OpenGL
    InitTomTextBasicLib(comp, &glWin, &glText);
    InitGLBasicLib_gl(comp);
    InitGLBasicLib_glu(comp);
	InitTomJoystickBasicLib(comp, &glWin);
    InitTomTrigBasicLib(comp);							// Trigonometry library
    InitTomFileIOBasicLib(comp, &files);				// File I/O library
	InitTomNetBasicLib(comp);
    InitTomSoundBasicLib(comp, &files);                 // Audiere based sound engine
    InitTomCompilerBasicLib(comp, &app, &files);

	// Compile program
	comp.ClearError ();
	comp.Compile ();
    if (comp.Error ()) {
		ReportProgramError((std::string)"Compile error: " + comp.GetError(), comp.Line(), comp.Col(), preprocessor);	
//		MessageBox (glWin.GetHWND (), ((std::string) "Compile error: " + comp.GetError () + ", line " + IntToString (comp.Line () + 1) + ", col " + IntToString (comp.Col () + 1)).c_str (), "ERROR!", MB_OK);
		return 0;
	}
	
	// Activate window
	glWin.Activate ();
	glWin.OpenGLDefaults ();

	// Reset virtual machine
	vm.Reset ();
	try {
		do {

			// Run VM for a number of steps
			vm.Continue (TC_STEPSBETWEENREFRESH);

			// Process windows messages to keep application responsive
			glWin.ProcessWindowsMessages ();
		} while (!vm.Error () && !vm.Done () && !glWin.Closing ());
	}
	catch (...) {
		vm.MiscError ("An exception occured");
	}

	if (vm.Error ()) {

		// Runtime program error

		// Switch back from fullscreen mode (if fullscreen)
		if (glWin.FullScreen ())
			glWin.Hide ();

		// Get error position
		int line, col;
		vm.GetIPInSourceCode (line, col);

		// Display runtime error
		ReportProgramError((std::string)"Runtime error: " + vm.GetError(), line, col, preprocessor);			
//		MessageBox (glWin.GetHWND (), ((std::string) vm.GetError () + ", line " + IntToString (line + 1) + ", col " + IntToString (col + 1)).c_str (), "Runtime error", MB_OK);
	}
	else if (vm.Done ()) {

		// Wait for a keypress (or explicit window close)
		glWin.ProcessWindowsMessages(); 
		while (glWin.GetKey () != 0 && !glWin.Closing ()) glWin.ProcessWindowsMessages();
		while (glWin.GetKey () == 0 && !glWin.Closing ()) glWin.ProcessWindowsMessages();
	}

	// Finished
	return 0;
}
