//---------------------------------------------------------------------------


#pragma hdrstop

#include "vmData.h"

//---------------------------------------------------------------------------

#ifndef _MSC_VER
#pragma package(smart_init)
#endif

////////////////////////////////////////////////////////////////////////////////
// vmIntHandleResources

void vmIntHandleResources::Clear () {

    // Delete set elements
    for (   std::set<int>::iterator i = m_handles.begin ();
            i != m_handles.end ();
            i++)
        DeleteHandle (*i);

    // Clear set
    m_handles.clear ();
}

////////////////////////////////////////////////////////////////////////////////
// vmData

void vmData::InitData (int i, vmValType& type, vmTypeLibrary& typeLib) {
    assert (typeLib.TypeValid (type));

    // Bail out if doesn't contain array
    if (!typeLib.ContainsArray (type))
        return;

    // Type IS array?
    if (type.m_arrayLevel > 0) {

        // Find element type
        vmValType elementType = type;
        elementType.m_arrayLevel--;
        int elementSize = typeLib.DataSize (elementType);

        // Set array header
        assert (IndexValid (i));
        assert (IndexValid (i + 1));
        m_data [i].IntVal ()        = type.m_arrayDims [type.m_arrayLevel - 1]; // First value  = # of elements
        m_data [i + 1].IntVal ()    = elementSize;                              // Second value = element size

        // Initialise elements (if necessary)
        if (typeLib.ContainsArray (elementType)) {
            for (unsigned int i2 = 0; i2 < type.m_arrayDims [type.m_arrayLevel - 1]; i2++)
                InitData (i + 2 + i2 * elementSize, elementType, typeLib);
        }
    }

    // Or type is structure containing array?
    else if (type.m_basicType >= 0) {

        // Initialise each field
        vmStructure& structure = typeLib.Structures () [type.m_basicType];
        for (int i2 = 0; i2 < structure.m_fieldCount; i2++) {
            vmStructureField& field = typeLib.Fields () [structure.m_firstField + i2];
            if (typeLib.ContainsArray (field.m_type))
                InitData (i + field.m_dataOffset, field.m_type, typeLib);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// vmValue

#ifdef VM_STATE_STREAMING
void vmValue::StreamOut (std::ostream& stream) const {

    // There may be some potential cross-platform streaming issues because:
    //  1. We are unioning two data types together.
    //  2. We don't know at stream time what data type it is.
    stream.write ((char *) &m_val, sizeof (m_val));
}

void vmValue::StreamIn (std::istream& stream) {
    stream.read ((char *) &m_val, sizeof (m_val));
}
#endif

////////////////////////////////////////////////////////////////////////////////
// vmProgramDataElement
#ifdef VM_STATE_STREAMING
void vmProgramDataElement::StreamOut (std::ostream& stream) const {
    WriteLong (stream, m_type);
    m_value.StreamOut (stream);
}

void vmProgramDataElement::StreamIn (std::istream& stream) {
    m_type = (vmBasicValType) ReadLong (stream);
    m_value.StreamIn (stream);
}
#endif
