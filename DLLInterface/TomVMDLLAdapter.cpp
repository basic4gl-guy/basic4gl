//---------------------------------------------------------------------------


#pragma hdrstop

#include "TomVMDLLAdapter.h"
#include <stdint.h>
#include <cstdarg>
using namespace std;

//---------------------------------------------------------------------------

#pragma package(smart_init)

////////////////////////////////////////////////////////////////////////////////
//  TomVMDLLAdapter

void TomVMDLLAdapter::ReadIntArray(
    int dataIndex,
    int *dest,
    int dimensionCount,
    int *dimensions) {

    // First two entries are the element count and size
    int elementCount = vm.Data().Data()[dataIndex].IntVal();
    int elementSize = vm.Data().Data()[dataIndex + 1].IntVal();

    // Move to start of array data
    dataIndex += 2;

    // Recursively extract array data
    if (dimensionCount > 1) {

        // Calculate # of elements in one entry in this dimension
        int destElements = 1;
        for (int d = 1; d < dimensionCount; d++)
            destElements *= dimensions[d];

        // Recursively copy sub dimensions
        for (int i = 0; i < dimensions[0]; i++) {
            if (i < elementCount)
                ReadIntArray(
                    dataIndex + i * elementSize,
                    &dest[i * destElements],
                    dimensionCount - 1,
                    &dimensions[1]);
            else
                memset(&dest[i * destElements], 0, destElements * sizeof(int));
        }
    }
    else {
        // Down to lowest dimension.
        // Copy elements
        for (int i = 0; i < dimensions[0]; i++)
            if (i < elementCount)
                dest[i] = vm.Data().Data()[dataIndex + i].IntVal();
            else
                dest[i] = 0;
    }
}

void TomVMDLLAdapter::ReadFloatArray(
	int dataIndex,
	float *dest,
	int dimensionCount,
	int *dimensions) {

	// First two entries are the element count and size
	int elementCount = vm.Data().Data()[dataIndex].IntVal();
	int elementSize = vm.Data().Data()[dataIndex + 1].IntVal();

	// Move to start of array data
	dataIndex += 2;

	// Recursively extract array data
	if (dimensionCount > 1) {

		// Calculate # of elements in one entry in this dimension
		int destElements = 1;
		for (int d = 1; d < dimensionCount; d++)
			destElements *= dimensions[d];

		// Recursively copy sub dimensions
		for (int i = 0; i < dimensions[0]; i++) {
			if (i < elementCount)
				ReadFloatArray(
				dataIndex + i * elementSize,
				&dest[i * destElements],
				dimensionCount - 1,
				&dimensions[1]);
			else
				memset(&dest[i * destElements], 0, destElements * sizeof(int));
		}
	}
	else {
		// Down to lowest dimension.
		// Copy elements
		for (int i = 0; i < dimensions[0]; i++)
			if (i < elementCount)
				dest[i] = vm.Data().Data()[dataIndex + i].RealVal();
			else
				dest[i] = 0;
	}
}

void TomVMDLLAdapter::WriteIntArray(
    int dataIndex,
    int *src,
    int dimensionCount,
    int *dimensions) {

    // First two entries are the element count and size
    int elementCount = vm.Data().Data()[dataIndex].IntVal();
    int elementSize = vm.Data().Data()[dataIndex + 1].IntVal();
    int min = elementCount < dimensions[0] ? elementCount : dimensions[0];

    // Move to start of array data
    dataIndex += 2;

    // Recursively copy data to array
    if (dimensionCount > 1) {

        // Calculate # of elements in one entry in this dimension
        int destElements = 1;
        for (int d = 1; d < dimensionCount; d++)
            destElements *= dimensions[d];

        for (int i = 0; i < min; i++)
            WriteIntArray(
                dataIndex + i * elementSize,
                &src[i * destElements],
                dimensionCount - 1,
                &dimensions[1]);
    }
    else {
        // Down to lowest dimension.
        // Can copy elements
        for (int i = 0; i < min; i++)
            vm.Data().Data()[dataIndex + i].IntVal() = src[i];
    }
}

void TomVMDLLAdapter::WriteFloatArray(
	int dataIndex,
	float *src,
	int dimensionCount,
	int *dimensions) {

	// First two entries are the element count and size
	int elementCount = vm.Data().Data()[dataIndex].IntVal();
	int elementSize = vm.Data().Data()[dataIndex + 1].IntVal();
	int min = elementCount < dimensions[0] ? elementCount : dimensions[0];

	// Move to start of array data
	dataIndex += 2;

	// Recursively copy data to array
	if (dimensionCount > 1) {

		// Calculate # of elements in one entry in this dimension
		int destElements = 1;
		for (int d = 1; d < dimensionCount; d++)
			destElements *= dimensions[d];

		for (int i = 0; i < min; i++)
			WriteFloatArray(
			dataIndex + i * elementSize,
			&src[i * destElements],
			dimensionCount - 1,
			&dimensions[1]);
	}
	else {
		// Down to lowest dimension.
		// Can copy elements
		for (int i = 0; i < min; i++)
			vm.Data().Data()[dataIndex + i].RealVal() = src[i];
	}
}

int TomVMDLLAdapter::CreateTempArray(
    vmValType type,
    int dimensionCount,
    int *dimensions) {

    // Create Basic4GL type to define array
    for (int d = 0; d < dimensionCount; d++)
        type << dimensions[d];

    // Allocate temporary array, and initialise it
    return CreateTempData(type);
}

int TomVMDLLAdapter::CreateTempData(vmValType& type) {

    // Allocate temporary data
    int dataIndex = vm.Data().AllocateTemp(vm.DataTypes().DataSize(type), true);

    // Initialize it
    vm.Data().InitData(dataIndex, type, vm.DataTypes());

    // Return data index
    return dataIndex;
}

int     DLLFUNC TomVMDLLAdapter::GetIntParam(int index) {
    return vm.GetIntParam(index);
}

float   DLLFUNC TomVMDLLAdapter::GetFloatParam(int index) {
    return vm.GetRealParam(index);
}

const char*   DLLFUNC TomVMDLLAdapter::GetStringParam(int index) {
    return vm.GetStringParam(index).c_str();
}

void DLLFUNC TomVMDLLAdapter::SetIntResult(int result) {
    vm.Reg().IntVal() = result;
}

void DLLFUNC TomVMDLLAdapter::SetFloatResult(float result) {
    vm.Reg().RealVal() = result;
}

void DLLFUNC TomVMDLLAdapter::SetStringResult(const char *result) {
    vm.RegString() = (string) result;
}

void DLLFUNC TomVMDLLAdapter::GetIntArrayParam(
        int index,
		int *array,
        int dimensions,
        int dimension0Size,
        ...) {
	assert(dimensions < VM_MAXDIMENSIONS);
	int dimensionArray[VM_MAXDIMENSIONS];
	if (dimensions > 0)
	{
		dimensionArray[0] = dimension0Size;
		va_list vl;
		va_start(vl, dimension0Size);
		for (int i = 1; i < dimensions; i++)
			dimensionArray[i] = va_arg(vl, int);
		va_end(vl);
	}

    ReadIntArray(
        vm.GetIntParam(index),
        array,
        dimensions,
        dimensionArray);
}

void DLLFUNC TomVMDLLAdapter::GetFloatArrayParam(
        int index,
		float *array,
        int dimensions,
        int dimension0Size,
        ...) {
	assert(dimensions < VM_MAXDIMENSIONS);
	int dimensionArray[VM_MAXDIMENSIONS];
	if (dimensions > 0)
	{
		dimensionArray[0] = dimension0Size;
		va_list vl;
		va_start(vl, dimension0Size);
		for (int i = 1; i < dimensions; i++)
			dimensionArray[i] = va_arg(vl, int);
		va_end(vl);
	}

	ReadFloatArray(
        vm.GetIntParam(index),
        array,
        dimensions,
		dimensionArray);
}

int DLLFUNC TomVMDLLAdapter::GetArrayParamDimension(
		int index,
		int dimension) {
    return ArrayDimensionSize(vm.Data(), vm.GetIntParam(index), dimension);
}

void DLLFUNC TomVMDLLAdapter::SetIntArrayResult(
		int *array,
        int dimensions,
        int dimension0Size,
        ...) {
	assert(dimensions < VM_MAXDIMENSIONS);
	int dimensionArray[VM_MAXDIMENSIONS];
	if (dimensions > 0)
	{
		dimensionArray[0] = dimension0Size;
		va_list vl;
		va_start(vl, dimension0Size);
		for (int i = 1; i < dimensions; i++)
			dimensionArray[i] = va_arg(vl, int);
		va_end(vl);
	}

    // Allocate temporary array
    int dataIndex = CreateTempArray(VTP_INT, dimensions, dimensionArray);

    // Fill it with data
    WriteIntArray(dataIndex, array, dimensions, dimensionArray);

    // Assign array to virtual machine register
    vm.Reg().IntVal() = dataIndex;
}

void DLLFUNC TomVMDLLAdapter::SetFloatArrayResult(
		float *array,
        int dimensions,
        int dimension0Size,
        ...) {
	assert(dimensions < VM_MAXDIMENSIONS);
	int dimensionArray[VM_MAXDIMENSIONS];
	if (dimensions > 0)
	{
		dimensionArray[0] = dimension0Size;
		va_list vl;
		va_start(vl, dimension0Size);
		for (int i = 1; i < dimensions; i++)
			dimensionArray[i] = va_arg(vl, int);
		va_end(vl);
	}

    // Allocate temporary array
    int dataIndex = CreateTempArray(VTP_INT, dimensions, dimensionArray);

    // Fill it with data
    WriteFloatArray(dataIndex, array, dimensions, dimensionArray);

    // Assign array to virtual machine register
    vm.Reg().IntVal() = dataIndex;
}

int DLLFUNC TomVMDLLAdapter::DirectGetInt(int memAddr) {
    assert(memAddr > 0);
    assert(memAddr < vm.Data().Data().size());
    return vm.Data().Data()[memAddr].IntVal();
}

float DLLFUNC TomVMDLLAdapter::DirectGetFloat(int memAddr) {
    assert(memAddr > 0);
    assert(memAddr < vm.Data().Data().size());
    return vm.Data().Data()[memAddr].RealVal();
}

char *DLLFUNC TomVMDLLAdapter::DirectGetString(int memAddr, char* str, int maxLen) {
    assert(memAddr > 0);
    assert(memAddr < vm.Data().Data().size());
    int index = vm.Data().Data()[memAddr].IntVal();
    string s = vm.GetString(index);

    // Copy string to buffer
    int len = s.length();
    if (len > maxLen - 1)
        len = maxLen - 1;
    for (int i = 0; i < len; i++)
        str[i] = s.c_str()[i];
    str[len] = 0;

    return str;
}

void DLLFUNC TomVMDLLAdapter::DirectSetInt(int memAddr, int value) {
    assert(memAddr > 0);
    assert(memAddr < vm.Data().Data().size());
    vm.Data().Data()[memAddr].IntVal() = value;
}

void DLLFUNC TomVMDLLAdapter::DirectSetFloat(int memAddr, float value) {
    assert(memAddr > 0);
    assert(memAddr < vm.Data().Data().size());
    vm.Data().Data()[memAddr].RealVal() = value;
}

void DLLFUNC TomVMDLLAdapter::DirectSetString(int memAddr, const char *str) {
    assert(memAddr > 0);
    assert(memAddr < vm.Data().Data().size());
    vmValue& dest = vm.Data().Data()[memAddr];

    // Allocate string space if necessary
    if (dest.IntVal() == 0)
        dest.IntVal() = vm.AllocString();

    // Store string
    vm.GetString(dest.IntVal()) = str;
}

void DLLFUNC TomVMDLLAdapter::BeginLongRunningFn(IDLL_Basic4GL_LongRunningFunction* handler)
{
	vm.BeginLongRunningFn(handler);
}

void DLLFUNC TomVMDLLAdapter::EndLongRunningFn()
{
	vm.EndLongRunningFunction();
}

void DLLFUNC TomVMDLLAdapter::FunctionError(const char* text)
{
	vm.FunctionError(text);
}

void TomVMDLLAdapter::TimeshareBreakRequired()
{
	vm.TimeshareBreakRequired();
}

void TomVMDLLAdapter::CArrayFromBasicArray(
        const PluginDataType& type,
        char*& cData,
        int& basicDataIndex) {

    assert(type.pointerLevel == 0);
    assert(type.arrayLevel > 0);

    // Extract element info
    int basicCount = vm.Data().Data()[basicDataIndex].IntVal();
    int size = vm.Data().Data()[basicDataIndex + 1].IntVal();
    int arrayStart = basicDataIndex + 2;

    // If greater than # requested, use the lesser (so as not to overflow
    // target C array).
    int count = basicCount;
    if (count > type.arrayDims[type.arrayLevel - 1])
        count = type.arrayDims[type.arrayLevel - 1];

    // Get element type
    PluginDataType elementType(type);
    elementType.MakeIntoElementType();

    // Convert each element
    for (int i = 0; i < count; i++) {
        int elementDataIndex = arrayStart + i * size;
        CDataFromBasicData(elementType, cData, elementDataIndex);
    }

    // Advance data index
    basicDataIndex = arrayStart + basicCount * size;
}

void TomVMDLLAdapter::CStructureFromBasicStructure(
        const PluginDataType& type,
        char*& cData,
        int& basicDataIndex) {

    // Find plugin structure
    PluginStructure *structure = structureManager.GetStructure(type.baseType);

    // Convert each field
    for (int i = 0; i < structure->FieldCount(); i++)
        CDataFromBasicData(structure->GetField(i).dataType, cData, basicDataIndex);
}

void TomVMDLLAdapter::CValueFromBasicValue(
        const PluginDataType& type,
        char*& cData,
        vmValue& value) {

    assert(type.pointerLevel == 0);
    assert(type.arrayLevel == 0);
    assert(type.baseType < 0);

    int strIndex, len;
    string str;
    switch (type.baseType) {
        case DLL_BASIC4GL_EXT_BYTE:
            // Copy Basic4GL int to C byte
            *((byte*)cData) = value.IntVal();
            cData += sizeof(byte);
            break;

        case DLL_BASIC4GL_EXT_WORD:
            // Copy Basic4GL int to C word
            *((int16_t*)cData) = value.IntVal();
            cData += sizeof(int16_t);
            break;

        case DLL_BASIC4GL_EXT_INT:
            // Copy Basic4GL int to C int
            *((int*)cData) = value.IntVal();
            cData += sizeof(int);
            break;

        case DLL_BASIC4GL_EXT_INT64:
            // Copy Basic4GL int to C 64bit integer
            *((int64_t*)cData) = value.IntVal();
            cData += sizeof(int64_t);
            break;

        case DLL_BASIC4GL_EXT_FLOAT:
            // Copy Basic4GL real to C float
            *((float*)cData) = value.RealVal();
            cData += sizeof(float);
            break;

        case DLL_BASIC4GL_EXT_DOUBLE:
            // Copy Basic4GL real to C double
            *((double*)cData) = value.RealVal();
            cData += sizeof(double);
            break;

        case DLL_BASIC4GL_EXT_STRING:

            // Fetch string
            strIndex = value.IntVal();
            str = vm.GetString(strIndex);

            // Copy characters
            len = str.length();
            if (len > type.stringSize - 1)
                len = type.stringSize - 1;
            memcpy(cData, str.c_str(), len);
            cData[len] = 0;                 // (Terminate string)
            cData += type.stringSize;
            break;
            
        default:
            assert(false);
    }
}

void TomVMDLLAdapter::CValueFromBasicValue(
        const PluginDataType& type,
        char*& cData,
        int& basicDataIndex) {

    assert(type.pointerLevel == 0);
    assert(type.arrayLevel == 0);
    assert(type.baseType < 0);

    if (type.baseType == DLL_BASIC4GL_EXT_PADDING)
        cData += type.stringSize;
    else {
        CValueFromBasicValue(type, cData, vm.Data().Data()[basicDataIndex]);
        basicDataIndex++;
    }
}

void TomVMDLLAdapter::CDataFromBasicData(
        const PluginDataType& type,
        char*& cData,
        int& basicDataIndex) {

    if (type.pointerLevel > 0) {

        // Note: We cannot convert pointers, so we just store NULL.
        *((void**)cData) = NULL;
        cData += sizeof(void*);
        basicDataIndex++;
    }
    else if (type.arrayLevel > 0) {

        // Convert array
        CArrayFromBasicArray(type, cData, basicDataIndex);

    }
    else if (type.baseType > 0) {

        // Convert structure
        CStructureFromBasicStructure(type, cData, basicDataIndex);

    }
    else {
    
        // Convert value
        CValueFromBasicValue(type, cData, basicDataIndex);
    }
}

void TomVMDLLAdapter::BasicArrayFromCArray(
        const PluginDataType& type, int& basicDataIndex, char*& cData) {

    assert(type.pointerLevel == 0);
    assert(type.arrayLevel > 0);

    // Get array size
    int basicCount = vm.Data().Data()[basicDataIndex].IntVal();
    int size = vm.Data().Data()[basicDataIndex + 1].IntVal();
    int arrayStart = basicDataIndex + 2;

	// If greater than # requested, use the lesser (so as not to overrun
	// source C array).
	int count = basicCount;
	if (count > type.arrayDims[type.arrayLevel - 1])
		count = type.arrayDims[type.arrayLevel - 1];

	// Get element type
    PluginDataType elementType(type);
    elementType.MakeIntoElementType();

    // Convert each element
    for (int i = 0; i < count; i++) {
        int elementDataIndex = arrayStart + i * size;
        BasicDataFromCData(elementType, elementDataIndex, cData);
    }

    // Advance data index
    basicDataIndex = arrayStart + basicCount * size;
}

void TomVMDLLAdapter::BasicStructureFromCStructure(
        const PluginDataType& type, int& basicDataIndex, char*& cData) {

    // Find plugin structure
    PluginStructure *structure = structureManager.GetStructure(type.baseType);

    // Convert each field
    for (int i = 0; i < structure->FieldCount(); i++)
        BasicDataFromCData(structure->GetField(i).dataType, basicDataIndex, cData);
}

void TomVMDLLAdapter::BasicValueFromCValue(
        const PluginDataType& type,
        vmValue& value,
        char*& cData) {

    assert(type.pointerLevel == 0);
    assert(type.arrayLevel == 0);
    assert(type.baseType < 0);

    int strIndex;
    switch (type.baseType) {

        case DLL_BASIC4GL_EXT_BYTE:
            // Copy C byte to Basic4GL int
            value.IntVal() = *((byte*)cData);
            cData += sizeof(byte);
            break;

        case DLL_BASIC4GL_EXT_WORD:
            // Copy Basic4GL int to C word
            value.IntVal() = *((int16_t*)cData);
            cData += sizeof(int16_t);
            break;

        case DLL_BASIC4GL_EXT_INT:
            // Copy Basic4GL int to C int
            value.IntVal() = *((int*)cData);
            cData += sizeof(int);
            break;

        case DLL_BASIC4GL_EXT_INT64:
            // Copy Basic4GL int to C 64bit integer
            value.IntVal() = *((int64_t*)cData);
            cData += sizeof(int64_t);
            break;

        case DLL_BASIC4GL_EXT_FLOAT:
            // Copy Basic4GL real to C float
            value.RealVal() = *((float*)cData);
            cData += sizeof(float);
            break;

        case DLL_BASIC4GL_EXT_DOUBLE:
            // Copy Basic4GL real to C double
            value.RealVal() = *((double*)cData);
            cData += sizeof(double);
            break;

        case DLL_BASIC4GL_EXT_STRING:

            // Allocate string handle (if necessary)
            strIndex = value.IntVal();
            if (strIndex == 0) {
                strIndex = vm.AllocString();
                value.IntVal() = strIndex;
            }

            // Copy string to Basic4GL
            vm.GetString(strIndex) = cData;
            cData += type.stringSize;
            break;

        default:
            assert(false);
    }
}

void TomVMDLLAdapter::BasicValueFromCValue(
        const PluginDataType& type, int& basicDataIndex, char*& cData) {

    assert(type.pointerLevel == 0);
    assert(type.arrayLevel == 0);
    assert(type.baseType < 0);

    if (type.baseType == DLL_BASIC4GL_EXT_PADDING)
        cData += type.stringSize;
    else {
        BasicValueFromCValue(type, vm.Data().Data()[basicDataIndex], cData);
        basicDataIndex++;
    }
}

void TomVMDLLAdapter::BasicDataFromCData(
    const PluginDataType& type,
    int& basicDataIndex,
    char*& cData) {

    if (type.pointerLevel > 0) {

        // Note: We cannot convert pointers, so we just store NULL.
        vm.Data().Data()[basicDataIndex].IntVal() = 0;
        cData += sizeof(void*);
        basicDataIndex++;
    }
    else if (type.arrayLevel > 0) {

        // Convert array
        BasicArrayFromCArray(type, basicDataIndex, cData);

    }
    else if (type.baseType > 0) {

        // Convert structure
        BasicStructureFromCStructure(type, basicDataIndex, cData);

    }
    else {

        // Convert value
        BasicValueFromCValue(type, basicDataIndex, cData);
    }
}

void DLLFUNC TomVMDLLAdapter::SetType(
		int baseType) {
    currentType = PluginDataType::SimpleType(baseType);
}

void DLLFUNC TomVMDLLAdapter::SetStringType(
		int size) {
    currentType = PluginDataType::String(size);
}

void DLLFUNC TomVMDLLAdapter::ModTypeArray(
		int dimensions,
		int dimension0Size,
		...) {
	assert(dimensions < VM_MAXDIMENSIONS);
	int dimensionArray[VM_MAXDIMENSIONS];
	if (dimensions > 0)
	{
		dimensionArray[0] = dimension0Size;
		va_list vl;
		va_start(vl, dimension0Size);
		for (int i = 1; i < dimensions; i++)
			dimensionArray[i] = va_arg(vl, int);
		va_end(vl);
	}

	currentType.arrayLevel = dimensions;
    int *dimensionSize = dimensionArray;
    for (int i = dimensions - 1; i >= 0; i--) {
        currentType.arrayDims[i] = *dimensionSize;
        dimensionSize++;
    }
}

void DLLFUNC TomVMDLLAdapter::ModTypeReference() {
    if (!currentType.byReference) {
        currentType.pointerLevel++;
        currentType.byReference = true;
    }
}

void TomVMDLLAdapter::FixCurrentType() {

    // Convert structures and arrays to by-reference
    if (currentType.pointerLevel == 0 &&
            (currentType.baseType > 0 || currentType.arrayLevel > 0)) {
        currentType.pointerLevel = 1;
        currentType.byReference = true;
    }
}

void DLLFUNC TomVMDLLAdapter::GetParam(
		int index,
		void *dst) {

    // Ensure data type is complete
    FixCurrentType();

    // Simple type?
    if (currentType.arrayLevel == 0 &&
            currentType.pointerLevel == 0 &&
            currentType.baseType < 0) {
        CValueFromBasicValue(currentType, (char*&)dst, vm.GetParam(index));
    }
    else {

        // Dereference
        int dataIndex = vm.GetParam(index).IntVal();

        // Get dereferenced type
        PluginDataType derefType = currentType;
        derefType.Deref();

        // Convert data
        CDataFromBasicData(derefType, (char*&)dst, dataIndex);
    }
}

void DLLFUNC TomVMDLLAdapter::SetParam(
		int index,
		void *src) {

    // Ensure data type is complete
    FixCurrentType();

    // Simple type
    if (currentType.arrayLevel == 0 &&
            currentType.pointerLevel == 0 &&
            currentType.baseType < 0) {
        BasicValueFromCValue(currentType, vm.GetParam(index), (char*&)src);
    }
    else {

        // Dereference
        int dataIndex = vm.GetParam(index).IntVal();

        // Get dereferenced type
        PluginDataType derefType = currentType;
        derefType.Deref();

        // Convert data
        BasicDataFromCData(derefType, dataIndex, (char*&)src);
    }
}

void DLLFUNC TomVMDLLAdapter::SetReturnValue(
		void *src) {

    // Ensure data type is complete
    FixCurrentType();

    // Simple type
    if (currentType.arrayLevel == 0 &&
            currentType.pointerLevel == 0 &&
            currentType.baseType < 0) {

        // Special case! String values are written to RegString
        if (currentType.baseType == DLL_BASIC4GL_EXT_STRING)
            vm.RegString() = (char*)src;
        else
            BasicValueFromCValue(currentType, vm.Reg(), (char*&)src);
    }
    else {

        // Non simple.

        // Get dereferenced type
        PluginDataType derefType = currentType;
        derefType.Deref();

        // Allocate temporary storage space for result.
        vmValType vmType = structureManager.vmTypeFromPluginType(derefType);
        int returnDataIndex = CreateTempData(vmType);

        // Convert data and write to temp space
        int dataIndex = returnDataIndex;
        BasicDataFromCData(derefType, dataIndex, (char*&)src);

        // Return reference to result data in register
        vm.Reg().IntVal() = returnDataIndex;
    }
}

