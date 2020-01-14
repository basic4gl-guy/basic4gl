/*	MainFile.h
	Copyright (C) Tom Mulgrew 2008 (tmulgrew@slingshot.co.nz)

	Main source file implementation.
    Wraps around another ISourceFile. Provides methods for extracting the plugin
    DLL information at the start of the file. The remaining file can then be
    passed into the preprocessor and compiler.
*/

#ifndef _mainfile_h
#define _mainfile_h

#include "PreprocessorInterfaces.h"

class MainSourceFile : public ISourceFile {
    ISourceFile* file;
    std::string line;
    bool eof;
    int lineNumber;

    void NextLine();
public:
    MainSourceFile(ISourceFile* _file);

    // ISourceFile
    virtual std::string GetNextLine();
    virtual std::string Filename();
    virtual int LineNumber();
    virtual bool Eof();
    virtual void Release();

    // Plugin detection
    // Returns the name of the next plugin to load and moves to the next line.
    // If line does not contain a plugin, returns ""
    std::string GetPlugin();
};

#endif
