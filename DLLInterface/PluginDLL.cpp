//---------------------------------------------------------------------------


#pragma hdrstop

#include "PluginDLL.h"
#ifdef VM_STATE_STREAMING
#include "streaming.h"
#endif
#include <windows.h>

#define BASIC4GL_DLL_MIN_VERSION 3			// Minimum plugin version Basic4GL is compatible with.
											// (DJLinux's extensive plugin library is built with version 3, so maintaining backwards compatibility is important.)

using namespace std;

//---------------------------------------------------------------------------

#pragma package(smart_init)

////////////////////////////////////////////////////////////////////////////////
//  Helper functions
bool LoadFileDetails(HINSTANCE dll, PluginDLLFile &details) {
    assert(dll != NULL);

    // Find query function
    DLL_Basic4GL_QueryFunction query = (DLL_Basic4GL_QueryFunction) GetProcAddress(dll, "Basic4GL_Query");
    if (query == NULL)
        return false;

    // Query the DLL for details
    char detailStr[256];
    memset(detailStr, 0, 256);
    details.version.major = 0;
    details.version.minor = 0;
    int version = query(detailStr, &details.version.major, &details.version.minor);
    details.description = detailStr;

    // Note: Calling code must calculate details.filename and details.loaded itself
	return version >= BASIC4GL_DLL_MIN_VERSION && version <= BASIC4GL_DLL_VERSION;
}

////////////////////////////////////////////////////////////////////////////////
//	PluginDLL

PluginDLL::PluginDLL(PluginDLLManager &_manager, std::string path, std::string _filename, bool isStandaloneExe)
    : PluginLibrary(_manager)
{
    // Save filename
    filename = LowerCase(_filename);

    // Load DLL
    handle = (int) LoadLibrary((path + filename).c_str());
    if (handle == NULL)
        return;

    // Query file details
    fileDetails.filename = filename;
    fileDetails.loaded = true;
    if (!LoadFileDetails((HINSTANCE) handle, fileDetails))
        return;

    // Find Init function
    DLL_Basic4GL_InitFunction init = (DLL_Basic4GL_InitFunction) GetProcAddress((HINSTANCE) handle, "Basic4GL_Init");
    if (init == NULL)
        return;

    // Call init function to get plugin interface
    plugin = init();
    if (plugin == NULL)
        return;

    // Inform plugin it has been loaded. Let it register its functions.
    if (!plugin->Load(*static_cast<IDLL_Basic4GL_FunctionRegistry*>(this), isStandaloneExe))
        return;
    CompleteFunction();

    // DLL successfully initialised
    failed = false;
}

PluginDLL::~PluginDLL() {

    // Unload plugin first
    Unload();

    // Unload DLL
    if (handle != NULL)
        FreeLibrary((HINSTANCE) handle);
}

std::string PluginDLL::Error() {

	if (handle == NULL)
        return "Could not load plugin DLL: " + filename;

    else if (plugin == NULL)
        return "Could not initialise plugin: " + filename;

    else 
		return PluginLibrary::Error();
}

bool PluginDLL::IsDLL() {
	return true;
}

std::string PluginDLL::Description() {
	return fileDetails.filename;
}

////////////////////////////////////////////////////////////////////////////////
//  PluginDLLManager

PluginDLLManager::PluginDLLManager(std::string _directory, bool _isStandaloneExe)
    : PluginManager(_isStandaloneExe), directory(_directory) {

    // Postfix a closing slash if necessary
    if (directory != "" && directory[directory.length() - 1] != '\\')
        directory += '\\';
}

DLLFileVector PluginDLLManager::DLLFiles() {
    DLLFileVector result;

    // Scan directory for files
    std::string searchPath = directory + "*.dll";
    WIN32_FIND_DATA seekData;
    HANDLE seekHandle = FindFirstFile(searchPath.c_str(), &seekData);
    if (seekHandle == INVALID_HANDLE_VALUE)
        return result;

    // Scan DLL files in directory and add to list
    bool foundFile = true;
    while (foundFile) {

        // Open the DLL and query it for details
        HINSTANCE dll = LoadLibrary((directory + seekData.cFileName).c_str());
        if (dll != NULL) {

            // Query details from DLL
            PluginDLLFile details;
            details.filename = seekData.cFileName;
            details.loaded = IsLoaded(details.filename);
            if (LoadFileDetails(dll, details)) {

                // Add DLL file entries
                result.push_back(details);
            }

            // Unload the DLL
            FreeLibrary(dll);
        }

        // Find next DLL in directory
        foundFile = FindNextFile(seekHandle, &seekData);
    }

    // File search finished
    FindClose(seekHandle);

    return result;
}

PluginLibraryVector::iterator PluginDLLManager::FindItor(std::string filename) {

    // Search loaded DLLs for given filename
    std::string lcase = LowerCase(filename);
	for (PluginLibraryVector::iterator i = dlls.begin();
			i != dlls.end();
			i++) {

		// Filter to DLLs
		if ((*i)->IsDLL()) {
			PluginDLL *dll = static_cast<PluginDLL*>(*i);
			if (dll->Filename() == lcase)
				return i;
		}
	}

	return dlls.end();
}

PluginDLL *PluginDLLManager::Find(std::string filename) {

    // Find itor
    PluginLibraryVector::iterator i = FindItor(filename);

    // Return DLL
    return i == dlls.end() ? NULL : static_cast<PluginDLL*>(*i);
}

bool PluginDLLManager::LoadDLL(std::string filename) {

    // Attempt to load DLL
    // First check that it's not already loaded
    if (IsLoaded(filename)) {
        error = "A plugin DLL by this name is already loaded";
        return false;
    }

    // Load DLL
    PluginDLL *dll = new PluginDLL(*this, directory, filename, isStandaloneExe);
    if (dll->Failed()) {
        error = dll->Error();
        delete dll;
        return false;
    }

    // Add to list
    dlls.push_back(dll);

    return true;
}

bool PluginDLLManager::UnloadDLL(std::string filename) {

    // Find DLL
    PluginLibraryVector::iterator i = FindItor(filename);
    if (i == dlls.end()) {
        error = "This plugin DLL is not loaded";
        return false;
    }

    // Check that DLL can be unloaded.
    // If the DLL owns objects that are currently used by other DLLs, then it
    // cannot be unloaded before the other DLLs have been.
	PluginDLL* dll = static_cast<PluginDLL*>(*i);
    if (dll->IsReferenced()) {
        error = (std::string) "The following plugin DLL(s) must be unloaded first:\r\n" + dll->DescribeReferencingDLLs();
        return false;
    }

    // Inform other DLLs that this one is being unloaded
	for (PluginLibraryVector::iterator j = dlls.begin(); j != dlls.end(); j++)
		if ((*i) != (*j) && (*j)->IsDLL())
			static_cast<PluginDLL*>(*j)->RemoveReferencingDLL(dll);

    // Unregister all interfaces owned by this DLL
    std::map<std::string,PluginDLLSharedInterface>::iterator k = sharedInterfaces.begin();
    while (k != sharedInterfaces.end()) {
        std::map<std::string,PluginDLLSharedInterface>::iterator current = k;
        k++;
        if (current->second.owner == *i)
            sharedInterfaces.erase(current);
    }

    // Unload it
    delete *i;

    // Remove it from our list
    dlls.erase(i);

    return true;
}

#ifdef VM_STATE_STREAMING

void PluginDLLManager::StreamOut(std::ostream& stream) const {
    // Write DLL filenames, and versions
    WriteLong(stream, dlls.size());
    for (int i = 0; i < dlls.size(); i++) {
		if (dlls[i]->IsDLL()) {
			PluginDLL* dll = static_cast<PluginDLL*>(dlls[i]);
			string filename = dll->FileDetails().filename;
			WriteString(stream, filename);
			WriteLong(stream, dll->FileDetails().version.major);
			WriteLong(stream, dll->FileDetails().version.minor);
		}
    }
}

bool PluginDLLManager::StreamIn(std::istream& stream) {

    // Clear out any existing plugins
    Clear();

    int count = ReadLong(stream);
    for (int i = 0; i < count; i++) {

        // Read file details
        string filename = ReadString(stream);
        PluginVersion version;
        version.major = ReadLong(stream);
        version.minor = ReadLong(stream);

        // Attempt to load DLL
        if (!LoadDLL(filename))
            return false;

        // Check version number
        PluginDLL *dll = Find(filename);
        if (!dll->FileDetails().version.Matches(version)) {
            error = "Plugin DLL " + filename + " is the wrong version.\r\n" +
                "Version is " + dll->FileDetails().version.VersionString() +
                ", expected " + version.VersionString();
            return false;
        }
    }
    return true;
}
#endif

const vector<PluginDLL*> PluginDLLManager::LoadedDLLs() {
    vector<PluginDLL*> result;
    for (   PluginLibraryVector::iterator i = dlls.begin();
            i != dlls.end();
            i++) {
        if ((*i)->IsDLL())
            result.push_back(static_cast<PluginDLL*>(*i));
    }

    return result;
}
