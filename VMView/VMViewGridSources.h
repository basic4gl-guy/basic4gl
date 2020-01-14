/*  Created 21-Dec-07: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

    Interfaces for communicating between application and virtual machine view
*/
#ifndef _VMViewGridSources_h
#define _VMViewGridSources_h

#include "VMViewUI.h"
#include "TomComp.h"
#include "VMViewCallbackInterfaces.h"

////////////////////////////////////////////////////////////////////////////////
//  VMCodeGridSource
//
/// Grid source for displaying virtual machine op-codes

class VMCodeGridSource : public IGridSource {
    TomBasicCompiler& comp;
    TomVM& vm;
    ISourceLineFormatter* sourceLineFormatter;

    std::string GetOpCodeData(vmInstruction& i);
    std::string GetSourcePos(vmInstruction& i);

public:
    VMCodeGridSource(
        TomBasicCompiler& _comp,
        ISourceLineFormatter* _sourceLineFormatter);

    // IGridSource
    virtual int GetRowCount();
    virtual int GetFirstRowIndex();
    virtual std::string GetValue(int row, int col);
};

////////////////////////////////////////////////////////////////////////////////
//  VMValueGridSource
//
/// Grid source for displaying vmValues. (Abstract base class)

class VMValueGridSource : public IGridSource {
protected:
    TomVM& vm;
    VMValueGridSource(TomVM& _vm);
    virtual vmValue& GetValueObject(int index) = 0;

public:
    // IGridDataSource
    virtual std::string GetValue(int row, int col);    
};

////////////////////////////////////////////////////////////////////////////////
//  VMDataGridSource
//
/// Grid source for displaying data (heap, stack, temp etc)

class VMDataGridSource : public VMValueGridSource {
    int start, end;

protected:
    VMDataGridSource(TomVM& _vm, int _start, int _end);
    virtual vmValue& GetValueObject(int index);
    
public:
    // IGridSource
    virtual int GetRowCount();
    virtual int GetFirstRowIndex();
};

class VMHeapDataGridSource : public VMDataGridSource {
public:
    VMHeapDataGridSource(TomVM& _vm)
        : VMDataGridSource(_vm, _vm.Data().Permanent(), _vm.Data().Data().size()) {}
};

class VMStackDataGridSource : public VMDataGridSource {
public:
    VMStackDataGridSource(TomVM& _vm)
        : VMDataGridSource(_vm, _vm.Data().StackTop(), _vm.Data().Permanent()) {}
};

class VMTempDataGridSource : public VMDataGridSource {
public:
    VMTempDataGridSource(TomVM& _vm)
        : VMDataGridSource(_vm, 1, _vm.Data().TempData()) {}
};

////////////////////////////////////////////////////////////////////////////////
//  VMVariableGridSource
//
/// Grid source for displaying variables

class VMVariableGridSource : public IGridSource {
protected:
    TomVM& vm;

    VMVariableGridSource(TomVM& _vm);
    virtual void GetVar(int variableIndex, int& dataIndex, vmValType& type) = 0;
    virtual std::string GetVarName(int variableIndex) = 0;
public:

    // IGridSource
    virtual int GetFirstRowIndex();
    virtual std::string GetValue(int row, int col);
};

class VMGlobalVariableGridSource : public VMVariableGridSource {
protected:
    virtual void GetVar(int variableIndex, int& dataIndex, vmValType& type);
    virtual std::string GetVarName(int variableIndex);

public:
    VMGlobalVariableGridSource(TomVM& _vm);

    // IGridSource
    virtual int GetRowCount();
};

class VMStackVariableGridSource : public VMVariableGridSource {
private:
    int frameIndex;
    bool FrameIsValid();
    vmUserFuncStackFrame& Frame();
    vmUserFunc& Func();
    vmUserFuncPrototype& Prototype();

protected:
    virtual void GetVar(int variableIndex, int& dataIndex, vmValType& type);
    virtual std::string GetVarName(int variableIndex);

public:
    VMStackVariableGridSource(
            TomVM& _vm,
            int _frameIndex);

    // IGridSource
    virtual int GetRowCount();
};

////////////////////////////////////////////////////////////////////////////////
//  VMStackValueGridSource
//
/// Grid source for the VM expression evaluation stack
class VMStackValueGridSource : public VMValueGridSource {
protected:
    virtual vmValue& GetValueObject(int index);

public:
    VMStackValueGridSource(TomVM& _vm);
    
    // IGridSource
    virtual int GetRowCount();
    virtual int GetFirstRowIndex();
};

////////////////////////////////////////////////////////////////////////////////
//  VMStringStoreGridSource
//
/// Grid source for VM string stores
class VMStringStoreGridSource : public IGridSource {
private:
    vmStore<vmString>& store;
public:
    VMStringStoreGridSource(vmStore<vmString>& _store);

    // IGridSource
    virtual int GetRowCount();
    virtual int GetFirstRowIndex();
    virtual std::string GetValue(int row, int col);
};

#endif
