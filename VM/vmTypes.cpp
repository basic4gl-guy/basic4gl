//---------------------------------------------------------------------------
/*  Created 5-Sep-2003: Thomas Mulgrew

    Basic types and constants for TomVM virtual machine v 2             
*/


#pragma hdrstop

#include "vmTypes.h"
#include <sstream>
#include "Misc.h"

//---------------------------------------------------------------------------

#ifndef _MSC_VER
#pragma package(smart_init)
#endif

std::string vmBasicValTypeName (vmBasicValType type) {
    if (type < 0) {
        switch (type) {
        case VTP_INT:       return "INT";
        case VTP_REAL:      return "REAL";
        case VTP_STRING:    return "STRING";
		case VTP_FUNC_PTR:	return "FUNC_PTR";
        case VTP_NULL:      return "NULL";
        case VTP_UNDEFINED: return "UNDEFINED";
		case VTP_VOID:		return "VOID";
        default:            return "???";
        };
    }
    else
        return "ADVANCED TYPE";
}

std::string LowerCase (std::string str) {
    for (unsigned int i = 0; i < str.length (); i++)
        str [i] = tolower (str [i]);
    return str;
}
std::string UpperCase (std::string str) {
    for (unsigned int i = 0; i < str.length (); i++)
        str [i] = toupper (str [i]);
    return str;
}

////////////////////////////////////////////////////////////////////////////////
// vmValType

void vmValType::AddDimension (unsigned int elements) {
    assert (m_arrayLevel < VM_MAXDIMENSIONS);
    assert (elements > 0);

    // Bump up existing elements
    for (int i = m_arrayLevel; i > 0; i--)
        m_arrayDims [i] = m_arrayDims [i - 1];
    m_arrayLevel++;

    // Add new element count at dimension 0
    m_arrayDims [0] = elements;
}

unsigned int vmValType::ArraySize (unsigned int elementSize) const {

    // Calculate the array size based on the element size
    unsigned int result = elementSize;

    // Note: Array data format is
    //      Data                        Data size (# 4 byte values)
    //      ----                        ---------
    //      * Count                     1
    //      * Element size              1
    //      * Element 0                 Element size
    //      * Element 1                 Element size
    //          ...
    //      * Element Count - 1         Element size
    //
    // Thus total storage size (for 1D array) = 2 + (Count * Element size)
    // An N+1 dimension array is simply an array of N dimension arrays.

    for (int i = 0; i < m_arrayLevel; i++) {
        result *= m_arrayDims [i];
        result += 2;
    }

    return result;
}

bool vmValType::ArraySizeBiggerThan (unsigned int size, unsigned int elementSize) {

    // Returns true if the array size is bigger than size.
    // This is logically equivalent to "ArraySize (elementSize) > size", except
    // ArraySize can fail if the calculated size doesn't fit into an integer.

    // A major goal of Basic4GL is to provide a safe environment to experiment
    // in without worrying about breaking things.
    // Therefore we want to prevent people from trying to allocate unrealistic
    // amounts of memory. Thus we check array sizes upon allocation/declaration.

    unsigned int arraySize = elementSize;
    if (arraySize > size)
        return true;

    for (int i = 0; i < m_arrayLevel; i++) {
        if (size < 2 || (size - 2) / m_arrayDims [i] < arraySize)
            return true;
        arraySize *= m_arrayDims [i];
        arraySize += 2;
    }

    return false;
}

bool vmValType::Equals (const vmValType& t) const {

    // Return true if types match
    // Compare basic types and array and pointer levels
    if (    m_basicType                != t.m_basicType
        ||  m_arrayLevel               != t.m_arrayLevel
        ||  m_pointerLevel             != t.m_pointerLevel)
        return false;

    // Compare array dimensions (if not a pointer)
    if (m_pointerLevel == 0)
        for (int i = 0; i < m_arrayLevel; i++)
            if (m_arrayDims [i] != t.m_arrayDims [i])
                return false;

	// Check prototype index if function pointer
	if (m_basicType == VTP_FUNC_PTR && m_prototypeIndex != t.m_prototypeIndex)
		return false;

    return true;
}

bool vmValType::ExactEquals (const vmValType& t) const {

    // Equals returns true if the types are identical in implementation.
    // This means it will return true if one is a pointer and the other is a
    // reference (as both are the same internally).

    // ExactEquals returns false if one is a pointer and the other is a reference.

    // Note: The overloaded == operator uses Equals function.

    if (!Equals (t))
        return false;

    if (m_byRef != t.m_byRef)
        return false;

    return true;
}

#ifdef VM_STATE_STREAMING
void vmValType::StreamOut (std::ostream& stream) const {

    // Write vmValType to stream
    WriteLong (stream, m_basicType);
    WriteByte (stream, m_arrayLevel);
    WriteByte (stream, m_pointerLevel);
    WriteByte (stream, m_byRef);
    for (int i = 0; i < VM_MAXDIMENSIONS; i++)
        WriteLong (stream, m_arrayDims [i]);
}

void vmValType::StreamIn (std::istream& stream) {

    // Read vmValType from stream
    m_basicType     = (vmBasicValType) ReadLong (stream);
    m_arrayLevel    = ReadByte (stream);
    m_pointerLevel  = ReadByte (stream);
    m_byRef         = ReadByte (stream);
    for (int i = 0; i < VM_MAXDIMENSIONS; i++)
        m_arrayDims [i] = ReadLong (stream);
}
#endif
    
////////////////////////////////////////////////////////////////////////////////
// vmTypeLibrary

int vmTypeLibrary::GetStruc (std::string name) {
    name = LowerCase (name);
    for (unsigned int i = 0; i < m_structures.size (); i++) {
        if (m_structures [i].m_name == name)
            return i;
    }
    return -1;
}

int vmTypeLibrary::GetField (vmStructure& struc, std::string fieldName) {
    fieldName = LowerCase (fieldName);
    for (   int i = struc.m_firstField;
            i < struc.m_firstField + struc.m_fieldCount;
            i++) {
        assert (i >= 0);
        assert ((unsigned)i < m_fields.size ());
        if (m_fields [i].m_name == fieldName)
            return i;
    }
    return -1;
}
 
int vmTypeLibrary::DataSize (vmValType& type) {

    // How big would a variable of type "type" be?
    if (type.PhysicalPointerLevel () > 0)       // Pointers are always one element long
        return 1;

    if (type.m_arrayLevel > 0) {                // Calculate array size
        vmValType basicValType(type.m_basicType);
        return type.ArraySize(DataSize(basicValType));
    }

    if (type.m_basicType >= 0) {

        // Structured type. Lookup and return size of structure.
        assert ((unsigned)type.m_basicType < m_structures.size ());
        return m_structures [type.m_basicType].m_dataSize;
    }

    // Otherwise is basic type
    return 1;
}

bool vmTypeLibrary::DataSizeBiggerThan (vmValType& type, int size) {

    // Return true if data size > size.
    // This is logically equivalent to: DataSize (type) > size,
    // except it correctly handles integer overflow for really big types.
    assert (TypeValid (type));
    if (type.PhysicalPointerLevel () == 0 && type.m_arrayLevel > 0) {
        vmValType element = type;
        element.m_arrayLevel = 0;
        return type.ArraySizeBiggerThan (size, DataSize (element));
    }
    else
        return DataSize (type) > size;
}

bool vmTypeLibrary::TypeValid (vmValType& type) {

    return      type.m_basicType > VTP_MARKER
            &&  type.m_basicType != VTP_UNDEFINED
            &&  (type.m_basicType < 0 || (unsigned)type.m_basicType < m_structures.size ())
            &&  type.m_arrayLevel < VM_MAXDIMENSIONS;
}

std::string vmTypeLibrary::DescribeVariable (std::string name, vmValType& type) {

    if (!TypeValid (type))
        return (std::string) "INVALID TYPE " + name;

    // Return a string describing what the variable stores
    std::string result;

    // Variable type
    if (type.m_basicType >= 0)
        result = m_structures [type.m_basicType].m_name;      // Structure type
    else
        result = vmBasicValTypeName (type.m_basicType);       // Basic type

	if (name != "") result += " ";

    // Append pointer prefix
    int i;
    for (i = 0; i < type.VirtualPointerLevel (); i++)
        result += "&";

    // Variable name
    result += name;

    // Append array indices
    for (i = type.m_arrayLevel - 1; i >= 0; i--) {
        result += "(";
		if (type.VirtualPointerLevel() == 0) {
			int size = type.m_arrayDims[i];
			if (size > 0)
				result += IntToString(size - 1);
		}
        result += ")";
    }

    return result;
}

#ifdef VM_STATE_STREAMING
void vmTypeLibrary::StreamOut (std::ostream& stream) {
    int i;

    // Write out fields
    WriteLong (stream, m_fields.size ());
    for (i = 0; i < m_fields.size (); i++)
        m_fields [i].StreamOut (stream);

    // Write out structures
    WriteLong (stream, m_structures.size ());
    for (i = 0; i < m_structures.size (); i++)
        m_structures [i].StreamOut (stream);
}

void vmTypeLibrary::StreamIn (std::istream& stream) {
    int i, count;

    // Clear existing data
    m_fields.clear ();
    m_structures.clear ();

    // Read fields
    count = ReadLong (stream);
    m_fields.resize (count);
    for (i = 0; i < count; i++)
        m_fields [i].StreamIn (stream);

    // Read structures
    count = ReadLong (stream);
    m_structures.resize (count);
    for (i = 0; i < count; i++)
        m_structures [i].StreamIn (stream);
}
#endif

////////////////////////////////////////////////////////////////////////////////
// vmValTypeSet

int vmValTypeSet::GetIndex (vmValType& type) {

    // Get index of type "type" in our set.
    // If type is not present, create a new one and return an index to that.

    // Look for type
    unsigned int i;
    for (i = 0; i < m_types.size (); i++)
        if (m_types [i].Equals (type))
            return i;

    // Otherwise create new one
    i = m_types.size ();
    m_types.push_back (type);
    return i;
}

bool vmValType::CanStoreInRegister() {
    return m_pointerLevel > 0 ||                // Pointers fit in a register
        (m_arrayLevel == 0 && m_basicType < 0); // Or single basic types
}

vmValType vmValType::RegisterType() {

    // Return the actual type that will be stored in a register when referring
    // to data of this type.
    // For values that fit into a register, the register type is the same as
    // the original type represented.
    // For large values like structures and arrays, the register will store an
    // implicit reference to the data instead.

    // Copy this value type
    vmValType result = *this;

    // Check if type is an array or structure
    if (!result.CanStoreInRegister()) {     

        // A structure or array cannot fit into a register.
        // What is stored is an implicit by-reference pointer.
        result.m_pointerLevel++;
        result.m_byRef = true;
    } 

    return result;
}

#ifdef VM_STATE_STREAMING
void vmValTypeSet::StreamOut (std::ostream& stream) const {
    WriteLong (stream, m_types.size ());
    for (int i = 0; i < m_types.size (); i++)
        m_types [i].StreamOut (stream);
}

void vmValTypeSet::StreamIn (std::istream& stream) {
    int count = ReadLong (stream);
    m_types.resize (count);
    for (int i = 0; i < count; i++)
        m_types [i].StreamIn (stream);
}
#endif

////////////////////////////////////////////////////////////////////////////////
// vmStructureField

#ifdef VM_STATE_STREAMING
void vmStructureField::StreamOut (std::ostream& stream) {
#ifdef STREAM_NAMES
    WriteString (stream, m_name);
#endif
    m_type.StreamOut (stream);
    WriteLong (stream, m_dataOffset);
}

void vmStructureField::StreamIn (std::istream& stream) {
#ifdef STREAM_NAMES
    m_name = ReadString (stream);
#endif
    m_type.StreamIn (stream);
    m_dataOffset = ReadLong (stream);
}
#endif

////////////////////////////////////////////////////////////////////////////////
// vmStructure

#ifdef VM_STATE_STREAMING
void vmStructure::StreamOut (std::ostream& stream) {
	assert(m_isDefined);			// Definition should have been completed
#ifdef STREAM_NAMES
    WriteString (stream, m_name);
#endif
    WriteLong (stream, m_firstField);
    WriteLong (stream, m_fieldCount);
    WriteLong (stream, m_dataSize);
    WriteByte (stream, m_containsString);
    WriteByte (stream, m_containsArray);
}

void vmStructure::StreamIn (std::istream& stream) {
#ifdef STREAM_NAMES
    m_name = ReadString (stream);
#endif
    m_firstField        = ReadLong (stream);
    m_fieldCount        = ReadLong (stream);
    m_dataSize          = ReadLong (stream);
    m_containsString    = ReadByte (stream);
    m_containsArray     = ReadByte (stream);
	m_isDefined = true;
}
#endif
