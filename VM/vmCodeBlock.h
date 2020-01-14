//---------------------------------------------------------------------------
// Created 28-Jan-08: Thomas Mulgrew
//
// A separate code block
#ifndef vmCodeBlockH
#define vmCodeBlockH

#include "vmStackFrame.h"

////////////////////////////////////////////////////////////////////////////////
//  vmCodeBlock
//
/// Represents a block of code.
/// The program is code block 0. Any other files/strings compiled at run time
/// are also separate code blocks.
struct vmCodeBlock {
private:
	std::string filename;

public:
	int programOffset;          // -1 if code block is invalid (e.g. because of compile error)
    std::vector<vmRuntimeFunction> runtimeFunctions;
	std::map<std::string, int> userFunctions;

    vmCodeBlock() : programOffset(-1) {}
    void SetLengthAtLeast(int length);
    vmRuntimeFunction& GetRuntimeFunction(int index);
	void SetFilename(std::string _filename);
	bool FilenameEquals(std::string _filename);

#ifdef VM_STATE_STREAMING
    void StreamOut (std::ostream& stream) const;
    void StreamIn (std::istream& stream);
#endif
};

#endif
