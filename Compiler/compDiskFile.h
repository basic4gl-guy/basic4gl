/*  Created 20-May-2007: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

    Represents a file on disk.
*/
#ifndef compDiskFileH
#define compDiskFileH

#include "compParseInterfaces.h"
#include <fstream>

////////////////////////////////////////////////////////////////////////////////
//  DiskSourceFile
//
/// Implements ISourceFile for disk files.
class DiskSourceFile : public ISourceFile {
private:
    std::ifstream file;
    
public:
    DiskSourceFile(std::string filename);

    // ISourceFile
    virtual void Reset();
    virtual std::string GetNextLine();
    virtual bool Eof();
};

////////////////////////////////////////////////////////////////////////////////
//  DiskFileServer
//
/// Implements ISourceFileServer for disk files.
class DiskFileServer : public ISourceFileServer {
public:
    // ISourceFileServer    
    virtual ISourceFile* GetFile(std::string filename);
    virtual std::string ProcessPath(std::string path);
};  

#endif
