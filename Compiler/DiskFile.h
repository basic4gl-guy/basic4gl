/*  Created 2-Jun-07: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

    Disk file implementation of ISourceFile and ISourceFileServer.
*/
#ifndef DiskFileH
#define DiskFileH

#include "PreprocessorInterfaces.h"
#include <fstream>
#include <string>

////////////////////////////////////////////////////////////////////////////////
//  DiskFile
//
/// Disk file implementation of ISourceFile
class DiskFile : public ISourceFile {
    std::ifstream file;
    std::string filename;
    int lineNo;
public:
    DiskFile(std::string _filename);

    bool Fail();

    // ISourceFile methods
    virtual std::string Filename();
    virtual int LineNumber();
    virtual std::string GetNextLine();
    virtual bool Eof();
    virtual void Release();
};

////////////////////////////////////////////////////////////////////////////////
//  DiskFileServer
//
/// Disk file implementation of ISourceFileServer
class DiskFileServer : public ISourceFileServer {
public:
	virtual ~DiskFileServer()
	{
	}

	// ISourceFileServer methods
    virtual ISourceFile* OpenSourceFile(std::string filename);
};

#endif
