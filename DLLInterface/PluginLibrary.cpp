#include "PluginLibrary.h"
#include <cstdarg>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
//  Helper functions
vmBasicValType ValTypeFromDLLTypeCode(DLL_Basic4GL_TypeCode typeCode) {
	switch (typeCode) {
	case DLL_BASIC4GL_INT:      return VTP_INT;
	case DLL_BASIC4GL_FLOAT:    return VTP_REAL;
	case DLL_BASIC4GL_STRING:   return VTP_STRING;
	case DLL_BASIC4GL_FUNCPTR:	return VTP_UNTYPED_FUNC_PTR;
	default:                    return VTP_UNDEFINED;
	}
}

////////////////////////////////////////////////////////////////////////////////
//  PluginDLLObjectStore
//
/// A resource store for plugin DLLs

class PluginDLLObjectStore : public IDLL_Basic4GL_ObjectStore {
private:
	vmStore<void*> store;
	IDLL_Basic4GL_ObjectStoreCallback *callback;

	void DeleteObject(int handle);
public:
	PluginDLLObjectStore(IDLL_Basic4GL_ObjectStoreCallback *_callback)
		: store(NULL),
		callback(_callback) {
		store.Clear();			// Required to allocate single blank object
	}

	// IDLL_Basic4GL_ObjectStore methods
	virtual int DLLFUNC Add(void *object);
	virtual void DLLFUNC Remove(int handle);
	virtual bool DLLFUNC HandleIsValid(int handle);
	virtual void *DLLFUNC Object(int handle);
	virtual void Clear();
};

void PluginDLLObjectStore::DeleteObject(int handle) {
	// Free object
	void *object = Object(handle);
	if (callback != NULL)
		callback->ObjectRemoved(this, object);
}

int DLLFUNC PluginDLLObjectStore::Add(void *object) {
	int handle = store.Alloc();
	store.Value(handle) = object;
	return handle;
}

void DLLFUNC PluginDLLObjectStore::Remove(int handle) {
	if (HandleIsValid(handle)) {

		// Free object
		DeleteObject(handle);

		// Deallocate handle
		store.Free(handle);
	}
}

bool DLLFUNC PluginDLLObjectStore::HandleIsValid(int handle) {
	return store.IndexStored(handle);
}

void *DLLFUNC PluginDLLObjectStore::Object(int handle) {
	return HandleIsValid(handle) ? store.Value(handle) : NULL;
}

void PluginDLLObjectStore::Clear() {
	// Delete each individual element
	// Note: Skip index 0, as this is the dummy no-object index.
	for (unsigned int i = 1; i < store.Array().size(); i++)
		if (store.ValAllocated()[i])
			DeleteObject(i);

	// Clear store
	store.Clear();
}

////////////////////////////////////////////////////////////////////////////////
//  PluginLibrary
PluginLibrary::PluginLibrary(PluginManager &_manager)
	: manager(_manager), plugin(NULL), currentStructure(NULL) {

	// Set defaults
	definingFunc = false;
	failed = true;			// Descendent class must set to true on successful create
}

PluginLibrary::PluginLibrary(PluginManager &_manager, IDLL_Basic4GL_Plugin *_plugin, bool isStandaloneExe)
	: manager(_manager), plugin(_plugin), currentStructure(NULL) {

	// Set defaults
	definingFunc = false;
	failed = true;

	// Inform plugin it has been loaded. Let it register its functions.
	if (!plugin->Load(*this, isStandaloneExe))
		return;
	CompleteFunction();

	failed = false;
}

PluginLibrary::~PluginLibrary() {
	Unload();
}

void PluginLibrary::Unload() {
	if (plugin != NULL) {
		// Clear object stores
		ClearObjectStores();

		// Clear structures
		manager.StructureManager().RemoveOwnedStructures(this);

		// Unload plugin
		if (plugin != NULL)
			plugin->Unload();

		// Delete object stores
		for (unsigned int i = 0; i < objectStores.size(); i++)
			delete objectStores[i];

		plugin = NULL;
	}
}

void PluginLibrary::CompleteFunction() {
	if (!definingFunc)
		return;

	// Fix up return and parameter types
	if (currentSpec.m_isFunction)
		FixType(currentSpec.m_returnType);
	for (vmValTypeList::iterator i = currentSpec.m_paramTypes.Params().begin();
		i != currentSpec.m_paramTypes.Params().end();
		i++)
		FixType(*i);

	// Finish off the current function

	// Store pointer and specification
	currentSpec.m_index = functions.size();
	functions.push_back(currentFunc);
	pluginFunctionSpecs.push_back(currentSpec);

	// Add name->index mapping
	functionLookup.insert(std::make_pair(currentName, currentSpec.m_index));

	definingFunc = false;
}

void PluginLibrary::NewFunction(std::string name, DLL_Basic4GL_Function function) {

	// Complete any partially defined function
	CompleteFunction();

	// Save function info
	currentName = LowerCase(name);
	currentFunc = function;

	// Set function defaults
	currentSpec.m_paramTypes.Params().clear();
	currentSpec.m_brackets = true;
	currentSpec.m_isFunction = false;
	currentSpec.m_returnType = VTP_INT;
	currentSpec.m_timeshare = false;
	currentSpec.m_conditionalTimeshare = false;
	currentSpec.m_index = 0;
	currentSpec.m_freeTempData = false;

	definingFunc = true;
}

void DLLFUNC PluginLibrary::RegisterVoidFunction(
	const char *name,
	DLL_Basic4GL_Function function) {

	// Create new void function
	NewFunction(name, function);
}

void DLLFUNC PluginLibrary::RegisterFunction(
	const char *name,
	DLL_Basic4GL_Function function,
	DLL_Basic4GL_TypeCode typeCode) {

	// Create new function
	NewFunction(name, function);

	// Set return type
	currentSpec.m_isFunction = true;
	currentSpec.m_returnType = ValTypeFromDLLTypeCode(typeCode);
}

void DLLFUNC PluginLibrary::RegisterArrayFunction(
	const char *name,
	DLL_Basic4GL_Function function,
	DLL_Basic4GL_TypeCode typeCode,
	int dimensions) {

	// Create new function
	NewFunction(name, function);

	// Set return type
	currentSpec.m_isFunction = true;
	currentSpec.m_returnType = vmValType(ValTypeFromDLLTypeCode(typeCode), dimensions, 1, true);
	currentSpec.m_freeTempData = true;
}

void DLLFUNC PluginLibrary::ModNoBrackets() {
	// Note: Basic4GL parsing doesn't handle functions with no parameters properly.
	// Therefore we only set no-brackets if the function is void.
	if (!currentSpec.m_isFunction)
		currentSpec.m_brackets = false;
}

void DLLFUNC PluginLibrary::ModTimeshare() {
	currentSpec.m_timeshare = true;
}

void PluginLibrary::ModConditionalTimeshare()
{
	currentSpec.m_conditionalTimeshare = true;
}

void DLLFUNC PluginLibrary::AddParam(DLL_Basic4GL_TypeCode typeCode) {
	currentSpec.m_paramTypes << ValTypeFromDLLTypeCode(typeCode);
}

void DLLFUNC PluginLibrary::AddArrayParam(
	DLL_Basic4GL_TypeCode typeCode,
	int dimensions) {
	currentSpec.m_paramTypes << vmValType(ValTypeFromDLLTypeCode(typeCode), dimensions, 1, true);
}

void DLLFUNC PluginLibrary::AddStrucParam(int handle) {
	currentSpec.m_paramTypes << vmValType((vmBasicValType)handle);
}

void PluginLibrary::FindFunctions(
	std::string& name,
	compExtFuncSpec *functions,
	int& count,
	int max,
	int pluginIndex) {

	// Find matching functions
	for (
		std::multimap<std::string, unsigned int>::const_iterator i = functionLookup.find(name);
		i != functionLookup.end() && i->first == name && count < max;
	i++) {

		// Append entry to array
		if (vmFunctionSpecs.size() > i->second) {
			compFuncSpec *spec = &vmFunctionSpecs[i->second];
			functions[count].m_spec = spec;
			functions[count].m_builtin = false;
			functions[count].m_pluginIndex = pluginIndex;
			count++;
		}
	}
}

void DLLFUNC PluginLibrary::RegisterInterface(
	void *object,
	const char *name,
	int major,
	int minor) {
	manager.RegisterInterface(object, name, major, minor, this);
}

void *DLLFUNC PluginLibrary::FetchInterface(
	const char *name,
	int major,
	int minor) {
	return manager.FetchInterface(name, major, minor, this);
}

std::string PluginLibrary::FunctionName(int index) {
	for (
		std::multimap<std::string, unsigned int>::iterator i = functionLookup.begin();
		i != functionLookup.end();
	i++)
		if ((*i).second == index)
			return (*i).first;

	return "???";
}

void DLLFUNC PluginLibrary::RegisterStringConstant(
	const char *name,
	const char *value) {
	constants[LowerCase((string)name)] = compConstant((string) "S" + value);
}
void DLLFUNC PluginLibrary::RegisterIntConstant(
	const char *name,
	int value) {
	constants[LowerCase((string)name)] = compConstant(value);
}
void DLLFUNC PluginLibrary::RegisterFloatConstant(
	const char *name,
	float value) {
	constants[LowerCase((string)name)] = compConstant(value);
}

IDLL_Basic4GL_ObjectStore *DLLFUNC PluginLibrary::CreateObjectStore(IDLL_Basic4GL_ObjectStoreCallback *callback) {

	// Create object store
	IDLL_Basic4GL_ObjectStore *store = new PluginDLLObjectStore(callback);

	// Register it
	objectStores.push_back(store);

	return store;
}

bool PluginLibrary::FindConstant(std::string name, compConstantMap::const_iterator& i) {
	i = constants.find(name);
	return i != constants.end();
}

void PluginLibrary::ClearObjectStores() {
	for (unsigned int i = 0; i < objectStores.size(); i++)
		objectStores[i]->Clear();
}

int DLLFUNC PluginLibrary::RegisterStructure(
	const char *name, int versionMajor, int versionMinor) {

	// Structure with same name must not already be registered
	if (manager.StructureManager().FindStructure(name) != 0)
		return 0;

	// Create new structure
	currentStructure = new PluginStructure(this, name, versionMajor, versionMinor);

	// Register and retrieve handle
	return manager.StructureManager().Add(currentStructure);
}

void DLLFUNC PluginLibrary::AddStrucPadding(int numBytes) {
	if (currentStructure != NULL)
		currentStructure->AddField(
		PluginStructureField("", PluginDataType::Padding(numBytes)));
}

void DLLFUNC PluginLibrary::AddStrucField(const char *name, int type) {
	if (currentStructure != NULL)
		currentStructure->AddField(
		PluginStructureField(name, PluginDataType::SimpleType(type)));
}

void DLLFUNC PluginLibrary::AddStrucStringField(const char *name, int size) {
	if (currentStructure != NULL)
		currentStructure->AddField(
		PluginStructureField(name, PluginDataType::String(size)));
}

void DLLFUNC PluginLibrary::ModStrucFieldArray(int dimensions, int dimension1Size, ...) {

	assert(dimensions < VM_MAXDIMENSIONS);
	int dimensionArray[VM_MAXDIMENSIONS];
	if (dimensions > 0)
	{
		dimensionArray[0] = dimension1Size;
		va_list vl;
		va_start(vl, dimension1Size);
		for (int i = 1; i < dimensions; i++)
			dimensionArray[i] = va_arg(vl, int);
		va_end(vl);
	}

	// Find last defined field
	if (currentStructure != NULL && currentStructure->FieldCount() > 0) {
		PluginStructureField& field = currentStructure->GetField(currentStructure->FieldCount() - 1);

		// Convert data type to array
		PluginDataType& dataType = field.dataType;
		dataType.arrayLevel = dimensions;

		// Set array dimension sizes
		int *dimensionSize = dimensionArray;
		for (int i = dimensions - 1; i >= 0; i--) {
			dataType.arrayDims[i] = *dimensionSize;
			dimensionSize++;
		}
	}
}

void DLLFUNC PluginLibrary::ModStrucFieldPointer(int level) {
	// Find last defined field
	if (currentStructure != NULL && currentStructure->FieldCount() > 0) {
		PluginStructureField& field = currentStructure->GetField(currentStructure->FieldCount() - 1);

		// Convert data type to pointer
		field.dataType.pointerLevel = level;
	}
}

int DLLFUNC PluginLibrary::FetchStructure(
	const char *name,
	int versionMajor,
	int versionMinor) {
	return manager.FetchStructure(name, versionMajor, versionMinor, this);
}

void PluginLibrary::CreateVMFunctionSpecs() {

	// Create virtual machine version of each function definition.
	// These are basically identical to the plugin version, except that
	// plugin structure indices in each data type are replaced with the
	// corresponding virtual machine structure index.
	vmFunctionSpecs.clear();
	for (vector<compFuncSpec>::iterator i = pluginFunctionSpecs.begin();
		i != pluginFunctionSpecs.end();
		i++) {

		// Create function spec (convert return type)
		compFuncSpec& src = *i;
		compFuncSpec dst(src);
		if (dst.m_isFunction)
			dst.m_returnType = manager.StructureManager().GetVMType(dst.m_returnType);

		// Convert parameter types
		dst.m_paramTypes.Params().clear();
		for (vmValTypeList::iterator j = src.m_paramTypes.Params().begin();
			j != src.m_paramTypes.Params().end();
			j++)
			dst.m_paramTypes << manager.StructureManager().GetVMType(*j);
		vmFunctionSpecs.push_back(dst);
	}
}

void DLLFUNC PluginLibrary::RegisterStructureFunction(
	const char *name,
	DLL_Basic4GL_Function function,
	int structureTypeHandle) {

	// Create new function
	NewFunction(name, function);

	// Set return type
	currentSpec.m_isFunction = true;
	currentSpec.m_freeTempData = true;
	currentSpec.m_returnType = vmValType((vmBasicValType)structureTypeHandle);
}

void DLLFUNC PluginLibrary::ModReturnArray(int dimensions) {
	currentSpec.m_returnType.m_arrayLevel = dimensions;
	currentSpec.m_freeTempData = true;
}

void DLLFUNC PluginLibrary::ModReturnPointer(int level) {
	currentSpec.m_returnType.m_pointerLevel = level;
}

void DLLFUNC PluginLibrary::ModParamArray(int dimensions) {
	// Find last added param
	if (currentSpec.m_paramTypes.Params().size() > 0) {
		vmValType& param = currentSpec.m_paramTypes.Params().back();

		// Convert into array
		param.m_arrayLevel = dimensions;
	}
}

void DLLFUNC PluginLibrary::ModParamPointer(int level) {
	// Find last added param
	if (currentSpec.m_paramTypes.Params().size() > 0) {
		vmValType& param = currentSpec.m_paramTypes.Params().back();

		// Convert into pointer
		param.m_pointerLevel = level;
	}
}

void DLLFUNC PluginLibrary::ModParamReference() {
	// Find last added param
	if (currentSpec.m_paramTypes.Params().size() > 0) {
		vmValType& param = currentSpec.m_paramTypes.Params().back();

		// Convert into by-reference param
		if (!param.m_byRef) {
			param.m_byRef = true;
			param.m_pointerLevel++;
		}
	}
}

void PluginLibrary::FixType(vmValType& type) {

	// Fix up a function return type or parameter type.
	// Convert arrays and structures to by-reference.
	if (type.m_pointerLevel == 0 &&
		(type.m_basicType >= 0 || type.m_arrayLevel > 0)) {
		type.m_pointerLevel = 1;
		type.m_byRef = true;
	}
}

std::string PluginLibrary::Error() {

	// Query plugin for error text
	// We must allocate and pass our own buffer
	char buffer[1024];
	memset(buffer, 0, 1024);
	plugin->GetError(buffer);
	return (std::string) buffer;
}

bool PluginLibrary::IsDLL() {
	return false;
}

std::string PluginLibrary::Description() {
	return "Builtin library";
}

void PluginLibrary::AddReferencingDLL(PluginLibrary *dll) {

	// (Ignore self-references)
	if (dll != this)
		referencingDLLs.insert(dll);
}

void PluginLibrary::RemoveReferencingDLL(PluginLibrary *dll) {
	referencingDLLs.erase(dll);
}

std::string PluginLibrary::DescribeReferencingDLLs() const {

	// Build a comma separated list of plugin DLLs which reference this one
	std::string result = "";
	for (
		std::set<PluginLibrary*>::const_iterator i = referencingDLLs.begin();
		i != referencingDLLs.end();
	i++) {
		if (i != referencingDLLs.begin())
			result += ", ";
		result += (*i)->Description();
	}
	return result;
}

////////////////////////////////////////////////////////////////////////////////
//	LocalPlugin
void DLLFUNC LocalPlugin::Unload() {}
bool DLLFUNC LocalPlugin::Start() { return true; }
void DLLFUNC LocalPlugin::End() {}
void DLLFUNC LocalPlugin::Pause() {}
void DLLFUNC LocalPlugin::Resume() {}
void DLLFUNC LocalPlugin::DelayedResume() {}
void DLLFUNC LocalPlugin::ProcessMessages() {}

void DLLFUNC LocalPlugin::GetError(char *error) {

	// Get error text. Truncate to 1023 characters if necessary.
	// (Caller passes a 1024 character char array to receive the error).
	string text = errorText;
	if (text.length() > 1023)
		text = text.substr(0, 1023);

	// Copy to error array
	strcpy(error, text.c_str());
	error[text.length()] = 0;
}

////////////////////////////////////////////////////////////////////////////////
//  PluginManager

bool PluginManager::Plugin(IDLL_Basic4GL_Plugin* plugin) {

	// Wrap in library object.
	// This will also initialise the plugin and report back any error
	PluginLibrary* library = new PluginLibrary(*this, plugin, isStandaloneExe);
	if (library->Failed()) {
		error = library->Error();
		delete library;
		return false;
	}

	// Add to list
	dlls.push_back(library);
	return true;
}

void PluginManager::Clear() {

	// Unload all DLLs
	for (unsigned int i = 0; i < dlls.size(); i++)
		delete dlls[i];

	// Clear list
	dlls.clear();
}

bool PluginManager::IsPluginFunction(std::string& name) const {

	// Look for matching function name
	for (
		PluginLibraryVector::const_iterator i = dlls.begin();
		i != dlls.end();
	i++)
		if ((*i)->IsFunction(name))
			return true;

	// (None found)
	return false;
}

void PluginManager::FindFunctions(
	std::string& name,
	compExtFuncSpec *functions,
	int& count,
	int max) {

	// Pass call through to all loaded plugin dlls.
	for (unsigned int i = 0; i < dlls.size(); i++)
		dlls[i]->FindFunctions(name, functions, count, max, i);
}

bool PluginManager::ProgramStart() {
	for (unsigned int i = 0; i < dlls.size(); i++) {
		// Attempt to start plugin
		if (!dlls[i]->Plugin()->Start()) {
			// Plugin failed!

			// Get error
			error = (string)"Error initialising plugin " + dlls[i]->Description() + ": " + dlls[i]->Error();

			// Stop all plugins that have just been started
			for (int j = i - 1; j >= 0; j--)
				dlls[j]->Plugin()->End();

			// Abort program
			return false;
		}
	}

	// All plugins started successfully
	error = "";
	return true;
}

void PluginManager::ProgramEnd() {
	for (unsigned int i = 0; i < dlls.size(); i++) {
		dlls[i]->ClearObjectStores();           // Also clear object stores.
		dlls[i]->Plugin()->End();
	}
}

void PluginManager::ProgramPause() {
	for (unsigned int i = 0; i < dlls.size(); i++)
		dlls[i]->Plugin()->Pause();
}

void PluginManager::ProgramResume() {
	for (unsigned int i = 0; i < dlls.size(); i++)
		dlls[i]->Plugin()->Resume();
}

void PluginManager::ProgramDelayedResume() {
	for (unsigned int i = 0; i < dlls.size(); i++)
		dlls[i]->Plugin()->DelayedResume();
}

void PluginManager::ProcessMessages() {
	for (unsigned int i = 0; i < dlls.size(); i++)
		dlls[i]->Plugin()->ProcessMessages();
}

void PluginManager::RegisterInterface(
	void *intf,
	std::string name,
	int major,
	int minor,
	PluginLibrary *owner) {

	// Construct unique string key
	std::string key = SharedInterfaceKey(name, major, minor);

	// Save interface to key
	sharedInterfaces[key] = PluginDLLSharedInterface(intf, owner);
}

void *PluginManager::FetchInterface(
	std::string name,
	int major,
	int minor,
	PluginLibrary *requester) {

	// Construct unique string key
	std::string key = SharedInterfaceKey(name, major, minor);

	// Fetch object
	std::map<std::string, PluginDLLSharedInterface>::const_iterator i = sharedInterfaces.find(key);
	if (i == sharedInterfaces.end())
		return NULL;

	// Add a dependency between the requester and the owner.
	if (requester != NULL && i->second.owner != NULL)
		i->second.owner->AddReferencingDLL(requester);

	// Return the interface
	return i->second.intf;
}

int PluginManager::FetchStructure(
	std::string name,
	int major,
	int minor,
	PluginLibrary *requester) {

	// Find handle
	int handle = StructureManager().FindStructure(name);
	if (handle != 0) {

		// Find structure and check version
		PluginStructure* structure = StructureManager().GetStructure(handle);
		if (structure->VersionMajor() == major &&
			structure->VersionMinor() == minor) {

			// Add dependency between structure owner and requester
			if (structure->Owner() != NULL && structure->Owner() != requester)
				((PluginLibrary *)structure->Owner())->AddReferencingDLL(requester);

			return handle;
		}
	}

	// Not found or version mismatch
	return 0;
}

std::string PluginManager::SharedInterfaceKey(
	std::string name,
	int major,
	int minor) {
	return name + "_" + IntToString(major) + "_" + IntToString(minor);
}

PluginManager::PluginManager(bool _isStandaloneExe) : isStandaloneExe(_isStandaloneExe)
{
}

PluginManager::~PluginManager()
{
	Clear();
}

std::string PluginManager::FunctionName(int dllIndex, int functionIndex) {
	if (dllIndex >= 0 && dllIndex < dlls.size())
		return dlls[dllIndex]->Description() + " " + dlls[dllIndex]->FunctionName(functionIndex);
	else
		return "???";
}

bool PluginManager::FindConstant(
	std::string name,
	compConstantMap::const_iterator& itor) {

	// Search all DLLs for constant
	for (unsigned int i = 0; i < dlls.size(); i++)
		if (dlls[i]->FindConstant(name, itor))
			return true;

	return false;
}

void PluginManager::CreateVMFunctionSpecs() {
	for (PluginLibraryVector::iterator i = dlls.begin();
		i != dlls.end();
		i++)
		(*i)->CreateVMFunctionSpecs();
}

#ifdef VM_STATE_STREAMING
void PluginManager::StreamOut(std::ostream& stream) const
{
}

bool PluginManager::StreamIn(std::istream& stream)
{
	return true;
}
#endif
