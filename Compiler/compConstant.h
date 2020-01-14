//---------------------------------------------------------------------------
// Created 05-Aug-06: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

#ifndef compConstantH
#define compConstantH

#include "vmTypes.h"
#include <map>

// compConstant
//
// Recognised constants (e.g. "true", "false")

struct compConstant {
    vmBasicValType  m_valType;          // Value type
    vmInt           m_intVal;           // Value
    vmReal          m_realVal;
    vmString        m_stringVal;

    compConstant ()
        :   m_valType (VTP_STRING),
            m_stringVal (""),
            m_intVal (0),
            m_realVal (0) { ; }
    compConstant (const std::string& s)
        :   m_valType (VTP_STRING),
            m_stringVal (s),
            m_intVal (0),
            m_realVal (0) { ; }
    compConstant (int i)
        :   m_valType (VTP_INT),
            m_intVal (i),
            m_stringVal (""),
            m_realVal (0) { ; }
    compConstant (unsigned int i)
        :   m_valType (VTP_INT),
            m_intVal (i),
            m_stringVal (""),
            m_realVal (0) { ; }
    compConstant (float r)
        :   m_valType (VTP_REAL),
            m_realVal (r),
            m_stringVal (""),
            m_intVal (0) { ; }
    compConstant (double r)
        :   m_valType (VTP_REAL),
            m_realVal ((float)r),
            m_stringVal (""),
            m_intVal (0) { ; }
    compConstant (const compConstant& c)
        :   m_valType (c.m_valType),
            m_realVal (c.m_realVal),
            m_intVal (c.m_intVal),
            m_stringVal (c.m_stringVal) { ; }

    std::string ToString() const {
        switch(m_valType) {
            case VTP_INT:    return IntToString(m_intVal);
            case VTP_REAL:   return RealToString(m_realVal);
            case VTP_STRING: return m_stringVal;
            default:    return "???";
        }
    }

#ifdef VM_STATE_STREAMING
    void StreamOut(std::ostream& stream);
    void StreamIn(std::istream& stream);
#endif
};

typedef std::map<std::string,compConstant> compConstantMap;


//---------------------------------------------------------------------------
#endif
