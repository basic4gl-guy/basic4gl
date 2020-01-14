//---------------------------------------------------------------------------
/*  Created 12-May-07: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

    Support for plugin DLLs to define and access their own structure types.
*/

#ifndef PluginDataStructuresH
#define PluginDataStructuresH

#include "vmTypes.h"
#include "vmData.h"
#include "Basic4GLDLLInterface.h"
#include <string>
#include <vector>
#include <map>

class PluginStructureManager;

////////////////////////////////////////////////////////////////////////////////
//  PluginDataType
//
/// Represents a data type in a plugin structure.
/// Note: We store the C data type, which includes types not directly supported
///     by Basic4GL, such as bytes, words, double (64 bit floating point
///     numbers).
///     The Basic4GL representation is derived from the C type.
///     By knowing both the C and Basic4GL storage formats, we can convert
///     between Basic4GL structures and C structures upon request.
struct PluginDataType {

    // Basic data type.
    // Can be a DLL_Basic4GL_Ext_TypeCode constant, or a structure handle.
    int baseType;

    // Level of pointer indirection.
    // 0 = Value (not a pointer at all)
    // 1 = Pointer
    // 2 = Pointer to pointer
    // etc
    int pointerLevel;

    // True if pointer is by reference.
    // (Applies when pointerLevel > 0 only).
    bool byReference;

    // # of array dimensions
    // 0 = Not an array
    // 1 = 1D array
    // 2 = 2D array etc
    int arrayLevel;

    // Array dimension sizes
    unsigned int arrayDims[VM_MAXDIMENSIONS];

    // String sizes (in characters, including 0 terminator. Applies when base
    // type = DLL_BASIC4GL_EXT_STRING only).
    // Also doubles as padding size in byte (type = DLL_BASIC4GL_EXT_PADDING)
    unsigned int stringSize;

    PluginDataType() :
        baseType(0),
        pointerLevel(0),
        byReference(0),
        arrayLevel(0),
        stringSize(0) { ; }

    /// Convert array type into array element type
    void MakeIntoElementType() {
        assert(arrayLevel > 0);
        arrayLevel--;
    }

    void Deref() {
        assert(pointerLevel > 0);
        pointerLevel--;
        byReference = false;
    }

    // Constructors
    static PluginDataType Padding(int bytes) {
        PluginDataType result;
        result.baseType = DLL_BASIC4GL_EXT_PADDING;
        result.stringSize = bytes;
        return result;
    }

    static PluginDataType SimpleType(int baseType) {
        PluginDataType result;
        result.baseType = baseType;
        return result;
    }

    static PluginDataType String(int size) {
        PluginDataType result;
        result.baseType = DLL_BASIC4GL_EXT_STRING;
        result.stringSize = size;
        return result;
    }
};

////////////////////////////////////////////////////////////////////////////////
//  PluginStructureField
struct PluginStructureField {
    std::string fieldname;
    PluginDataType dataType;

    // Constructor
    PluginStructureField(std::string fieldname, const PluginDataType& dataType) {
        this->fieldname = LowerCase(fieldname);
        this->dataType = dataType;
    } 

    // Copy constructor
    PluginStructureField(const PluginStructureField& f) {
        fieldname = f.fieldname;
        dataType = f.dataType;
    }
};

////////////////////////////////////////////////////////////////////////////////
//  PluginStructure
//
/// A structure defined by a plugin
class PluginStructure {
private:

    // Structure definition
    std::string name;
    int versionMajor, versionMinor;
    std::vector<PluginStructureField> fields;

    // Index of corresponding Basic4GL structure type in compiler/virtual machine.
    // Note: This is valid only while compiling, or while the program is running,
    // as Basic4GL structures are only created from PluginStructures at the
    // beginning of a compilation.
    int vmStructureIndex;

    // Owning object
    void *owner;

    // Handle passed to plugins
    int handle;

public:

    PluginStructure() {
        name = "";
        versionMajor = 0;
        versionMinor = 0;
        vmStructureIndex = 0;
        owner = NULL;
        handle = 0;
    }
    PluginStructure(void *owner, std::string name, int versionMajor, int versionMinor) {
        this->owner = owner;
        this->name = LowerCase(name);
        this->versionMajor = versionMajor;
        this->versionMinor = versionMinor;
        vmStructureIndex = 0;
        handle = 0;
    }

    // Structure building
    void AddField(const PluginStructureField& field) {
        fields.push_back(field);
    }

    // Member access
    void *Owner()      { return owner; }
    std::string Name() { return name; }
    int VersionMajor() { return versionMajor; }
    int VersionMinor() { return versionMinor; }

    // Field access
    int FieldCount() { return fields.size(); }
    PluginStructureField& GetField(int index) {
        assert(index >= 0);
        assert(index < FieldCount());
        return fields[index];
    }                                 

    int GetVMStructureIndex()           { return vmStructureIndex; }
    void SetVMStructureIndex(int value) { vmStructureIndex = value; }
    int GetHandle()                     { return handle; }
    void SetHandle(int value)           { handle = value; }

    void AddToVM(vmTypeLibrary& typeLib, PluginStructureManager& manager);
};

////////////////////////////////////////////////////////////////////////////////
//  PluginStructureManager
//
/// Stores and manages structures defined by plugins.
class PluginStructureManager {
private:

    // Structure handle store
    vmPointerResourceStore<PluginStructure> structureStore;

    // Structures in the order they were created
    std::list<PluginStructure *> structures;

    // Name->structure index lookup
    std::map<std::string,int> nameIndex;

public:
    PluginStructureManager();

    /// Create a new structure and return its handle.
    /// Note: Structure manager takes responsibility for deleting the structure.
    int Add(PluginStructure *structure);

    PluginStructure *GetStructure(int handle);      // Returns NULL if handle invalid
    void Clear();

    /// Find structure by name. Returns its handle, or 0 if not found. 
    int FindStructure(std::string name);

    /// Remove all structures owned by a particular object
    void RemoveOwnedStructures(void *owner);

    /// Return list of structures owned by owner
    std::string DescribeOwnedStructures(void *owner);

    /// Add structures to virtual machine
    void AddVMStructures(vmTypeLibrary& typeLib);

    /// Find the VM datatype that corresponds to plugin data type.
    vmValType vmTypeFromPluginType(PluginDataType& pluginType);

    /// Get virtual machine equivalent type from plugin type.
    /// (Will replace plugin structure indices with corresponding virtual machine
    /// indices).
    vmValType GetVMType(vmValType& src);
};

#endif
