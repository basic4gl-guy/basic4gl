/*  Main.cpp

    Copyright (C) 2004, Tom Mulgrew (tmulgrew@slingshot.co.nz)

    Simplified, cut down Basic4GL compiler
        * Compiles to console application
        * No UI
        * No OpenGL or other libraries
        * Only two functions (print & printr)

	Should be quite portable.
*/

#include "TomComp.h"
#include <cstdlib>
#include <iostream>

using namespace std;

// Test program
char buffer [1024];

// Simple "print" function

void DLLFUNC WrapPrint(IDLL_Basic4GL_Runtime &basic4gl) {

	// Regular print
	cout << basic4gl.GetStringParam(1);
}

void DLLFUNC WrapPrintr(IDLL_Basic4GL_Runtime &basic4gl) {

	// Print with newline
	cout << basic4gl.GetStringParam(1) << endl;
}

void DLLFUNC WrapPrintr2(IDLL_Basic4GL_Runtime &basic4gl) {
	cout << endl;
}

void DLLFUNC WrapInput(IDLL_Basic4GL_Runtime &basic4gl) {
	char text[1024];
	cin >> text;
	basic4gl.SetStringResult(text);
}

void DLLFUNC WrapVal(IDLL_Basic4GL_Runtime &basic4gl) {
	float f;
	std::stringstream(basic4gl.GetStringParam(1)) >> f;
	basic4gl.SetFloatResult(f);
}

class SimpleLibrary : public LocalPlugin {
public:
    virtual bool DLLFUNC Load(IDLL_Basic4GL_FunctionRegistry &registry, bool isStandaloneExe);
};

bool DLLFUNC SimpleLibrary::Load(IDLL_Basic4GL_FunctionRegistry &registry, bool isStandaloneExe) {
	registry.RegisterVoidFunction("print", WrapPrint);
		registry.AddParam(DLL_BASIC4GL_STRING);

	registry.RegisterVoidFunction("printr", WrapPrintr);
		registry.AddParam(DLL_BASIC4GL_STRING);

	registry.RegisterVoidFunction("printr", WrapPrintr2);	

	registry.RegisterFunction("input$", WrapInput, DLL_BASIC4GL_STRING);

	registry.RegisterFunction("val", WrapVal, DLL_BASIC4GL_FLOAT);
		registry.AddParam(DLL_BASIC4GL_STRING);

	return true;
}

// Virtual machine requires a debugger object.
// We don't do any actual debugging, so can pass it a dummy object.
class DummyDebugger : public IVMDebugger {
    virtual int UserBreakPtCount();
    virtual int UserBreakPtLine(int index);
};
int DummyDebugger::UserBreakPtCount()           { return 0; }
int DummyDebugger::UserBreakPtLine(int index)   { return 0; }

int main (void) {

	cout << "TomBasicCore BASIC compiler and virtual machine (C) Tom Mulgrew 2003-2004" << endl;
	cout << "Enter Basic program then hit Ctrl+Z (On Windows)"<< endl;
	cout << "Or pipe a text file into stdin" << endl;

	// Create compiler and virtual machine
	PluginDLLManager	dlls(".", true);
	DummyDebugger		debugger;
	TomVM				vm(dlls, debugger);
	TomBasicCompiler	comp(vm, dlls);
	comp.DefaultSyntax() = LS_TRADITIONAL_PRINT;

	// Register function wrappers
	SimpleLibrary library;
	dlls.Plugin(&library);
    ////comp.AddFunction ("print",  WrapPrint,  compParamTypeList () << VTP_STRING, false, false,  VTP_INT);
    ////comp.AddFunction ("printr", WrapPrintr, compParamTypeList () << VTP_STRING, false, false,  VTP_INT);

	// Read source from cin
	cin.getline (buffer, 1023);
	while (!cin.fail ()) {
		comp.Parser ().SourceCode ().push_back (buffer);
		cin.getline (buffer, 1023);
	};
	cin.clear();

	// Compile program
	comp.ClearError ();
	comp.Compile ();
	if (comp.Error ()) {
		cout << endl << "COMPILE ERROR!: " << comp.GetError ().c_str () << endl;
		return 0;
	}

	// Run program
	vm.Reset ();
	do {

		// Execute 1000 VM op-codes (maximum)
		vm.Continue (1000);

	} while (!(vm.Error () || vm.Done ()));

	// Check for virtual machine error
	if (vm.Error ())
		cout << endl << "RUNTIME ERROR!: " << vm.GetError ().c_str () << endl;
	else
		cout << endl << "Done!" << endl;

    system("PAUSE");
	return 0;
};
