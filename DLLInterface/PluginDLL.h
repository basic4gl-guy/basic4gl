//---------------------------------------------------------------------------
/*  Created 15-Feb-06: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

    Interfaces the Basic4GL compiler to plugin DLLs
*/

#ifndef PluginDLLH
#define PluginDLLH

#include "PluginLibrary.h"
#include "vmTypes.h"
#include <string>

////////////////////////////////////////////////////////////////////////////////
//  PluginDLLFile
//
/// File details of a plugin DLL.
struct PluginDLLFile {
    std::string filename;
    std::string description;
    bool loaded;
    PluginVersion version;
};

typedef std::vector<PluginDLLFile> DLLFileVector;

class PluginDLLManager;

////////////////////////////////////////////////////////////////////////////////
//	PluginDLL
//
/// A plugin library implemented in a DLL.
class PluginDLL : public PluginLibrary {

	/// Filename (excluding path)
    std::string filename;

    /// DLL handle.
    /// Note: This is really a HINSTANCE, but I'm damned if I'll include the entire
    /// <windows.h> in this header just for that defn.
    int handle;

    /// DLL file details
    PluginDLLFile fileDetails;

public:
    PluginDLL(PluginDLLManager &_manager, std::string path, std::string _filename, bool isStandaloneExe);
    virtual ~PluginDLL();

	std::string Filename() { return filename; }
    const PluginDLLFile &FileDetails() const    { return fileDetails; }

	/// Plugin description for error reporting etc
	virtual std::string Description();

    /// Error text if failed
    virtual std::string Error();

	/// Return true if this is the PluginDLL class
	virtual bool IsDLL();
};

////////////////////////////////////////////////////////////////////////////////
//  PluginDLLManager
//
/// Manages loading and maintaining plugin DLLs
class PluginDLLManager : public PluginManager {
    std::string directory;

	/// Find itor for DLL with filename
	PluginLibraryVector::iterator FindItor(std::string filename);
public:
    PluginDLLManager(std::string _directory, bool _isStandaloneExe);

    /// Iterate loaded DLLs
    const std::vector<PluginDLL*> LoadedDLLs();

    /// Find and list DLL files.
    DLLFileVector DLLFiles();

    /// Find plugin DLL by name
    PluginDLL *Find(std::string filename);

    /// Return true if a DLL file is loaded
    bool IsLoaded(std::string filename) { return Find(filename) != NULL; }

    /// Load DLL. Returns true if DLL loaded successfully, or false if an error
    /// occurred (use Error() to retrive text).
    bool LoadDLL(std::string filename);

    /// Unload DLL. Returns true if DLL unloaded successfully.
    bool UnloadDLL(std::string filename);

#ifdef VM_STATE_STREAMING
	void StreamOut(std::ostream& stream) const override;
	bool StreamIn(std::istream& stream) override;
#endif
};

#endif
