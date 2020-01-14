#pragma once

#include "TomComp.h"
#include <QTextStream>
#include <QCompleter>

class ISourceFileFormHost
{
public:
	virtual ~ISourceFileFormHost() 
	{
	}

	virtual void SourceIsModified() = 0;
	virtual TomBasicCompiler& GetCompiler() = 0;
	virtual void FilenameIsModified(bool isMainFile) = 0;
	virtual void OpenIncludeFile(QString relativeFilename) = 0;
	virtual bool IsBreakpoint(QString absoluteFilename, int lineNumber) = 0;
	virtual void ToggleBreakpoint(QString absoluteFilename, int lineNumber) = 0;
	virtual bool IsInstructionPointer(QString absoluteFilename, int lineNumber) = 0;
	virtual QString EvaluateVariable(QString variable, bool canCallFunctions) = 0;
	virtual bool IsProgramRunning() = 0;
	virtual void WriteFileHeader(QTextStream& stream) = 0;
	virtual QCompleter* GetCompleter() = 0;
	virtual QString GetFunctionSignature(QString function, int paramNumber) = 0;
	virtual bool IsAutocompleteEnabled() = 0;
	virtual bool IsFunctionSignatureEnabled() = 0;
};

