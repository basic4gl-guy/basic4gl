//---------------------------------------------------------------------------
/*  Created 7-Dec-2003: Thomas Mulgrew

    Debugger types and routines
*/
#ifndef vmDebuggerH
#define vmDebuggerH
//---------------------------------------------------------------------------
#include "DebuggerInterfaces.h"
#include "compPreprocessor.h"

////////////////////////////////////////////////////////////////////////////////
//  Debugger
//
/// Debugger helper object.
class Debugger : public IVMDebugger {
    std::vector<SourceUserBreakPt> userBreakPts;
    ILineNumberMapping& mapping;

    // Return breakpoint position or userBreakPts.end() if not found.
    std::vector<SourceUserBreakPt>::iterator FindBreakPt(std::string filename, int lineNo);
public:
    Debugger(ILineNumberMapping& _mapping);

    // UI interface
    void ClearUserBreakPts();
    void AddUserBreakPt(std::string filename, int lineNo);
    void RemoveUserBreakPt(std::string filename, int lineNo);
    void ToggleUserBreakPt(std::string filename, int lineNo);
    bool IsUserBreakPt(std::string filename, int lineNo);

    /// Adjust debugging information in response to an insertion/deletion of
    /// one or more lines.
    /// If delta is positive, |delta| lines will be inserted.
    /// If delta is negative, |delta| lines will be deleted.
    /// Breakpoints will be moved (or deleted) appropriately.
    void InsertDeleteLines(std::string filename, int fileLineNo, int delta); 

    // IVMDebugger methods
    virtual int UserBreakPtCount();
    virtual int UserBreakPtLine(int index);
};

#endif
