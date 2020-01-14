//---------------------------------------------------------------------------
// Created 8-Dec-07: Thomas Mulgrew
//
// Virtual Machine run-time stack-frame entry, for user defined functions.
#ifndef vmStackFrameH
#define vmStackFrameH

#include "vmTypes.h"
#include <vector>
#include <map>
#include <string>

////////////////////////////////////////////////////////////////////////////////
//  vmUserFuncPrototype
//
/// Describes a user function call signiture and return value

struct vmUserFuncPrototype {

    // Paramter and local variable types
    // 0..paramCount-1                      -> Parameters
    // paramCount..localVarTypes.size()-1   -> Local variables
    std::vector<vmValType> localVarTypes;       // Local variable data types
    std::map<std::string,int> localVarIndex;    // Name->index lookup (parameters and local vars)
    int paramCount;
    bool hasReturnVal;
    vmValType returnValType;

    vmUserFuncPrototype();

    void Reset();

    // Return index of local variable, or if not found
    int GetLocalVar(std::string name);

    // Add a new local variable and return its index
    int NewLocalVar(std::string name, vmValType& type);
    int NewParam(std::string name, vmValType& type);

    // Find local variable name.
    // This is inefficient. Used for debugging views etc where performance is
    // not a big priority.
    std::string GetLocalVarName(int index) const;

    // Return true if this function protype matches the other one.
    bool Matches(const vmUserFuncPrototype& func) const;

	// Return true if this function is compatible with the other one.
	// This is similar to "Matches", except that "Matches" requires parameter names to match,
	// whereas "IsCompatibleWith" only requires parameters to be the same types.
	bool IsCompatibleWith(const vmUserFuncPrototype& func) const;

#ifdef VM_STATE_STREAMING
    void StreamOut(std::ostream& stream) const;
    void StreamIn(std::istream& stream);
#endif
};

////////////////////////////////////////////////////////////////////////////////
// vmUserFunc
//
// Describes a user function

struct vmUserFunc {
    int prototypeIndex;
    bool implemented;
    int programOffset;

    vmUserFunc() {};
    vmUserFunc(int _prototypeIndex, bool _implemented, int _programOffset = -1);

#ifdef VM_STATE_STREAMING
    void StreamOut(std::ostream& stream) const;
    void StreamIn(std::istream& stream);
#endif
};

////////////////////////////////////////////////////////////////////////////////
// vmUserFuncStackFrame
//
// Stack frame created when a user function is called.

struct vmUserFuncStackFrame {

    // Corresponding function definition. -1 if is a simple GOSUB
    int userFuncIndex;

    // Return address
    int returnAddr;

    // Previous stack frame info
    // (ignored for GOSUBs)
    int prevStackTop;
    int prevTempDataLock;
    int prevCurrentFrame;

    // Local variables and parameters
    // Stores offset of each variable in data array (0 = unallocated).
    // Elements 0..paramCount-1 are parameters, paramCount..size()-1 are local
    // variables.
    // (also ignored for GOSUBs)
    std::vector<int> localVarDataOffsets;

    void InitForGosub(int _returnAddr);
    void InitForUserFunction(vmUserFuncPrototype& prototype, int _userFuncIndex);
};

////////////////////////////////////////////////////////////////////////////////
//  vmStackDestructor
//
/// Indicates data in the function or temp stack area that needs to be destroyed
/// when the stack unwinds.
/// Currently our only "destruction" logic is for deallocating strings
/// referenced by the data.
struct vmStackDestructor {

    /// Address of data on stack or in temp space
    int addr;

    /// Index of data type
    int dataTypeIndex;

    vmStackDestructor(int _addr, int _dataTypeIndex) : addr(_addr), dataTypeIndex(_dataTypeIndex) {}
};

////////////////////////////////////////////////////////////////////////////////
//  vmProtectedStackRange
//
/// Indicates a range of data on the stack protected from destruction.
/// (Used when collapsing temporary data, to destroy everything except the data
/// to be returned).
struct vmProtectedStackRange {
    int startAddr, endAddr;
    vmProtectedStackRange() : startAddr(0), endAddr(0) {}
    vmProtectedStackRange(int _startAddr, int _endAddr) : startAddr(_startAddr), endAddr(_endAddr) {}

    bool ContainsAddr(int addr) { return addr >= startAddr && addr < endAddr; }
    bool ContainsRange(int start, int end) { return start >= startAddr && end <= endAddr; }
};

////////////////////////////////////////////////////////////////////////////////
//  vmRuntimeFunction
//
struct vmRuntimeFunction {

    /// Index of implementing function, or -1 if none.
    int functionIndex;

    vmRuntimeFunction() : functionIndex(-1) {}
    vmRuntimeFunction(int _functionIndex) : functionIndex(_functionIndex) {}
    
#ifdef VM_STATE_STREAMING
    void StreamOut (std::ostream& stream) const;
    void StreamIn (std::istream& stream);
#endif
};

#endif
