//---------------------------------------------------------------------------
// Created 28-Sep-2003: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
// Copyright (C) Thomas Mulgrew
#ifndef compFunctionH
#define compFunctionH
//---------------------------------------------------------------------------
#include "vmTypes.h"
#include "vmFunction.h"
#include <vector>

////////////////////////////////////////////////////////////////////////////////
//  Compile time parameter validation callback.
//  Called at compile time to validate any parameters whose types are specified
//  as VTP_UNDEFINED.
//  Note: Index is 0 for the leftmost parameter and so on (unlike at run time)
//  Callback should return true if parameter type is valid.
typedef bool (*compParamValidationCallback)(int index, vmValType type);

////////////////////////////////////////////////////////////////////////////////
// compParamTypeList
typedef std::vector<vmValType> vmValTypeList;

class compParamTypeList {
    vmValTypeList m_params;

public:
    compParamTypeList () { ; }
    compParamTypeList (compParamTypeList& list);
    compParamTypeList& operator<< (vmValType val) {
        m_params.push_back (val);
        return *this;
    }

    vmValTypeList& Params() { return m_params; }
	vmValTypeList const& Params() const { return m_params; }
};

////////////////////////////////////////////////////////////////////////////////
// compFuncSpec
//
// Specifies a function and parameters
struct compFuncSpec {
    compParamTypeList           m_paramTypes;
    bool                        m_brackets,         // True if function requires brackets around parameters
                                m_isFunction;       // True if function returns a value
    vmValType                   m_returnType;
    bool                        m_timeshare;        // True if virtual machine should perform a timesharing break immediately after returning
	bool						m_conditionalTimeshare;		// True if virtual machine should check the m_timeshare flag and if set perform a timesharing break immediately after returning.
    int                         m_index;            // Index in Virtual Machine's "functions" array
    bool                        m_freeTempData;     // True if function allocates temporary data that should be freed before the next instruction
    compParamValidationCallback m_paramValidationCallback;


    compFuncSpec () { ; }
    compFuncSpec (  compParamTypeList&  paramTypes,
                    bool                brackets,
                    bool                isFunction,
                    vmValType           returnType,
                    bool                timeshare,
					bool                conditionalTimeshare,
                    int                 index,
                    bool                freeTempData,
                    compParamValidationCallback paramValidationCallback)
        :   m_paramTypes    (paramTypes),
            m_brackets      (brackets),
            m_isFunction    (isFunction),
            m_returnType    (returnType),
            m_timeshare     (timeshare),
			m_conditionalTimeshare(conditionalTimeshare),
            m_index         (index),
            m_freeTempData  (freeTempData),
            m_paramValidationCallback(paramValidationCallback) { ; }
    compFuncSpec (  const compFuncSpec& spec)
        :   m_isFunction    (spec.m_isFunction),
            m_brackets      (spec.m_brackets),
            m_returnType    (spec.m_returnType),
            m_timeshare     (spec.m_timeshare),
			m_conditionalTimeshare(spec.m_conditionalTimeshare),
            m_index         (spec.m_index),
            m_freeTempData  (spec.m_freeTempData),
            m_paramValidationCallback(spec.m_paramValidationCallback) { m_paramTypes = spec.m_paramTypes; }
};

#endif
