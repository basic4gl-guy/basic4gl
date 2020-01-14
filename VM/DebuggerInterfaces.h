/*  Created 03-Jun-07: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

    Interfaces for VM debugger objects.
*/
#ifndef DebuggerInterfacesH
#define DebuggerInterfacesH

#include "vmCode.h"
#include <list>
#include <string>

////////////////////////////////////////////////////////////////////////////////
// Breakpoint types

struct vmPatchedBreakPt {
    unsigned int    m_offset;                                       // Op-Code offset in program
    vmOpCode        m_replacedOpCode;                               // For active breakpoints: The op-code that has been replaced with the OP_BREAKPT.
};

struct vmTempBreakPt {
    unsigned int    m_offset;
};

struct vmUserBreakPt {
    unsigned int    m_offset;                                       // Note: Set to 0xffff if line is invalid.
};

struct SourceUserBreakPt {
    std::string sourceFile;
    int lineNo;

    SourceUserBreakPt(std::string _sourceFile, int _lineNo)
        : sourceFile(_sourceFile), lineNo(_lineNo) { }
    SourceUserBreakPt(const SourceUserBreakPt& b)
        : sourceFile(b.sourceFile), lineNo(b.lineNo) { }
};

typedef std::list<vmPatchedBreakPt> vmPatchedBreakPtList;
typedef std::list<vmTempBreakPt>    vmTempBreakPtList;
//typedef std::map<unsigned int, vmUserBreakPt> vmUserBreakPts;       // Maps line number to breakpoint structure

/*inline vmTempBreakPt TempBreakPt(int offset) {
    vmTempBreakPt bp;
    bp.m_offset = offset;
    return bp;
}*/

////////////////////////////////////////////////////////////////////////////////
//  IVMDebugger
//
/// Debugger interface presented to the virtual machine itself.
class IVMDebugger {
public:

    // Breakpoint access
    virtual int UserBreakPtCount() = 0;

    /// Return the line number of a user breakpoint.
    /// Note that this is the line number of the breakpoint in the main file.
    virtual int UserBreakPtLine(int index) = 0;
};

#endif
