#include "vmStackFrame.h"
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// vmUserFuncPrototype

vmUserFuncPrototype::vmUserFuncPrototype() {
    Reset();
}

void vmUserFuncPrototype::Reset() {
    paramCount = 0;
    localVarTypes.clear();
    localVarIndex.clear();
    hasReturnVal = false;
    returnValType = VTP_UNDEFINED;
}

int vmUserFuncPrototype::GetLocalVar(std::string name) {
    map<string,int>::iterator i = localVarIndex.find(name);
    return i == localVarIndex.end() ? -1 : i->second;
}

std::string vmUserFuncPrototype::GetLocalVarName(int index) const {

    // Return user variable name, given its index.
    // Not particularly efficient. Used for debugging functions (VM view etc)
    for (map<string,int>::const_iterator i = localVarIndex.cbegin();
            i != localVarIndex.cend();
            i++)
        if (i->second == index)
            return i->first;
    return "???";
}

bool vmUserFuncPrototype::Matches(const vmUserFuncPrototype& func) const {

    // Match return value
    if (hasReturnVal != func.hasReturnVal)
        return false;

    if (hasReturnVal && !returnValType.ExactEquals(func.returnValType))
        return false;

    // Match parameters
    if (paramCount != func.paramCount)
        return false;

    // Loop through local vars/params
    for (   map<string,int>::const_iterator i = localVarIndex.begin();
            i != localVarIndex.end();
            i++) {

        // Only interested in params
        if (i->second < paramCount) {

            // Other function must have param of same name and parameter index
            map<string,int>::const_iterator funcI = func.localVarIndex.find(i->first);

            // Param must exist
            if (funcI == func.localVarIndex.end())
                return false;

            // Must have same index
            if (i->second != funcI->second)
                return false;

            // Types must match
            if (!localVarTypes[i->second].ExactEquals(func.localVarTypes[funcI->second]))
                return false;
        }
    }

    // No differences found
    return true;
}

bool vmUserFuncPrototype::IsCompatibleWith(const vmUserFuncPrototype& func) const
{
	// Match return value
	if (hasReturnVal != func.hasReturnVal)
		return false;

	if (hasReturnVal && !returnValType.ExactEquals(func.returnValType))
		return false;

	// Match parameters
	if (paramCount != func.paramCount)
		return false;

	for (int i = 0; i < paramCount; i++)
		if (!localVarTypes[i].ExactEquals(func.localVarTypes[i]))
			return false;

	// No differences found
	return true;
}

int vmUserFuncPrototype::NewLocalVar(string name, vmValType& type) {
    int index = localVarTypes.size();

    // Create new variable definition
    localVarTypes.push_back(type);

    // Link name to index
    localVarIndex[name] = index;
    return index;
}

int vmUserFuncPrototype::NewParam(std::string name, vmValType& type) {

    // Parameters must be added before local variables
    assert((unsigned)paramCount == localVarTypes.size());

    // Add parameter like a local variable
    int index = NewLocalVar(name, type);

    // Track that it is a parameter
    paramCount++;

    return index;
}

#ifdef VM_STATE_STREAMING
void vmUserFuncPrototype::StreamOut(ostream& stream) const {

    // Return value
    WriteByte(stream, hasReturnVal);
    if (hasReturnVal)
        returnValType.StreamOut(stream);

    // Parameters/local variables
    WriteLong(stream, localVarTypes.size());
    for (int i = 0; i < localVarTypes.size(); i++) {
#ifdef STREAM_NAMES
        string name = GetLocalVarName(i);
        WriteString(stream, name);
#endif
        localVarTypes[i].StreamOut(stream);
    }
    WriteLong(stream, paramCount);
}

void vmUserFuncPrototype::StreamIn(istream& stream) {
    Reset();

    // Return value
    hasReturnVal = ReadByte(stream) != 0;
    if (hasReturnVal)
        returnValType.StreamIn(stream);

    // Parameters/local variables
    long count = ReadLong(stream);
    localVarTypes.resize(count);
    for (int i = 0; i < localVarTypes.size(); i++) {
#ifdef STREAM_NAMES
        string name = ReadString(stream);
        localVarIndex[name] = i;
#endif
        localVarTypes[i].StreamIn(stream);
    }
    paramCount = ReadLong(stream);
}
#endif


////////////////////////////////////////////////////////////////////////////////
// vmUserFunc

vmUserFunc::vmUserFunc(int _prototypeIndex, bool _implemented, int _programOffset)
        :   prototypeIndex(_prototypeIndex),
            implemented(_implemented),
            programOffset(_programOffset) {    
}

#ifdef VM_STATE_STREAMING
void vmUserFunc::StreamOut(std::ostream& stream) const
{

    // Assume program is complete, i.e all functions are implemented, before
    // streaming occurs.
    assert(implemented);
    WriteLong(stream, prototypeIndex);
    WriteLong(stream, programOffset);
}

void vmUserFunc::StreamIn(std::istream& stream) {
    prototypeIndex = ReadLong(stream);
    programOffset = ReadLong(stream);
    implemented = true;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// vmUserFuncStackFrame
void vmUserFuncStackFrame::InitForGosub(int _returnAddr) {
    userFuncIndex = -1;
    returnAddr = _returnAddr;
    localVarDataOffsets.clear();
}

void vmUserFuncStackFrame::InitForUserFunction(vmUserFuncPrototype& prototype, int _userFuncIndex) {
    userFuncIndex = _userFuncIndex;
    returnAddr = -1;

    // Allocate local variable data offsets
    localVarDataOffsets.resize(prototype.localVarTypes.size(), 0);
}

////////////////////////////////////////////////////////////////////////////////
// vmRuntimeFunction
#ifdef VM_STATE_STREAMING
void vmRuntimeFunction::StreamOut (std::ostream& stream) const
{
    WriteLong(stream, functionIndex);
}

void vmRuntimeFunction::StreamIn (std::istream& stream) {
    functionIndex = ReadLong(stream);
}
#endif

