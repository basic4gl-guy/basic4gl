//---------------------------------------------------------------------------
/*  Created 5-Sep-2003: Thomas Mulgrew

    Basic types and constants for TomVM virtual machine v 2              

    Notes:                                                  

        Basic4GL's basic type is a 4 bytes, which can store either:
            * An integer
            * A float
            * An index to a string
            * A pointer

        We store all program variable data in a single resizeable array
        (std::vector) of these values.

        "Pointers" are stored as indices into the array, rather than pointing
        directly to the address.
        (This is so we can quickly check that they are valid and avoid access
        violations e.t.c.)

        Likewise the "size" of a variable means the number of elements in the
        array, rather than the actual size of the data.
*/

#ifndef vmTypesH
#define vmTypesH
//---------------------------------------------------------------------------

#include "Misc.h"
#include <string>
#include <vector>
#include <sstream>
#include <assert.h>
#include <math.h>
#include "streaming.h"
#ifndef _MSC_VER
#ifndef __ANDROID__
#ifndef __EMSCRIPTEN__
#include <mem.h>
#endif
#endif
#endif

// Constants
#define VM_MAXDIMENSIONS 10         // Maximum dimensions in an array

// Basic supported variable types
typedef std::string     vmString;
typedef int             vmInt;
typedef float           vmReal;
#define vmRealIsFloat
//#define vmRealIsDouble

// Other internal types
typedef bool            vmBool;

#ifndef byte
typedef unsigned char   byte;
#endif

////////////////////////////////////////////////////////////////////////////////
// vmBasicValType
//
// Basic4GL data types are indicated by an integer.
// -1 is special, and indicates an undefined type.
// Other negative numbers are the basic supported types of language. That is,
// string, integer and floating point ("real").
//
// Positive numbers represent structure types, and the value is the index of
// the structure in the structures array.
//
// Note:    Basic value types are loaded into registers directly.
//          Other types (arrays and structures) are loaded as pointers.

enum vmBasicValType {   VTP_MARKER		= -9,			// Not an actual type. Marks the start of valid types (VTP_MARKER + 1 and up)
						VTP_UNTYPED_FUNC_PTR = -8,		// Like VTP_FUNC_PTR, but prototype not known at compile time. Runtime check required before assigning to a VTP_FUNC_PTR
						VTP_VOID		= -7,			// Used to indicate an expression evaluated to a sub call (i.e. has no value)
                        VTP_FUNC_PTR	= -6,			// Function pointer. (Internally stored as an integer)
						VTP_INT         = -5,
                        VTP_REAL        = -4,
                        VTP_STRING      = -3,
                        VTP_NULL        = -2,
                        VTP_UNDEFINED   = -1,
                        };

// Displaying basic types and values
std::string vmBasicValTypeName (vmBasicValType type);

// Basic string conversion routines
inline std::string IntToString (vmInt i)   {
    std::stringstream s;
    s << i;
    return s.str ();
}
inline std::string RealToString (vmReal r) {
    std::stringstream s;
    s << r;
    return s.str ();
}

inline vmInt StringToInt (std::string& s) {
    vmInt i;

    // Check if it's a hex string
    std::string s2 = s.substr(0, 2);
    std::string s3 = s.substr(0, 3);
    if (s2 == "0x" || s2 == "0X")
        std::stringstream(s.substr(2)) >> std::hex >> i;        // Strip 0x and parse as hex
    else if (s3 == "-0x" || s3 == "-0X") {
        std::stringstream(s.substr(3)) >> std::hex >> i;        // Strip -0x and parse as hex
        i = -i;                                                 // Then negate
    }
    else
        std::stringstream(s) >> i;                              // Parse as regular string
    return i;
}

inline vmReal StringToReal (std::string& s)  {
    vmReal r;
    std::stringstream (s) >> r;
    return r;
}
std::string LowerCase (std::string str);
std::string UpperCase (std::string str);

////////////////////////////////////////////////////////////////////////////////
// vmValType
//
// An extended value type.
// Can be either of:
//      * A basic value
//      * A structure
//      * An array (one or more dimensions) of either of the above.
//        Note: Indexing an N dimensional results in an N-1 dimensional array.
//              Unless N = 1, in which case it results in an element.
//      * A pointer to any of the above.
//      * A "value by reference".
//        This is a pointer 'pretending' to be a value. Used to refer to
//        structures and anything else that can't fit into a register.

class vmValType {
public:
    vmBasicValType  m_basicType;                        // Basic type
    byte            m_arrayLevel;                       // 0 = value, 1 = array, 2 = 2D array
    byte            m_pointerLevel;                     // 0 = value, 1 = pointer to value, 2 = pointer to pointer to value, ...
    bool            m_byRef;
    unsigned int    m_arrayDims [VM_MAXDIMENSIONS];     // # of elements in each array dimension
	int				m_prototypeIndex;					// Index of function prototype. Valid when m_basicType = VTP_FUNC_PTR.

    vmValType ()                        { Set (VTP_UNDEFINED); }
    vmValType (const vmBasicValType t)  { Set (t); }
    vmValType (const vmValType& t)      { Set (t); }
    vmValType ( vmBasicValType t,
                byte array,
                byte pointer = 0,
                bool byRef = false)     { Set (t, array, pointer, byRef); }

    vmValType& Set (vmBasicValType t,
                    byte array     = 0,
                    byte pointer   = 0,
                    bool byRef     = false) {
        assert (array <= VM_MAXDIMENSIONS);
        m_basicType     = t;
        m_arrayLevel    = array;
        m_pointerLevel  = pointer;
        m_byRef         = byRef;
		m_prototypeIndex = -1;
		memset(m_arrayDims, 0, VM_MAXDIMENSIONS * sizeof(m_arrayDims[0]));
        return *this;
    }
    vmValType& Set (const vmValType& t)  {
        m_basicType     = t.m_basicType;
        m_arrayLevel    = t.m_arrayLevel;
        m_pointerLevel  = t.m_pointerLevel;
        m_byRef         = t.m_byRef;
		m_prototypeIndex = t.m_prototypeIndex;
        memcpy (m_arrayDims, t.m_arrayDims, VM_MAXDIMENSIONS * sizeof (m_arrayDims [0]));
        return *this;
    }
    bool Equals (const vmValType& t) const;
    bool Equals (const vmBasicValType t) const { return Equals (vmValType (t)); }
    bool ExactEquals (const vmValType& t) const;
    bool ExactEquals (const vmBasicValType t) const { return ExactEquals (vmValType (t)); }
    bool IsNull () const { return m_basicType == VTP_NULL; }
	bool IsFuncPtr() const
    {
		return m_basicType == VTP_FUNC_PTR
			&& m_pointerLevel == 0
			&& m_arrayLevel == 0;
    }
    int PhysicalPointerLevel () const { return m_pointerLevel; }
    int VirtualPointerLevel  () const { return m_pointerLevel + (m_byRef ? -1 : 0); }
    void AddDimension (unsigned int elements);
    unsigned int ArraySize (unsigned int elementSize) const;
    bool ArraySizeBiggerThan (unsigned int size, unsigned int elementSize);
    bool CanStoreInRegister();
    vmValType RegisterType();

    vmBasicValType StoredType () {

        // Type of actual data stored inside virtual machine register.
        if (m_pointerLevel == 0 && m_arrayLevel == 0)   return m_basicType;
        else                                            return VTP_INT;
    }
    bool IsBasic () {
        return m_pointerLevel == 0 && m_arrayLevel == 0 && m_basicType < 0;
    }

#ifdef VM_STATE_STREAMING
    // Streaming
    void StreamOut (std::ostream& stream) const;
    void StreamIn (std::istream& stream);
#endif

    vmValType&  operator= (const vmValType& t)          { Set (t); return *this; }
    vmValType&  operator= (const vmBasicValType& t)     { Set (t); return *this; }
    vmValType&  operator<< (int elements)               { AddDimension (elements); return *this; }
    bool        operator== (const vmValType& t)         { return Equals (t); }
    bool        operator== (const vmBasicValType t)     { return Equals (t); }
};

////////////////////////////////////////////////////////////////////////////////
// vmStructure
//
// 1 or more vmValTypes combined into a structure type.
//
// E.g.
// Struc
//  dim angle#, names$ (100), x, y
// EndStruc

struct vmStructureField {
    std::string m_name;                 // Field name
    vmValType   m_type;                 // Data type
    int         m_dataOffset;           // Data offset from top of structure
    vmStructureField (std::string& name, vmValType& type, int dataOffset = 0) {
        m_name          = LowerCase (name);
        m_type          = type;
        m_dataOffset    = dataOffset;
    }
	vmStructureField () {
		m_name			= "";
		m_type			= VTP_INT;
		m_dataOffset	= 0;
	}

#ifdef VM_STATE_STREAMING
    // Streaming
    void StreamOut (std::ostream& stream);
    void StreamIn (std::istream& stream);
#endif
};

struct vmStructure {
    std::string m_name;                 // Type name
	bool		m_isDefined;			// Has structure been defined. False if being defined, or has been forward declared.
    int         m_firstField;           // Index of first field
    int         m_fieldCount;           // # of fields in structure
    int         m_dataSize;             // Size of data
    bool        m_containsString;       // Contains one or more strings. (Requires special handling when copying data.)
    bool        m_containsArray;        // Contains one or more arrays.  (Requires special handling when allocating data.)
    bool        m_containsPointer;      // Contains one or more pointers. (Requires pointer validity checking when copying data.)

    vmStructure (std::string name = (std::string) "", int firstField = 0) {
        m_name          = LowerCase (name);
		m_isDefined		= false;
        m_firstField    = firstField;
        m_fieldCount    = 0;
        m_dataSize      = 0;
        m_containsString    = false;
        m_containsArray     = false;
        m_containsPointer   = false;
    }

#ifdef VM_STATE_STREAMING
    // Streaming
    void StreamOut (std::ostream& stream);
    void StreamIn (std::istream& stream);
#endif
};

////////////////////////////////////////////////////////////////////////////////
// vmTypeLibrary
//
// Used to store structure definitions, and operate on data types

class vmTypeLibrary {
    std::vector<vmStructureField>   m_fields;
    std::vector<vmStructure>        m_structures;
	int m_currentStrucIndex;
public:
    std::vector<vmStructureField>&  Fields ()       { return m_fields; }
    std::vector<vmStructure>&       Structures ()   { return m_structures; }

    void Clear () {
        m_fields.clear ();
        m_structures.clear ();
		m_currentStrucIndex = -1;
    }

    // Finding structures and fields
    bool    Empty ()                                                { return m_structures.empty (); }
    bool    StrucStored (std::string& name)                         { return GetStruc (name) >= 0; }
    bool    FieldStored (vmStructure& struc, std::string& fieldName) { return GetField (struc, fieldName) >= 0; }
    int     GetStruc (std::string name);
    int     GetField (vmStructure& struc, std::string fieldName);

    // Data type operations
    int DataSize (vmValType& type);
    bool DataSizeBiggerThan (vmValType& type, int size);
    bool TypeValid (vmValType& type);
    bool ContainsString (vmValType& type) {
        assert (TypeValid (type));

        // Pointers to objects don't *contain* anything.
        // (Pointing to something that contains a string doesn't count.)
        if (type.VirtualPointerLevel () > 0)
            return false;

        // Examine data type
        if (type.m_basicType < 0)   return type.m_basicType == VTP_STRING;
        else                        return m_structures [type.m_basicType].m_containsString;
    }
    bool ContainsArray (vmValType& type) {
        assert (TypeValid (type));

        return      type.VirtualPointerLevel () == 0                                                // Must not be a pointer
                && (    type.m_arrayLevel > 0                                                       // Can be an array
                    ||  (type.m_basicType >= 0 && m_structures [type.m_basicType].m_containsArray));// or a structure containing an array
    }
    bool ContainsPointer(vmValType& type) {
        assert(TypeValid(type));

        // Type is a pointer?
        if (type.m_pointerLevel > 0)
            return true;

        // Is a structure (or array of structures) containing a pointer?
        if (type.m_basicType >= 0)
            return m_structures[type.m_basicType].m_containsPointer;

        return false;
    }

    // Building structures
    vmStructure& CurrentStruc () {
		assert(m_currentStrucIndex >= 0 && m_currentStrucIndex < m_structures.size());
		return m_structures[m_currentStrucIndex];
    }
    vmStructureField& CurrentField () {
        assert (m_fields.size () > (unsigned)CurrentStruc().m_firstField);           // Current structure must have at least 1 field
        return m_fields.back ();
    }
    vmStructure& NewStruc (std::string& name) {                             // Create a new structure and make it current

        // Name must be valid and not already used
        assert (name != "");
        assert (!StrucStored (name));

        // Create new structure
		m_currentStrucIndex = m_structures.size();
        m_structures.push_back (vmStructure (name, m_fields.size ()));
        return CurrentStruc ();
    }
	vmStructure& MakeStrucCurrent(std::string& name)
    {
		assert(StrucStored(name));
		m_currentStrucIndex = GetStruc(name);
		return CurrentStruc();
    }
	void DefineExistingStruc()
    {
		assert(!CurrentStruc().m_isDefined);
		CurrentStruc().m_firstField = m_fields.size();
    }
    vmStructureField& NewField (std::string& name, vmValType& type) {        // Create a new field and assign it to the current structure

        // Name must be valid and not already used within current structure
        assert (name != "");
        assert (!FieldStored (CurrentStruc (), name));

        // Type must be valid, and not an instance of the current structure type
        // (or an array. Can be a pointer though.)
        assert (TypeValid (type));
        assert (!type.m_byRef);
        assert (    type.m_pointerLevel > 0
                ||  type.m_basicType < 0
                ||  (unsigned)type.m_basicType + 1 < m_structures.size ());

        // Create new field
        m_fields.push_back (vmStructureField (name, type, CurrentStruc ().m_dataSize));
        CurrentStruc().m_fieldCount++;
        CurrentStruc().m_dataSize += DataSize(CurrentField().m_type);

        // Update current structure statistics
        CurrentStruc().m_containsString     = CurrentStruc().m_containsString
                                            || ContainsString(type);
        CurrentStruc().m_containsArray      = CurrentStruc().m_containsArray
                                            || ContainsArray(type);
        CurrentStruc().m_containsPointer    = CurrentStruc().m_containsPointer
                                            || ContainsPointer(type);
        return CurrentField();
    }

    // Debugging/output
    std::string DescribeVariable (std::string name, vmValType& type);

    // Streaming
#ifdef VM_STATE_STREAMING
    void StreamOut (std::ostream& stream);
    void StreamIn (std::istream& stream);
#endif
};

////////////////////////////////////////////////////////////////////////////////
// vmValTypeSet
//
// A small number of VM op-codes operate on vmValType advanced data types
// (e.g. OP_COPY). Op-codes don't have storage space to specify an advanced data
// type, so instead they specify an index into this set array.
class vmValTypeSet {
    std::vector<vmValType> m_types;
public:
    void Clear () { m_types.clear (); }
    int GetIndex (vmValType& type);
    vmValType& GetValType (int index) {
        assert (index >= 0);
        assert ((unsigned)index < m_types.size ());
        return m_types [index];
    }

#ifdef VM_STATE_STREAMING
    // Streaming
    void StreamOut (std::ostream& stream) const;
    void StreamIn (std::istream& stream);
#endif
};

#endif

