//---------------------------------------------------------------------------
/*  Created 02-Nov-2003: Thomas Mulgrew

    File I/O routines
*/                                                  


#pragma hdrstop

#include "TomFileIOBasicLib.h"
#include "vmData.h"
#include <fstream>
#include <windows.h>
#include <shlobj.h>
#include "EmbeddedFiles.h"
#include "AppSettings.h"
#include "Basic4GLStandardObjects.h"
#include "FilenameRoutines.h"

using namespace std;

//---------------------------------------------------------------------------

#ifndef _MSC_VER
#pragma package(smart_init)
#endif

inline bool IsSandboxMode() {
    return GetAppSettings()->IsSandboxMode();
}

////////////////////////////////////////////////////////////////////////////////
//  FileAccessorAdapter
//
//
class FileAccessorAdapter : public IB4GLFileAccessor {
public:
    // IB4GLFileAccessor interface
    virtual void DLLFUNC GetFilenameForRead(const char* filename, char* dst);
};

////////////////////////////////////////////////////////////////////////////////
//  FileStream

FileStream::~FileStream () {
    if (in != NULL)
        delete in;
    if (out != NULL)
        delete out;
}

// State variables
static FileOpener *files;
FileAccessorAdapter pluginAdapter;
FileStreamStore fileStreams;
std::string lastError = "";
HANDLE findFileHandle = NULL;
WIN32_FIND_DATA findFileData;

void CloseFind() {
    if (findFileHandle != NULL && findFileHandle != INVALID_HANDLE_VALUE) {
        FindClose(findFileHandle);
        findFileHandle = NULL;
    }
}

// Pre-run initialisation
static void Init (TomVM& vm) {

    // Clear error state
    lastError = "";
    CloseFind();
}

FileStream *stream;
bool GetStream (int index) {

    // Get file stream and store in stream variable
    if (index > 0 && fileStreams.IndexStored (index)) {
        stream = fileStreams.Value (index);
        lastError = "";
        return true;
    }
    else {
	    stream = NULL;
        lastError = "Invalid file handle";
        return false;
    }
}

bool GetIStream (int index) {
	if (!GetStream (index))
		return false;
	if (stream->in == NULL) {
		lastError = "File not in INPUT mode";
		return false;
	}
	return true;
}

bool GetOStream (int index) {
	if (!GetStream (index))
		return false;
	if (stream->out == NULL) {
		lastError = "File not in OUTPUT mode";
		return false;
	}
	return true;
}

bool UpdateError (std::string operation) {
	if (    (stream != NULL)
        &&  (   (stream->in	!= NULL && stream->in->fail ())
		    ||	(stream->out != NULL && stream->out->fail ())
        )) {
        lastError = operation + " failed";
        return false;
    }
    else {
        lastError = "";
        return true;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Function wrappers
void WrapFileError (TomVM& vm) {
    vm.RegString () = lastError;
}

void WrapEndOfFile (TomVM& vm) {
    vm.Reg ().IntVal () = -1;
    if (!GetStream (vm.GetIntParam (1)))
        return;
	if (	(stream->in	!= NULL && !stream->in->eof  ())
		||	(stream->out != NULL && !stream->out->eof ()))
        vm.Reg ().IntVal () = 0;
}

int InternalOpenFileRead (std::string filename) {

	// Attempt to open file
	GenericIStream *file = files->OpenRead (filename, IsSandboxMode());
	if (file == NULL) {
		lastError = files->GetError ();
		return 0;
	}
	else {
		lastError = "";
		return fileStreams.Alloc (new FileStream (file));
	}
}

int InternalOpenFileWrite (std::string filename) {

	// Attempt to open file
	GenericOStream *file = files->OpenWrite (filename, IsSandboxMode());
	if (file == NULL) {
		lastError = files->GetError ();
		return 0;
	}
	else {
		lastError = "";
		return fileStreams.Alloc (new FileStream (file));
	}
}

int InternalOpenAppDataRead(std::string subFolder, std::string filename, bool allUsers) {

    // Get application data path
    std::string appDataPath = GetAppDataFolder(allUsers);
    if (appDataPath == "") return 0;

    // Build file path, and open file
	std::string fullFilename = IncludeSlash(appDataPath) + "Basic4GL\\" + IncludeSlash(subFolder) + filename;
    return InternalOpenFileRead(fullFilename);
}

int InternalOpenAppDataWrite(std::string subFolder, std::string filename, bool allUsers) {

    // Get application data path
    std::string appDataPath = GetAppDataFolder(allUsers);
    if (appDataPath == "") return 0;

    // Create folder if it doesn't already exist
	std::string basic4glAppDataPath = IncludeSlash(appDataPath) + "Basic4GL";
    std::string path = basic4glAppDataPath + "\\" + subFolder;

	// Check access to path before creating folder.
	// Anything inside appdata\basic4gl should be allowed, but this should catch any attempts
	// to break out using the parent path specifier (e.g. subfolder = "..\..\some other folder").
	// Do this before creating any directory.
	if (!files->CheckFilesFolder(path))
	{
		lastError = files->GetError();
		return 0;
	}

	// Create basic4GL folder
	if (!CreateDirectory(basic4glAppDataPath.c_str(), nullptr))
	{
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
			lastError = "Unable to create Basic4GL folder in App Data";
			return 0;
		}
	}

	// Create subfolder in Basic4GL folder
    if (!CreateDirectory(path.c_str(), NULL)) {
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            lastError = "Unable to create folder in App Data";
            return 0;
        }
    }

    // Build file path, and open file
    std::string fullFilename = IncludeSlash(path) + filename;
    return InternalOpenFileWrite(fullFilename);
}

/*int InternalOpenFile (std::string filename, int mode) {
    int result = 0;                                                     // 0 = failed

    // Check whether we are allowed to do this
    if (AllowedToOpenFile (filename)) {

        // Attempt to open file for input
        fstream *f = new fstream (filename.c_str (), mode);
        if (!f->fail ()) {
            result = fileStreams.Alloc (f);                             // Return handle. (Guaranteed not to be 0)
            lastError = "";
        }
        else {
            delete f;
            lastError = (std::string) "Failed to open: " + filename;
        }
    }
    return result;
}*/

void WrapOpenFileRead (TomVM& vm) {
	vm.Reg ().IntVal () = InternalOpenFileRead (vm.GetStringParam (1));
}

void WrapOpenFileWrite (TomVM& vm) {
	vm.Reg ().IntVal () = InternalOpenFileWrite (vm.GetStringParam (1));
}

void WrapOpenAppDataRead (TomVM& vm) {
	vm.Reg ().IntVal () = InternalOpenAppDataRead (vm.GetStringParam(3), vm.GetStringParam(2), vm.GetIntParam(1) != 0);
}

void WrapOpenAppDataWrite (TomVM& vm) {
	vm.Reg ().IntVal () = InternalOpenAppDataWrite (vm.GetStringParam(3), vm.GetStringParam(2), vm.GetIntParam(1) != 0);
}

void WrapCloseFile (TomVM& vm) {
    int handle = vm.GetIntParam (1);
    if (handle > 0 && fileStreams.IndexStored (handle)) {
        fileStreams.Free (handle);
        lastError = "";
    }
    else
        lastError = "Invalid file handle";
}
void WrapWriteChar (TomVM& vm) {
	if (!GetOStream (vm.GetIntParam (2)))
		return;
    
    // Write a single character
    char c = 0;
    c = vm.GetStringParam (1).c_str () [0];
    stream->out->write(&c, sizeof (c));
    UpdateError ("Write");
}
void WrapWriteString (TomVM& vm) {
    if (!GetOStream (vm.GetIntParam (2)))
        return;
                              
    // Write string. (Excludes 0 terminator)
    std::string str = vm.GetStringParam (1);
    if (str != "") {
        stream->out->write (str.c_str (), str.length ());
        UpdateError ("Write");
    }
}
void WrapWriteLine (TomVM& vm) {
    if (!GetOStream (vm.GetIntParam (2)))
        return;

    // Write string. (Excludes 0 terminator)
    std::string str = vm.GetStringParam (1) + "\r\n";
    if (str != "") {
        stream->out->write (str.c_str (), str.length ());
        UpdateError ("Write");
    }
}
void WrapWriteByte (TomVM& vm) {
    if (!GetOStream (vm.GetIntParam (2)))
        return;

    byte element = vm.GetIntParam (1);
    stream->out->write ((char *) &element, sizeof (element));
    UpdateError ("Write");
}
void WrapWriteWord (TomVM& vm) {
    if (!GetOStream (vm.GetIntParam (2)))
        return;

    WORD element = vm.GetIntParam (1);
    stream->out->write ((char *) &element, sizeof (element));
    UpdateError ("Write");
}
void WrapWriteInt (TomVM& vm) {
    if (!GetOStream (vm.GetIntParam (2)))
        return;

    int element = vm.GetIntParam (1);
    stream->out->write ((char *) &element, sizeof (element));
    UpdateError ("Write");
}
void WrapWriteFloat (TomVM& vm) {
    if (!GetOStream (vm.GetIntParam (2)))
        return;

    float element = vm.GetRealParam (1);
    stream->out->write ((char *) &element, sizeof (element));
    UpdateError ("Write");
}
void WrapWriteDouble (TomVM& vm) {
    if (!GetOStream (vm.GetIntParam (2)))
        return;

    double element = vm.GetRealParam (1);
    stream->out->write ((char *) &element, sizeof (element));
    UpdateError ("Write");
}
void WrapReadLine (TomVM& vm) {
    vm.RegString () = "";
    if (!GetIStream (vm.GetIntParam (1)))
        return;
    if (!UpdateError ("Read"))
        return;

    // Skip returns and linefeeds
    char c;
    stream->in->read (&c, sizeof (c));
    while (!stream->in->fail () && !stream->in->eof () && (c == 10 || c == 13))
        stream->in->read (&c, sizeof (c));

    // Read printable characters
    while (!stream->in->fail () && !stream->in->eof () && c != 10 && c != 13) {
        vm.RegString () += c;
        stream->in->read (&c, sizeof (c));
    }

    // Don't treat eof as an error
    if (!stream->in->eof ())
        UpdateError ("Read");
    else
        lastError = "";
}
void WrapReadChar (TomVM& vm) {
    vm.RegString () = "";
    if (!GetIStream (vm.GetIntParam (1)))
        return;

    // Read char
    char c;
    stream->in->read (&c, sizeof (c));
    if (UpdateError ("Read"))
        vm.RegString () = c;
}
void WrapReadByte (TomVM& vm) {
    vm.Reg ().IntVal () = 0;
    if (!GetIStream (vm.GetIntParam (1)))
        return;

    // Read byte
    byte element;
    stream->in->read ((char *) &element, sizeof (element));
    if (UpdateError ("Read"))
        vm.Reg ().IntVal () = element;
}
void WrapReadWord (TomVM& vm) {
    vm.Reg ().IntVal () = 0;
    if (!GetIStream (vm.GetIntParam (1)))
        return;

    // Read byte
    WORD element;
    stream->in->read ((char *) &element, sizeof (element));
    if (UpdateError ("Read"))
        vm.Reg ().IntVal () = element;
}
void WrapReadInt (TomVM& vm) {
    vm.Reg ().IntVal () = 0;
    if (!GetIStream (vm.GetIntParam (1)))
        return;

    // Read byte
    int element;
    stream->in->read ((char *) &element, sizeof (element));
    if (UpdateError ("Read"))
        vm.Reg ().IntVal () = element;
}
void WrapReadFloat (TomVM& vm) {
    vm.Reg ().IntVal () = 0;
    if (!GetIStream (vm.GetIntParam (1)))
        return;

    // Read byte
    float element;
    stream->in->read ((char *) &element, sizeof (element));
    if (UpdateError ("Read"))
        vm.Reg ().RealVal () = element;
}
void WrapReadDouble (TomVM& vm) {
    vm.Reg ().IntVal () = 0;
    if (!GetIStream (vm.GetIntParam (1)))
        return;

    // Read byte
    double element;
    stream->in->read ((char *) &element, sizeof (element));
    if (UpdateError ("Read"))
        vm.Reg ().RealVal () = element;
}
void WrapSeek (TomVM& vm) {
    if (!GetStream (vm.GetIntParam (2)))
        return;
	if (stream->in != NULL)	stream->in->seekg  (vm.GetIntParam (1));
	if (stream->out != NULL)	stream->out->seekp (vm.GetIntParam (1));
    UpdateError ("Seek");
}
void WrapReadText (TomVM& vm) {

    // Read a string of non whitespace tokens
    if (!GetIStream (vm.GetIntParam (2)))
        return;
    if (!UpdateError ("Read"))
        return;
    bool skipNewLines = vm.GetIntParam (1) != 0;

    // Skip leading whitespace
    char c = 0;
    vm.RegString () = "";
    while ( stream->in->read (&c, sizeof (c))
            && (c != '\n' || skipNewLines)
            && c <= ' ')
        if (!UpdateError ("Read"))
            return;

    // Read non whitespace
    while (c > ' ') {
        vm.RegString () = vm.RegString () + c;
        stream->in->read (&c, sizeof (c));
        if (!UpdateError ("Read"))
            return;
    }

    // Backup one character, so that we don't skip the following whitespace
    stream->in->seekg (-1, std::ios::cur);
}

void WrapFindFirstFile(TomVM& vm) {

    // Close any previous find
    CloseFind();
    
    // Get filename
    std::string filename = vm.GetStringParam(1);

    // Check path is in files folder
    if (IsSandboxMode() && !files->CheckFilesFolder(filename)) {
        lastError = files->GetError();
        vm.RegString() = "";
        return;
    }

    // Find file
    lastError = "";
    findFileHandle = FindFirstFile(filename.c_str(), &findFileData);

    // Return filename
    if (findFileHandle != INVALID_HANDLE_VALUE)
        vm.RegString() = findFileData.cFileName;
    else
        vm.RegString() = "";
}

void WrapFindNextFile(TomVM& vm) {

    // Return data
    if (findFileHandle != NULL && findFileHandle != INVALID_HANDLE_VALUE && FindNextFile(findFileHandle, &findFileData))
        vm.RegString() = findFileData.cFileName;
    else
        vm.RegString() = "";
}

void WrapFindClose(TomVM& vm) {
    CloseFind();
}

void WrapDeleteFile(TomVM& vm) {
    std::string filename = vm.GetStringParam(1);
    if (files->Delete(filename.c_str(), IsSandboxMode())) {
        lastError = "";
        vm.Reg().IntVal() = -1;
    }
    else {
        lastError = files->GetError();
        vm.Reg().IntVal() = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Initialisation

void InitTomFileIOBasicLib (TomBasicCompiler& comp, FileOpener *_files) {

	// Save file opener pointer
	assert (_files != NULL);
	files = _files;

    // Wire up plugin adapter
    comp.Plugins().RegisterInterface(
        static_cast<IB4GLFileAccessor*>(&pluginAdapter),
        "IB4GLFileAccessor",
        IB4GLFILEACCESSOR_MAJOR,
        IB4GLFILEACCESSOR_MINOR,
        NULL);

    // Register resources
    comp.VM().AddResources(fileStreams);

    // Register initialisation functions
    comp.VM().AddInitFunc (Init);

    // Register function wrappers
    comp.AddFunction ("OpenFileRead",       WrapOpenFileRead,       compParamTypeList () << VTP_STRING,                 true, true,  VTP_INT, true);
    comp.AddFunction ("OpenFileWrite",      WrapOpenFileWrite,      compParamTypeList () << VTP_STRING,                 true, true,  VTP_INT, true);
    comp.AddFunction ("OpenAppDataRead",    WrapOpenAppDataRead,    compParamTypeList () << VTP_STRING << VTP_STRING << VTP_INT, true, true,  VTP_INT, true);
    comp.AddFunction ("OpenAppDataWrite",   WrapOpenAppDataWrite,   compParamTypeList () << VTP_STRING << VTP_STRING << VTP_INT, true, true,  VTP_INT, true);
    comp.AddFunction ("CloseFile",          WrapCloseFile,          compParamTypeList () << VTP_INT,                    true, false, VTP_INT, true);
    comp.AddFunction ("FileError",          WrapFileError,          compParamTypeList (),                               true, true,  VTP_STRING, true);
    comp.AddFunction ("EndOfFile",          WrapEndOfFile,          compParamTypeList () << VTP_INT,                    true, true,  VTP_INT, true);
    comp.AddFunction ("WriteChar",          WrapWriteChar,          compParamTypeList () << VTP_INT << VTP_STRING,      true, false, VTP_INT, true);
    comp.AddFunction ("WriteString",        WrapWriteString,        compParamTypeList () << VTP_INT << VTP_STRING,      true, false, VTP_INT, true);
    comp.AddFunction ("WriteLine",          WrapWriteLine,          compParamTypeList () << VTP_INT << VTP_STRING,      true, false, VTP_INT, true);
    comp.AddFunction ("WriteByte",          WrapWriteByte,          compParamTypeList () << VTP_INT << VTP_INT,         true, false, VTP_INT, true);
    comp.AddFunction ("WriteWord",          WrapWriteWord,          compParamTypeList () << VTP_INT << VTP_INT,         true, false, VTP_INT, true);
    comp.AddFunction ("WriteInt",           WrapWriteInt,           compParamTypeList () << VTP_INT << VTP_INT,         true, false, VTP_INT, true);
    comp.AddFunction ("WriteFloat",         WrapWriteFloat,         compParamTypeList () << VTP_INT << VTP_REAL,        true, false, VTP_INT, true);
    comp.AddFunction ("WriteReal",          WrapWriteFloat,         compParamTypeList () << VTP_INT << VTP_REAL,        true, false, VTP_INT, true);  // (WriteReal is a synonym for WriteFloat)
    comp.AddFunction ("WriteDouble",        WrapWriteDouble,        compParamTypeList () << VTP_INT << VTP_REAL,        true, false, VTP_INT, true);
    comp.AddFunction ("ReadLine",           WrapReadLine,           compParamTypeList () << VTP_INT,                    true, true,  VTP_STRING, true);
    comp.AddFunction ("ReadChar",           WrapReadChar,           compParamTypeList () << VTP_INT,                    true, true,  VTP_STRING, true);
    comp.AddFunction ("ReadByte",           WrapReadByte,           compParamTypeList () << VTP_INT,                    true, true,  VTP_INT, true);
    comp.AddFunction ("ReadWord",           WrapReadWord,           compParamTypeList () << VTP_INT,                    true, true,  VTP_INT, true);
    comp.AddFunction ("ReadInt",            WrapReadInt,            compParamTypeList () << VTP_INT,                    true, true,  VTP_INT, true);
    comp.AddFunction ("ReadFloat",          WrapReadFloat,          compParamTypeList () << VTP_INT,                    true, true,  VTP_REAL, true);
    comp.AddFunction ("ReadReal",           WrapReadFloat,          compParamTypeList () << VTP_INT,                    true, true,  VTP_REAL, true);
    comp.AddFunction ("ReadDouble",         WrapReadDouble,         compParamTypeList () << VTP_INT,                    true, true,  VTP_REAL, true);
    comp.AddFunction ("Seek",               WrapSeek,               compParamTypeList () << VTP_INT << VTP_INT,         true, false, VTP_REAL, true);
    comp.AddFunction ("ReadText",           WrapReadText,           compParamTypeList () << VTP_INT << VTP_INT,         true, true,  VTP_STRING, true);
    comp.AddFunction ("FindFirstFile",      WrapFindFirstFile,      compParamTypeList () << VTP_STRING,                 true, true,  VTP_STRING, true);
    comp.AddFunction ("FindNextFile",       WrapFindNextFile,       compParamTypeList (),                               true, true,  VTP_STRING, true);
    comp.AddFunction ("FindClose",          WrapFindClose,          compParamTypeList (),                               true, false, VTP_INT, true);
    comp.AddFunction ("DeleteFile",         WrapDeleteFile,         compParamTypeList () << VTP_STRING,                 true, true,  VTP_INT, true);
}

////////////////////////////////////////////////////////////////////////////////
//  FileAccessorAdapter
void FileAccessorAdapter::GetFilenameForRead(const char* filename, char* dst) {
    assert(files != NULL);
    
    // Call corresponding opener method. Convert result to char*
    strcpy(
        dst,
        files->FilenameForRead(filename, false).c_str());
}

