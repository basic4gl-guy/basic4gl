//---------------------------------------------------------------------------


#pragma hdrstop

#include "compFunction.h"

//---------------------------------------------------------------------------

#ifndef _MSC_VER
#pragma package(smart_init)
#endif

////////////////////////////////////////////////////////////////////////////////
// compParamTypeList

compParamTypeList::compParamTypeList (compParamTypeList& list) {
    for (vmValTypeList::iterator i = list.m_params.begin (); i != list.m_params.end (); i++)
        m_params.push_back (*i);
}