/*	EmbeddedFiles.cpp
	Copyright (C) Tom Mulgrew, 2004 (tmulgrew@slingshot.co.nz)

	Mechanism for embedding files inside executables.
*/

#include "EmbeddedFiles.h"
#include <assert.h>
#include <string.h>
#include <windows.h>
#include "vmTypes.h"
#include "AppSettings.h"

///////////////////////////////////////////////////////////////////////////////
// EmbeddedFile

EmbeddedFile::EmbeddedFile (char *rawData) {
	int offset = 0;
	EmbeddedFile (rawData, offset);
}
EmbeddedFile::EmbeddedFile (char *rawData, int& offset) {
	assert (rawData != NULL);

	// Read filename length
	int nameLength = *((int *) (rawData + offset));
	offset += sizeof (nameLength);

	// Read filename
	m_filename = rawData + offset;
	offset += nameLength;

	// Read length
	m_length = *((int *) (rawData + offset));
	offset += sizeof (m_length);

	// Save pointer to data
	m_data = rawData + offset;
	offset += m_length;
}
GenericIStream *EmbeddedFile::AsStream () {
	std::stringstream *result = new std::stringstream;		// Use a string stream as temp buffer
//	MessageBox(NULL, IntToString(m_length).c_str(), "DEBUG", MB_OK);
	result->write (m_data, m_length);						// Copy file data into it
	result->seekg (0, std::ios::beg);						// Reset to start
	return result;
}

///////////////////////////////////////////////////////////////////////////////
// EmbeddedFiles
EmbeddedFiles::EmbeddedFiles (char *rawData) {
	int offset = 0;
	EmbeddedFiles (rawData, offset);
}
EmbeddedFiles::EmbeddedFiles (char *rawData, int& offset) {
	AddFiles (rawData, offset);
}
void EmbeddedFiles::AddFiles (char *rawData, int& offset) {
	assert (rawData != NULL);
	
	// Read # of files
	int count = *((int *) (rawData + offset));
	offset += sizeof (count);

	// Read in each file
	for (int i = 0; i < count; i++) {
		EmbeddedFile f (rawData, offset);
		m_files [f.Filename ()] = f;
//		MessageBox(NULL, f.Filename().c_str(), "Debug", MB_OK);
	}	
}
bool EmbeddedFiles::IsStored (std::string filename) {
	std::string processedName = ProcessPath (filename);
	return m_files.find (processedName) != m_files.end ();
}
GenericIStream *EmbeddedFiles::Open (std::string filename) {
	return IsStored (filename)
		? m_files [ProcessPath (filename)].AsStream ()
		: NULL;
}
GenericIStream *EmbeddedFiles::Open(std::string filename, int& length) {
    if (IsStored(filename)) {
        EmbeddedFile& file = m_files[ProcessPath(filename)];
        length = file.Length();
        return file.AsStream();
    }
    else {
        length = 0;
        return NULL;
    }
}

GenericIStream *EmbeddedFiles::OpenOrLoad (std::string filename) {

	// Try embedded files first
	GenericIStream *result = Open (filename);
	if (result == NULL) {

		// Otherwise try to load from file
		std::ifstream *diskFile = new std::ifstream (filename.c_str (), std::ios::binary | std::ios::in);
		if (!diskFile->fail ())
			result = diskFile;
		else
			delete diskFile;
	}
	return result;
}

GenericIStream *EmbeddedFiles::OpenOrLoad (std::string filename, int& length) {

    // 17-Apr-06:
    // There's an issue with Borland C++ v5.5's implementation of stringstream
    // in that:
    //      seekg(0, std::ios::end);
    //      length = tellg();
    // always sets length to 0.
    //
    // Therefore we have this method for returning a file stream AND its length.
    // If the file is stored as an embedded file, we simply return the length
    // from the embedded file.
    // If the file is loaded from disk we use seekg and tellg to get the file
    // length (which still works fine for ifstream streams).

    GenericIStream *result = Open(filename, length);
    if (result == NULL) {
		// Otherwise try to load from file
		std::ifstream *diskFile = new std::ifstream (filename.c_str (), std::ios::binary | std::ios::in);
		if (!diskFile->fail ()) {

            // Get file length
            diskFile->seekg(0, std::ios::end);      // Seek to end
            length = diskFile->tellg();             // Return length
            diskFile->seekg(0, std::ios::beg);      // Seek back to beginning

			result = diskFile;
        }
		else
			delete diskFile;
	}
	return result;
}

///////////////////////////////////////////////////////////////////////////////
// FileOpener


FileOpener::FileOpener () { 
	; 
}
FileOpener::FileOpener (char *rawData) : m_embeddedFiles (rawData) { 
	; 
}

FileOpener::~FileOpener() {

    // Delete any temporary files created
    for (   std::set<std::string>::iterator i = tempFiles.begin();
            i != tempFiles.end();
            i++)
        DeleteFile(i->c_str());
}

void FileOpener::AddFiles (char *rawData) {
	int offset = 0;
	m_embeddedFiles.AddFiles (rawData, offset);
}
void FileOpener::AddFiles (char *rawData, int& offset) {
	m_embeddedFiles.AddFiles (rawData, offset);
}

bool StartsWith(std::string str, std::string start)
{
	return str.length() >= start.length() && strncmp(str.c_str(), start.c_str(), start.length()) == 0;
}

bool FileOpener::CheckFilesFolder (std::string filename) {
	std::string fullPath = PrepPathForComp(GetFullPath(filename));

	// Can access files in the current folder
	std::string currentDir = PrepPathForComp(IncludeSlash(GetCurrentFolder()));
	if (StartsWith(fullPath, currentDir)) return true;

	// Can access files in App_Data\Basic4GL for the current user
	std::string appDataUser = PrepPathForComp(IncludeSlash(GetAppDataFolder(false)) + "Basic4GL\\");
	if (StartsWith(fullPath, appDataUser)) return true;

	// Can access files in App_Data\Basic4GL for all users
	std::string AppDataAll = PrepPathForComp(IncludeSlash(GetAppDataFolder(true)) + "Basic4GL\\");
	if (StartsWith(fullPath, AppDataAll)) return true;

	SetError("You can only open files in the current directory or below, or in the Basic4GL App Data folder.");
	return false;

	//// Find full pathname
	//char fullPath[1024], currentDir[1024];
	//char *fileBit;
	//GetFullPathName(filename.c_str (), 1024, fullPath, &fileBit);
	//GetCurrentDirectory(1024, currentDir);
	//PrepPathForComp(fullPath);
	//PrepPathForComp(currentDir);

 //   // Truncate
 //   fullPath[strlen(currentDir)] = 0;
 //   if (strcmp(fullPath, currentDir) == 0)
 //       return true;
 //   else {
 //       SetError("You can only open files in the current directory or below.");
 //       return false;
 //   }
/*
	// Return true if the relative filename starts with "files\"
	// I.e the file is in the files\ subdirectory of the current directory
	if (ProcessPath (filename).substr (0, 6) == "files\\")
		return true;
	else {
		SetError ("You can only open files in the \"files\" subdirectory.");
		return false;
	}*/
}
GenericIStream *FileOpener::OpenRead (std::string filename, bool filesFolder) {
	ClearError ();
	if (filesFolder && !CheckFilesFolder (filename))
		return NULL;
	else {
		GenericIStream *result = m_embeddedFiles.OpenOrLoad (filename);
		if (result == NULL)
			SetError ("Failed to open " + filename);
		return result;
	}
}
GenericIStream *FileOpener::OpenRead(std::string filename, bool filesFolder, int& length) {
    ClearError();
	if (filesFolder && !CheckFilesFolder (filename)) {
        length = 0;
		return NULL;
    }
    else {
        GenericIStream *result = m_embeddedFiles.OpenOrLoad(filename, length);
        if (result == NULL)
            SetError("Failed to open " + filename);
        return result;
    }
}
GenericOStream *FileOpener::OpenWrite (std::string filename, bool filesFolder) {
	ClearError ();
	if (filesFolder && !CheckFilesFolder (filename))
		return NULL;
	else {
		std::ofstream *file = new std::ofstream (filename.c_str (), std::ios::binary | std::ios::out);
		if (file->fail ()) {
			delete file;
			SetError ("Failed to open " + filename);
			return NULL;
		}
		else
			return file;
	}
}

std::string FileOpener::ExtractStoredFile(std::string filename) {
//			MessageBox(NULL, "Embedded file", "Debug", MB_OK);

    // Find data in embedded file
    int len = 0;
    GenericIStream *data = m_embeddedFiles.Open(filename, len);
    if (data == NULL) {
//				MessageBox(NULL, "Error opening embedded file", "Debug", MB_OK);
        SetError ("Error opening embedded file");
        return "";
    }

    // Create a temporary file in OSes temp directory
    char path[MAX_PATH], tempFilename[MAX_PATH];
    GetTempPath(MAX_PATH - 1, path);
    GetTempFileName(path, "b4gltmp", 0, tempFilename);

    // Copy to temp file
    std::ofstream tempFile(tempFilename, std::ios::out | std::ios::binary);
    data->seekg(0, std::ios::beg);
    CopyStream(*data, tempFile, len);
    delete data;
    if (tempFile.fail ()) {
//				MessageBox(NULL, "Error creating temp file", "Debug", MB_OK);
        SetError ("Error creating temp file");
        return "";
    }

    // Track temp filename, so we can delete it later
    std::string name = (char*)tempFilename;
    tempFiles.insert(name);

    // Return new filename
    return name;
}

std::string FileOpener::FilenameForRead (std::string filename, bool filesFolder) {
//	MessageBox(NULL, filename.c_str(), "Debug", MB_OK);

	ClearError ();
	if (filesFolder && !CheckFilesFolder (filename))
		return "";
    else {

        // Stored in embedded file?
        if (m_embeddedFiles.IsStored (filename))
        {
            return ExtractStoredFile(filename);
        }
        else {

//			MessageBox(NULL, "Disk file", "Debug", MB_OK);

            // Not embedded. Return input filename.
            return filename;
		}
    }
}

bool FileOpener::Delete(std::string filename, bool isSandboxMode) {

    ClearError();

    if (isSandboxMode) {
        SetError("Deleting files is not allowed when safe mode is switched on");
        return false;
    }

    if (DeleteFile(filename.c_str()))
        return true;
    else {
        SetError("Delete failed");
        return false;
    }
}

void FileOpener::DeleteTempFile(std::string filename) {
    if (tempFiles.find(filename) != tempFiles.end()) {
        DeleteFile(filename.c_str());
        tempFiles.erase(filename);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Routines

void CopyStream (GenericIStream& src, GenericOStream& dst, int len) {

	// Copy stream to stream
	char buffer [0x4000];
	while (len > 0x4000 || len < 0 && !src.fail()) {
		src.read (buffer, 0x4000);
		dst.write (buffer, 0x4000);
		len -= 0x4000;
	}
	if (len > 0) {
		src.read (buffer, len);
		dst.write (buffer, len);
	}
}

bool EmbedFile (std::string filename, GenericOStream& stream) {
	
	// Open file
	std::ifstream file (filename.c_str (), std::ios::binary | std::ios::in);
	if (file.fail ())
		return false;

	// Convert filename to relative
	std::string relName = ProcessPath (filename);

	// Calculate lengths
	int nameLen = relName.length () + 1;		// +1 for 0 terminator
	file.seekg (0, std::ios::end);
	int fileLen = file.tellg ();
	file.seekg (0, std::ios::beg);

	// Write data to stream
	stream.write ((char *) &nameLen, sizeof (nameLen));
	stream.write (relName.c_str (), nameLen);
	stream.write ((char *) &fileLen, sizeof (fileLen));
    CopyStream (file, stream, fileLen);
	return true;
}
