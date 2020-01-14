#include "vmDebugger.h"
using namespace std;

////////////////////////////////////////////////////////////////////////////////
//  Debugger

Debugger::Debugger(ILineNumberMapping& _mapping)
        : mapping(_mapping) {
}

void Debugger::InsertDeleteLines(string filename, int fileLineNo, int delta) {

    // Move or remove user breakpoints
    vector<SourceUserBreakPt>::iterator i = userBreakPts.begin();
    while (i != userBreakPts.end()) {
        if (i->sourceFile == filename) {

            // Breakpoints before modified line(s) are unaffected
            if (i->lineNo >= fileLineNo) {
                if (delta >= 0) {
                    // Insert
                    i->lineNo += delta;
                    i++;
                }
                else {
                    // Delete
                    // Is breakpoint on a deleted line?
                    if (i->lineNo < fileLineNo - delta)
                        i = userBreakPts.erase(i);
                    else {
                        i->lineNo += delta;
                        i++;
                    }
                }
            }
            else
                i++;
        }
        else
            i++;
    }
}

int Debugger::UserBreakPtCount() {
    return userBreakPts.size();
}

int Debugger::UserBreakPtLine(int index) {
    assert(index >= 0);
    assert(index < UserBreakPtCount());

    // Find breakpoint and convert to main line number
    return mapping.MainFromSource(
        userBreakPts[index].sourceFile,
        userBreakPts[index].lineNo);
}

vector<SourceUserBreakPt>::iterator Debugger::FindBreakPt(string filename, int lineNo) {

    // Search for matching breakpt
    vector<SourceUserBreakPt>::iterator i = userBreakPts.begin();

    // Stop when end is reached or breakpt found
    while (i != userBreakPts.end() &&
            !(i->sourceFile == filename && i->lineNo == lineNo))
        i++;

    return i;
}

bool Debugger::IsUserBreakPt(string filename, int lineNo) {
    return FindBreakPt(filename, lineNo) != userBreakPts.end();
}

void Debugger::ClearUserBreakPts() {
    userBreakPts.clear();
}

void Debugger::AddUserBreakPt(string filename, int lineNo) {
    if (!IsUserBreakPt(filename, lineNo))
        userBreakPts.push_back(SourceUserBreakPt(filename, lineNo));
}

void Debugger::RemoveUserBreakPt(string filename, int lineNo) {
    vector<SourceUserBreakPt>::iterator i = FindBreakPt(filename, lineNo);
    if (i != userBreakPts.end())
        userBreakPts.erase(i);
}

void Debugger::ToggleUserBreakPt(string filename, int lineNo) {
    if (IsUserBreakPt(filename, lineNo))
        RemoveUserBreakPt(filename, lineNo);
    else
        AddUserBreakPt(filename, lineNo);
}

