//---------------------------------------------------------------------------
/*  Created 8-Mar-06: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

    Adapts a TomVM to a IDLL_Basic4GL_Runtime.
*/

#ifndef TomVMDLLAdapterH
#define TomVMDLLAdapterH
//---------------------------------------------------------------------------

#include "TomVM.h"
#include "Basic4GLDLLInterface.h"
#include "PluginDataStructures.h"

////////////////////////////////////////////////////////////////////////////////
//  TomVMDLLAdapter
//
/// Adapts the IDLL_Basic4GL_Runtime interface to a TomVM virtual machine.
class TomVMDLLAdapter : public IDLL_Basic4GL_Runtime {
    TomVM&  vm;
    PluginStructureManager& structureManager;

    // Working
    PluginDataType currentType;

    void ReadIntArray(
        int dataIndex,
        int *dest,
        int dimensionCount,
        int *dimensions);

	void ReadFloatArray(
		int dataIndex,
		float *dest,
		int dimensionCount,
		int *dimensions);

	void WriteIntArray(
        int dataIndex,
        int *src,
        int dimensionCount,
        int *dimensions);

	void WriteFloatArray(
		int dataIndex,
		float *src,
		int dimensionCount,
		int *dimensions);

    int CreateTempArray(
        vmValType type,
        int dimensionCount,
        int *dimensions);

    int CreateTempData(vmValType& type);
    void FixCurrentType();

    // Convert Basic data to C data
    void CArrayFromBasicArray(const PluginDataType& type, char*& cData, int& basicDataIndex);
    void CStructureFromBasicStructure(const PluginDataType& type, char*& cData, int& basicDataIndex);
    void CValueFromBasicValue(const PluginDataType& type, char*& cData, vmValue& value);
    void CValueFromBasicValue(const PluginDataType& type, char*& cData, int& basicDataIndex);
    void CDataFromBasicData(const PluginDataType& type, char*& cData, int& basicDataIndex);

    // Convert C data to basic data
    void BasicArrayFromCArray(const PluginDataType& type, int& basicDataIndex, char*& cData);
    void BasicStructureFromCStructure(const PluginDataType& type, int& basicDataIndex, char*& cData);
    void BasicValueFromCValue(const PluginDataType& type, vmValue& value, char*& cData);
    void BasicValueFromCValue(const PluginDataType& type, int& basicDataIndex, char*& cData);
    void BasicDataFromCData(const PluginDataType& type, int& basicDataIndex, char*& cData);

public:
    TomVMDLLAdapter(TomVM& _vm, PluginStructureManager& _structureManager) :
        vm(_vm),
        structureManager(_structureManager) { ; }

    // IDLL_Basic4GL_Runtime interface

    // Regular parameters
    virtual int             DLLFUNC GetIntParam(int index);
    virtual float           DLLFUNC GetFloatParam(int index);
    virtual const char*     DLLFUNC GetStringParam(int index);

    // Regular return values
    virtual void DLLFUNC SetIntResult(int result);
    virtual void DLLFUNC SetFloatResult(float result);
    virtual void DLLFUNC SetStringResult(const char *result);

    // Array parameters
    virtual void DLLFUNC GetIntArrayParam(
        int index,
		int *array,
        int dimensions,
        int dimension0Size,
        ...);
    virtual void DLLFUNC GetFloatArrayParam(
        int index,
		float *array,
        int dimensions,
        int dimension0Size,
        ...);
	virtual int DLLFUNC GetArrayParamDimension(
		int index,
		int dimension);

    // Array results
    virtual void DLLFUNC SetIntArrayResult(
		int *array,
        int dimensions,
        int dimension0Size,
        ...);
    virtual void DLLFUNC SetFloatArrayResult(
		float *array,
        int dimensions,
        int dimension0Size,
        ...);

    // General purpose data access routines
	virtual void DLLFUNC SetType(
		int baseType);
	virtual void DLLFUNC SetStringType(
		int size);
	virtual void DLLFUNC ModTypeArray(
		int dimensions,
		int dimension0Size,
		...);
	virtual void DLLFUNC ModTypeReference();
	virtual void DLLFUNC GetParam(
		int index,
		void *dst);
	virtual void DLLFUNC SetParam(
		int index,
		void *src);
	virtual void DLLFUNC SetReturnValue(
		void *src);

    // Direct data access
	virtual int DLLFUNC DirectGetInt(int memAddr);
	virtual float DLLFUNC DirectGetFloat(int memAddr);
	virtual char *DLLFUNC DirectGetString(int memAddr, char* str, int maxLen);
	virtual void DLLFUNC DirectSetInt(int memAddr, int value);
	virtual void DLLFUNC DirectSetFloat(int memAddr, float value);
	virtual void DLLFUNC DirectSetString(int memAddr, const char *str);

	// Long running functions
	virtual void DLLFUNC BeginLongRunningFn(IDLL_Basic4GL_LongRunningFunction* handler);
	virtual void DLLFUNC EndLongRunningFn();

	// Runtime error reporting
	void DLLFUNC FunctionError(const char* text) override;

	// Conditional timesharing break

	void DLLFUNC TimeshareBreakRequired() override;
};

#endif
