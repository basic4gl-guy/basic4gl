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
#include "TomNetBasicLib.h"
#include "TomSoundBasicLib.h"
#include "TomCompilerBasicLib.h"
#include "compPreprocessor.h"
#include "DiskFile.h"
#include "AppSettings.h"
#include "PluginDLL.h"
#include "VMStateExport.h"
#include <fstream>

using namespace std;

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
    virtual bool IsSandboxMode() override;
	int Syntax() override;
};

bool DummyAppSettings::IsSandboxMode() {
	return false;
}

int DummyAppSettings::Syntax()
{
	return 1;
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

// Main source file type.
// Has extra methods for extracting the plugin DLL information at the start of
// the file. The remaining file can then be passed into the preprocessor and
// compiler.
class MainSourceFile : public ISourceFile {
    ISourceFile* file;
    string line;
    bool eof;
    int lineNumber;

    void NextLine();
public:
    MainSourceFile(ISourceFile* _file);

    // ISourceFile
    virtual std::string GetNextLine();
    virtual std::string Filename();
    virtual int LineNumber();
    virtual bool Eof();
    virtual void Release();

    // Plugin detection
    // Returns the name of the next plugin to load and moves to the next line.
    // If line does not contain a plugin, returns ""
    string GetPlugin();
};

MainSourceFile::MainSourceFile(ISourceFile* _file) : file(_file) {

    // Extract first line
    NextLine();
}

void MainSourceFile::NextLine() {
    eof = file->Eof();
    if (!eof) {
        lineNumber = file->LineNumber();
        line = file->GetNextLine();
    }
}

std::string MainSourceFile::GetNextLine() {
    if (eof)
        return "";
    else {

        // Return line from buffer
        string result = line;

        // Swap next line into buffer
        NextLine();
        return result;
    }
}

std::string MainSourceFile::Filename() {
    return file->Filename();
}

int MainSourceFile::LineNumber() {
    return lineNumber;
}

bool MainSourceFile::Eof() {
    return eof;
}

void MainSourceFile::Release() {
    file->Release();
    delete this;
}

string MainSourceFile::GetPlugin() {

    // End of file?
    if (eof)
        return "";

    // Not a plugin line?
    if (line.substr(0, 8) != "#plugin ")
        return "";

    // Get plugin name
    string result = line.substr(8, line.length() - 8);

    // Skip line
    NextLine();
    return result;
}

void ReportError(string text, string errorFile) {
    if (errorFile == "")
        MessageBox(NULL, text.c_str(), "B4GL", MB_OK);
    else {
        ofstream file(errorFile.c_str());
        file << text.c_str() << endl;
    }
}

void ReportProgramError(std::string text, int line, int col, compPreprocessor& preprocessor, string errorFile) {
	// Lookup file and row in file
	std::string filename;
	int fileLine;
	preprocessor.LineNumberMap().SourceFromMain(filename, fileLine, line);

	// Report error
    string fullText = IntToString(col + 1) + ":" + IntToString(fileLine + 1) + ":\"" + filename + "\" " + text;

    ReportError(fullText, errorFile);
	//MessageBox(NULL, (text + ", in file " + filename + ", line " + IntToString(fileLine + 1) + ", col " + IntToString(col + 1)).c_str(), "ERROR!", MB_OK);
}

vector<string> GetArguments(string s) {
    vector<string> result;
    int p = 0;
    bool inString = false;
    while (p < s.length()) {

        string arg = "";

        // Skip whitespace
        while (p < s.length() && s[p] <= ' ')
            p++;

        // Quoted?
        if (p < s.length() && s[p] == '"') {

            // Read quoted string
            p++;
            while (p < s.length() && s[p] != '"') {
                arg += s[p];
                p++;
            }

            // Skip end quote
            if (p < s.length())
                p++;
        }
        // Non-quoted
        else {
            // Read up to next whitespace
            while (p < s.length() && s[p] != '"' && s[p] > ' ') {
                arg += s[p];
                p++;
            }
        }

        if (arg != "")
            result.push_back(arg);
    }

    return result;
}

//int APIENTRY WinMain(HINSTANCE hInstance,
//                     HINSTANCE hPrevInstance,
//                     LPSTR     lpCmdLine,
//                     int       nCmdShow)
void CompileAndRun(std::string commandLine)
{
	char buffer [4096];

    // Parse command line
    vector<string> args = GetArguments(commandLine);
    bool windowed = false;
    int width = 640;
    int height = 480;
    int bpp = 16;
    bool stencil = false;
    bool resizeable = false;
    string errorFile = "";
	bool createVMStateFile = false;

    int p = 0;
    bool foundArg = false;
    do {
        foundArg = false;
        if (p < args.size()) {
            string arg = LowerCase(args[p]);
            if (arg == "-window") {
                windowed = true;
                foundArg = true;
            }
            else if (arg == "-stencil") {
                stencil = true;
                foundArg = true;
            }
            else if (arg == "-resize") {
                resizeable = true;
                foundArg = true;
            }
            else if (arg == "-width") {
                p++;
                if (p < args.size()) {
                    width = StringToInt(args[p]);
                    foundArg = true;
                }
            }
            else if (arg == "-height") {
                p++;
                if (p < args.size()) {
                    height = StringToInt(args[p]);
                    foundArg = true;
                }
            }
            else if (arg == "-bpp") {
                p++;
                if (p < args.size()) {
                    bpp = StringToInt(args[p]);
                    foundArg = true;
                }
            }
            else if (arg == "-e") {
                p++;
                if (p < args.size()) {
                    errorFile = args[p];
                    foundArg = true;
                }
            }
			else if (arg == "-o")
			{
				createVMStateFile = true;
				foundArg = true;
			}
            else if (arg == "-?" || arg == "-help" || arg == "-h") {
                MessageBox(
                    NULL,
"Usage: b4gl [-options] program.gb [arguments]\n\
Options:\n\
  -window\t\tWindowed mode\n\
  -width XXX\tWidth in pixels\n\
  -height XXX\tHeight in pixels\n\
  -bpp XXX\tBits per pixel\n\
  -resize\t\tAllow window resizing\n\
  -stencil\t\tEnable stencil buffer\n\
  -e FILENAME\tWrite errors to FILENAME\n\
  -o\t\tOutput virtual machine state file 'program.vm'\n\
  \n\
  [arguments] are passed to BASIC program as arguments",
                    "B4GL",
                    MB_OK);
                return;
            }
        }
        if (foundArg)
            p++;
    } while(foundArg);

    if (p >= args.size()) {
        ReportError("Invalid command line arguments. Use b4gl -h for help.", errorFile);
        return;
    }

    // Extract program filename
    string programFilename = args[p];

	std::string directory = GetExeDirectory();

	// Allocate objects
	PluginDLLManagerForStateExport plugins("", true);
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

	DiskFileServer* fileServer = new DiskFileServer();					// Note: The preprocessor takes ownership of the fileserver object, and will destroy it when destroyed itself.
	compPreprocessor preprocessor(1, fileServer);
	ISourceFile* file = fileServer->OpenSourceFile(programFilename);
	if (file == NULL) {
        ReportError("Failed to open: " + programFilename, errorFile);
//		MessageBox (NULL, "Failed to open input file\r\nUsage: B4GL filename.gb", "Error opening file", MB_OK);
		return;
	}

    // Use MainSourceFile wrapper class to extract plugin information from file
    MainSourceFile* mainFile = new MainSourceFile(file);
    string plugin = mainFile->GetPlugin();
    while (plugin != "") {
        // Look for plugin DLL in current directory
        if (!plugins.LoadDLL(plugin)) {

            // If that fails, try again in same directory as exe
            if (!plugins.LoadDLL(directory + "\\" + plugin)) {
                ReportError(plugins.Error().c_str(), errorFile);
//                MessageBox(NULL, plugins.Error().c_str(), "ERROR!", MB_OK);
                mainFile->Release();
                return;
            }
        }
        plugin = mainFile->GetPlugin();
    }

    // Pass remainder of main file through to pre-processor, parser and compiler.
	bool result = preprocessor.Preprocess(mainFile, comp.Parser());
	if (!result) {
		ReportProgramError(
            (std::string)"Preprocess error: " + preprocessor.GetError(),
            comp.Parser().SourceCode().size() - 1,
            0,
            preprocessor,
            errorFile);
		return;
	}

	// Create OpenGL window
    glTextGridWindow	glWin (	!windowed,					// Fullscreen mode
								true,						// Frame around window (windowed mode only)
								width,						// Width
								height,						// Height
								bpp, 						// Bpp (fullscreen mode only, 0 = same as desktop, otherwise 16 or 32)
								stencil,					// No stencil
								programFilename,			// Window title
								resizeable,					// Allow resizing
								false,						// Fit to work area
								RGM_RECREATEWINDOW);		// Not used. How to REcreate OpenGL context. But we're only initialising OpenGL once before exiting, so not important.

	// Initialise text and sprite engine
    glSpriteEngine glText((char*)(directory + "\\charset.png").c_str(), &files, 25, 40, 16, 16);	// Setup a text grid for the window
    glWin.SetTextGrid (&glText);
	if (glText.Error ()) {
        ReportError(glText.GetError().c_str(), errorFile);
//		MessageBox (glWin.GetHWND (), glText.GetError ().c_str (), "ERROR!", MB_OK);
		return;
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

    // Set program arguments
    vector<string> programArgs;
    for (unsigned int i = p + 1; i < args.size(); i++)
        programArgs.push_back(args[i]);
    SetProgramArguments(programArgs);

	// Compile program
	comp.ClearError ();
	comp.Compile ();
    if (comp.Error ()) {
		ReportProgramError(
            (std::string)"Compile error: " + comp.GetError(),
            comp.Line(),
            comp.Col(),
            preprocessor,
            errorFile);	
//		MessageBox (glWin.GetHWND (), ((std::string) "Compile error: " + comp.GetError () + ", line " + IntToString (comp.Line () + 1) + ", col " + IntToString (comp.Col () + 1)).c_str (), "ERROR!", MB_OK);
		return;
	}

	// Output virtual machine state to file
	if (createVMStateFile)
	{
		glWin.Hide();
		try {
			ExportVMState("program.vm", vm, comp);
		}
		catch (exception& ex)
		{
			ReportError(ex.what(), errorFile);
		}
		return;
	}
	
	// Activate window
	glWin.Activate ();
	glWin.OpenGLDefaults ();
	
	if (!plugins.ProgramStart()) {
		glWin.Hide();
		ReportError(plugins.Error(), errorFile);
		return;
	}

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
		ReportProgramError(
            (std::string)"Runtime error: " + vm.GetError(),
            line,
            col,
            preprocessor,
            errorFile);			
//		MessageBox (glWin.GetHWND (), ((std::string) vm.GetError () + ", line " + IntToString (line + 1) + ", col " + IntToString (col + 1)).c_str (), "Runtime error", MB_OK);
	}
	else if (vm.Done ()) {

		// Wait for a keypress (or explicit window close)
		glWin.ProcessWindowsMessages(); 
		while (glWin.GetKey () != 0 && !glWin.Closing ()) glWin.ProcessWindowsMessages();
		while (glWin.GetKey () == 0 && !glWin.Closing ()) glWin.ProcessWindowsMessages();
	}
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow) {
    CompileAndRun(lpCmdLine);
    return 0;
}
