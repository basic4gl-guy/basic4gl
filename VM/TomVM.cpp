//---------------------------------------------------------------------------
/*  Created 1-Sep-2003: Thomas Mulgrew

    'TomVM' Virtual Machine.                          
*/


#pragma hdrstop

#include "TomVM.h"
#include "TomVMDLLAdapter.h"

//---------------------------------------------------------------------------

#ifndef _MSC_VER
#pragma package(smart_init)
#endif

// Constants
std::string streamHeader = "Basic4GL stream";
int         streamVersion = 3;

const char *blankString = "";

////////////////////////////////////////////////////////////////////////////////
// Helper functions
vmTempBreakPt MakeTempBreakPt(int offset) {
    vmTempBreakPt breakPt;
    breakPt.m_offset = offset;
    return breakPt;
}

////////////////////////////////////////////////////////////////////////////////
// TomVM

TomVM::TomVM (
        PluginManager& plugins,
        IVMDebugger& debugger,
        int maxDataSize,
        int maxStackSize)
        :   m_data              (maxDataSize, maxStackSize),
            m_variables         (m_data, m_dataTypes),
            m_strings           (blankString),
            m_stack             (m_strings),
            m_plugins           (plugins),
            m_debugger          (debugger),
			m_longRunningFn     (nullptr),
			m_longRunningFnDoneNotified (nullptr) {

    // Create plugin runtime
    m_pluginRuntime = new TomVMDLLAdapter(*this, m_plugins.StructureManager());

    New ();
}

void TomVM::New () {

	// Cancel any long running function
	CancelLongRunningFn();
	
	// Clear variables, data and data types
    Clr();
    m_variables.Clear();
    m_dataTypes.Clear();
    m_programData.clear();
    m_codeBlocks.clear();

    // Clear string constants
    m_stringConstants.clear();

    // Deallocate code
    m_code.clear();
    m_typeSet.Clear();
    m_userFunctions.clear();
    m_userFunctionPrototypes.clear();
    m_ip = 0;
    m_paused = false;

    // Clear breakpoints
    m_patchedBreakPts.clear();
    m_tempBreakPts.clear();
    m_breakPtsPatched = false;
}

void TomVM::Clr () {
    m_variables.Deallocate ();          // Deallocate variables
    m_data.Clear ();                    // Deallocate variable data
    m_strings.Clear ();                 // Clear strings
    m_stack.Clear ();                   // Clear runtime stacks
    m_userCallStack.clear();
    m_stackDestructors.clear();
    m_tempDestructors.clear();
    m_currentUserFrame = -1;
    m_boundCodeBlock = 0;

    // Clear resources
    ClearResources ();

    // Init registers
    m_reg.IntVal ()     = 0;
    m_reg2.IntVal ()    = 0;
    m_regString         = "";
    m_reg2String        = "";
    m_programDataOffset = 0;
}

void TomVM::ClearResources () {

    // Clear resources
    for (   std::list<vmResources *>::iterator j = m_resources.begin ();
            j != m_resources.end ();
            j++)
        (*j)->Clear ();
}

void TomVM::Reset () {

	// Cancel any long running function
	CancelLongRunningFn();

    // Clear error state
    ClearError ();

    // Deallocate variables
    Clr ();

    // Call registered initialisation functions
    for (unsigned int i = 0; i < m_initFunctions.size (); i++)
        m_initFunctions [i] (*this);

    // Move to start of program
    m_ip = 0;
    m_paused = false;
}

const char  *ErrNotImplemented          = "Opcode not implemented",
			*ErrInvalid                 = "Invalid opcode",
			*ErrUnDIMmedVariable        = "UnDIMmed variable",
			*ErrBadArrayIndex           = "Array index out of range",
			*ErrReDIMmedVariable        = "ReDIMmed variable",
			*ErrBadDeclaration          = "Variable declaration error",
			*ErrVariableTooBig          = "Variable is too big",
			*ErrOutOfMemory             = "Ran out of variable memory",
			*ErrBadOperator             = "Operator cannot be applied to data type",
			*ErrReturnWithoutGosub      = "Return without gosub",
			*ErrStackError              = "Stack error",
			*ErrUnsetPointer            = "Unset pointer",
			*ErrStackOverflow           = "Stack overflow",
			*ErrArraySizeMismatch       = "Array sizes are different",
			*ErrZeroLengthArray         = "Array size must be 0 or greater",
			*ErrOutOfData               = "Out of DATA",
			*ErrDataIsString            = "Expected to READ a number, got a text string instead",
			*ErrRunCalledInsideExecute  = "Cannot execute RUN in dynamic code",
			*ErrUserFuncStackOverflow   = "Ran out of function variable memory",
			*ErrNoValueReturned         = "Function did not return a value",
			*ErrPointerScopeError       = "Pointer scope error",
			*ErrNoRuntimeFunction       = "Runtime function not implemented",
			*ErrInvalidCodeBlock        = "Could not find runtime code to execute",
			*ErrFuncPtrIncompatible		= "Function/sub pointer is incompatible";

void TomVM::Continue (unsigned int steps) {
    ClearError ();
    m_paused = false;
	m_timeshare = false;

	// Handle long running functions
	if (m_longRunningFn != nullptr)
	{
		if (m_longRunningFn->IsPolled())
			m_longRunningFn->Poll();
		return;
	}

    ////////////////////////////////////////////////////////////////////////////
    // Virtual machine main loop
    vmInstruction *instruction;
    unsigned int stepCount = 0;
    vmValue temp;
    unsigned int tempI;

    goto step;

nextStep:

    // Proceed to next instruction
    m_ip++;

step:

    // Count steps
    if (++stepCount > steps)
        return;

    instruction = &m_code [m_ip];
    switch (instruction->m_opCode) {
    case OP_NOP:                goto nextStep;
    case OP_END:                break;
    case OP_LOAD_CONST:

        // Load value
        if (instruction->m_type == VTP_STRING) {
            assert (instruction->m_value.IntVal () >= 0);
            assert ((unsigned)instruction->m_value.IntVal () < m_stringConstants.size ());
            RegString () = m_stringConstants [instruction->m_value.IntVal ()];
        }
        else
            Reg () = instruction->m_value;
        goto nextStep;

    case OP_LOAD_VAR: {

        // Load variable.
        // Instruction contains index of variable.
        assert (m_variables.IndexValid (instruction->m_value.IntVal ()));
        vmVariable& var = m_variables.Variables () [instruction->m_value.IntVal ()];
        if (var.Allocated ()) {
            // Load address of variable's data into register
            m_reg.IntVal () = var.m_dataIndex;
            goto nextStep;
        }
        SetError (ErrUnDIMmedVariable);
        break;
    }

    case OP_LOAD_LOCAL_VAR: {

        // Find current stack frame
        assert(m_currentUserFrame >= 0);
        assert((unsigned)m_currentUserFrame < m_userCallStack.size());
        vmUserFuncStackFrame& currentFrame = m_userCallStack[m_currentUserFrame];

        // Find variable
        int index = instruction->m_value.IntVal();

        // Instruction contains index of variable.
        if (currentFrame.localVarDataOffsets[index] != 0) {
            // Load address of variable's data into register
            m_reg.IntVal() = currentFrame.localVarDataOffsets[index];
            goto nextStep;
        }
        SetError(ErrUnDIMmedVariable);
        break;
    }

    case OP_DEREF: {

        // Dereference reg.
        if (m_reg.IntVal () != 0) {
            assert (m_data.IndexValid (m_reg.IntVal ()));
            vmValue& val = m_data.Data () [m_reg.IntVal ()];        // Find value that reg points to
            switch (instruction->m_type) {
            case VTP_INT:
            case VTP_REAL:
			case VTP_FUNC_PTR:
                m_reg = val;
                goto nextStep;
            case VTP_STRING:
                assert (m_strings.IndexValid (val.IntVal ()));
                m_regString = m_strings.Value (val.IntVal ());
                goto nextStep;
            }
            assert (false);
        }
        SetError (ErrUnsetPointer);
        break;
    }
    case OP_ADD_CONST:
        // Check pointer
        if (m_reg.IntVal () != 0) {
            m_reg.IntVal () += instruction->m_value.IntVal ();
            goto nextStep;
        }
        SetError (ErrUnsetPointer);
        break;

    case OP_ARRAY_INDEX:

        if (m_reg2.IntVal () != 0) {
            // Input:   m_reg2 = Array address
            //          m_reg  = Array index
            // Output:  m_reg  = Element address
            assert (m_data.IndexValid (m_reg2.IntVal ()));
            assert (m_data.IndexValid (m_reg2.IntVal () + 1));

            // m_reg2 points to array header (2 values)
            // First value is highest element (i.e number of elements + 1)
            // Second value is size of array element.
            // Array data immediately follows header
            if (m_reg.IntVal () >= 0 && m_reg.IntVal () < m_data.Data () [m_reg2.IntVal ()].IntVal ()) {
                assert (m_data.Data () [m_reg2.IntVal () + 1].IntVal () >= 0);
                m_reg.IntVal () = m_reg2.IntVal () + 2 + m_reg.IntVal () * m_data.Data () [m_reg2.IntVal () + 1].IntVal ();
                goto nextStep;
            }
            SetError (ErrBadArrayIndex);
            break;
        }
        SetError (ErrUnsetPointer);
        break;

    case OP_PUSH:

        // Push register to stack
        if (instruction->m_type == VTP_STRING)  m_stack.PushString (RegString ());
        else                                    m_stack.Push (Reg ());
        goto nextStep;

    case OP_POP:

        // Pop reg2 from stack
        if (instruction->m_type == VTP_STRING)  m_stack.PopString (Reg2String ());
        else                                    m_stack.Pop (Reg2 ());
        goto nextStep;

    case OP_SAVE: {

        // Save reg into [reg2]
        if (m_reg2.IntVal () > 0) {
            assert (m_data.IndexValid (m_reg2.IntVal ()));
            vmValue& dest = m_data.Data () [m_reg2.IntVal ()];
            switch (instruction->m_type) {
            case VTP_INT:
            case VTP_REAL:
			case VTP_FUNC_PTR:
                dest = m_reg;
                goto nextStep;
            case VTP_STRING:

                // Allocate string space if necessary
                if (dest.IntVal () == 0)
                    dest.IntVal () = m_strings.Alloc ();

                // Copy string value
                m_strings.Value (dest.IntVal ()) = m_regString;
                goto nextStep;
            }
            assert (false);
        }
        SetError (ErrUnsetPointer);
        break;
    }

    case OP_COPY: {

        // Copy data
        if (CopyData (  m_reg.IntVal (),
                        m_reg2.IntVal (),
                        m_typeSet.GetValType (instruction->m_value.IntVal ())))
            goto nextStep;
        else
            break;
    }
    case OP_DECLARE: {

        // Allocate variable.
        assert (m_variables.IndexValid (instruction->m_value.IntVal ()));
        vmVariable& var = m_variables.Variables () [instruction->m_value.IntVal ()];

        // Must not already be allocated
        if (var.Allocated ()) {
            SetError (ErrReDIMmedVariable);
            break;
        }

        // Pop and validate array dimensions sizes into type (if applicable)
        if (var.m_type.PhysicalPointerLevel () == 0
        && !PopArrayDimensions (var.m_type))
            break;

        // Validate type size
        if (!ValidateTypeSize (var.m_type))
            break;

        // Allocate variable
        var.Allocate (m_data, m_dataTypes);
        goto nextStep;
    }
    case OP_DECLARE_LOCAL: {

        // Allocate local variable

        // Find current stack frame
        assert(m_currentUserFrame >= 0);
        assert((unsigned)m_currentUserFrame < m_userCallStack.size());
        vmUserFuncStackFrame& currentFrame = m_userCallStack[m_currentUserFrame];
        vmUserFunc& userFunc = m_userFunctions[currentFrame.userFuncIndex];
        vmUserFuncPrototype& prototype = m_userFunctionPrototypes[userFunc.prototypeIndex];

        // Find variable type
        int index = instruction->m_value.IntVal();
        assert(index >= 0);
        assert((unsigned)index < prototype.localVarTypes.size());
        vmValType& type = prototype.localVarTypes[index];

        // Must not already be allocated
        if (currentFrame.localVarDataOffsets[index] != 0) {
            SetError (ErrReDIMmedVariable);
            break;
        }

        // Pop and validate array dimensions sizes into type (if applicable)
        if (type.PhysicalPointerLevel() == 0 && !PopArrayDimensions(type))
            break;

        // Validate type size
        if (!ValidateTypeSizeForStack(type))
            break;

        // Allocate new data
        int dataIndex = m_data.AllocateStack(m_dataTypes.DataSize(type));

        // Initialise it
        m_data.InitData(dataIndex, type, m_dataTypes);

        // Store data index in stack frame
        currentFrame.localVarDataOffsets[index] = dataIndex;

        // Also store in register, so that OP_REG_DESTRUCTOR can be used
        m_reg.IntVal() = dataIndex;

        goto nextStep;

    }
    case OP_JUMP:

        // Jump
        assert (instruction->m_value.IntVal () >= 0);
        assert ((unsigned)instruction->m_value.IntVal () < m_code.size ());
        m_ip = instruction->m_value.IntVal ();
        goto step;

    case OP_JUMP_TRUE:

        // Jump if reg != 0
        assert (instruction->m_value.IntVal () >= 0);
        assert ((unsigned)instruction->m_value.IntVal () < m_code.size ());
        if (Reg ().IntVal () != 0) {
            m_ip = instruction->m_value.IntVal ();
            goto step;
        }
        goto nextStep;

    case OP_JUMP_FALSE:

        // Jump if reg == 0
        assert (instruction->m_value.IntVal () >= 0);
        assert ((unsigned)instruction->m_value.IntVal () < m_code.size ());
        if (Reg ().IntVal () == 0) {
            m_ip = instruction->m_value.IntVal ();
            goto step;
        }
        goto nextStep;

    case OP_OP_NEG:
        if (instruction->m_type == VTP_INT)         Reg ().IntVal ()  = -Reg ().IntVal ();
        else if (instruction->m_type == VTP_REAL)   Reg ().RealVal () = -Reg ().RealVal ();
        else {
            SetError (ErrBadOperator);
            break;
        }
        goto nextStep;

    case OP_OP_PLUS:
        if (instruction->m_type == VTP_INT)         Reg ().IntVal () += Reg2 ().IntVal ();
        else if (instruction->m_type == VTP_REAL)   Reg ().RealVal () += Reg2 ().RealVal ();
        else if (instruction->m_type == VTP_STRING) RegString () = Reg2String () + RegString ();
        else {
            SetError (ErrBadOperator);
            break;
        }
        goto nextStep;

    case OP_OP_MINUS:
        if (instruction->m_type == VTP_INT)         Reg ().IntVal () = Reg2 ().IntVal () - Reg ().IntVal ();
        else if (instruction->m_type == VTP_REAL)   Reg ().RealVal () = Reg2 ().RealVal () - Reg ().RealVal ();
        else {
            SetError (ErrBadOperator);
            break;
        }
        goto nextStep;

    case OP_OP_TIMES:
        if (instruction->m_type == VTP_INT)         Reg ().IntVal () *= Reg2 ().IntVal ();
        else if (instruction->m_type == VTP_REAL)   Reg ().RealVal () *= Reg2 ().RealVal ();
        else {
            SetError (ErrBadOperator);
            break;
        }
        goto nextStep;

    case OP_OP_DIV:
        if (instruction->m_type == VTP_INT)         Reg ().IntVal () = Reg2 ().IntVal () / Reg ().IntVal ();
        else if (instruction->m_type == VTP_REAL)   Reg ().RealVal () = Reg2 ().RealVal () / Reg ().RealVal ();
        else {
            SetError (ErrBadOperator);
            break;
        }
        goto nextStep;

    case OP_OP_MOD:
        if (instruction->m_type == VTP_INT) {
            vmInt i = Reg2 ().IntVal () % Reg ().IntVal ();
            if (i >= 0) Reg ().IntVal () = i;
            else        Reg ().IntVal () += i;
        }
        else {
            SetError (ErrBadOperator);
            break;
        }
        goto nextStep;

    case OP_OP_NOT:
        if (instruction->m_type == VTP_INT)         Reg ().IntVal () = Reg ().IntVal () == 0 ? -1 : 0;
        else {
            SetError (ErrBadOperator);
            break;
        }
        goto nextStep;

    case OP_OP_EQUAL:
        if (instruction->m_type == VTP_INT)         Reg ().IntVal () = Reg2 ().IntVal () == Reg ().IntVal () ? -1 : 0;
        else if (instruction->m_type == VTP_REAL)   Reg ().IntVal () = Reg2 ().RealVal () == Reg ().RealVal () ? -1 : 0;
        else if (instruction->m_type == VTP_STRING) Reg ().IntVal () = Reg2String () == RegString () ? -1 : 0;
        else {
            SetError (ErrBadOperator);
            break;
        }
        goto nextStep;

    case OP_OP_NOT_EQUAL:
        if (instruction->m_type == VTP_INT)         Reg ().IntVal () = Reg2 ().IntVal () != Reg ().IntVal () ? -1 : 0;
        else if (instruction->m_type == VTP_REAL)   Reg ().IntVal () = Reg2 ().RealVal () != Reg ().RealVal () ? -1 : 0;
        else if (instruction->m_type == VTP_STRING) Reg ().IntVal () = Reg2String () != RegString () ? -1 : 0;
        else {
            SetError (ErrBadOperator);
            break;
        }
        goto nextStep;

    case OP_OP_GREATER:
        if (instruction->m_type == VTP_INT)         Reg ().IntVal () = Reg2 ().IntVal () > Reg ().IntVal () ? -1 : 0;
        else if (instruction->m_type == VTP_REAL)   Reg ().IntVal () = Reg2 ().RealVal () > Reg ().RealVal () ? -1 : 0;
        else if (instruction->m_type == VTP_STRING) Reg ().IntVal () = Reg2String () > RegString () ? -1 : 0;
        else {
            SetError (ErrBadOperator);
            break;
        }
        goto nextStep;

    case OP_OP_GREATER_EQUAL:
        if (instruction->m_type == VTP_INT)         Reg ().IntVal () = Reg2 ().IntVal () >= Reg ().IntVal () ? -1 : 0;
        else if (instruction->m_type == VTP_REAL)   Reg ().IntVal () = Reg2 ().RealVal () >= Reg ().RealVal () ? -1 : 0;
        else if (instruction->m_type == VTP_STRING) Reg ().IntVal () = Reg2String () >= RegString () ? -1 : 0;
        else {
            SetError (ErrBadOperator);
            break;
        }
        goto nextStep;

    case OP_OP_LESS:
        if (instruction->m_type == VTP_INT)         Reg ().IntVal () = Reg2 ().IntVal () < Reg ().IntVal () ? -1 : 0;
        else if (instruction->m_type == VTP_REAL)   Reg ().IntVal () = Reg2 ().RealVal () < Reg ().RealVal () ? -1 : 0;
        else if (instruction->m_type == VTP_STRING) Reg ().IntVal () = Reg2String () < RegString () ? -1 : 0;
        else {
            SetError (ErrBadOperator);
            break;
        }
        goto nextStep;

    case OP_OP_LESS_EQUAL:
        if (instruction->m_type == VTP_INT)         Reg ().IntVal () = Reg2 ().IntVal () <= Reg ().IntVal () ? -1 : 0;
        else if (instruction->m_type == VTP_REAL)   Reg ().IntVal () = Reg2 ().RealVal () <= Reg ().RealVal () ? -1 : 0;
        else if (instruction->m_type == VTP_STRING) Reg ().IntVal () = Reg2String () <= RegString () ? -1 : 0;
        else {
            SetError (ErrBadOperator);
            break;
        }
        goto nextStep;

    case OP_CONV_INT_REAL:
        Reg ().RealVal () = Reg ().IntVal ();
        goto nextStep;

    case OP_CONV_INT_REAL2:
        Reg2 ().RealVal () = Reg2 ().IntVal ();
        goto nextStep;

    case OP_CONV_REAL_INT:
        Reg ().IntVal () = Reg ().RealVal ();
        goto nextStep;

    case OP_CONV_REAL_INT2:
        Reg2 ().IntVal () = Reg2 ().RealVal ();
        goto nextStep;

    case OP_CONV_INT_STRING:
        RegString () = IntToString (Reg ().IntVal ());
        goto nextStep;

    case OP_CONV_REAL_STRING:
        RegString () = RealToString (Reg ().RealVal ());
        goto nextStep;

    case OP_CONV_INT_STRING2:
        Reg2String () = IntToString (Reg2 ().IntVal ());
        goto nextStep;

    case OP_CONV_REAL_STRING2:
        Reg2String () = RealToString (Reg2 ().RealVal ());
        goto nextStep;

    case OP_OP_AND:
        Reg ().IntVal () = Reg ().IntVal () & Reg2 ().IntVal ();
        goto nextStep;

    case OP_OP_OR:
        Reg ().IntVal () = Reg ().IntVal () | Reg2 ().IntVal ();
        goto nextStep;

    case OP_OP_XOR:
        Reg ().IntVal () = Reg ().IntVal () ^ Reg2 ().IntVal ();
        goto nextStep;

    case OP_CALL_FUNC:

        assert (instruction->m_value.IntVal () >= 0);
        assert ((unsigned)instruction->m_value.IntVal () < m_functions.size ());

        // Call external function
        m_functions [instruction->m_value.IntVal ()] (*this);
        if (!Error ())
            goto nextStep;
        break;

	case OP_TIMESHARE:
        m_ip++;                             // Move on to next instruction
        break;                              // And return

	case OP_COND_TIMESHARE:
		if (!m_timeshare) goto nextStep;	// If no timeshare flagged, continue executing
		m_ip++;                             // Move on to next instruction
		break;                              // And return

	case OP_FREE_TEMP:

        // Free temporary data
        UnwindTemp();
        m_data.FreeTemp();
        goto nextStep;

    case OP_ALLOC: {

        // Extract type, and array dimensions
        vmValType type = m_typeSet.GetValType (instruction->m_value.IntVal ());
        if (!PopArrayDimensions (type))
            break;

        // Validate type size
        if (!ValidateTypeSize (type))
            break;

        // Allocate and initialise new data
        m_reg.IntVal() = m_data.Allocate (m_dataTypes.DataSize (type));
        m_data.InitData (m_reg.IntVal(), type, m_dataTypes);

        goto nextStep;
    }

    case OP_CALL: {

        // Call (GOSUB)
        assert (instruction->m_value.IntVal () >= 0);
        assert ((unsigned)instruction->m_value.IntVal () < m_code.size ());

        // Check for stack overflow
        if (m_userCallStack.size () >= VM_MAXUSERSTACKCALLS) {
            SetError(ErrStackOverflow);
            break;
        }

        // Push stack frame, with return address
        m_userCallStack.push_back(vmUserFuncStackFrame());
        vmUserFuncStackFrame& stackFrame = m_userCallStack.back();
        stackFrame.InitForGosub(m_ip + 1);

        // Jump to subroutine
        m_ip = instruction->m_value.IntVal ();
        goto step;
    }
    case OP_RETURN:

        // Return from GOSUB

        // Pop and validate return address
        if (m_userCallStack.empty ()) {
            SetError (ErrReturnWithoutGosub);
            break;
        }

        assert(m_userCallStack.back().userFuncIndex == -1);     // -1 means GOSUB. Should be impossible to execute an OP_RETURN if stack top is not a GOSUB 

        tempI = m_userCallStack.back().returnAddr;
        m_userCallStack.pop_back ();
        if (tempI >= m_code.size ()) {
            SetError (ErrStackError);
            break;
        }

        // Jump to return address
        m_ip = tempI;
        goto step;

    case OP_CALL_DLL: {

        // Call plugin DLL function
        unsigned int index = instruction->m_value.IntVal();
        m_plugins.LoadedLibraries()[index >> 24]->GetFunction(index & 0x00ffffff) (*m_pluginRuntime);

        if (!Error ())
            goto nextStep;
        break;
    }

    case OP_CREATE_USER_FRAME: {

        // Check for stack overflow
        if (m_userCallStack.size () >= VM_MAXUSERSTACKCALLS) {
            SetError(ErrStackOverflow);
            break;
        }

        // Create and initialize stack frame
        int funcIndex = instruction->m_value.IntVal();
        m_userCallStack.push_back(vmUserFuncStackFrame());
        vmUserFuncStackFrame& stackFrame = m_userCallStack.back();
        stackFrame.InitForUserFunction(
            m_userFunctionPrototypes[m_userFunctions[funcIndex].prototypeIndex],
            funcIndex);

        // Save previous stack frame data
        m_data.SaveState(stackFrame.prevStackTop, stackFrame.prevTempDataLock);

        goto nextStep;
    }

	case OP_CREATE_FUNC_PTR_FRAME: {

		// This is identical to OP_CREATE_USER_FRAME, except that the function 
		// index will be in reg, rather than the instruction.

		// Check for null function pointer
		if (m_reg.IntVal() == 0)
		{
			SetError(ErrUnsetPointer);
			break;
		}

		// Check for stack overflow
		if (m_userCallStack.size() >= VM_MAXUSERSTACKCALLS) {
			SetError(ErrStackOverflow);
			break;
		}

		// Function index + 1 is in reg
		// (+1 is so that we can use 0 for null)
		int funcIndex = m_reg.IntVal() - 1;

		m_userCallStack.push_back(vmUserFuncStackFrame());
		vmUserFuncStackFrame& stackFrame = m_userCallStack.back();
		stackFrame.InitForUserFunction(
			m_userFunctionPrototypes[m_userFunctions[funcIndex].prototypeIndex],
			funcIndex);

		// Save previous stack frame data
		m_data.SaveState(stackFrame.prevStackTop, stackFrame.prevTempDataLock);

		goto nextStep;
	}

	case OP_CHECK_FUNC_PTR: {

		// Function pointer can be null (0)
		if (m_reg.IntVal() == 0) goto nextStep;

		// Function index + 1 is in reg
		// (+1 is so that we can use 0 for null)
		int funcIndex = m_reg.IntVal() - 1;

		// Check function prototype is compatible with prototype referenced by instruction
		auto const& srcProto = m_userFunctionPrototypes[m_userFunctions[funcIndex].prototypeIndex];
		auto const& dstProto = m_userFunctionPrototypes[instruction->m_value.IntVal()];

		if (!srcProto.IsCompatibleWith(dstProto))
		{
			SetError(ErrFuncPtrIncompatible);
			break;
		}

		goto nextStep;
	}

    case OP_CREATE_RUNTIME_FRAME: {
        assert(!m_codeBlocks.empty());

        // Find function index
        int funcIndex = -1;

        // Look for function in bound code block
        int runtimeIndex = instruction->m_value.IntVal();
        if (m_boundCodeBlock > 0 && (unsigned)m_boundCodeBlock < m_codeBlocks.size()) {
            vmCodeBlock& codeBlock = m_codeBlocks[m_boundCodeBlock];
            if (codeBlock.programOffset >= 0)
                funcIndex = codeBlock.GetRuntimeFunction(runtimeIndex).functionIndex;
        }

        // If not found, look in main program
        if (funcIndex < 0)
            funcIndex = m_codeBlocks[0].GetRuntimeFunction(runtimeIndex).functionIndex;

        // No function => Runtime error
        if (funcIndex < 0) {
            SetError(ErrNoRuntimeFunction);
            break;
        }

        // From here on the logic is the same as OP_CREATE_USER_FRAME
        // Check for stack overflow
        if (m_userCallStack.size () >= VM_MAXUSERSTACKCALLS) {
            SetError(ErrStackOverflow);
            break;
        }

        // Create and initialize stack frame
        m_userCallStack.push_back(vmUserFuncStackFrame());
        vmUserFuncStackFrame& stackFrame = m_userCallStack.back();
        stackFrame.InitForUserFunction(
            m_userFunctionPrototypes[m_userFunctions[funcIndex].prototypeIndex],
            funcIndex);

        // Save previous stack frame data
        m_data.SaveState(stackFrame.prevStackTop, stackFrame.prevTempDataLock);

        goto nextStep;
    }

    case OP_CALL_USER_FUNC: {

        // Call user defined function
        vmUserFuncStackFrame& stackFrame = m_userCallStack.back();
        vmUserFunc& userFunc = m_userFunctions[stackFrame.userFuncIndex];

        // Make active
        stackFrame.prevCurrentFrame = m_currentUserFrame;
        m_currentUserFrame = m_userCallStack.size() - 1;

        // Call function
        stackFrame.returnAddr = m_ip + 1;
        m_ip = userFunc.programOffset;
        goto step;
    }

    case OP_RETURN_USER_FUNC: {
        assert(m_userCallStack.size() > 0);

        // Find current stack frame
        vmUserFuncStackFrame& stackFrame = m_userCallStack.back();
        assert(stackFrame.userFuncIndex >= 0);

        // Restore previous stack frame data
        bool doFreeTempData = instruction->m_value.IntVal() == 1;
        if (doFreeTempData)
            UnwindTemp();
        UnwindStack(stackFrame.prevStackTop);
        m_data.RestoreState(
                stackFrame.prevStackTop,
                stackFrame.prevTempDataLock,
                doFreeTempData);

        // Return to return address
        m_ip = stackFrame.returnAddr;

        // Make previous frame active
        m_currentUserFrame = stackFrame.prevCurrentFrame;

        // Remove stack frame
        m_userCallStack.pop_back();

        goto step;
    }

    case OP_NO_VALUE_RETURNED:
        SetError(ErrNoValueReturned);
        break;

    case OP_BINDCODE:
        m_boundCodeBlock = m_reg.IntVal();
        goto nextStep;

    case OP_EXEC:

        // Call runtime compiled code block.
        // Call is like a GOSUB.
        // RETURN will return back to the next op-code
        if (m_boundCodeBlock > 0 && (unsigned)m_boundCodeBlock < m_codeBlocks.size()) {
            vmCodeBlock& codeBlock = m_codeBlocks[m_boundCodeBlock];
            if (codeBlock.programOffset >= 0) {

                // From here the code is the same as OP_CALL
                assert(codeBlock.programOffset >= 0);
                assert((unsigned)codeBlock.programOffset < m_code.size ());

                // Check for stack overflow
                if (m_userCallStack.size () >= VM_MAXUSERSTACKCALLS) {
                    SetError(ErrStackOverflow);
                    break;
                }

                // Push stack frame, with return address
                m_userCallStack.push_back(vmUserFuncStackFrame());
                vmUserFuncStackFrame& stackFrame = m_userCallStack.back();
                stackFrame.InitForGosub(m_ip + 1);

                // Jump to subroutine
                m_ip = codeBlock.programOffset;
                goto step;
            }
        }

        SetError(ErrInvalidCodeBlock);
        break;

    case OP_END_CALLBACK:
        break;          // Timeshare break. Calling code will then detect this op-code has been reached

    case OP_DATA_READ:

        // Read program data into register
        if (ReadProgramData ((vmBasicValType) instruction->m_type))
            goto nextStep;
        else
            break;

    case OP_DATA_RESET:

        m_programDataOffset = instruction->m_value.IntVal ();
        goto nextStep;

    case OP_SAVE_PARAM: {

        // Allocate parameter data
        if (!m_data.StackRoomFor(1)) {
            SetError(ErrUserFuncStackOverflow);
            break;
        }
        int dataIndex = m_data.AllocateStack(1);
        int paramIndex = instruction->m_value.IntVal();

        // Initialize parameter
        assert(!m_userCallStack.empty());
        m_userCallStack.back().localVarDataOffsets[paramIndex] = dataIndex;

        // Transfer register value to parameter
        vmValue& dest = m_data.Data()[dataIndex];
        switch (instruction->m_type) {
        case VTP_INT:
        case VTP_REAL:
		case VTP_FUNC_PTR:
            dest = m_reg;
            break;
        case VTP_STRING:

            // Allocate string space
            dest.IntVal () = m_strings.Alloc ();

            // Copy string value
            m_strings.Value (dest.IntVal ()) = m_regString;
            break;
        default:
            assert (false);
        }

        // Save parameter offset in register (so that OP_REG_DESTRUCTOR will work)
        m_reg.IntVal() = dataIndex;
        goto nextStep;
    }

    case OP_COPY_USER_STACK: {

        // Copy data pointed to by m_reg into next stack frame parameter.
        // Instruction value points to the parameter data type.
        if (CopyToParam(
                m_reg.IntVal(),
                m_typeSet.GetValType(instruction->m_value.IntVal())))
            goto nextStep;
        else
            break;
    }

    case OP_MOVE_TEMP: {

        if (MoveToTemp(
                m_reg.IntVal(),
                m_typeSet.GetValType(instruction->m_value.IntVal())))
            goto nextStep;
        else
            break;
    }

    case OP_CHECK_PTR: {

        if (CheckPointer(m_reg2.IntVal(), m_reg.IntVal()))
            goto nextStep;
        else {
            SetError(ErrPointerScopeError);
            break;
        }

/*        // Null pointer case
        if (m_reg.IntVal() == 0)
            goto nextStep;

        // Check whether pointer points to temporary stack data
        if (m_reg.IntVal() < m_data.Permanent()) {

            int dest = m_reg2.IntVal();

            // Such pointers can only be stored in variables in the current
            // stack frame.
            if (m_userCallStack.empty() || !(dest >= m_data.StackTop() && dest < m_userCallStack.back().prevStackTop)) {
                SetError(ErrPointerScopeError);
                break;
            }
        }
        goto nextStep;*/
    }

    case OP_CHECK_PTRS: {
        if (CheckPointers(
                m_reg.IntVal(),
                m_typeSet.GetValType(instruction->m_value.IntVal()),
                m_reg2.IntVal()))
            goto nextStep;
        else {
            SetError(ErrPointerScopeError);
            break;
        }
    }

    case OP_REG_DESTRUCTOR: {

        // Register destructor for data pointed to by m_reg.
        int ptr = m_reg.IntVal();
        assert(ptr >= 0);
        if (ptr == 0)
            ;
        else if (ptr < m_data.TempData()) {

            // Pointer into temp data found
            assert(m_tempDestructors.empty() || m_tempDestructors.back().addr < ptr);
            m_tempDestructors.push_back(vmStackDestructor(ptr, instruction->m_value.IntVal()));
        }
        else if (ptr >= m_data.StackTop() && ptr < m_data.Permanent()) {

            // Pointer into stack data found
            assert(m_stackDestructors.empty() || m_stackDestructors.back().addr > ptr);
            m_stackDestructors.push_back(vmStackDestructor(ptr, instruction->m_value.IntVal()));
        }
        goto nextStep;
    }

	case OP_SWAP: {

		// Swap registers
		vmValue temp1 = m_reg;
		m_reg = m_reg2;
		m_reg2 = temp1;

		std::string temp2 = m_regString;
		m_regString = m_reg2String;
		m_reg2String = temp2;

		goto nextStep;
	}

    case OP_SAVE_PARAM_PTR: {

        // Save register pointer into param pointer
        assert(!m_userCallStack.empty());
        m_userCallStack.back().localVarDataOffsets[instruction->m_value.IntVal()] = m_reg.IntVal();
        goto nextStep;
    }

    case OP_RUN:

        // If the stack is not empty, it means we are inside an Execute() call.
        // Resetting the program is not a good idea, as we will lose all the
        // stacked state that the Execute() call is expecting to be present when
        // it returns.
        if (!m_stack.Empty())
            SetError(ErrRunCalledInsideExecute);
		else {
			Reset();                       // Reset program

			// Reset plugin libraries
			m_plugins.ProgramEnd();
			m_plugins.ProgramStart();
		}
        break;                              // Timeshare break

    case OP_BREAKPT:
        m_paused = true;                    // Pause program
        break;                              // Timeshare break

    default:
        SetError (ErrInvalid);
    }
}

int TomVM::StoreStringConstant (std::string str) {
    int index = m_stringConstants.size ();
    m_stringConstants.push_back (str);
    return index;
}

void TomVM::BlockCopy      (int sourceIndex, int destIndex, int size) {

    // Block copy data
    assert (m_data.IndexValid (sourceIndex));
    assert (m_data.IndexValid (sourceIndex + size - 1));
    assert (m_data.IndexValid (destIndex));
    assert (m_data.IndexValid (destIndex + size - 1));
    for (int i = 0; i < size; i++)
        m_data.Data () [destIndex + i] = m_data.Data () [sourceIndex + i];
}

void TomVM::CopyStructure  (int sourceIndex, int destIndex, vmValType& type) {
    assert (m_dataTypes.TypeValid (type));
    assert (type.VirtualPointerLevel () == 0);
    assert (type.m_arrayLevel == 0);
    assert (type.m_basicType >= 0);

    // Find structure definition
    vmStructure &s = m_dataTypes.Structures () [type.m_basicType];

    // Copy fields in structure
    for (int i = 0; i < s.m_fieldCount; i++) {
        vmStructureField& f = m_dataTypes.Fields () [s.m_firstField + i];
        CopyField (sourceIndex + f.m_dataOffset, destIndex + f.m_dataOffset, f.m_type);
    }
}

void TomVM::CopyArray      (int sourceIndex, int destIndex, vmValType& type) {
    assert (m_dataTypes.TypeValid (type));
    assert (type.VirtualPointerLevel () == 0);
    assert (type.m_arrayLevel > 0);
    assert (m_data.IndexValid (sourceIndex));
    assert (m_data.IndexValid (destIndex));
    assert (m_data.Data () [sourceIndex].IntVal () == m_data.Data () [destIndex].IntVal ());             // Array sizes match
    assert (m_data.Data () [sourceIndex + 1].IntVal () == m_data.Data () [destIndex + 1].IntVal ());     // Element sizes match

    // Find element type and size
    vmValType elementType = type;
    elementType.m_arrayLevel--;
    int elementSize = m_data.Data () [sourceIndex + 1].IntVal ();

    // Copy elements
    for (int i = 0; i < m_data.Data () [sourceIndex].IntVal (); i++) {
        if (elementType.m_arrayLevel > 0)
            CopyArray ( sourceIndex + 2 + i * elementSize,
                        destIndex + 2 + i * elementSize,
                        elementType);
        else
            CopyField ( sourceIndex + 2 + i * elementSize,
                        destIndex + 2 + i * elementSize,
                        elementType);
    }
}

void TomVM::CopyField      (int sourceIndex, int destIndex, vmValType& type) {

    assert (m_dataTypes.TypeValid (type));

    // If type is basic string, copy string value
    if (type == VTP_STRING) {
        vmValue& src = m_data.Data()[sourceIndex];
        vmValue& dest = m_data.Data()[destIndex];
        if (src.IntVal() > 0 || dest.IntVal() > 0) {

            // Allocate string space if necessary
            if (dest.IntVal () == 0)
                dest.IntVal () = m_strings.Alloc ();

            // Copy string value
            m_strings.Value (dest.IntVal ()) = m_strings.Value (m_data.Data () [sourceIndex].IntVal ());
        }
    }

    // If type is basic, or pointer then just copy value
    else if (type.IsBasic () || type.VirtualPointerLevel () > 0)
        m_data.Data () [destIndex] = m_data.Data () [sourceIndex];

    // If contains no strings, can just block copy
    else if (!m_dataTypes.ContainsString (type))
        BlockCopy (sourceIndex, destIndex, m_dataTypes.DataSize (type));

    // Otherwise copy array or structure
    else if (type.m_arrayLevel > 0)
        CopyArray (sourceIndex, destIndex, type);
    else
        CopyStructure (sourceIndex, destIndex, type);
}

bool TomVM::CopyData       (int sourceIndex, int destIndex, vmValType type) {
    assert (m_dataTypes.TypeValid (type));
    assert (type.VirtualPointerLevel () == 0);

    // If a referenced type (which it should always be), convert to regular type.
    // (To facilitate comparisons against basic types such as VTP_STRING.)
    if (type.m_byRef)
        type.m_pointerLevel--;
    type.m_byRef = false;

    // Check pointers are valid
    if (!m_data.IndexValid (sourceIndex) || !m_data.IndexValid (destIndex)
    ||  sourceIndex == 0 || destIndex == 0) {
        SetError (ErrUnsetPointer);
        return false;
    }

    // Calculate element size
    int size = 1;
    if (type.m_basicType >= 0)
        size = m_dataTypes.Structures () [type.m_basicType].m_dataSize;

    // If the data types are arrays, then their sizes could be different.
    // If so, this is a run-time error.
    if (type.m_arrayLevel > 0) {
        int s = sourceIndex + (type.m_arrayLevel - 1) * 2,
            d = destIndex   + (type.m_arrayLevel - 1) * 2;
        for (int i = 0; i < type.m_arrayLevel; i++) {
            assert (m_data.IndexValid (s));
            assert (m_data.IndexValid (s + 1));
            assert (m_data.IndexValid (d));
            assert (m_data.IndexValid (d + 1));
            if (m_data.Data () [s].IntVal () != m_data.Data () [d].IntVal ()) {
                SetError (ErrArraySizeMismatch);
                return false;
            }

            // Update data size
            size *= m_data.Data () [s].IntVal ();
            size += 2;

            // Point to first element in array
            s -= 2;
            d -= 2;
        }
    }

    // If data type doesn't contain strings, can do a straight block copy
    if (!m_dataTypes.ContainsString (type))
        BlockCopy (sourceIndex, destIndex, size);
    else
        CopyField (sourceIndex, destIndex, type);

    return true;
}

bool TomVM::CheckPointers(int index, vmValType type, int destIndex) {

    // Check that pointers in data at "index" of type "type" can be stored at
    // "destIndex" without any pointer scope errors.
    // (See CheckPointer for defn of "pointer scope error")
    assert(m_data.IndexValid(index));
    assert(m_dataTypes.TypeValid(type));

    // Type does not contain any pointers?
    if (!m_dataTypes.ContainsPointer(type))
        return true;

    // Type is a pointer?
    if (type.m_pointerLevel > 0)
        return CheckPointer(destIndex, m_data.Data()[index].IntVal());

    // Type is not a pointer, but contains one or more pointers.
    // Need to recursively break down object and check

    // Type is array?
    if (type.m_arrayLevel > 0) {

        // Find and check elements
        int elements    = m_data.Data()[index].IntVal();
        int elementSize = m_data.Data()[index + 1].IntVal();
        int arrayStart  = index + 2;

        // Calculate element type
        vmValType elementType = type;
        elementType.m_arrayLevel--;

        // Check each element
        for (int i = 0; i < elements; i++)
            if (!CheckPointers(arrayStart + i * elementSize, elementType, destIndex))
                return false;

        return true;
    }
    else {

        // Must be a structure
        assert(type.m_basicType >= 0);

        // Find structure definition
        vmStructure &s = m_dataTypes.Structures()[type.m_basicType];

        // Check each field in structure
        for (int i = 0; i < s.m_fieldCount; i++) {
            vmStructureField& f = m_dataTypes.Fields()[s.m_firstField + i];
            if (!CheckPointers(index + f.m_dataOffset, f.m_type, destIndex))
                return false;
        }

        return true;
    }
}

bool TomVM::CheckPointer(int dest, int ptr) {

    // Check that pointer "ptr" can be stored at "dest" without a pointer scope
    // error.
    // By "pointer scope error" we mean that a pointer of a longer lived scope
    // is pointing to data of a shorter lived scope.
    // For example, a global pointer to a local variable.

    // We treat this as a runtime error to prevent system instability that would
    // result if we allowed the program to continue to run.

    // Note that this approach is a bit of an experiment at the moment.
    assert(m_data.IndexValid(dest));

    // Null pointer case
    if (ptr == 0)
        return true;

    // Check whether pointer points to temporary stack data
    if (ptr < m_data.Permanent()) {

        // Such pointers can only be stored in variables in the current
        // stack frame.
        if (m_userCallStack.empty() || !(dest >= m_data.StackTop() && dest < m_userCallStack.back().prevStackTop))
            return false;
    }

    return true;
}

bool TomVM::PopArrayDimensions (vmValType& type) {
    assert (m_dataTypes.TypeValid (type));
    assert (type.VirtualPointerLevel () == 0);

    // Pop and validate array indices from stack into type
    int i;
    vmValue v;
    for (i = 0; i < type.m_arrayLevel; i++) {
        m_stack.Pop (v);
        int size = v.IntVal () + 1;
        if (size < 1) {
            SetError (ErrZeroLengthArray);
            return false;
        }
        type.m_arrayDims [i] = size;
    }

    return true;
}

bool TomVM::ValidateTypeSize (vmValType& type) {

    // Enforce variable size limitations.
    // This prevents rogue programs from trying to allocate unrealistic amounts
    // of data.
    if (m_dataTypes.DataSizeBiggerThan (type, m_data.MaxDataSize ())) {
        SetError (ErrVariableTooBig);
        return false;
    }

    if (!m_data.RoomFor (m_dataTypes.DataSize (type))) {
        SetError (ErrOutOfMemory);
        return false;
    }

    return true;
}

bool TomVM::ValidateTypeSizeForStack(vmValType& type) {
    // Enforce variable size limitations.
    // This prevents rogue programs from trying to allocate unrealistic amounts
    // of data.
    if (m_dataTypes.DataSizeBiggerThan(type, m_data.MaxDataSize ())) {
        SetError(ErrVariableTooBig);
        return false;
    }

    if (!m_data.StackRoomFor(m_dataTypes.DataSize(type))) {
        SetError(ErrUserFuncStackOverflow);
        return false;
    }

    return true;
}

void TomVM::PatchInBreakPt (unsigned int offset) {

    // Only patch if offset is valid and there is no breakpoint there already.
    // Note: Don't patch in breakpoint to last instruction of program as this is
    // always OP_END anyway.
    if (offset < m_code.size () - 1 && m_code [offset].m_opCode != OP_BREAKPT) {

        // Record previous op-code
        vmPatchedBreakPt bp;
        bp.m_offset         = offset;
        bp.m_replacedOpCode = (vmOpCode) m_code [offset].m_opCode;
        m_patchedBreakPts.push_back (bp);

        // Patch in breakpoint
        m_code [offset].m_opCode = OP_BREAKPT;
    }
}

void TomVM::InternalPatchOut () {

    // Patch out breakpoints and restore program to its no breakpoint state.
    for (   vmPatchedBreakPtList::iterator i = m_patchedBreakPts.begin ();
            i != m_patchedBreakPts.end ();
            i++)
        if ((*i).m_offset < m_code.size ())
            m_code [(*i).m_offset].m_opCode = (*i).m_replacedOpCode;
    m_patchedBreakPts.clear ();
    m_breakPtsPatched = false;
}

void TomVM::InternalPatchIn () {

    // Patch breakpoint instructions into the virtual machine code program.
    // This consists of swapping the virtual machine op-codes with OP_BREAKPT
    // codes.
    // We record the old op-code in the m_patchedBreakPts list, so we can restore
    // the program when we've finished.

    // User breakpts
    for (int i = 0; i < m_debugger.UserBreakPtCount(); i++) {

        // Find line number
        int line = m_debugger.UserBreakPtLine(i);

        // Convert to offset
        unsigned int offset = 0;
        while (offset < m_code.size() && m_code[offset].m_sourceLine < (unsigned)line)
            offset++;

        // Patch in breakpt
        if (offset < m_code.size())
            PatchInBreakPt(offset);
    }

/*    // User breakpts
    // Convert from line numbers to offsets
    unsigned int offset = 0;
    vmUserBreakPts::iterator i;
    for (   i  = m_userBreakPts.begin ();                                               // Loop through breakpoints
            i != m_userBreakPts.end ();
            i++) {
        while (offset < m_code.size () && m_code [offset].m_sourceLine < (*i).first)
            offset++;
        if (offset < m_code.size () && m_code [offset].m_sourceLine == (*i).first)      // Is breakpoint line valid?
            (*i).second.m_offset = offset;
        else
            (*i).second.m_offset = 0xffff;                                              // 0xffff means line invalid
    }

    // Patch in user breakpts
    for (   i  = m_userBreakPts.begin ();                                               // Loop through breakpoints
            i != m_userBreakPts.end ();
            i++)
        PatchInBreakPt ((*i).second.m_offset);*/

    // Patch in temp breakpts
    for (   vmTempBreakPtList::iterator j = m_tempBreakPts.begin ();
            j != m_tempBreakPts.end ();
            j++)
        PatchInBreakPt ((*j).m_offset);

    m_breakPtsPatched = true;
}

unsigned int TomVM::CalcBreakPtOffset (unsigned int line) {
    unsigned int offset = 0;
    while (offset < m_code.size () && m_code [offset].m_sourceLine < line)
        offset++;
    if (offset < m_code.size () && m_code [offset].m_sourceLine == line)        // Is breakpoint line valid?
        return offset;
    else
        return 0xffff;                                                          // 0xffff means line invalid
}

void TomVM::AddStepBreakPts (bool stepInto) {

    // Add temporary breakpoints to catch execution after stepping over the current line
    if (m_ip >= m_code.size ())
        return;
    PatchOut ();

    // Calculate op-code range that corresponds to the current line.
    unsigned int line, startOffset, endOffset;
    startOffset = m_ip;
    line        = m_code [startOffset].m_sourceLine;

    // Search for start of line
    while (startOffset > 0 && m_code [startOffset - 1].m_sourceLine == line)
        startOffset--;

    // Search for start of next line
    endOffset = m_ip + 1;
    while (endOffset < m_code.size () && m_code [endOffset].m_sourceLine == line)
        endOffset++;

    // Create breakpoint on next line
    m_tempBreakPts.push_back(MakeTempBreakPt(endOffset));

    // Scan for jumps, and place breakpoints at destination addresses
	bool bpAllFunctions = false;
    for (unsigned int i = startOffset; i < endOffset; i++) {
        unsigned int dest = 0xffffffff;
        switch (m_code [i].m_opCode) {
            case OP_CALL:
                if (!stepInto)                                  // If stepInto then fall through to JUMP handling.
                    break;                                      // Otherwise break out, and no BP will be set.
            case OP_JUMP:
            case OP_JUMP_TRUE:
            case OP_JUMP_FALSE:
                dest = m_code[i].m_value.IntVal();              // Destination jump address
                break;

            case OP_RETURN:
            case OP_RETURN_USER_FUNC:
                if (!m_userCallStack.empty())
                    dest = m_userCallStack.back().returnAddr;
                break;

            case OP_CREATE_USER_FRAME:
                if (stepInto && !bpAllFunctions)
                    dest = m_userFunctions[m_code[i].m_value.IntVal()].programOffset;
                break;

			case OP_CREATE_RUNTIME_FRAME:
			case OP_CREATE_FUNC_PTR_FRAME:
				if (stepInto && !bpAllFunctions)
				{
					// Can't (easily) determine which function will be called, so must add breakpoints to all
					for (const auto& fn : m_userFunctions)
						if (fn.programOffset < m_code.size()
						&& (fn.programOffset < startOffset || fn.programOffset >= endOffset))
							m_tempBreakPts.push_back(MakeTempBreakPt(fn.programOffset));

					bpAllFunctions = true;						// Optimisation: Flag that we've already breakpointed all functions
					break;
				}
        }

        if (dest < m_code.size ()                               // Destination valid?
        &&  (dest < startOffset || dest >= endOffset))          // Destination outside line we are stepping over?
            m_tempBreakPts.push_back(MakeTempBreakPt(dest));    // Add breakpoint
    }
}

bool TomVM::AddStepOutBreakPt () {

    // Call stack must contain at least 1 return
    if (!m_userCallStack.empty()) {
        unsigned int returnAddr = m_userCallStack.back().returnAddr;
        if (returnAddr < m_code.size ()) {                                      // Validate it
            m_tempBreakPts.push_back(MakeTempBreakPt(returnAddr));              // Place breakpoint
            return true;
        }
    }
    return false;
}

vmState TomVM::GetState () {
    vmState s;

    // Instruction pointer
    s.ip = m_ip;

    // Registers
    s.reg           = m_reg;
    s.reg2          = m_reg2;
    s.regString     = m_regString;
    s.reg2String    = m_reg2String;

    // Stacks
    s.stackTop          = m_stack.Size();
//    s.callStackTop      = m_callStack.size();
    s.userFuncStackTop  = m_userCallStack.size();
    s.currentUserFrame  = m_currentUserFrame;

    // Top of program
    s.codeSize          = InstructionCount ();
    s.codeBlockCount    = m_codeBlocks.size();

    // Variable data
    m_data.SaveState(s.stackDataTop, s.tempDataLock);

    // Error state
    s.error         = Error ();
    if (s.error)    s.errorString   = GetError ();
    else            s.errorString   = "";

	// Current long running function
	s.longRunningFn = m_longRunningFn;
	m_longRunningFn = nullptr;

    // Other state
    s.paused        = m_paused;
    
    return s;
}

void TomVM::SetState (vmState& state) {

    // Instruction pointer
    m_ip                = state.ip;

    // Registers
    m_reg               = state.reg;
    m_reg2              = state.reg2;
    m_regString         = state.regString;
    m_reg2String        = state.reg2String;

    // Stacks
    if ((unsigned)state.stackTop < m_stack.Size())
        m_stack.Resize(state.stackTop);
//    if (state.callStackTop < m_callStack.size())
//        m_callStack.resize(state.callStackTop);
    if (state.userFuncStackTop < m_userCallStack.size())
        m_userCallStack.resize(state.userFuncStackTop);
    m_currentUserFrame = state.currentUserFrame;

    // Top of program
    if (state.codeSize < m_code.size ())
        m_code.resize (state.codeSize);
    if (state.codeBlockCount < m_codeBlocks.size())
        m_codeBlocks.resize(state.codeBlockCount);

    // Variable data
    UnwindTemp();
    UnwindStack(state.stackDataTop);
    m_data.RestoreState(state.stackDataTop, state.tempDataLock, true);

	// Long running function
	CancelLongRunningFn();
	m_longRunningFn = state.longRunningFn;

    // Error state
    if (state.error)    SetError (state.errorString);
    else                ClearError ();

    // Other state
    m_paused = state.paused;
}

std::string TomVM::BasicValToString (vmValue val, vmBasicValType type, bool constant) {
    switch (type) {
	case VTP_FUNC_PTR:
    case VTP_INT:       return IntToString (val.IntVal ());
    case VTP_REAL:      return RealToString (val.RealVal ());
    case VTP_STRING:    
        if (constant) {
            if (val.IntVal () >= 0 && (unsigned)val.IntVal () < m_stringConstants.size ())
                return "\"" + m_stringConstants [val.IntVal ()] + "\"";
            else
                return "???";
        }
        else
            return m_strings.IndexValid (val.IntVal ()) ? "\"" + m_strings.Value (val.IntVal ()) + "\"" : (std::string) "???";
	}
    return "???";
}

inline std::string TrimToLength (std::string str, int& length) {
    if (str.length () > (unsigned)length) {
        length = 0;
        return str.substr(0, length);
    }
    else {
        length -= str.length ();
        return str;
    }
}

void TomVM::Deref (vmValue& val, vmValType& type) {
    type.m_pointerLevel--;
    if (type.m_pointerLevel == 0 && !type.IsBasic ()) {

        // Can't follow pointer, as type is not basic (and therefore cannot be loaded into a register)
        // Use value by reference instead
        type.m_pointerLevel = 1;
        type.m_byRef = true;
    }
    else {

        // Follow pointer
        if (!m_data.IndexValid(val.IntVal()))            // DEBUGGING!!!
            assert (m_data.IndexValid (val.IntVal ()));
        val = m_data.Data () [val.IntVal ()];
    }
}

std::string TomVM::ValToString (vmValue val, vmValType type, int& maxChars) {
    assert (m_dataTypes.TypeValid (type));
    assert (type.PhysicalPointerLevel () > 0 || type.IsBasic ());

    if (maxChars <= 0)
        return "";

    // Basic types
    if (type.IsBasic ())
        return TrimToLength (BasicValToString (val, type.m_basicType, false), maxChars);

    // Pointer types
    if (type.VirtualPointerLevel () > 0) {

        // Follow pointer down
        if (val.IntVal () == 0)
            return TrimToLength ("[UNSET POINTER]", maxChars);
        Deref (val, type);
        return TrimToLength ("&", maxChars) + ValToString (val, type, maxChars);
    }

    // Type is not basic, or a pointer. Must be a value by reference. Either a structure or an array
    assert (type.m_pointerLevel == 1);
    assert (type.m_byRef);
    int dataIndex = val.IntVal ();              // Points to data
    if (dataIndex == 0)
        return TrimToLength ("[UNSET]", maxChars);
    std::string result = "";

    // Arrays
    if (type.m_arrayLevel > 0) {
        assert (m_data.IndexValid (dataIndex));
        assert (m_data.IndexValid (dataIndex + 1));

        // Read array header
        int elements    = m_data.Data () [dataIndex].IntVal ();
        int elementSize = m_data.Data () [dataIndex + 1].IntVal ();
        int arrayStart  = dataIndex + 2;

        // Enumerate elements
        std::string result = TrimToLength ("{", maxChars);
        for (int i = 0; i < elements && maxChars > 0; i++) {
            vmValue element = vmValue (arrayStart + i * elementSize);   // Address of element
            vmValType elementType = type;       // Element type.
            elementType.m_arrayLevel--;         // Less one array level.
            elementType.m_pointerLevel = 1;     // Currently have a pointer
            elementType.m_byRef = false;

            // Deref to reach data
            Deref (element, elementType);

            // Describe element
            result += ValToString (element, elementType, maxChars);
            if (i < elements - 1)
                result += TrimToLength (", ", maxChars);
        }
        result += TrimToLength ("}", maxChars);
        return result;
    }

    // Structures
    if (type.m_basicType >= 0) {
        std::string result = TrimToLength ("{", maxChars);
        vmStructure& structure = m_dataTypes.Structures () [type.m_basicType];
        for (int i = 0; i < structure.m_fieldCount && maxChars > 0; i++) {
            vmStructureField& field = m_dataTypes.Fields () [structure.m_firstField + i];
            vmValue fieldVal    = vmValue (dataIndex + field.m_dataOffset);
            vmValType fieldType = field.m_type;
            fieldType.m_pointerLevel++;
            Deref (fieldVal, fieldType);
            result += TrimToLength (field.m_name + "=", maxChars) + ValToString (fieldVal, fieldType, maxChars);
            if (i < structure.m_fieldCount - 1)
                result += TrimToLength (", ", maxChars);
        }
        result += TrimToLength ("}", maxChars);
        return result;
    }

    return "???";
}

std::string TomVM::VarToString (vmVariable& v, int maxChars) {
/*    vmValue     val = vmValue ((int) v.m_dataIndex);
    vmValType   type = v.m_type;
    type.m_pointerLevel++;
    Deref (val, type);
    return ValToString (val, type, maxChars);*/
    return DataToString(v.m_dataIndex, v.m_type, maxChars);
}

std::string TomVM::DataToString(int dataIndex, vmValType type, int maxChars) {
    vmValue val = vmValue(dataIndex);
    type.m_pointerLevel++;
    Deref(val, type);
    return ValToString(val, type, maxChars);
}

bool TomVM::ReadProgramData    (vmBasicValType type) {

    // Read program data into register.

    // Check for out-of-data.
    if (m_programDataOffset >= m_programData.size ()) {
        SetError (ErrOutOfData);
        return false;
    }

    // Find program data
    vmProgramDataElement e = m_programData [m_programDataOffset++];

    // Convert to requested type
    switch (type) {
    case VTP_STRING:

        // Convert type to int.
        switch (e.Type ()) {
        case VTP_STRING:
            assert (e.Value ().IntVal () >= 0);
            assert ((unsigned)e.Value ().IntVal () < m_stringConstants.size ());
            RegString () = m_stringConstants [e.Value ().IntVal ()];
            return true;
        case VTP_INT:
            RegString () = IntToString (e.Value ().IntVal ());
            return true;
        case VTP_REAL:
            RegString () = RealToString (e.Value ().RealVal ());
            return true;
        }
        break;

    case VTP_INT:
        switch (e.Type ()) {
        case VTP_STRING:
            SetError (ErrDataIsString);
            return false;
        case VTP_INT:
            Reg ().IntVal () = e.Value ().IntVal ();
            return true;
        case VTP_REAL:
            Reg ().IntVal () = e.Value ().RealVal ();
            return true;
        }
        break;

    case VTP_REAL:
        switch (e.Type ()) {
        case VTP_STRING:
            SetError (ErrDataIsString);
            return false;
        case VTP_INT:
            Reg ().RealVal () = e.Value ().IntVal ();
            return true;
        case VTP_REAL:
            Reg ().RealVal () = e.Value ().RealVal ();
            return true;
        }
        break;
    }
    assert (false);
    return false;
}

int TomVM::StoredDataSize(int sourceIndex, vmValType& type) {
    assert(!type.m_byRef);

    if (type.m_pointerLevel == 0 && type.m_arrayLevel > 0) {
        // Calculate actual array size
        // Array is prefixed by element count and element size.
        return m_data.Data()[sourceIndex].IntVal() * m_data.Data()[sourceIndex + 1].IntVal() + 2;
    }
    else
        return m_dataTypes.DataSize(type);
}

bool TomVM::CopyToParam(int sourceIndex, vmValType& type) {

    // Check source index is valid
    if (!m_data.IndexValid(sourceIndex) || sourceIndex == 0) {
        SetError (ErrUnsetPointer);
        return false;
    }

    // Calculate data size.
    // Note that the "type" does not specify array dimensions (if type is an array),
    // so they must be read from the source array.
    int size = StoredDataSize(sourceIndex, type);

    // Allocate data for parameter on stack
    if (!m_data.StackRoomFor(size)) {
        SetError(ErrUserFuncStackOverflow);
        return false;
    }
    int dataIndex = m_data.AllocateStack(size);

    // Block copy the data
    BlockCopy(sourceIndex, dataIndex, size);

    // Duplicate any contained strings
    if (m_dataTypes.ContainsString(type))
        DuplicateStrings(dataIndex, type);

    // Store pointer in register
    m_reg.IntVal() = dataIndex;

    return true;
}

void TomVM::DuplicateStrings(int dataIndex, vmValType& type) {

    // Called after data is block copied.
    // Strings are stored as pointers to string objects. After a block copy,
    // the source and destination blocks end up pointing to the same string
    // objects.
    // This method traverses the destination block, creates duplicate copies of
    // any contained strings and fixes up the pointers to point to these new
    // string objects.
    assert(m_dataTypes.ContainsString(type));
    assert(!type.m_byRef);
    assert(type.m_pointerLevel == 0);

    // Type IS string case
    if (type == VTP_STRING) {

        vmValue& val = m_data.Data()[dataIndex];
        // Empty strings (index 0) can be ignored
        if (val.IntVal() != 0) {

            // Allocate new string
            int newStringIndex = m_strings.Alloc();

            // Copy previous string
            m_strings.Value(newStringIndex) = m_strings.Value(val.IntVal());

            // Point to new string
            val.IntVal() = newStringIndex;
        }
    }

    // Array case
    else if (type.m_arrayLevel > 0) {

        // Read array header
        int elements    = m_data.Data()[dataIndex].IntVal();
        int elementSize = m_data.Data()[dataIndex + 1].IntVal();
        int arrayStart  = dataIndex + 2;

        // Calculate element type
        vmValType elementType = type;
        elementType.m_arrayLevel--;

        // Duplicate strings in each array element
        for (int i = 0; i < elements; i++)
            DuplicateStrings(arrayStart + i * elementSize, elementType);
    }

    // Otherwise must be a structure
    else {
        assert(type.m_basicType >= 0);

        // Find structure definition
        vmStructure &s = m_dataTypes.Structures()[type.m_basicType];

        // Duplicate strings for each field in structure
        for (int i = 0; i < s.m_fieldCount; i++) {
            vmStructureField& f = m_dataTypes.Fields()[s.m_firstField + i];
            if (m_dataTypes.ContainsString(f.m_type))
                DuplicateStrings(dataIndex + f.m_dataOffset, f.m_type);
        }
    }
}

bool TomVM::MoveToTemp(int sourceIndex, vmValType& type) {

    // Free temp data
    // Note: We can do this because we know that the data isn't really freed,
    // just marked as free. This is significant, because the data we are moving
    // into the temp data area may be in temp-data already.

    // Special handling is required if data being copied is already in the
    // temp region
    bool sourceIsTemp = sourceIndex > 0 && sourceIndex < m_data.TempData();

    // Destroy temp data.
    // However, if the data being copied is in temp data, we use a
    // protected-stack-range to prevent it being destroyed.
    if (sourceIsTemp)
        UnwindTemp(vmProtectedStackRange(sourceIndex, sourceIndex + StoredDataSize(sourceIndex, type)));
    else
        UnwindTemp();

    // Free the data
    m_data.FreeTemp();

    // Calculate data size.
    // Note that the "type" does not specify array dimensions (if type is an array),
    // so they must be read from the source array.
    int size = StoredDataSize(sourceIndex, type);

    // Allocate data for parameter on stack
    if (!m_data.StackRoomFor(size)) {
        SetError(ErrUserFuncStackOverflow);
        return false;
    }
    int dataIndex = m_data.AllocateTemp(size, false);

    // Block copy the data
    BlockCopy(sourceIndex, dataIndex, size);

    // Extra logic required to manage strings
    if (m_dataTypes.ContainsString(type)) {

        // If source was NOT temp data, then the object has been copied, rather
        // than moved, and we must duplicate all the contained string objects.
        if (!sourceIsTemp)
            DuplicateStrings(dataIndex, type);
    }

    // Store pointer in register
    m_reg.IntVal() = dataIndex;

    return true;
}

void TomVM::UnwindTemp() {
    UnwindTemp(vmProtectedStackRange());
}

void TomVM::UnwindTemp(vmProtectedStackRange protect) {
    int newTop = m_data.TempDataLock();

    // Run destrution logic over data that is about to be deallocated.
    while (!m_tempDestructors.empty() && m_tempDestructors.back().addr >= newTop) {
        DestroyData(m_tempDestructors.back(), protect);
        m_tempDestructors.pop_back();
    }

    // Note: We don't actually remove the data from the stack. Calling code must
    // handle that instead.
}

void TomVM::UnwindStack(int newTop) {

    // Run destruction logic over data that is about to be deallocated.
    while (!m_stackDestructors.empty() && m_stackDestructors.back().addr < newTop) {
        DestroyData(m_stackDestructors.back(), vmProtectedStackRange());
        m_stackDestructors.pop_back();
    }

    // Note: We don't actually remove the data from the stack. Calling code must
    // handle that instead.
}

void TomVM::DestroyData(vmStackDestructor& d, vmProtectedStackRange protect) {

    // Apply destructor logic to data block.
    DestroyData(d.addr, m_typeSet.GetValType(d.dataTypeIndex), protect);
}

void TomVM::DestroyData(int index, vmValType type, vmProtectedStackRange protect) {
    assert(m_dataTypes.ContainsString(type));
    assert(!type.m_byRef);

    // Note: Current "destruction" logic involves deallocating strings stored
    // in the data. (But could later be extended to something more general
    // purpose.)
    if (type == VTP_STRING) {

        // Don't destroy if in protected range
        if (protect.ContainsAddr(index))
            return;

        // Type IS string case

        // Deallocate the string (if allocated)
        int stringIndex = m_data.Data()[index].IntVal();
        if (stringIndex != 0)
            m_strings.Free(stringIndex);
    }
    else if (type.m_arrayLevel > 0) {

        // Array case
        vmValType elementType = type;
        elementType.m_arrayLevel--;
        int count = m_data.Data()[index].IntVal();
        int elementSize = m_data.Data()[index + 1].IntVal();
        int arrayStart = index + 2;

        // Don't destroy if in protected range
        if (protect.ContainsRange(index, arrayStart + count * elementSize))
            return;

        // Recursively destroy each element
        for (int i = 0; i < count; i++)
            DestroyData(arrayStart + i * elementSize, elementType, protect);
    }
    else {

        // At this point we know the type contains a string and is not a string
        // or array.
        // Can only be a structure.
        assert(type.m_pointerLevel == 0);
        assert(type.m_basicType >= 0);

        // Recursively destroy each structure field (that contains a string)
        vmStructure& s = m_dataTypes.Structures()[(int)type.m_basicType];

        // Don't destroy if in protected range
        if (protect.ContainsRange(index, index + s.m_dataSize))
            return;

        for (int i = 0; i < s.m_fieldCount; i++) {

            // Get field info
            vmStructureField& f = m_dataTypes.Fields()[s.m_firstField + i];

            // Destroy if contains string(s)
            if (m_dataTypes.ContainsString(f.m_type))
                DestroyData(index + f.m_dataOffset, f.m_type, protect);
        }
    }
}

void TomVM::CancelLongRunningFn()
{
	if (m_longRunningFn != nullptr)
	{
		bool deleteWhenDone = m_longRunningFn->DeleteWhenDone();
		m_longRunningFn->Cancel();
		if (deleteWhenDone)
			delete m_longRunningFn;
		m_longRunningFn = nullptr;

		// Notify
		if (m_longRunningFnDoneNotified)
			m_longRunningFnDoneNotified->LongRunningFnDone(true);
	}
}

int TomVM::NewCodeBlock() {
    m_codeBlocks.push_back(vmCodeBlock());

    // Set pointer to code
    m_codeBlocks.back().programOffset = m_code.size();

    // Bind code block
    m_boundCodeBlock = m_codeBlocks.size() - 1;

    // Return index of new code block
    return m_boundCodeBlock;
}

bool TomVM::IsCodeBlockValid(int index) {
    return index >= 0 && (unsigned)index < m_codeBlocks.size();
}

int TomVM::GetCodeBlockOffset(int index) {
    assert(IsCodeBlockValid(index));
    return m_codeBlocks[index].programOffset;
}

void TomVM::BeginLongRunningFn(IDLL_Basic4GL_LongRunningFunction* handler)
{
	// Should never be any existing long running fn, but just in case
	CancelLongRunningFn();

	// Set handler as the new long running fn.
	// This will prevent VM from executing any more op-codes
	m_longRunningFn = handler;
}

void TomVM::EndLongRunningFunction()
{
	if (m_longRunningFn != nullptr)
	{
		// Delete if required
		if (m_longRunningFn->DeleteWhenDone())
			delete m_longRunningFn;

		// Unhook handler. This allows VM to continue executing op-codes.
		m_longRunningFn = nullptr;

		// Notify
		if (m_longRunningFnDoneNotified)
			m_longRunningFnDoneNotified->LongRunningFnDone(false);
	}
}

vmRollbackPoint TomVM::GetRollbackPoint() {
    vmRollbackPoint r;

    r.codeBlockCount            = m_codeBlocks.size();
    r.boundCodeBlock            = m_boundCodeBlock;
    r.functionPrototypeCount    = m_userFunctionPrototypes.size();
    r.functionCount             = m_userFunctions.size();
    r.dataCount                 = m_programData.size();
    r.instructionCount          = m_code.size();

    return r;
}

void TomVM::Rollback(vmRollbackPoint rollbackPoint) {

    // Rollback virtual machine
    m_codeBlocks.resize(rollbackPoint.codeBlockCount);
    m_boundCodeBlock = rollbackPoint.boundCodeBlock;
    m_userFunctionPrototypes.resize(rollbackPoint.functionPrototypeCount);
    m_userFunctions.resize(rollbackPoint.functionCount);
    m_programData.resize(rollbackPoint.dataCount);
    m_code.resize(rollbackPoint.instructionCount);
}

#ifdef VM_STATE_STREAMING
void TomVM::StreamOut (std::ostream& stream) const {
    int i;

    // Stream header
    WriteString (stream, streamHeader);
    WriteLong (stream, streamVersion);

    // Plugin DLLs
    m_plugins.StreamOut(stream);

    // Variables
    m_variables.StreamOut (stream);         // Note: m_variables automatically streams out m_dataTypes

    // String constants
    WriteLong (stream, m_stringConstants.size ());
    for (i = 0; i < m_stringConstants.size (); i++)
        WriteString (stream, m_stringConstants [i]);
    
    // Data type lookup table
    m_typeSet.StreamOut (stream);

    // Program code
    WriteLong (stream, m_code.size ());
    for (i = 0; i < m_code.size (); i++)
        m_code [i].StreamOut (stream);

    // Program data (for "DATA" statements)
    WriteLong (stream, m_programData.size ());
    for (i = 0; i < m_programData.size (); i++)
        m_programData [i].StreamOut (stream);

    // User function prototypes
    WriteLong(stream, m_userFunctionPrototypes.size());
    for (i = 0; i < m_userFunctionPrototypes.size(); i++)
        m_userFunctionPrototypes[i].StreamOut(stream);

    // User functions
    WriteLong(stream, m_userFunctions.size());
    for (i = 0; i < m_userFunctions.size(); i++)
        m_userFunctions[i].StreamOut(stream);

    // Code blocks
    WriteLong(stream, m_codeBlocks.size());
    for (i = 0; i < m_codeBlocks.size(); i++)
        m_codeBlocks[i].StreamOut(stream);
    
}

bool TomVM::StreamIn (std::istream& stream) {

    // Read and validate stream header
    if (ReadString(stream) != streamHeader)
        return false;
    if (ReadLong(stream) != streamVersion)
        return false;

    // Plugin DLLs
    if (!m_plugins.StreamIn(stream)) {
        SetError(m_plugins.Error());
        return false;
    }

    // Register plugin structures and functions in VM
    m_plugins.StructureManager().AddVMStructures(DataTypes());
    m_plugins.CreateVMFunctionSpecs();

    // Variables
    m_variables.StreamIn(stream);

    // String constants
    int count, i;
    count = ReadLong (stream);
    m_stringConstants.resize (count);
    for (i = 0; i < count; i++)
        m_stringConstants[i] = ReadString (stream);

    // Data type lookup table
    m_typeSet.StreamIn(stream);

    // Program code
    count = ReadLong(stream);
    m_code.resize(count);
    for (i = 0; i < count; i++)
        m_code[i].StreamIn(stream);

    // Program data (for "DATA" statements)
    count = ReadLong(stream);
    m_programData.resize(count);
    for (i = 0; i < count; i++)
        m_programData[i].StreamIn(stream);

    // User function prototypes
    count = ReadLong(stream);
    m_userFunctionPrototypes.resize(count);
    for (i = 0; i < count; i++)
        m_userFunctionPrototypes[i].StreamIn(stream);

    // User functions
    count = ReadLong(stream);
    m_userFunctions.resize(count);
    for (i = 0; i < count; i++)
        m_userFunctions[i].StreamIn(stream);

    // Code blocks
    count = ReadLong(stream);
    m_codeBlocks.resize(count);
    for (i = 0; i < count; i++)
        m_codeBlocks[i].StreamIn(stream);

    return true;
}
#endif
