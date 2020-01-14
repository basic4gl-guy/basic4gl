/*  Created 03-Nov-15: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

	Plug in libraries of Basic4GL functions.
	There is no reference to DLLs or Windows specific code at this level.
	(See PluginDLL.h/cpp for the Windows DLL implementation.)
*/

#pragma once

#include "compFunction.h"
#include "Basic4GLDLLInterface.h"
#include "compConstant.h"
#include "PluginDataStructures.h"

////////////////////////////////////////////////////////////////////////////////
// compExtFuncSpec
//
// Extended function specification including information about where the
// function is stored (whether it's built in, or stored in a DLL and which one.)
// Used by the DLL manager to pass info to the compiler.
struct compExtFuncSpec {

	// Pointer to main specification
	compFuncSpec    *m_spec;

	// Function details
	bool            m_builtin;            // True = builtin, false = plugin DLL function
	int             m_pluginIndex;        // Index of DLL (if applicable). Note m_spec.m_index holds index of function WITHIN the DLL.
};

////////////////////////////////////////////////////////////////////////////////
//  PluginVersion
//
struct PluginVersion {
	int major;
	int minor;

	PluginVersion() : major(0), minor(0) {}
	PluginVersion(int _major, int _minor) : major(_major), minor(_minor) {}
	PluginVersion(const PluginVersion& p) : major(p.major), minor(p.minor) {}

	bool Matches(const PluginVersion& p) const {
		return p.minor == minor && p.major == major;
	}
	std::string VersionString() const {
		return IntToString(major) + "." + IntToString(minor);
	}
};

class PluginManager;

////////////////////////////////////////////////////////////////////////////////
//  PluginLibrary
//
/// A plugin library of functions, structure types and constants, as the 
/// Basic4GL compiler and virtual machine sees it.
/// Can be implemented as a DLL (see PluginDLL), or a local library version
class PluginLibrary : public IDLL_Basic4GL_FunctionRegistry {

	/// Main DLL manager
	PluginManager &manager;

	// IDLL_Basic4GL_FunctionRegistry data
	bool definingFunc;                          // True if currently defining a function
	DLL_Basic4GL_Function   currentFunc;        // Pointer to current function
	compFuncSpec            currentSpec;        // Current DLL runtime function being defined
	std::string             currentName;        // Name of current function

	// DLL constants
	compConstantMap         constants;

	// DLL functions
	std::vector<DLL_Basic4GL_Function>  functions;
	std::multimap<std::string, unsigned int> functionLookup;     // Maps function name to index

	// Function specifications
	// Note: We maintain 2 versions of each.
	// The first version uses the plugin structure index when refering to structures
	// (for return types or parameter types). The second uses the virtual machine
	// structure index.
	std::vector<compFuncSpec>           pluginFunctionSpecs;
	std::vector<compFuncSpec>           vmFunctionSpecs;

	// Resource stores
	std::vector<IDLL_Basic4GL_ObjectStore*> objectStores;

	// Structure building
	PluginStructure *currentStructure;

	// DLL->DLL references
	std::set<PluginLibrary*> referencingDLLs;                   // DLLs that reference this one

protected:

	/// Main plugin interface
	IDLL_Basic4GL_Plugin *plugin;

	/// True if loading failed. Calling code should not try to use library.
	bool failed;

	PluginLibrary(PluginManager &_manager);

	// Routines
	void CompleteFunction();
	void NewFunction(std::string name, DLL_Basic4GL_Function function);
	void FixType(vmValType& type);
	void Unload();

public:
	PluginLibrary(PluginManager &_manager, IDLL_Basic4GL_Plugin *_plugin, bool isStandaloneExe);
	virtual ~PluginLibrary();

	// IDLL_Basic4GL_FunctionRegistry methods
	virtual void DLLFUNC RegisterStringConstant(
		const char *name,
		const char *value);
	virtual void DLLFUNC RegisterIntConstant(
		const char *name,
		int value);
	virtual void DLLFUNC RegisterFloatConstant(
		const char *name,
		float value);
	virtual void DLLFUNC RegisterVoidFunction(
		const char *name,
		DLL_Basic4GL_Function function);
	virtual void DLLFUNC RegisterFunction(
		const char *name,
		DLL_Basic4GL_Function function,
		DLL_Basic4GL_TypeCode typeCode);
	virtual void DLLFUNC RegisterArrayFunction(
		const char *name,
		DLL_Basic4GL_Function function,
		DLL_Basic4GL_TypeCode typeCode,
		int dimensions);
	virtual void DLLFUNC RegisterStructureFunction(
		const char *name,
		DLL_Basic4GL_Function function,
		int structureTypeHandle);
	virtual void DLLFUNC ModReturnArray(int dimensions);
	virtual void DLLFUNC ModReturnPointer(int level);
	virtual void DLLFUNC ModNoBrackets();
	virtual void DLLFUNC ModTimeshare();
	virtual void DLLFUNC ModConditionalTimeshare();
	virtual void DLLFUNC AddParam(DLL_Basic4GL_TypeCode typeCode);
	virtual void DLLFUNC AddArrayParam(
		DLL_Basic4GL_TypeCode typeCode,
		int dimensions);
	virtual void DLLFUNC AddStrucParam(int handle);
	virtual void DLLFUNC ModParamArray(int dimensions);
	virtual void DLLFUNC ModParamPointer(int level);
	virtual void DLLFUNC ModParamReference();
	virtual void DLLFUNC RegisterInterface(
		void *object,
		const char *name,
		int major,
		int minor);
	virtual void *DLLFUNC FetchInterface(
		const char *name,
		int major,
		int minor);
	virtual IDLL_Basic4GL_ObjectStore *DLLFUNC CreateObjectStore(IDLL_Basic4GL_ObjectStoreCallback *callback);

	virtual int DLLFUNC RegisterStructure(const char *name, int versionMajor, int versionMinor);
	virtual void DLLFUNC AddStrucPadding(int numBytes);
	virtual void DLLFUNC AddStrucField(const char *name, int type);
	virtual void DLLFUNC AddStrucStringField(const char *name, int size);
	virtual void DLLFUNC ModStrucFieldArray(int dimensions, int dimension1Size, ...);
	virtual void DLLFUNC ModStrucFieldPointer(int level);
	virtual int DLLFUNC FetchStructure(
		const char *name,
		int versionMajor,
		int versionMinor);

	IDLL_Basic4GL_Plugin *Plugin() { return plugin; }
	const std::vector<compFuncSpec>& FunctionSpecs() const { return pluginFunctionSpecs; }

	/// Return true if plugin failed to load and/or initialise
	bool Failed() const { return failed; }

	/// Plugin description for error reporting etc
	virtual std::string Description();

	/// Search for a constant
	bool FindConstant(std::string name, compConstantMap::const_iterator& i);

	/// Return true if name matches a function in this DLL
	bool IsFunction(std::string& name) const {
		return functionLookup.find(LowerCase(name)) != functionLookup.end();
	}

	/// Find all functions of a given name within this DLL and add them to the array
	void FindFunctions(
		std::string& name,
		compExtFuncSpec *functions,
		int& count,
		int max,
		int pluginIndex);

	// Function retrieval by index
	int Count() { return functions.size(); }

	/// Retrieve function by index
	DLL_Basic4GL_Function GetFunction(int index) {
		assert(index >= 0);
		assert(index < Count());
		return functions[index];
	}

	/// Retrieve function name by index
	std::string FunctionName(int index);

	/// Iterate constants
	const compConstantMap Constants() { return constants; }

	/// Clear all resource stores allocated by DLL
	void ClearObjectStores();

	/// Create VM versions of function specifications
	void CreateVMFunctionSpecs();

	/// Error text if failed
	virtual std::string Error();

	/// Log that we are referenced by another DLL.
	/// (Thus we cannot be unloaded until the other DLL has been).
	void AddReferencingDLL(PluginLibrary *dll);

	/// Log that we are no longer referenced by another specific DLL.
	/// (Typically means that DLL has been unloaded)
	void RemoveReferencingDLL(PluginLibrary *dll);

	/// Return true if we are referenced by at least one other DLL
	bool IsReferenced() const { return !referencingDLLs.empty(); }

	/// List DLLs that reference this one
	std::string DescribeReferencingDLLs() const;

	/// Return true if this is the PluginDLL class
	virtual bool IsDLL();

	// Raw function specification access
	std::multimap<std::string, unsigned int> const& FunctionLookup() const { return functionLookup; }
	std::vector<compFuncSpec> const& VMFunctionSpecs() const { return vmFunctionSpecs; }
};

typedef std::vector<PluginLibrary *> PluginLibraryVector;

////////////////////////////////////////////////////////////////////////////////
//  PluginDLLSharedInterface
//
//  Tracks shared interfaces registered by plugin DLLs
struct PluginDLLSharedInterface {
	void        *intf;      // Interface
	PluginLibrary   *owner;     // DLL that owns the object
	PluginDLLSharedInterface(void *_intf, PluginLibrary *_owner)
		: intf(_intf), owner(_owner) {
		;
	}
	PluginDLLSharedInterface(const PluginDLLSharedInterface& s)
		: intf(s.intf), owner(s.owner) {
		;
	}
	PluginDLLSharedInterface()
		: intf(NULL), owner(NULL) {
		;
	}
};

////////////////////////////////////////////////////////////////////////////////
//	LocalPlugin
//
///	Base class for local plugin libraries. Supplies some default functionality.
/// Sub-classes must override the IDLL_Basic4GL_Plugin::Load() method as a minimum.
class LocalPlugin : public IDLL_Basic4GL_Plugin {
	std::string errorText;
protected:
	/// Report an error (e.g. failure to Start()/Load())
	void SetError(std::string error) { errorText = error; }
public:
	virtual ~LocalPlugin()
	{
	}

	virtual bool DLLFUNC Start();
	virtual void DLLFUNC Unload();
	virtual void DLLFUNC End();
	virtual void DLLFUNC Pause();
	virtual void DLLFUNC Resume();
	virtual void DLLFUNC DelayedResume();
	virtual void DLLFUNC GetError(char *error);
	virtual void DLLFUNC ProcessMessages();
};

///////////////////////////////////////////////////////////////////////////////
//	PluginManager
//
/// Manages plugin libraries
class PluginManager
{
protected:
	PluginLibraryVector   dlls;
	std::string error;
	std::map<std::string, PluginDLLSharedInterface> sharedInterfaces;
	bool isStandaloneExe;

	// Data structures defined by plugins
	PluginStructureManager structureManager;

	std::string SharedInterfaceKey(
		std::string name,
		int major,
		int minor);

public:
	explicit PluginManager(bool _isStandaloneExe);
	virtual ~PluginManager();

	// Member access
	PluginStructureManager& StructureManager() { return structureManager; }

	// Library registration
	bool Plugin(IDLL_Basic4GL_Plugin* plugin);

	// DLL loading/unloading

	/// Iterate loaded libraries
	const PluginLibraryVector& LoadedLibraries() const { return dlls; }

	/// Return text of last error.
	std::string Error() const { return error; }

	/// Unload all files
	void Clear();

	// Find DLL by name

	/// Return true if name matches a DLL function
	bool IsPluginFunction(std::string& name) const;

	/// Find constant with a given name within all loaded DLLs
	bool FindConstant(
		std::string name,
		compConstantMap::const_iterator& itor);

	/// Find functions of a given name within all loaded DLLs and append to array
	void FindFunctions(
		std::string& name,
		compExtFuncSpec *functions,
		int& count,
		int max);

	std::string FunctionName(int dllIndex, int functionIndex);

	/// Create virtual machine version of function specifications.
	/// Called immediately before a compile, but AFTER the VM versions of the
	/// plugin structures have been created.
	void CreateVMFunctionSpecs();

	// DLL events
	bool ProgramStart();        // Returns true if all plugins started successfully. Otherwise program should not proceed.
	void ProgramEnd();
	void ProgramPause();
	void ProgramResume();
	void ProgramDelayedResume();
	void ProcessMessages();

	// Interface sharing
	void RegisterInterface(
		void *intf,
		std::string name,
		int major,
		int minor,
		PluginLibrary *owner);

	void *FetchInterface(
		std::string name,
		int major,
		int minor,
		PluginLibrary *requester);

	int FetchStructure(
		std::string name,
		int major,
		int minor,
		PluginLibrary *requester);

#ifdef VM_STATE_STREAMING
	virtual void StreamOut(std::ostream& stream) const;
	virtual bool StreamIn(std::istream& stream);
#endif
};
