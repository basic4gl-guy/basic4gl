//---------------------------------------------------------------------------
/*  Created 7-Sep-2003: Thomas Mulgrew

    Variable storage structures.
*/

#ifndef vmVariablesH
#define vmVariablesH
//---------------------------------------------------------------------------

#include "vmData.h"
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// vmVariable
//
// Stores a variable.
// Binds a variable name to data and type information.
class vmVariable {
public:
    std::string     m_name;             // Variable name
    unsigned int    m_dataIndex;        // Index of data in vmData array. 0 = not allocated.
    vmValType       m_type;             // Data type

	vmVariable () {
		m_name		= "";
		m_dataIndex	= 0;
		m_type		= VTP_INT;
	}
	vmVariable (std::string& name, vmValType& type) {
        m_name      = LowerCase (name);
        m_type      = type;
        Deallocate ();
    }
    ~vmVariable () {
        Deallocate ();
    }

    bool            Allocated ()    { return m_dataIndex > 0; }

    // Allocating
    void Deallocate ()              { m_dataIndex = 0; }
    void Allocate (vmData& data, vmTypeLibrary& typeLib);

    // Streaming
#ifdef VM_STATE_STREAMING
    void StreamOut (std::ostream& stream) const;
    void StreamIn (std::istream& stream);
#endif
};

////////////////////////////////////////////////////////////////////////////////
// vmVariables
//
// An array of variables.

typedef std::vector<vmVariable> vmVariableArray;
class vmVariables {
    vmVariableArray m_variables;            // Variables
    vmData&         m_data;                 // Data
    vmTypeLibrary&  m_types;                // Type information
public:
    vmVariables (vmData& data, vmTypeLibrary& types)
        : m_data (data), m_types (types) { ; }

    vmVariableArray&    Variables ()    { return m_variables; }
    vmData&             Data ()         { return m_data; }
    vmTypeLibrary&      Types ()        { return m_types; }

    void Deallocate () {

        // Deallocate all variable data.
        // Variables and data type info remain.
        for (   vmVariableArray::iterator i = m_variables.begin ();
                i != m_variables.end ();
                i++)
            (*i).Deallocate ();
        m_data.Clear ();
    }
    void Clear () {

        // Deallocate everything.
        // No variables, data or type information remains
        m_variables.clear ();
        m_data.Clear ();
        m_types.Clear ();
    }

    // Finding variables
    int GetVar (std::string name);
    bool VarStored (std::string& name)  { return GetVar (name) >= 0; }
    int Size ()                         { return m_variables.size (); }
    bool IndexValid (int index)         { return index >= 0 && index < Size (); }

    // Variable creation and allocation
    int NewVar (std::string& name, vmValType& type) {
        assert (!VarStored (name));

        // Allocate new variable and return index
        int top = m_variables.size ();
        m_variables.push_back (vmVariable (name, type));
        return top;
    }
    void AllocateVar (vmVariable& var) {
        var.Allocate (m_data, m_types);
    }

//    std::string VarValueAsString (const vmVariable& var);

    // Streaming
#ifdef VM_STATE_STREAMING
    void StreamOut (std::ostream& stream) const;
    void StreamIn (std::istream& stream);
#endif
};

#endif


