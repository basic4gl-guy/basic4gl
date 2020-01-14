#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Basic4GLDLLInterface.h"
#include "Basic4GLStandardObjects.h"
#include "TestConsole.h"

ITextConsole *console = NULL;
char *pluginError = NULL;
IB4GLCompiler* compiler = NULL;

///////////////////////////////////////////////////////////////////////////////
//	Plugin
//
///	Main object representing the plugin to Basic4GL
class Plugin : public IDLL_Basic4GL_Plugin {
public:
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
		char *detailSrc = "Basic4GL plugin example #2 - Also requires TestDLL.dll";
		int i;
		for (i = 0; detailSrc[i] != 0; i++)
			details[i] = detailSrc[i];
		*major = 1;
		*minor = 0;
		return BASIC4GL_DLL_VERSION;
	}

	// Main initialisation function
	DLLExport IDLL_Basic4GL_Plugin *DLLFUNC Basic4GL_Init() {
		return new Plugin();
	}
}

///////////////////////////////////////////////////////////////////////////////
// Runtime functions

void DLLFUNC Func_TextRect(IDLL_Basic4GL_Runtime &basic4gl) {
	int x  = basic4gl.GetIntParam(4),
		y  = basic4gl.GetIntParam(3),
		xs = basic4gl.GetIntParam(2),
		ys = basic4gl.GetIntParam(1);
	char c[2];
	c[0] = *basic4gl.GetStringParam(5);
	c[1] = 0;
	if (c[0] < ' ')
		return;

	for (int cy = y; cy < y + ys; cy++) {
		for (int cx = x; cx < x + xs; cx++) {
			console->Locate(cx, cy);
			console->Print(c);
		}
	}
}

void DLLFUNC Func_Callback(IDLL_Basic4GL_Runtime& basic4gl) {
	const char *text = basic4gl.GetStringParam(2);
	int count = basic4gl.GetIntParam(1);

	// Compile text
	int handle = compiler->Compile(text);
	if (handle == 0)
		return;					// Compiler error

	// Execute it
	for (int i = 0; i < count; i++) {
		if (!compiler->Execute(handle))
			return;				// Execution error
	}
}

///////////////////////////////////////////////////////////////////////////////
//	Plugin

bool DLLFUNC Plugin::Load(IDLL_Basic4GL_FunctionRegistry &registry, bool isStandaloneExe) {

	// Initialise DLL stuff
	console = (ITextConsole*) registry.FetchInterface("TextConsole", 1, 0);
	if (console == NULL) {
		pluginError = "Cannot find text console. You need to plugin TestDLL.dll first!";
		return false;
	}

	compiler = (IB4GLCompiler*) registry.FetchInterface("IB4GLCompiler", IB4GLCOMPILER_MAJOR, IB4GLCOMPILER_MINOR);
	if (compiler == NULL) {
		pluginError = "Cannot find compiler object.";
		return false;
	}

	// Register runtime functions
	registry.RegisterVoidFunction("TextRect", Func_TextRect);
	registry.AddParam(DLL_BASIC4GL_STRING);
	registry.AddParam(DLL_BASIC4GL_INT);
	registry.AddParam(DLL_BASIC4GL_INT);
	registry.AddParam(DLL_BASIC4GL_INT);
	registry.AddParam(DLL_BASIC4GL_INT);

	registry.RegisterVoidFunction("Callback", Func_Callback);
	registry.AddParam(DLL_BASIC4GL_STRING);
	registry.AddParam(DLL_BASIC4GL_INT);

	return true;
}

void DLLFUNC Plugin::Unload() {
	delete this;
}

bool DLLFUNC Plugin::Start() {
	return true;
}

void DLLFUNC Plugin::End() {

}

void DLLFUNC Plugin::GetError(char *error) {
	if (pluginError != NULL)
		for (int i = 0; pluginError[i] != 0; i++)
			error[i] = pluginError[i];
}

void DLLFUNC Plugin::Pause() {

}

void DLLFUNC Plugin::Resume() {

}

void DLLFUNC Plugin::DelayedResume() {

}

void DLLFUNC Plugin::ProcessMessages() {

    // Called periodically (in practice, quite frequently).
    // This would be a good place to process windows messages etc.
}
