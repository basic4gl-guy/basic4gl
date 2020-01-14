/*	EmbeddedFiles.h
	Copyright (C) Tom Mulgrew, 2004 (tmulgrew@slingshot.co.nz)

	Mechanism for embedding files inside executables.
*/

#ifndef _embeddedfiles_h
#define _embeddedfiles_h

#include "misc.h"				// Contains some #pragmas to disable annoying warning about identifier length
#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <set>
#include "HasErrorState.h"
#include "FilenameRoutines.h"

// Generic type for input/output streams
// (Compatible with ifstream and stringstream)
typedef std::basic_istream<char> GenericIStream;
typedef std::basic_ostream<char> GenericOStream;

///////////////////////////////////////////////////////////////////////////////
// EmbeddedFile
//
// A single file, embedded into the executable
class EmbeddedFile {
	std::string m_filename;
	int			m_length;
	char		*m_data;
public:
	EmbeddedFile () : m_filename (""), m_length (0), m_data (NULL) { ; }
	EmbeddedFile (char *rawData);
	EmbeddedFile (char *rawData, int& offset);
	std::string Filename () { return m_filename; }
	GenericIStream *AsStream ();		// Return file as a generic input stream
    int Length() { return m_length; }
};

///////////////////////////////////////////////////////////////////////////////
// EmbeddedFiles
//
// A set of embedded files, keyed by relative filename
typedef std::map<std::string,EmbeddedFile> EmbeddedFileMap;
class EmbeddedFiles {
	EmbeddedFileMap		m_files;
public:
	EmbeddedFiles () { ; };
	EmbeddedFiles (char *rawData);
	EmbeddedFiles (char *rawData, int& offset);
	void AddFiles (char *rawData, int& offset);

	bool IsStored (std::string filename);

	// Find stream.
	// Caller must free
	GenericIStream *Open        (std::string filename);		// Opens file. Returns NULL if not present.
    GenericIStream *Open        (std::string filename, int& length);    
	GenericIStream *OpenOrLoad  (std::string filename);		// Opens file. Falls back to disk if not present. Returns NULL if not present OR on disk
    GenericIStream *OpenOrLoad  (std::string filename, int& length);
};

///////////////////////////////////////////////////////////////////////////////
// FileOpener
//
// Abstraction layer for opening files.
// Files opened for input can either be embedded files or actual disk files.
// Can also apply security.

class FileOpener : public HasErrorState {
	EmbeddedFiles m_embeddedFiles;
    std::set<std::string> tempFiles;

    std::string ExtractStoredFile(std::string filename);
    void DeleteTempFile(std::string filename);

public:
	FileOpener ();
	FileOpener (char *rawData);
    ~FileOpener();
	void AddFiles (char *rawData);
	void AddFiles (char *rawData, int& offset);

	bool CheckFilesFolder (std::string filename);			// Returns true if file is in the "files\" subfolder in the current directory

	// Both functions return a newly allocated stream if successful, or NULL if not (use the Error() and GetError() methods to find what the problem was)
	// If files folder is true then the file will only be opened if it is in the "files\" subfolder in the current directory.
	GenericIStream *OpenRead(std::string filename, bool filesFolder = true);
	GenericOStream *OpenWrite(std::string filename, bool filesFolder = true);
    GenericIStream *OpenRead(std::string filename, bool filesFolder, int& length);

    // The following function returns a filename that can be opened in read mode.
    // If the input filename corresponds to an embedded file, the embedded file
    // is copied to a temporary file on the drive, and that filename is returned
    // instead.
    // (Use this when the file opening code expects to see a real disk file and
    // cannot be rewritten to work from a memory stream.)
    // Returns a blank string if not successful (use Error() and GetError() to retrieve the error description.)
    std::string FilenameForRead(std::string filename, bool filesFolder = true);

    // Delete a disk file
    bool Delete(std::string filename, bool isSandboxMode);
};

///////////////////////////////////////////////////////////////////////////////
// Routines

// Copy a GenericIStream into a GenericOStream
void CopyStream (GenericIStream& src, GenericOStream& dst, int len = -1);

// Create embedded representation of stream
bool EmbedFile (std::string filename, GenericOStream& stream);

#endif
