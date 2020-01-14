//---------------------------------------------------------------------------
/*  Created 1-Sep-2003: Thomas Mulgrew

    'TomVM' Virtual Machine.
*/

#ifndef TomVMH
#define TomVMH
//---------------------------------------------------------------------------
#include "vmVariables.h"
#include "vmCode.h"
#include "HasErrorState.h"
#include "vmFunction.h"
#include "DebuggerInterfaces.h"
#include "PluginLibrary.h"
#include "vmStackFrame.h"
#include "vmCodeBlock.h"

#define VM_MAXUSERSTACKCALLS    10000   // 10,000 user function stack calls
#define VM_MAXDATA          100000000   // 100,000,000 variables (.4 gig of memory)
#define VM_MAXSTACK            250000   // First 250,000 (1 meg) reserved for stack/temp data space
#define VM_DATATOSTRINGMAXCHARS   800   // Any more gets annoying...

////////////////////////////////////////////////////////////////////////////////
// VM state
//
// Used to save virtual machine state.
// (Debugger uses this to run small sections of code, without interrupting the
// main program.)

struct vmState {

    // Instruction pointer
    unsigned int ip;

    // Registers
    vmValue         reg, reg2;
    std::string     regString, reg2String;

    // Stacks
    unsigned int    stackTop, /*callStackTop, */userFuncStackTop;
    int             currentUserFrame;

    // Top of program
    unsigned int    codeSize;
    unsigned int    codeBlockCount;

    // Variable data
    int             stackDataTop, tempDataLock;

	// Current long running function
	IDLL_Basic4GL_LongRunningFunction* longRunningFn;

    // Error state
    bool            error;
    std::string     errorString;

    // Other state
    bool            paused;
};

struct vmRollbackPoint {
    
    // Registered code blocks
    int codeBlockCount;
    int boundCodeBlock;

    // User function prototypes
    int functionPrototypeCount;

    // User functions
    int functionCount;

    // Data statements
    int dataCount;

    // Program statements
    int instructionCount;
};

class IVMLongRunningFnDoneNotified
{
public:
	virtual void LongRunningFnDone(bool cancelled) = 0;
};

////////////////////////////////////////////////////////////////////////////////
// TomVM
//
// Virtual machine

class TomVM : public HasErrorState {

    ////////////////////////////////////
    // Data

    // Variables, data and data types
    vmTypeLibrary                       m_dataTypes;
    vmData                              m_data;
    vmVariables                         m_variables;
	std::vector<vmString>               m_stringConstants;  // Constant strings declared in program
    vmStore<vmString>                   m_strings;
    std::list<vmResources *>            m_resources;
    std::vector<vmUserFuncPrototype>    m_userFunctionPrototypes;
    std::vector<vmUserFunc>             m_userFunctions;

    // Program data
    vmProgramData                       m_programData;          // General purpose program data (e.g declared with "DATA" keyword in BASIC)
    unsigned int                        m_programDataOffset;

    // Registers
    vmValue                             m_reg,                  // Register values (when int or float)
                                        m_reg2;
    std::string                         m_regString,            // Register values when string
                                        m_reg2String;
    int                                 m_currentUserFrame;
        // The current active frame.
        // Note that this is often but NOT ALWAYS the top of the stack.
        // (When evaluating parameters for a impending function call, the pending
        // function stack frame is the top of the stack, but is not yet active).

    // Runtime stacks
    vmValueStack                        m_stack;                // Used for expression evaluation
    std::vector<vmUserFuncStackFrame>   m_userCallStack;        // Call stack for user functions

    // Individual code blocks
    std::vector<vmCodeBlock>            m_codeBlocks;
    int                                 m_boundCodeBlock;

    // Data destruction
    std::vector<vmStackDestructor>      m_stackDestructors;
    std::vector<vmStackDestructor>      m_tempDestructors;

    // Plugin DLLs
    PluginManager&                      m_plugins;
    IDLL_Basic4GL_Runtime               *m_pluginRuntime;

    // Debugger
    IVMDebugger&                        m_debugger;

    ////////////////////////////////////
    // Code

    // Instructions
    std::vector<vmInstruction>  m_code;
    vmValTypeSet                m_typeSet;

    // Instruction pointer
    unsigned int                m_ip;

    // Debugging
    vmPatchedBreakPtList        m_patchedBreakPts;      // Patched in breakpoints
    vmTempBreakPtList           m_tempBreakPts;         // Temporary breakpoints, generated for stepping over a line

    bool                        m_paused,               // Set to true when program hits a breakpoint. (Or can be set by caller.)
                                m_breakPtsPatched;      // Set to true if breakpoints are patched and in synchronisation with compiled code

	// Long running function
	IDLL_Basic4GL_LongRunningFunction*	m_longRunningFn;
	IVMLongRunningFnDoneNotified*		m_longRunningFnDoneNotified;

	// Conditional timeshare break
	bool					m_timeshare;				// Set to true 

    // Internal methods
    void BlockCopy          (int sourceIndex, int destIndex, int size);
    void CopyStructure      (int sourceIndex, int destIndex, vmValType& type);
    void CopyArray          (int sourceIndex, int destIndex, vmValType& type);
    void CopyField          (int sourceIndex, int destIndex, vmValType& type);
    bool CopyData           (int sourceIndex, int destIndex, vmValType type);
    bool CopyToParam        (int sourceIndex, vmValType& type);
    bool MoveToTemp         (int sourceIndex, vmValType& type);
    void DuplicateStrings   (int dataIndex, vmValType& type);
    bool PopArrayDimensions (vmValType& type);
    bool ValidateTypeSize   (vmValType& type);
    bool ValidateTypeSizeForStack(vmValType& type);
    bool ReadProgramData    (vmBasicValType type);
    void PatchInBreakPt (unsigned int offset);
    void InternalPatchOut ();
    void InternalPatchIn ();
    void PatchOut () {
        if (m_breakPtsPatched)
            InternalPatchOut ();
    }
    unsigned int CalcBreakPtOffset (unsigned int line);
    void Deref (vmValue& val, vmValType& type);
    int StoredDataSize(int sourceIndex, vmValType& type);
    bool CheckPointers(int index, vmValType type, int destIndex);
    bool CheckPointer(int dest, int ptr);
    void UnwindTemp();
    void UnwindTemp(vmProtectedStackRange protect);
    void UnwindStack(int newTop);
    void DestroyData(vmStackDestructor& d, vmProtectedStackRange protect);
    void DestroyData(int index, vmValType type, vmProtectedStackRange protect);
	void CancelLongRunningFn();

public:
    TomVM(  PluginManager& plugins,
            IVMDebugger& debugger,
            int maxDataSize = VM_MAXDATA,
            int maxStackSize = VM_MAXSTACK);

    // General
    void New ();                                        // New program
    void Clr ();                                        // Clear variables
    void Reset ();
    void Continue (unsigned int steps = 0xffffffff);    // Continue execution from last position
    bool Done () const {
        assert(IPValid());
        return m_longRunningFn == nullptr				// Not waiting for long running function to complete
			&& m_code [m_ip].m_opCode == OP_END;        // Reached end of program?
    }
    void GetIPInSourceCode (int& line, int& col) const {
        assert(IPValid());
        line    = m_code [m_ip].m_sourceLine;
        col     = m_code [m_ip].m_sourceChar;
    }

    // Code blocks
    int NewCodeBlock();
    void BindCodeBlock(int index) { m_boundCodeBlock = index; }
    int GetBoundCodeBlock() { return m_boundCodeBlock; }
	vmCodeBlock& GetCodeBlock(int index)
    {
		assert(IsCodeBlockValid(index));
		return m_codeBlocks[index];
    }

	// External functions
    std::vector<vmFunction>     m_functions;
        // m_functions are standard functions where the parameters are pushed
        // to the stack.

    // Initialisation functions
    std::vector<vmFunction>     m_initFunctions;

    // IP and registers
    unsigned int    IP ()           { return m_ip; }            
    vmValue&        Reg ()          { return m_reg; }
    vmValue&        Reg2 ()         { return m_reg2; }
    std::string&    RegString ()    { return m_regString; }
    std::string&    Reg2String ()   { return m_reg2String; }
    vmValueStack&   Stack ()        { return m_stack; }

    // Variables, data and data types
    vmTypeLibrary&  DataTypes ()    { return m_dataTypes; }
    vmData&         Data ()         { return m_data; }
    vmVariables&    Variables ()    { return m_variables; }
    vmProgramData&  ProgramData ()  { return m_programData; }

    // User functions
    std::vector<vmUserFuncPrototype>& UserFunctionPrototypes() { return m_userFunctionPrototypes; }
    std::vector<vmUserFunc>& UserFunctions() { return m_userFunctions; }
    std::vector<vmUserFuncStackFrame>& UserCallStack() { return m_userCallStack; }
    int CurrentUserFrame() { return m_currentUserFrame; }
	void TimeshareBreakRequired() { m_timeshare = true; }

    // Debugging
    bool            Paused ()           { return m_paused; }
    void            Pause ()            { m_paused = true; }
    bool            BreakPtsPatched ()  { return m_breakPtsPatched; }

    void ClearTempBreakPts () {
        PatchOut ();
        m_tempBreakPts.clear ();
    }
    void PatchIn () {
        if (!m_breakPtsPatched)
            InternalPatchIn ();
    }
    void RepatchBreakpts() {
        PatchOut();
        PatchIn();
    }
    void AddStepBreakPts (bool stepInto);               // Add temporary breakpoints to catch execution after stepping over the current line
    bool AddStepOutBreakPt ();                          // Add breakpoint to step out of gosub
    vmState GetState ();
    void SetState (vmState& state);
    void GotoInstruction (unsigned int offset) {
        assert(OffsetValid(offset));
        m_ip = offset;
    }
    bool SkipInstruction() {                            // USE WITH CARE!!!
        if (m_ip < InstructionCount() + 1) {
            m_ip++;
            return true;
        }
        else
            return false;
    }
    std::string RegToString (vmValType& type);
    bool OffsetValid(int offset) const {
        return offset >= 0 && (unsigned)offset < InstructionCount();
    }
    bool IPValid() const {
        return OffsetValid(m_ip);
    }

    // Building raw VM instructions
    unsigned int InstructionCount () const  { return m_code.size (); }
    void AddInstruction (vmInstruction i)   { PatchOut (); m_code.push_back (i); }
    void RollbackProgram (int size) {
        assert (size >= 0);
        assert ((unsigned)size <= InstructionCount());
        while ((unsigned)size < InstructionCount ())
            m_code.pop_back ();
    }
    vmInstruction& Instruction (unsigned int index)  {
        assert (index < m_code.size ());
        PatchOut ();
        return m_code [index];
    }
    vmInstruction const& Instruction (unsigned int index) const  {
        assert (index < m_code.size ());
		assert(!m_breakPtsPatched);			// Doesn't work if breakpoints have been patched in. Because this is a const fn we can't automatically patch them out.
        return m_code [index];
    }
    void RemoveLastInstruction () {
        m_code.pop_back ();
    }
    int StoreType (vmValType& type)         { return m_typeSet.GetIndex (type); }
    vmValType& GetStoredType (int index)    { return m_typeSet.GetValType (index); }

    // Program data
    void StoreProgramData (vmBasicValType t, vmValue v) {
        vmProgramDataElement d;
        d.Type () = t;
        d.Value () = v;
        m_programData.push_back (d);
    }

    // Constant strings
    int StoreStringConstant (vmString str);
    std::vector<vmString>& StringConstants () { return m_stringConstants; }

    // External functions
    int AddFunction (vmFunction func)   {
        int result = FunctionCount ();
        m_functions.push_back (func);
        return result;
    }
    int FunctionCount ()                { return m_functions.size (); }

    // Called by external functions
    vmValue& GetParam (int index) {
        // Read param from param stack.
        // Index 1 is TOS
        // Index 2 is TOS - 1
        // ...
        assert (index > 0);
        assert (index <= m_stack.Size ());
        return m_stack [m_stack.Size () - index];
    }
    vmInt GetIntParam (int index)                   { return GetParam (index).IntVal ();  }
    vmReal GetRealParam (int index)                 { return GetParam (index).RealVal (); }
    std::string& GetStringParam (int index)         { return m_strings.Value (GetIntParam (index)); }
    std::string& GetString(int index)               { return m_strings.Value(index); }
    int AllocString()                               { return m_strings.Alloc(); }
    int StoredStrings()                             { return m_strings.StoredElements(); }
    vmStore<vmString>& Strings()                    { return m_strings; }

    // Reference params (called by external functions)
    bool CheckNullRefParam (int index) {

        // Return true if param is not a null reference
        // Otherwise return false and set a virtual machine error
        bool result = GetIntParam (index) > 0;
        if (!result)
            FunctionError ("Unset pointer");
        return result;
    }
    vmValue& GetRefParam (int index) {

        // Get reference parameter.
        // Returns a reference to the actual vmValue object
        int ptr = GetIntParam (index);
        assert (ptr > 0);
        assert (m_data.IndexValid (ptr));
        return m_data.Data () [ptr];
    }

    void FunctionError (std::string name)       { SetError ((std::string) "Function error: " + name); }
    void MiscError (std::string name)           { SetError (name); }

    // Initialisation functions
    void AddInitFunc (vmFunction func) { m_initFunctions.push_back (func); }

    // Resources
    void AddResources (vmResources& resources) { m_resources.push_back (&resources); }
    void ClearResources ();

    // Displaying data
    std::string BasicValToString (vmValue val, vmBasicValType type, bool constant);
    std::string ValToString (vmValue val, vmValType type, int& maxChars);
    std::string VarToString (vmVariable& v, int maxChars);
    std::string DataToString(int dataIndex, vmValType type, int maxChars);

    // User functions
    vmUserFunc& AddUserFunction(int& index);

    // Plugin DLLs
    PluginManager& Plugins() { return m_plugins; }
    PluginManager const& Plugins() const { return m_plugins; }

    // Rollback after compile error
    vmRollbackPoint GetRollbackPoint();
    void Rollback(vmRollbackPoint rollbackPoint);

    // Codeblock access
    bool IsCodeBlockValid(int index);
    int GetCodeBlockOffset(int index);

    // Builtin/plugin function callback support
    bool IsEndCallback() {
        assert(IPValid());
        return m_code [m_ip].m_opCode == OP_END_CALLBACK;        // Reached end callback opcode?
    }

	// Long running functions
	void BeginLongRunningFn(IDLL_Basic4GL_LongRunningFunction* handler);
	void EndLongRunningFunction();
	bool IsInLongRunningFn() { return m_longRunningFn != nullptr; }
	bool CanWaitForEvents()
	{
		// If VM is waiting for a long running function that does not require polling,
		// then application can block and wait for next event before calling Continue() again.
		return m_longRunningFn != nullptr && !m_longRunningFn->IsPolled();
	}
	void SetLongRunningFnDoneNotified(IVMLongRunningFnDoneNotified* notified)
	{
		m_longRunningFnDoneNotified = notified;
	}

    // Streaming
#ifdef VM_STATE_STREAMING
    void StreamOut(std::ostream& stream) const;
    bool StreamIn(std::istream& stream);
#endif
};

#endif
