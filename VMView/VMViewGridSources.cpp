#include "VMViewGridSources.h"
using namespace std;

////////////////////////////////////////////////////////////////////////////////
//  VMCodeGridSource

VMCodeGridSource::VMCodeGridSource(
    TomBasicCompiler& _comp,
    ISourceLineFormatter* _sourceLineFormatter)
        : comp(_comp), vm(comp.VM()), sourceLineFormatter(_sourceLineFormatter) {}

int VMCodeGridSource::GetRowCount() {
    return vm.InstructionCount();
}

int VMCodeGridSource::GetFirstRowIndex() {
    return 0;
}

string VMCodeGridSource::GetValue(int row, int col) {
    vmInstruction& i = vm.Instruction(row);
    switch(col) {
        case 0:     // OP CODE
            return vmOpCodeName((vmOpCode)i.m_opCode);
        case 1:     // Data type
            return vmBasicValTypeName((vmBasicValType)i.m_type);
        case 2:     // Data
            return GetOpCodeData(i);
        case 3:
            return sourceLineFormatter->GetSourcePos(i.m_sourceLine);
        default:
            return "";
    }
}

string VMCodeGridSource::GetOpCodeData(vmInstruction& i) {

    // Display op-code data
    // For some op-codes the data type corresponds to the index of a function
    // or variable etc. To make the code easier to interpret we try to display
    // these instead, where we can.
    switch (i.m_opCode) {
        case OP_DECLARE:
        case OP_LOAD_VAR: {
            int index = i.m_value.IntVal();
            if (vm.Variables().IndexValid(index))
                return vm.Variables().Variables()[index].m_name;
            else
                return "???";
        }
        case OP_CALL_FUNC: {
            return comp.FunctionName(i.m_value.IntVal()) + "()";
        }
        case OP_CALL_DLL: {
            unsigned int index = i.m_value.IntVal();
            return vm.Plugins().FunctionName(index >> 24, index & 0x00ffffff) + "()";
        }
        case OP_COPY:
        case OP_ALLOC: {
            return vm.DataTypes().DescribeVariable("", vm.GetStoredType(i.m_value.IntVal()));
        }
		case OP_CREATE_USER_FRAME: {
			return comp.GetUserFunctionName(i.m_value.IntVal()) + "()";
		}
		case OP_LOAD_CONST: {
			if ((vmBasicValType)i.m_type == VTP_FUNC_PTR)
			{
				if (i.m_value.IntVal() == 0) return "[NULL]";
				return comp.GetUserFunctionName(i.m_value.IntVal() - 1) + "()";
			}
		}
    }
	return vm.BasicValToString(i.m_value, (vmBasicValType)i.m_type, true);
}

////////////////////////////////////////////////////////////////////////////////
//  VMValueGridSource

VMValueGridSource::VMValueGridSource(TomVM& _vm) : vm(_vm) {}

string VMValueGridSource::GetValue(int row, int col) {
    vmValue& v = GetValueObject(row);
    int lenShort = 20, lenLong = 200;
    switch(col) {
        case 0:     return vm.ValToString(v, VTP_INT, lenShort);
        case 1:     return vm.ValToString(v, VTP_REAL, lenShort);
        case 2:     return vm.ValToString(v, VTP_STRING, lenLong);
        default:    return "";
    }
}

////////////////////////////////////////////////////////////////////////////////
//  VMDataGridSource

VMDataGridSource::VMDataGridSource(TomVM& _vm, int _start, int _end)
        : VMValueGridSource(_vm), start(_start), end(_end) {
}

vmValue& VMDataGridSource::GetValueObject(int index) {
    return vm.Data().Data()[start + index];
}

int VMDataGridSource::GetRowCount() {
    return end - start;
}

int VMDataGridSource::GetFirstRowIndex() {
    return start;
}

////////////////////////////////////////////////////////////////////////////////
//  VMVariableGridSource

VMVariableGridSource::VMVariableGridSource(TomVM& _vm) : vm(_vm) {}

int VMVariableGridSource::GetFirstRowIndex() {
    return 0;
}

string VMVariableGridSource::GetValue(int row, int col) {

    // Can't always trust row will be in range
    if (row < 0 || row >= GetRowCount())
        return "???";

    // Get variable data and name
    int dataIndex;
    vmValType type;
    GetVar(row, dataIndex, type);
    string name = GetVarName(row);

    // Return variable information
    string result = "";
    switch(col) {
        case 0:
            result = vm.DataTypes().DescribeVariable(name, type);
            break;
        case 1:
            if (dataIndex == 0) 
                result = "[UNDIMMED]";
            else
                result = vm.DataToString(dataIndex, type, 1000);
            break;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////
//  VMGlobalVariableGridSource

void VMGlobalVariableGridSource::GetVar(int variableIndex, int& dataIndex, vmValType& type) {

    // Find global variable
    vmVariable& var = vm.Variables().Variables()[variableIndex];

    // Return its info
    dataIndex = var.m_dataIndex;
    type = var.m_type;
}

string VMGlobalVariableGridSource::GetVarName(int variableIndex) {

    // Return global variable name
    return vm.Variables().Variables()[variableIndex].m_name;
}

VMGlobalVariableGridSource::VMGlobalVariableGridSource(TomVM& _vm)
        : VMVariableGridSource(_vm) {}

int VMGlobalVariableGridSource::GetRowCount() {

    // Return # of global variables
    return vm.Variables().Variables().size();
}

////////////////////////////////////////////////////////////////////////////////
//  VMStackVariableGridSource

bool VMStackVariableGridSource::FrameIsValid() {
    return frameIndex >= 0 && frameIndex < vm.UserCallStack().size();
}

vmUserFuncStackFrame& VMStackVariableGridSource::Frame() {
    assert(FrameIsValid());
    return vm.UserCallStack()[frameIndex];
}

vmUserFunc& VMStackVariableGridSource::Func() {
    return vm.UserFunctions()[Frame().userFuncIndex];
}

vmUserFuncPrototype& VMStackVariableGridSource::Prototype() {
    return vm.UserFunctionPrototypes()[Func().prototypeIndex];    
}

void VMStackVariableGridSource::GetVar(int variableIndex, int& dataIndex, vmValType& type) {
    if (FrameIsValid()) {
        dataIndex = Frame().localVarDataOffsets[variableIndex];
        type = Prototype().localVarTypes[variableIndex];
    }
    else {
        dataIndex = 0;
        type = VTP_UNDEFINED;
    }
}

string VMStackVariableGridSource::GetVarName(int variableIndex) {
    if (FrameIsValid())
        return Prototype().GetLocalVarName(variableIndex);
    else
        return "";
}

VMStackVariableGridSource::VMStackVariableGridSource(
        TomVM& _vm,
        int _frameIndex)
    : VMVariableGridSource(_vm), frameIndex(_frameIndex) {}

int VMStackVariableGridSource::GetRowCount() {
    if (FrameIsValid())
        return Prototype().localVarTypes.size();
    else
        return 0;
}

////////////////////////////////////////////////////////////////////////////////
//  VMStackValueGridSource

vmValue& VMStackValueGridSource::GetValueObject(int index) {
    return vm.Stack()[index];
}

VMStackValueGridSource::VMStackValueGridSource(TomVM& _vm)
    : VMValueGridSource(_vm) {}

int VMStackValueGridSource::GetRowCount() {
    return vm.Stack().Size();
}

int VMStackValueGridSource::GetFirstRowIndex() {
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//  VMStringStoreGridSource

VMStringStoreGridSource::VMStringStoreGridSource(vmStore<vmString>& _store)
    : store(_store) {}

int VMStringStoreGridSource::GetRowCount() {
    return store.Array().size();
}

int VMStringStoreGridSource::GetFirstRowIndex() {
    return 0;
}

string VMStringStoreGridSource::GetValue(int row, int col) {
    if (store.ValAllocated()[row])
        return store.Value(row);
    else
        return "[NOT ALLOCATED]";
}

