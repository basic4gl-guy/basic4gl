/*  Created 2-Jun-2007: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

    Interfaces for preprocessor objects.
*/
#ifndef PreprocessorInterfacesH
#define PreprocessorInterfacesH

#include <string>

////////////////////////////////////////////////////////////////////////////////
//  ISourceFile
//
/// Interface to a source file.
class ISourceFile {
public:

    /// Return the next line of source code
    virtual std::string GetNextLine() = 0;

    /// The filename
    virtual std::string Filename() = 0;
    
    /// Return the line number. 0 = Top line of file.
    virtual int LineNumber() = 0;

    /// True if reached End of File
    virtual bool Eof() = 0;

    /// Called when preprocessor is finished with the source file
    virtual void Release() = 0;
};

////////////////////////////////////////////////////////////////////////////////
//  ISourceFileServer
//
/// Serves source files by filename
class ISourceFileServer {
public:
	virtual ~ISourceFileServer() {}

	/// Open source file and return interface.
    virtual ISourceFile* OpenSourceFile(std::string filename) = 0;
};

////////////////////////////////////////////////////////////////////////////////
//  ILineNumberMapping
//
/// Used to map line numbers in source files to line numbers of the main file
/// that is built after the pre-processor expands includes.
/// Mainly used for debugging, so that breakpoint and IP positions can be
/// displayed in the correct positions in the correct source files.
class ILineNumberMapping {
public:

    /// Return the filename and line number corresponding to a main line number.
    virtual void SourceFromMain(std::string& filename, int& fileLineNo, int lineNo) = 0;

    /// Return line number within a specific file that corresponds to a main line number.
    /// Returns -1 if line number does not correspond to the file specified.
    virtual int SourceFromMain(std::string filename, int lineNo) = 0;

    /// Returns main line number from file and line number
    virtual int MainFromSource(std::string filename, int fileLineNo) = 0;
};

#endif
