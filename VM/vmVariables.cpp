//---------------------------------------------------------------------------
/*  Created 7-Sep-2003: Thomas Mulgrew

    Variable storage structures.
*/

#pragma hdrstop
#include "vmVariables.h"

//---------------------------------------------------------------------------

#ifndef _MSC_VER
#pragma package(smart_init)
#endif

////////////////////////////////////////////////////////////////////////////////
// vmVariable
void vmVariable::Allocate (vmData& data, vmTypeLibrary& typeLib) {

    // Allocate new data
    assert (!Allocated ());
    m_dataIndex = data.Allocate (typeLib.DataSize (m_type));

    // Initialise it
    data.InitData (m_dataIndex, m_type, typeLib);
}

#ifdef VM_STATE_STREAMING
void vmVariable::StreamOut (std::ostream& stream) const
{
#ifdef STREAM_NAMES
    WriteString (stream, m_name);
#endif
    WriteLong (stream, m_dataIndex);
    m_type.StreamOut (stream);
}

void vmVariable::StreamIn (std::istream& stream) {
#ifdef STREAM_NAMES
    m_name = ReadString (stream);
#endif
    m_dataIndex = ReadLong (stream);
    m_type.StreamIn (stream);
}
#endif

////////////////////////////////////////////////////////////////////////////////
// vmVariables
int vmVariables::GetVar (std::string name) {
    name = LowerCase (name);
    for (unsigned int i = 0; i < m_variables.size (); i++)
        if (m_variables [i].m_name == name)
            return i;
    return -1;
}

#ifdef VM_STATE_STREAMING
void vmVariables::StreamOut (std::ostream& stream) const {

    // Stream out type data
    m_types.StreamOut (stream);

    // Stream out variables
    WriteLong (stream, m_variables.size ());
    for (int i = 0; i < m_variables.size (); i++)
        m_variables [i].StreamOut (stream);
}

void vmVariables::StreamIn (std::istream& stream) {

    // Stream in type data
    m_types.StreamIn (stream);

    // Stream in variables
    long count = ReadLong (stream);
    m_variables.resize (count);
    for (int i = 0; i < count; i++)
        m_variables [i].StreamIn (stream);
}
#endif
