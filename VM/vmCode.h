//---------------------------------------------------------------------------
/*  Created 7-Sep-2003: Thomas Mulgrew

    Code format and storage.
*/                                                 

#ifndef vmCodeH
#define vmCodeH
//---------------------------------------------------------------------------

#include "vmData.h"

#ifndef byte
typedef unsigned char byte;
#endif

////////////////////////////////////////////////////////////////////////////////
// vmOpCode
//
// Enumerated opcodes

enum vmOpCode {

    // General
    OP_NOP = 0x0,           // No operation
    OP_END,                 // End program
    OP_LOAD_CONST,          // Load constant into reg
    OP_LOAD_VAR,            // Load address of variable into reg
    OP_LOAD_LOCAL_VAR,      // Load address of local variable into reg
    OP_DEREF,               // Dereference reg (load value at [reg] into reg)
    OP_ADD_CONST,           // Add constant to reg. Used to address into a structure
    OP_ARRAY_INDEX,         // IN: reg = array index, reg2 = array address. OUT: reg = element address.
    OP_PUSH,                // Push reg to stack
    OP_POP,                 // Pop stack into reg2
    OP_SAVE,                // Save int, real or string in reg into [reg2]
    OP_COPY,                // Copy structure at [reg2] into [reg]. Instruction value points to data type.
    OP_DECLARE,             // Allocate a variable
    OP_DECLARE_LOCAL,       // Allocate a local variable
    OP_TIMESHARE,           // Perform a timesharing break
    OP_FREE_TEMP,           // Free temporary data
    OP_ALLOC,               // Allocate variable memory
    OP_DATA_READ,           // Read program data into data at [reg]. Instruction contains target data type.
    OP_DATA_RESET,          // Reset program data pointer
    OP_SAVE_PARAM,          // Save int, real or string in reg into parameter in pending stack frame. Instruction contains data type. Parameter # is however many parameters have been set since OP_CREATE_USER_FRAME was executed.
    OP_SAVE_PARAM_PTR,      // Save pointer to data in [reg] into parameter pointer.
    OP_COPY_USER_STACK,     // Copy data at [reg] to top of user stack. reg is then adjusted to point to the data in the user stack.
    OP_MOVE_TEMP,           // Free temp data and move data at [reg] into temp data. (Also handles if [reg] points to temp data).
    OP_CHECK_PTR,           // Check pointer scope before saving to variable. (Ptr is in reg, variable is [reg2])
    OP_CHECK_PTRS,          // Check all pointers in block at [reg] before copying to [reg2]. Instruction contains data type.
    OP_REG_DESTRUCTOR,      // Register string destruction block at [reg] (will be temp or stack depending on reg)
	OP_SWAP,				// Swap register 1 and 2
	OP_COND_TIMESHARE,		// Conditionally perform a timesharing break. Breaks only if m_timeshare has been set in VM.

    // Flow control
    OP_JUMP = 0x40,         // Unconditional jump
    OP_JUMP_TRUE,           // Jump if reg <> 0
    OP_JUMP_FALSE,          // Jump if reg == 0
    OP_CALL_FUNC,           // Call external function
	OP_UNUSED,				// UNUSED! Was OP_CALL_OPERATOR_FUNC
    OP_CALL_DLL,            // Call DLL function
    OP_CALL,                // Call VM function
    OP_CREATE_USER_FRAME,   // Create user stack frame in preparation for a call
    OP_CALL_USER_FUNC,      // Call user defined function
    OP_RETURN,              // Return from VM function
    OP_RETURN_USER_FUNC,    // Return from user defined function
    OP_NO_VALUE_RETURNED,   // Generates a runtime error if executed
    OP_BINDCODE,            // Bind a runtime code block to be executed
    OP_EXEC,                // Execute runtime code block
    OP_CREATE_RUNTIME_FRAME,// Create a stack frame to call a function/sub in runtime code block
    OP_END_CALLBACK,        // End callback initiated by built-in function or DLL function, and return control to that function.
	OP_CREATE_FUNC_PTR_FRAME,// Create stack frame for call to function pointer
	OP_CHECK_FUNC_PTR,		// Check function pointer in register is compatible with the specified prototype

    // Operations
    // Mathematical
    OP_OP_NEG = 0x60,
    OP_OP_PLUS,         // (Doubles as string concatenation)
    OP_OP_MINUS,
    OP_OP_TIMES,
    OP_OP_DIV,
    OP_OP_MOD,

    // Logical
    OP_OP_NOT = 0x80,
    OP_OP_EQUAL,
    OP_OP_NOT_EQUAL,
    OP_OP_GREATER,
    OP_OP_GREATER_EQUAL,
    OP_OP_LESS,
    OP_OP_LESS_EQUAL,
    OP_OP_AND,
    OP_OP_OR,
    OP_OP_XOR,

    // Conversion
    OP_CONV_INT_REAL = 0xa0,    // Convert integer in reg to real
    OP_CONV_INT_STRING,         // Convert integer in reg to string
    OP_CONV_REAL_STRING,        // Convert real in reg to string
    OP_CONV_REAL_INT,
    OP_CONV_INT_REAL2,          // Convert integer in reg2 to real
    OP_CONV_INT_STRING2,        // Convert integer in reg2 to string
    OP_CONV_REAL_STRING2,       // Convert real in reg2 to string
    OP_CONV_REAL_INT2,

    // Misc routine
    OP_RUN = 0xc0,              // Restart program. Reinitialises variables, display, state e.t.c

    // Debugging
    OP_BREAKPT = 0xe0           // Breakpoint
};

std::string vmOpCodeName (vmOpCode code);

////////////////////////////////////////////////////////////////////////////////
// vmInstruction
#pragma pack (push, 1)
struct vmInstruction {

    // Note:    vmInstruction size = 12 bytes.
    //          Ordering of member fields is important, as the two 4 byte members
    //          are first (and hence aligned to a 4 byte boundary).
    //          m_sourceChar is next, and aligned to a 2 byte boundary.
    //          Single byte fields are last, as their alignment is unimportant.
    vmValue         m_value;                // Value
    unsigned int    m_sourceLine;           // For debugging
    unsigned short  m_sourceChar;
    byte            m_opCode;               // (vmOpCode)
    signed char     m_type;                 // (vmBasicVarType)

    vmInstruction () {
        m_opCode        = 0;
        m_type          = 0;
        m_sourceLine    = 0;
        m_sourceChar    = 0;
        m_value         = 0;
    }
    vmInstruction (const vmInstruction& i) {
        m_opCode        = i.m_opCode;
        m_type          = i.m_type;
        m_sourceChar    = i.m_sourceChar;
        m_sourceLine    = i.m_sourceLine;
        m_value         = i.m_value;
    }
    vmInstruction (byte opCode, byte type, vmValue val, int sourceLine = 0, int sourceChar = 0) {
        m_opCode        = opCode;
        m_type          = type;
        m_value         = val;
        m_sourceLine    = sourceLine;
        m_sourceChar    = sourceChar;
    }

    // Streaming
#ifdef VM_STATE_STREAMING
    void StreamOut (std::ostream& stream) const;
    void StreamIn (std::istream& stream);
#endif
};
#pragma pack (pop)

#endif
