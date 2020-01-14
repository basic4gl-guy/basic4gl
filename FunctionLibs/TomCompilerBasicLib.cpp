/*  Created 17-Apr-06: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
    Copyright (C) Thomas Mulgrew

    Functions for compiling and executing code at runtime.
*/
#pragma hdrstop
#include "TomCompilerBasicLib.h"
#include "Basic4GLStandardObjects.h"
#include <windows.h>

#ifndef _MSC_VER
#pragma package(smart_init)
#endif

using namespace std;

////////////////////////////////////////////////////////////////////////////////
//  CompilerPluginAdapter
//
/// Exposes the compiler and virtual machine to plugins via the IB4GLCompiler
/// interface.
class CompilerPluginAdapter : public IB4GLCompiler {
private:
    int errorLine, errorCol;
    string errorText;

    void ClearError();
public:

    // IB4GLCompiler interface
    virtual int DLLFUNC Compile(const char *sourceText);
    virtual void DLLFUNC GetErrorText(char *buffer, int bufferLen);
    virtual int DLLFUNC GetErrorLine();
    virtual int DLLFUNC GetErrorCol();
    virtual bool DLLFUNC Execute(int codeHandle);
};

// Globals
static TomBasicCompiler *comp   = NULL;
static VMHostApplication *host  = NULL;
static FileOpener       *files  = NULL;
CompilerPluginAdapter compilerAdapter;

static std::string                  error = "";
int                                 errorLine = 0, errorCol = 0;
static std::vector<unsigned int>    runtimeRoutines;

////////////////////////////////////////////////////////////////////////////////
//  Helper routines
inline void ClearError() {
    error = "";
    errorLine   = 0;
    errorCol    = 0;
}

////////////////////////////////////////////////////////////////////////////////
//  Init function
static void Init (TomVM& vm) {
    runtimeRoutines.clear();
    runtimeRoutines.push_back(0);
    ClearError();
}

////////////////////////////////////////////////////////////////////////////////
//  Runtime functions

int DoNewCompile(TomVM& vm, std::string filename) {
    // Comp routine internal implementation.
    // Note: "Comp" and "Exec" replace the deprecated "Compile" and "Execute" routines
    // (which are still maintained for backwards compatibility).
    // The main differences are that:
    //  * Comp adds an OP_RETURN to the end of the code, rather than an OP_END.
    //  * Exec is a built-in keyword that effectively GOSUBs to the code.
    compRollbackPoint rollbackPoint = comp->GetRollbackPoint();
    comp->ClearError();
    unsigned int saveIP = vm.IP();

    // Create hook for builtin/DLL function callbacks.
    // This consists of a GOSUB call to the code to be executed, followed by an
    // END CALLBACK op-code to trigger the return to the calling function.
    vm.AddInstruction(vmInstruction(OP_CALL, VTP_INT, vmValue((int)vm.InstructionCount() + 2)));    // Add 2 to call the code after these 2 op-codes
    vm.AddInstruction(vmInstruction(OP_END_CALLBACK, VTP_INT, vmValue()));

    int codeBlock = 0;
    if (comp->CompileOntoEnd()) {

        // Replace OP_END with OP_RETURN
        assert(vm.InstructionCount() > 0);
        assert(vm.Instruction(vm.InstructionCount() - 1).m_opCode == OP_END);
        vm.Instruction(vm.InstructionCount() - 1) = vmInstruction(OP_RETURN, VTP_INT, vmValue(), 0, 0);

		// Write filename into new code block
		auto& block = comp->CurrentCodeBlock();
		block.SetFilename(filename);

        // No error
        ClearError();

        // Return code block index
		codeBlock = comp->CurrentCodeBlockIndex();
    }
    else {

        // Set error
        error       = comp->GetError();
        errorLine   = comp->Line();
        errorCol    = comp->Col();

        // Rollback compiler and virtual machine state
        comp->Rollback(rollbackPoint);
    }

    // Restore IP
    vm.GotoInstruction(saveIP);

    return codeBlock;
}

bool CheckForFunctions(TomVM& vm) {

    // Compile and execute cannot be used in programs that have functions/subs.
    // This is because errors during execute don't stop the program. The main
    // program can continue executing.
    // However functions/subs break this as the stack is too hard to clean up
    // correctly (with proper deallocation of evaluation stack and strings etc).

    // Therefore we simply disallow 'compile' and 'execute' for programs with
    // functions/subs.
    // This should be a non breaking change, as existing code that uses compile/
    // execute will be written in earlier Basic4GL versions, before functions
    // and subs were introduced.

    // The new commands 'comp' and 'exec' can be used instead.

    if (!vm.UserFunctionPrototypes().empty()) {
        vm.FunctionError("'Compile' and 'Execute' cannot be used in programs that have functions/subs. Use 'Comp' and 'Exec' instead");
        return false;
    }
    else
        return true;
}

void DoOldCompile(TomVM& vm) {

    // Not allowed in programs with functions/subs (see note in CheckForFunctions)
    if (!CheckForFunctions(vm))
        return;

    // Compiled code will be added to end of program
    unsigned int offset = vm.InstructionCount();

    // Attempt to compile text and append to end of existing program
    comp->ClearError();
    unsigned int saveIP = vm.IP();      // Compiler can set IP back to 0, so we need to preserve it explicitly
    if (comp->CompileOntoEnd()) {

        // No error
        ClearError();

        // Register this code block and return handle
        vm.Reg().IntVal() = runtimeRoutines.size();
        runtimeRoutines.push_back(offset);
    }
    else {

        // Set error
        error       = comp->GetError();
        errorLine   = comp->Line();
        errorCol    = comp->Col();

        // Return 0
        vm.Reg().IntVal() = 0;
    }

    // Restore IP
    vm.GotoInstruction(saveIP);
}

void DoCompile(TomVM& vm, bool useOldMethod, string filename) {
    if (useOldMethod)
        DoOldCompile(vm);
    else
        vm.Reg().IntVal() = DoNewCompile(vm, filename);
}

void DoCompileText(TomVM& vm, std::string text, bool useOldMethod) {
    // Load it into compiler
    comp->Parser().SourceCode().clear();
    comp->Parser().SourceCode().push_back(text);

    // Compile it
    DoCompile(vm, useOldMethod, "");
}

void WrapCompile(TomVM& vm) {
    DoCompileText(vm, vm.GetStringParam(1), true);
}

void WrapCompile2(TomVM& vm) {
    // Set compiler symbol prefix
    std::string oldPrefix = comp->SymbolPrefix();
    comp->SymbolPrefix() = comp->SymbolPrefix() + vm.GetStringParam(1);

    // Compile text
    DoCompileText(vm, vm.GetStringParam(2), true);

    // Restore symbol prefix
    comp->SymbolPrefix() = oldPrefix;
}

void DoCompileList(TomVM& vm, int index, bool useOldMethod) {

    // Find array size
    int arraySize = vm.Data().Data()[index].IntVal();
    index += 2;

    // Load array into compiler
    comp->Parser().SourceCode().clear();
    for (int i = 0; i < arraySize; i++) {

        // Find string index
        int stringIndex = vm.Data().Data()[index].IntVal();

        // Find text
        std::string text = vm.GetString(stringIndex);

        // Add to parser text
        comp->Parser().SourceCode().push_back(text);

        // Next line
        index++;
    }

    // Compile it
    DoCompile(vm, useOldMethod, "");
}

void WrapCompileList(TomVM& vm) {
    DoCompileList(vm, vm.GetIntParam(1), true);
}

void WrapCompileList2(TomVM& vm) {

    // Set compiler symbol prefix
    std::string oldPrefix = comp->SymbolPrefix();
    comp->SymbolPrefix() = comp->SymbolPrefix() + vm.GetStringParam(1);

    // Compile list
    DoCompileList(vm, vm.GetIntParam(2), true);

    // Restore symbol prefix
    comp->SymbolPrefix() = oldPrefix;
}

void DoCompileFile(TomVM& vm, std::string filename, bool useOldMethod) {
    ClearError();

    // Attempt to open file
    int length;
    GenericIStream *file = files->OpenRead(filename, false, length);
    if (file == NULL) {
        error = files->GetError();
        vm.Reg().IntVal() = 0;
    }
    else {

        // Read file into parser
        comp->Parser().SourceCode().clear();
        char buffer[4096];
        while (!file->eof()) {
            file->getline(buffer, 4095);
            comp->Parser().SourceCode().push_back((std::string) buffer);
        }
        delete file;

        // Compile it
        DoCompile(vm, useOldMethod, filename);
    }
}

void WrapCompileFile(TomVM& vm) {
    DoCompileFile(vm, vm.GetStringParam(1), true);
}

void WrapCompileFile2(TomVM& vm) {

    // Set compiler symbol prefix
    std::string oldPrefix = comp->SymbolPrefix();
    comp->SymbolPrefix() = comp->SymbolPrefix() + vm.GetStringParam(1);

    // Compile list
    DoCompileFile(vm, vm.GetStringParam(2), true);

    // Restore symbol prefix
    comp->SymbolPrefix() = oldPrefix;
}

// Move __try __except out of InternalExecute() function to prevent error C2712
int DoContinue(TomVM& vm)
{
	__try {
		vm.Continue(1000);
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
			return 1;
		default:
			return 2;
		}
	}

	return 0;
}

void InternalExecute(TomVM& vm, int offset, bool isCallback = false) {

    // Move IP to offset
    unsigned int saveIP = vm.IP();          // (Save current IP)
    vm.GotoInstruction(offset);

    // Run code in virtual machine
    do {
		switch (DoContinue(vm))
		{
		case 1:
			// Ignore floating point errors
			vm.SkipInstruction();
			break;
		case 2:
			// All other errors stop the program
			vm.MiscError("An exception occurred");
			break;
		}
    } while (       !vm.Error()
                &&  !vm.Done()
                &&  (isCallback || !vm.Paused())            // Note: User cannot pause inside a callback
                &&  !(isCallback && vm.IsEndCallback())
                &&  host->ProcessMessages());

    // Restore IP
    vm.GotoInstruction(saveIP);
}

void WrapExecute(TomVM& vm) {

    // Not allowed in programs with functions/subs (see note in CheckForFunctions)
    if (!CheckForFunctions(vm))
        return;

    ClearError();
    int result = -1;

    // Get code handle
    int handle = vm.GetIntParam(1);

    // Validate it
    if (handle < 0 || handle >= runtimeRoutines.size()) {
        error = "Invalid handle";
        vm.Reg().IntVal() = 0;
    }
    else if (handle > 0) {

        // Find code to execute
        unsigned int offset = runtimeRoutines[handle];

        // Run code in virtual machine
        InternalExecute(vm, offset);

        // Copy error text
        if (vm.Error()) {

            // Copy error to error variables
            error = vm.GetError();
            vm.GetIPInSourceCode (errorLine, errorCol);

            // Clear error from virtual machine, so that parent program can keep
            // on running.
            vm.ClearError();
            
            result = 0;
        }
    }
    vm.Reg().IntVal() = result;
}

void WrapCompilerError(TomVM& vm) {
    vm.RegString() = error;
}

void WrapCompilerErrorLine(TomVM& vm) {
    vm.Reg().IntVal() = errorLine;
}

void WrapCompilerErrorCol(TomVM& vm) {
    vm.Reg().IntVal() = errorCol;
}

////////////////////////////////////////////////////////////////////////////////
// New runtime compilation methods

void WrapComp(TomVM& vm) {
    DoCompileText(vm, vm.GetStringParam(1), false);
}

void WrapComp2(TomVM& vm) {
    // Set compiler symbol prefix
    std::string oldPrefix = comp->SymbolPrefix();
    comp->SymbolPrefix() = comp->SymbolPrefix() + vm.GetStringParam(1);

    // Compile text
    DoCompileText(vm, vm.GetStringParam(2), false);

    // Restore symbol prefix
    comp->SymbolPrefix() = oldPrefix;
}

void WrapCompList(TomVM& vm) {
    DoCompileList(vm, vm.GetIntParam(1), false);
}

void WrapCompList2(TomVM& vm) {

    // Set compiler symbol prefix
    std::string oldPrefix = comp->SymbolPrefix();
    comp->SymbolPrefix() = comp->SymbolPrefix() + vm.GetStringParam(1);

    // Compile list
    DoCompileList(vm, vm.GetIntParam(2), false);

    // Restore symbol prefix
    comp->SymbolPrefix() = oldPrefix;
}

void WrapCompFile(TomVM& vm) {
    DoCompileFile(vm, vm.GetStringParam(1), false);
}

void WrapCompFile2(TomVM& vm) {

    // Set compiler symbol prefix
    std::string oldPrefix = comp->SymbolPrefix();
    comp->SymbolPrefix() = comp->SymbolPrefix() + vm.GetStringParam(1);

    // Compile list
    DoCompileFile(vm, vm.GetStringParam(2), false);

    // Restore symbol prefix
    comp->SymbolPrefix() = oldPrefix;
}

void WrapGetFunctionByName(TomVM& vm)
{
	string name = LowerCase(vm.GetStringParam(1));

	// Lookup function
	auto index = comp->GlobalUserFunctionIndex();
	auto i = index.find(name);

	// Return function number + 1 if found, or 0 if not.
	// (0 is reserved to mean "null" function pointer. Adding 1 to the function number
	// allows us to distinguish null from a legitimate reference to function # 0)
	vm.Reg().IntVal() = i != index.cend() 
		? i->second + 1 
		: 0;
}

void WrapGetFunctionByName2(TomVM& vm)
{
	string name = LowerCase(vm.GetStringParam(2));
	int codeBlockIndex = vm.GetIntParam(1);

	// Lookup code block
	if (!vm.IsCodeBlockValid(codeBlockIndex))
	{
		vm.FunctionError("Invalid code block index");
		return;
	}
	vmCodeBlock& codeBlock = vm.GetCodeBlock(codeBlockIndex);

	// Lookup function
	const auto& index = codeBlock.userFunctions;
	auto i = index.find(name);

	// Return function number + 1 if found, or 0 if not.
	// (0 is reserved to mean "null" function pointer. Adding 1 to the function number
	// allows us to distinguish null from a legitimate reference to function # 0)
	vm.Reg().IntVal() = i != index.cend() 
		? i->second + 1 
		: 0;
}

void WrapGetCodeBlockByName(TomVM& vm)
{
	string filename = PrepPathForComp(vm.GetStringParam(1));

	// Look for matching code block
	for (int i = 1; vm.IsCodeBlockValid(i); i++) {
		if (vm.GetCodeBlock(i).FilenameEquals(filename)) {
			vm.Reg().IntVal() = i;
			return;
		}
	}

	vm.Reg().IntVal() = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Initialisation

void InitTomCompilerBasicLib(
    TomBasicCompiler& _comp,
    VMHostApplication *_host,
    FileOpener *_files) {

    // Save pointer to compiler and window
    comp    = &_comp;
    host    = _host;
    files   = _files;

    // Hookup and register compiler plugin adapter
    comp->Plugins().RegisterInterface(
        static_cast<IB4GLCompiler*>(&compilerAdapter),
        "IB4GLCompiler",
        IB4GLCOMPILER_MAJOR,
        IB4GLCOMPILER_MINOR,
        NULL);

    // Register initialisation function
    comp->VM().AddInitFunc(Init);

    // Register functions
    comp->AddFunction("compile",            WrapCompile,            compParamTypeList() << VTP_STRING,                                          true,   true,   VTP_INT,    true);
    comp->AddFunction("compile",            WrapCompile2,           compParamTypeList() << VTP_STRING << VTP_STRING,                            true,   true,   VTP_INT,    true);
    comp->AddFunction("compile",            WrapCompileList,        compParamTypeList() << vmValType(VTP_STRING, 1, 1, true),                   true,   true,   VTP_INT,    true);
    comp->AddFunction("compile",            WrapCompileList2,       compParamTypeList() << vmValType(VTP_STRING, 1, 1, true) << VTP_STRING,     true,   true,   VTP_INT,    true);
    comp->AddFunction("compilefile",        WrapCompileFile,        compParamTypeList() << VTP_STRING,                                          true,   true,   VTP_INT,    true);
    comp->AddFunction("compilefile",        WrapCompileFile2,       compParamTypeList() << VTP_STRING << VTP_STRING,                            true,   true,   VTP_INT,    true);
    comp->AddFunction("execute",            WrapExecute,            compParamTypeList() << VTP_INT,                                             true,   true,   VTP_INT,    true);
    comp->AddFunction("compilererror",      WrapCompilerError,      compParamTypeList(),                                                        true,   true,   VTP_STRING);
    comp->AddFunction("compilererrorline",  WrapCompilerErrorLine,  compParamTypeList(),                                                        true,   true,   VTP_INT);
    comp->AddFunction("compilererrorcol",   WrapCompilerErrorCol,   compParamTypeList(),                                                        true,   true,   VTP_INT);
    comp->AddFunction("comp",               WrapComp,               compParamTypeList() << VTP_STRING,                                          true,   true,   VTP_INT,    true);
    comp->AddFunction("comp",               WrapComp2,              compParamTypeList() << VTP_STRING << VTP_STRING,                            true,   true,   VTP_INT,    true);
    comp->AddFunction("comp",               WrapCompList,           compParamTypeList() << vmValType(VTP_STRING, 1, 1, true),                   true,   true,   VTP_INT,    true);
    comp->AddFunction("comp",               WrapCompList2,          compParamTypeList() << vmValType(VTP_STRING, 1, 1, true) << VTP_STRING,     true,   true,   VTP_INT,    true);
    comp->AddFunction("compfile",           WrapCompFile,           compParamTypeList() << VTP_STRING,                                          true,   true,   VTP_INT,    true);
    comp->AddFunction("compfile",           WrapCompFile2,          compParamTypeList() << VTP_STRING << VTP_STRING,                            true,   true,   VTP_INT,    true);
	comp->AddFunction("getfunctionbyname",	WrapGetFunctionByName,	compParamTypeList() << VTP_STRING,											true,	true,	VTP_UNTYPED_FUNC_PTR, false);
	comp->AddFunction("getfunctionbyname",	WrapGetFunctionByName2, compParamTypeList() << VTP_STRING << VTP_INT,								true,	true,	VTP_UNTYPED_FUNC_PTR, false);
	comp->AddFunction("getcodeblockbyname", WrapGetCodeBlockByName, compParamTypeList() << VTP_STRING,											true,   true,   VTP_INT);
}

////////////////////////////////////////////////////////////////////////////////
//  CompilerPluginAdapter

void CompilerPluginAdapter::ClearError() {
    errorText = "";
    errorLine = 0;
    errorCol = 0;
}

int DLLFUNC CompilerPluginAdapter::Compile(const char *sourceText) {
    assert(comp != NULL);
    TomVM& vm = comp->VM();

    // Load source text into compiler
    comp->Parser().SourceCode().clear();
    comp->Parser().SourceCode().push_back(sourceText);

    // Compile it
    return DoNewCompile(vm, "");
}

void DLLFUNC CompilerPluginAdapter::GetErrorText(char *buffer, int bufferLen) {

    // Truncate string to fit if necessary
    string text = errorText;
    if (text.length() > bufferLen - 1)
        text.resize(bufferLen - 1);

    // Copy string text
    strcpy(buffer, text.c_str());
}

int DLLFUNC CompilerPluginAdapter::GetErrorLine() {
    return errorLine;
}

int DLLFUNC CompilerPluginAdapter::GetErrorCol() {
    return errorCol;
}

bool DLLFUNC CompilerPluginAdapter::Execute(int codeHandle) {
    TomVM& vm = comp->VM();

    // Check code handle is valid
    if (codeHandle == 0 || !vm.IsCodeBlockValid(codeHandle)) {
        vm.FunctionError("Invalid code handle");
        return false;
    }

    // Save stack as if a sub is being called.
    // This is because we could be in a builtin/plugin function that is in the
    // middle of an expression, where temp data is saved to the stack.
    // Builtin/plugin functions don't normally protect the existing stack, so
    // there may be unprotected temp data that the callback code could trample.
    int stackTop, tempDataLock;
    vm.Data().SaveState(stackTop, tempDataLock);

    // Find code to execute
    // 2 op-codes earlier will be the callback hook.
    int offset = vm.GetCodeBlockOffset(codeHandle) - 2;

    // Execute code
    InternalExecute(vm, offset, true); 

    // Check for error/end program
    if (vm.Error() || vm.Done())
        return false;

    // Restore stack
    vm.Data().RestoreState(stackTop, tempDataLock, false);

    return true;
}

