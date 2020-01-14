//---------------------------------------------------------------------------
/*  Created 12-May-07: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

    Support for plugin DLLs to define and access their own structure types.
*/

#pragma hdrstop

#include "PluginDataStructures.h"
using namespace std;

////////////////////////////////////////////////////////////////////////////////
//  PluginStructure
void PluginStructure::AddToVM(
        vmTypeLibrary& typeLib,
        PluginStructureManager& manager) {

    // Create corresponding structure in type library

    // Create new structure
    vmStructureIndex = typeLib.Structures().size();
    typeLib.NewStruc(name);

    // Add and convert fields
    for (   vector<PluginStructureField>::iterator i = fields.begin();
            i != fields.end();
            i++) {

        // Find structure data type
        PluginStructureField& field = *i;
        PluginDataType& type = field.dataType;

        // Ignore padding fields
        if (type.baseType != DLL_BASIC4GL_EXT_PADDING) {

            // Calculate vm data type
            vmValType vmType = manager.vmTypeFromPluginType(type);

            // Add field
            typeLib.NewField(field.fieldname, vmType);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//  PluginStructureManager

PluginStructureManager::PluginStructureManager() {
    Clear();
}

int PluginStructureManager::Add(PluginStructure *structure) {
    assert(structure != NULL);

    // Structure must not already be stored
    if (FindStructure(structure->Name()) != 0)
        return 0;

    // Add to ordered list
    structures.push_back(structure);

    // Store and allocate a handle
    structure->SetHandle(structureStore.Alloc(structure));

    // Map name to handle
    nameIndex[structure->Name()] = structure->GetHandle();

    return structure->GetHandle();
}

PluginStructure *PluginStructureManager::GetStructure(int handle) {
    return structureStore.Value(handle);
}

void PluginStructureManager::Clear() {
    structureStore.Clear();
    structures.clear();
    nameIndex.clear();

    // Add a blank element to the store. It will be assigned the handle 0.
    // Valid elements are not allocated handle 0.
    // (We are using 0 as a "no handle" value).
    structureStore.Alloc(NULL);
}

int PluginStructureManager::FindStructure(string name) {

    // Lookup name
    map<string,int>::iterator i = nameIndex.find(LowerCase(name));

    // Return its index
    return i != nameIndex.end() ? i->second : 0;
}

void PluginStructureManager::RemoveOwnedStructures(void *owner) {

    // Iterate over structures
    list<PluginStructure*>::iterator i = structures.begin();
    while (i != structures.end()) {

        // Find structures owned by given owner
        if ((*i)->Owner() == owner) {

            // Remove from name index, structure store and structure list
            nameIndex.erase((*i)->Name());
            structureStore.Remove((*i)->GetHandle());
            i = structures.erase(i);
        }
        else
            i++;
    }
}

void PluginStructureManager::AddVMStructures(vmTypeLibrary& typeLib) {

    // Convert structures to virtual machine
    // Iterate structures in the order that they were defined
    for (   list<PluginStructure *>::iterator i = structures.begin();
            i != structures.end();
            i++)
        (*i)->AddToVM(typeLib, *this);
}

vmValType PluginStructureManager::GetVMType(vmValType& src) {

    // Copy value type
    vmValType dst(src);

    // Check if is a structure type
    if (dst.m_basicType > 0) {

        // Find corresponding structure
        PluginStructure *structure = GetStructure(dst.m_basicType);
        assert(structure != NULL);

        // Replace with virtual machine structure index
        dst.m_basicType = (vmBasicValType) structure->GetVMStructureIndex();
    }

    return dst;
}

vmValType PluginStructureManager::vmTypeFromPluginType(PluginDataType& pluginType) {

    vmValType vmType;

    // Structure?
    if (pluginType.baseType > 0) {

        // Find structure
        PluginStructure* structure = structureStore.Value(pluginType.baseType);

        // Set virtual machine structure type
        vmType.m_basicType = (vmBasicValType) structure->GetVMStructureIndex();
    }
    else {

        // Basic type.
        // Find closest corresponding Basic4GL type
        switch(pluginType.baseType) {
            case DLL_BASIC4GL_EXT_BYTE:
            case DLL_BASIC4GL_EXT_WORD:
            case DLL_BASIC4GL_EXT_INT:
            case DLL_BASIC4GL_EXT_INT64:
                vmType.m_basicType = VTP_INT;
                break;

            case DLL_BASIC4GL_EXT_FLOAT:
            case DLL_BASIC4GL_EXT_DOUBLE:
                vmType.m_basicType = VTP_REAL;
                break;

            case DLL_BASIC4GL_EXT_STRING:
                vmType.m_basicType = VTP_STRING;
                break;
        }
    }

    // Other type properties
    vmType.m_pointerLevel = pluginType.pointerLevel;
    vmType.m_arrayLevel = pluginType.arrayLevel;

    // Copy array dimensions
    for (int i = 0; i < vmType.m_arrayLevel; i++)
        vmType.m_arrayDims[i] = pluginType.arrayDims[i];

    return vmType;
}

string PluginStructureManager::DescribeOwnedStructures(void *owner) {
    string result = "";
    for (   list<PluginStructure*>::iterator i = structures.begin();
            i != structures.end();
            i++) {
        if ((*i)->Owner() == owner) {
            if (result != "")
                result += "\r\n";
            PluginStructure* structure = *i;
            result += "struc " + structure->Name();
            for (int j = 0; j < structure->FieldCount(); j++) {
                PluginStructureField& field = structure->GetField(j);
                result += "\r\n  dim ";
                if (field.dataType.baseType >= 0) {
                    // Find referred to structure
                    PluginStructure* fieldStructure = structureStore.Value(field.dataType.baseType);
                    if (fieldStructure != NULL)
                        result += fieldStructure->Name() + " ";
                    else
                        result += "??? ";
                }
                for (int k = 0; k < field.dataType.pointerLevel; k++)
                    result += "&";
                result += field.fieldname;
                for (int k = 0; k < field.dataType.arrayLevel; k++)
                    result += "(" + IntToString(field.dataType.arrayDims[field.dataType.arrayLevel - k - 1] - 1) + ")";

                char lastChar = field.fieldname[field.fieldname.length() - 1];
                switch (field.dataType.baseType) {
                    case DLL_BASIC4GL_EXT_FLOAT:
                    case DLL_BASIC4GL_EXT_DOUBLE:
                        if (lastChar != '#')
                            result += " as single";
                        break;
                    case DLL_BASIC4GL_EXT_STRING:
                        if (lastChar != '$')
                            result += " as string";
                        break;
                }
            }
            result += "\r\nendstruc";
        }
    }

    return result;
}

