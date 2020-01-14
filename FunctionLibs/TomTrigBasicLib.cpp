//---------------------------------------------------------------------------
/*  Created 22-Oct-2003: Thomas Mulgrew
    Copyright (C) Thomas Mulgrew

    Matrix and vector routines.
*/                                                                           


#pragma hdrstop

#include "TomTrigBasicLib.h"

//---------------------------------------------------------------------------

#ifndef _MSC_VER
#include <mem.h>
#pragma package(smart_init)
#endif

/*int _matherr(struct _exception *e)
{
    return 0;
}*/

////////////////////////////////////////////////////////////////////////////////
// Basic matrix creation functions
vmReal matrix [16];

inline void ReturnMatrix (TomVM& vm) {
    vm.Reg ().IntVal () = FillTempRealArray2D (vm.Data (), vm.DataTypes (), 4, 4, matrix);
}

////////////////////////////////////////////////////////////////////////////////
// Read vector/matrix
vmReal v1 [4], v2 [4], m1 [16], m2 [16];

int ReadVec (TomVM& vm, int index, vmReal *v) {
    assert (v != NULL);

    // Read a 3D vector.
    // This can be a 2, 3 or 4 element vector, but will always be returned as a 4
    // element vector. (z = 0 & w = 1 if not specified.)
    int size = ArrayDimensionSize (vm.Data (), index, 0);
    if (size < 2 || size > 4) {
        vm.FunctionError ("Vector must be 2, 3 or 4 element vector");
        return -1;                  // -1 = error
    }

    // Read in vector and convert to 4 element format
    v [2] = 0;
    v [3] = 1;
    ReadArray (vm.Data (), index, vmValType (VTP_REAL, 1, 1, true), v, size);

    // Return original size
    return size;
}

bool ReadMatrix (TomVM& vm, int index, vmReal *m) {
    assert (m != NULL);

    // Read 3D matrix.
    // Matrix must be 4x4
    if (ArrayDimensionSize (vm.Data (), index, 0) != 4
    ||  ArrayDimensionSize (vm.Data (), index, 1) != 4) {
        vm.FunctionError ("Matrix must be a 4x4 matrix (e.g 'dim matrix#(3)(3)' )");
        return false;
    }

    // Read in matrix
    ReadArray (vm.Data (), index, vmValType (VTP_REAL, 2, 1, true), m, 16);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Function wrappers

void WrapVec4 (TomVM& vm) {
    vmReal vec4 [4] = { vm.GetRealParam (4), vm.GetRealParam (3), vm.GetRealParam (2), vm.GetRealParam (1) };
    vm.Reg ().IntVal () = FillTempRealArray (vm.Data (), vm.DataTypes (), 4, vec4);
}
void WrapVec3 (TomVM& vm) {
    vmReal vec3 [3] = { vm.GetRealParam (3), vm.GetRealParam (2), vm.GetRealParam (1) };
    vm.Reg ().IntVal () = FillTempRealArray (vm.Data (), vm.DataTypes (), 3, vec3);
}
void WrapVec2 (TomVM& vm) {
    vmReal vec2 [2] = { vm.GetRealParam (2), vm.GetRealParam (1) };
    vm.Reg ().IntVal () = FillTempRealArray (vm.Data (), vm.DataTypes (), 2, vec2);
}
void WrapMatrixZero (TomVM& vm) {
    ClearMatrix ();
    ReturnMatrix (vm);
}
void WrapMatrixIdentity (TomVM& vm) {
    Identity ();
    ReturnMatrix (vm);
}
void WrapMatrixScale (TomVM& vm) {
    Scale (vm.GetRealParam(1));
    ReturnMatrix (vm);
}
void WrapMatrixScale_2 (TomVM& vm) {
    Scale (vm.GetRealParam (3), vm.GetRealParam (2), vm.GetRealParam (1));
    ReturnMatrix (vm);
}
void WrapMatrixTranslate (TomVM& vm) {
    Translate (vm.GetRealParam (3), vm.GetRealParam (2), vm.GetRealParam (1));
    ReturnMatrix (vm);
}
void WrapMatrixRotateX (TomVM& vm) { RotateX (vm.GetRealParam (1)); ReturnMatrix (vm); }
void WrapMatrixRotateY (TomVM& vm) { RotateY (vm.GetRealParam (1)); ReturnMatrix (vm); }
void WrapMatrixRotateZ (TomVM& vm) { RotateZ (vm.GetRealParam (1)); ReturnMatrix (vm); }
void WrapMatrixRotate (TomVM& vm) {
    if (ReadVec (vm, vm.GetIntParam(1), v1) < 0)
        return;
    RotateAxis(vm.GetRealParam(2), v1);
    ReturnMatrix(vm);
}
void WrapMatrixBasis (TomVM& vm) {
    ClearMatrix ();
    matrix [15] = 1;
    ReadArray (vm.Data (), vm.GetIntParam (3), vmValType (VTP_REAL, 1, 1, true), &matrix [0], 4);
    ReadArray (vm.Data (), vm.GetIntParam (2), vmValType (VTP_REAL, 1, 1, true), &matrix [4], 4);
    ReadArray (vm.Data (), vm.GetIntParam (1), vmValType (VTP_REAL, 1, 1, true), &matrix [8], 4);
    ReturnMatrix (vm);
}
void WrapMatrixCrossProduct (TomVM& vm) {
    if (ReadVec (vm, vm.GetIntParam (1), v1) < 0)
        return;
    CrossProduct (v1);
    ReturnMatrix (vm);
}
void WrapCross (TomVM& vm) {

    // Fetch vectors
    int s1 = ReadVec (vm, vm.GetIntParam (2), v1),
        s2 = ReadVec (vm, vm.GetIntParam (1), v2);
    if (s1 < 0 || s2 < 0)
        return;

    // Calculate cross product vector
    vmReal result [4];
    CrossProduct (v1, v2, result);

    // Return resulting vector
    // (Vector will be the same length as the first source vector)
    vm.Reg ().IntVal () = FillTempRealArray (vm.Data (), vm.DataTypes (), max (max (s1, s2), 3), result);
}
void WrapLength (TomVM& vm) {

    // Fetch vector
    if (ReadVec (vm, vm.GetIntParam (1), v1) < 0)
        return;

    // Calculate length
    vm.Reg ().RealVal () = Length (v1);
}
void WrapNormalize (TomVM& vm) {

    // Fetch vector
    int size = ReadVec (vm, vm.GetIntParam (1), v1);
    if (size < 0)
        return;

    // Normalize vector
    Normalize (v1);

    // Return resulting vector
    vm.Reg ().IntVal () = FillTempRealArray (vm.Data (), vm.DataTypes (), size, v1);
}
void WrapDeterminant (TomVM& vm) {

    // Fetch matrix
    if (!ReadMatrix (vm, vm.GetIntParam (1), m1))
        return;

    // Return result
    vm.Reg ().RealVal () = Determinant (m1);
}
void WrapTranspose (TomVM& vm) {

    // Fetch matrix
    if (!ReadMatrix (vm, vm.GetIntParam (1), m1))
        return;

    // Transpose
    Transpose (m1, m2);

    // Create new matrix and assign to register
    vm.Reg ().IntVal () = FillTempRealArray2D (vm.Data (), vm.DataTypes (), 4, 4, m2);
}
void WrapRTInvert (TomVM& vm) {

    // Fetch matrix
    if (!ReadMatrix (vm, vm.GetIntParam (1), m1))
        return;

    // RTInvert
    RTInvert (m1, m2);

    // Create new matrix and assign to register
    vm.Reg ().IntVal () = FillTempRealArray2D (vm.Data (), vm.DataTypes (), 4, 4, m2);
}
void WrapOrthonormalize (TomVM& vm) {

    // Fetch matrix
    if (!ReadMatrix (vm, vm.GetIntParam (1), m1))
        return;

    // Orthonormalize
    Orthonormalize (m1);

    // Create new matrix and assign to register
    vm.Reg ().IntVal () = FillTempRealArray2D (vm.Data (), vm.DataTypes (), 4, 4, m1);
}

////////////////////////////////////////////////////////////////////////////////
// Overloaded operators

// Helpers

void DoScaleVec(TomVM& vm, vmReal scale, int vecIndex) {
	int size = 4;
	if ((size = ReadVec(vm, vecIndex, v1)) >= 0)
	{
		Scale(v1, scale);
	}
	else
	{
		for (int i = 0; i < 4; i++) v1[i] = 0.0f;
	}
	vm.Reg().IntVal() = FillTempRealArray(vm.Data(), vm.DataTypes(), size, v1);
}

void DoScaleMatrix(TomVM& vm, vmReal scale, int matrixIndex) {
	if (ReadMatrix(vm, matrixIndex, m1))
	{
		ScaleMatrix(m1, scale);
	}
	else
	{
		for (int i = 0; i < 16; i++) m1[i] = 0.0f;
	}
	vm.Reg().IntVal() = FillTempRealArray2D(vm.Data(), vm.DataTypes(), 4, 4, m1);
}

// Operator function wrappers

void WrapOpVecTimesScalar(TomVM& vm)
{
	DoScaleVec(vm, vm.GetRealParam(1), vm.GetIntParam(2));
}

void WrapOpScalarTimesVec(TomVM& vm)
{
	DoScaleVec(vm, vm.GetRealParam(2), vm.GetIntParam(1));
}

void WrapOpMatrixTimesScalar(TomVM& vm)
{
	DoScaleMatrix(vm, vm.GetRealParam(1), vm.GetIntParam(2));
}

void WrapOpScalarTimesMatrix(TomVM& vm)
{
	DoScaleMatrix(vm, vm.GetRealParam(2), vm.GetIntParam(1));
}

void WrapOpVecDivScalar(TomVM& vm)
{
	DoScaleVec(vm, 1.0f / vm.GetRealParam(1), vm.GetIntParam(2));
}

void WrapOpMatrixDivScalar(TomVM& vm)
{
	DoScaleMatrix(vm, 1.0f / vm.GetRealParam(1), vm.GetIntParam(2));
}

void WrapOpMatrixTimesVec(TomVM& vm)
{
	vmReal result[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	int size = 4;
	if (ReadMatrix(vm, vm.GetIntParam(2), m1) && (size = ReadVec(vm, vm.GetIntParam(1), v1)) >= 0)
	{
		MatrixTimesVec(m1, v1, result);
	}
	vm.Reg().IntVal() = FillTempRealArray(vm.Data(), vm.DataTypes(), size, result);	
}

void WrapOpMatrixTimesMatrix(TomVM& vm)
{
	vmReal result[16];
	if (ReadMatrix(vm, vm.GetIntParam(2), m1) && ReadMatrix(vm, vm.GetIntParam(1), m2))
	{
		MatrixTimesMatrix(m1, m2, result);
	}
	else
	{
		for (int i = 0; i < 16; i++) result[i] = 0.0f;
	}
	vm.Reg().IntVal() = FillTempRealArray2D(vm.Data(), vm.DataTypes(), 4, 4, result);
}

void WrapOpVecTimesVec(TomVM& vm)
{
	if (ReadVec(vm, vm.GetIntParam(2), v1) >= 0
		&& ReadVec(vm, vm.GetIntParam(1), v2) >= 0)
		vm.Reg().RealVal() = DotProduct(v1, v2);
	else
		vm.Reg().RealVal() = 0.0f;
}

void WrapOpVecPlusVec(TomVM& vm)
{
	vmReal result[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	int s1 = 4, s2 = 4;
	if ((s1 = ReadVec(vm, vm.GetIntParam(2), v1)) >= 0 && (s2 = ReadVec(vm, vm.GetIntParam(1), v2)) >= 0)
		VecPlus(v1, v2, result);
	vm.Reg().IntVal() = FillTempRealArray(vm.Data(), vm.DataTypes(), max(s1, s2), result);
}

void WrapOpVecMinusVec(TomVM& vm)
{
	vmReal result[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	int s1 = 4, s2 = 4;
	if ((s1 = ReadVec(vm, vm.GetIntParam(2), v1)) >= 0 && (s2 = ReadVec(vm, vm.GetIntParam(1), v2)) >= 0)
		VecMinus(v1, v2, result);
	vm.Reg().IntVal() = FillTempRealArray(vm.Data(), vm.DataTypes(), max(s1, s2), result);
}

void WrapOpMatrixPlusMatrix(TomVM& vm)
{
	vmReal result[16];
	if (ReadMatrix(vm, vm.GetIntParam(2), m1)
		&& ReadMatrix(vm, vm.GetIntParam(1), m2))
	{
		MatrixPlus(m1, m2, result);
	}
	else
	{
		for (int i = 0; i < 16; i++) result[i] = 0.0f;
	}
	vm.Reg().IntVal() = FillTempRealArray2D(vm.Data(), vm.DataTypes(), 4, 4, result);
}

void WrapOpMatrixMinusMatrix(TomVM& vm)
{
	vmReal result[16];
	if (ReadMatrix(vm, vm.GetIntParam(2), m1)
		&& ReadMatrix(vm, vm.GetIntParam(1), m2))
	{
		MatrixMinus(m1, m2, result);
	}
	else
	{
		for (int i = 0; i < 16; i++) result[i] = 0.0f;
	}
	vm.Reg().IntVal() = FillTempRealArray2D(vm.Data(), vm.DataTypes(), 4, 4, result);
}

void WrapOpNegVec(TomVM& vm)
{
	DoScaleVec(vm, -1.0f, vm.GetIntParam(1));
}

void WrapOpNegMatrix(TomVM& vm)
{
	DoScaleMatrix(vm, -1.0f, vm.GetIntParam(1));
}

////////////////////////////////////////////////////////////////////////////////
// Initialisation

void InitTomTrigBasicLib (TomBasicCompiler& comp) {

    /////////////////////
    // Initialise state

    ///////////////////////
    // Register constants

    ///////////////////////
    // Register functions
    comp.AddFunction ("Vec4",               WrapVec4,               compParamTypeList () << VTP_REAL << VTP_REAL << VTP_REAL << VTP_REAL,   true,   true,   vmValType (VTP_REAL, 1, 1, true), false, true);
    comp.AddFunction ("Vec3",               WrapVec3,               compParamTypeList () << VTP_REAL << VTP_REAL << VTP_REAL,               true,   true,   vmValType (VTP_REAL, 1, 1, true), false, true);
    comp.AddFunction ("Vec2",               WrapVec2,               compParamTypeList () << VTP_REAL << VTP_REAL,                           true,   true,   vmValType (VTP_REAL, 1, 1, true), false, true);
    comp.AddFunction ("MatrixZero",         WrapMatrixZero,         compParamTypeList (),                                                   true,   true,   vmValType (VTP_REAL, 2, 1, true), false, true);
    comp.AddFunction ("MatrixIdentity",     WrapMatrixIdentity,     compParamTypeList (),                                                   true,   true,   vmValType (VTP_REAL, 2, 1, true), false, true);
    comp.AddFunction ("MatrixScale",        WrapMatrixScale,        compParamTypeList () << VTP_REAL,                                       true,   true,   vmValType (VTP_REAL, 2, 1, true), false, true);
    comp.AddFunction ("MatrixScale",        WrapMatrixScale_2,      compParamTypeList () << VTP_REAL << VTP_REAL << VTP_REAL,               true,   true,   vmValType (VTP_REAL, 2, 1, true), false, true);
    comp.AddFunction ("MatrixTranslate",    WrapMatrixTranslate,    compParamTypeList () << VTP_REAL << VTP_REAL << VTP_REAL,               true,   true,   vmValType (VTP_REAL, 2, 1, true), false, true);
    comp.AddFunction ("MatrixRotateX",      WrapMatrixRotateX,      compParamTypeList () << VTP_REAL,                                       true,   true,   vmValType (VTP_REAL, 2, 1, true), false, true);
    comp.AddFunction ("MatrixRotateY",      WrapMatrixRotateY,      compParamTypeList () << VTP_REAL,                                       true,   true,   vmValType (VTP_REAL, 2, 1, true), false, true);
    comp.AddFunction ("MatrixRotateZ",      WrapMatrixRotateZ,      compParamTypeList () << VTP_REAL,                                       true,   true,   vmValType (VTP_REAL, 2, 1, true), false, true);
    comp.AddFunction ("MatrixRotate",       WrapMatrixRotate,       compParamTypeList () << VTP_REAL << vmValType(VTP_REAL, 1, 1, true),    true,   true,   vmValType (VTP_REAL, 2, 1, true), false, true);
    comp.AddFunction ("MatrixBasis",        WrapMatrixBasis,        compParamTypeList () << vmValType (VTP_REAL, 1, 1, true) << vmValType (VTP_REAL, 1, 1, true) << vmValType (VTP_REAL, 1, 1, true), true,   true,   vmValType (VTP_REAL, 2, 1, true), false, true);
    comp.AddFunction ("MatrixCrossProduct", WrapMatrixCrossProduct, compParamTypeList () << vmValType (VTP_REAL, 1, 1, true),               true,   true,   vmValType (VTP_REAL, 2, 1, true), false, true);
    comp.AddFunction ("CrossProduct",       WrapCross,              compParamTypeList () << vmValType (VTP_REAL, 1, 1, true) << vmValType (VTP_REAL, 1, 1, true), true, true, vmValType (VTP_REAL, 1, 1, true), false, true);
    comp.AddFunction ("Length",             WrapLength,             compParamTypeList () << vmValType (VTP_REAL, 1, 1, true),               true,   true,   VTP_REAL);
    comp.AddFunction ("Normalize",          WrapNormalize,          compParamTypeList () << vmValType (VTP_REAL, 1, 1, true),               true,   true,   vmValType (VTP_REAL, 1, 1, true), false, true);
    comp.AddFunction ("Determinant",        WrapDeterminant,        compParamTypeList () << vmValType (VTP_REAL, 2, 1, true),               true,   true,   VTP_REAL);
    comp.AddFunction ("Transpose",          WrapTranspose,          compParamTypeList () << vmValType (VTP_REAL, 2, 1, true),               true,   true,   vmValType (VTP_REAL, 2, 1, true), false, true);
    comp.AddFunction ("RTInvert",           WrapRTInvert,           compParamTypeList () << vmValType (VTP_REAL, 2, 1, true),               true,   true,   vmValType (VTP_REAL, 2, 1, true), false, true);
    comp.AddFunction ("Orthonormalize",     WrapOrthonormalize,     compParamTypeList () << vmValType (VTP_REAL, 2, 1, true),               true,   true,   vmValType (VTP_REAL, 2, 1, true), false, true);

    //////////////////////////////////
    // Register overloaded operators
	comp.AddFunction("operator*", WrapOpVecTimesScalar, compParamTypeList() << vmValType(VTP_REAL, 1, 1, true) << VTP_REAL, true, true, vmValType(VTP_REAL, 1, 1, true), false, true);
	comp.AddFunction("operator*", WrapOpScalarTimesVec, compParamTypeList() << VTP_REAL << vmValType(VTP_REAL, 1, 1, true), true, true, vmValType(VTP_REAL, 1, 1, true), false, true);
	comp.AddFunction("operator*", WrapOpMatrixTimesScalar, compParamTypeList() << vmValType(VTP_REAL, 2, 1, true) << VTP_REAL, true, true, vmValType(VTP_REAL, 2, 1, true), false, true);
	comp.AddFunction("operator*", WrapOpScalarTimesMatrix, compParamTypeList() << VTP_REAL << vmValType(VTP_REAL, 2, 1, true), true, true, vmValType(VTP_REAL, 2, 1, true), false, true);
	comp.AddFunction("operator/", WrapOpVecDivScalar, compParamTypeList() << vmValType(VTP_REAL, 1, 1, true) << VTP_REAL, true, true, vmValType(VTP_REAL, 1, 1, true), false, true);
	comp.AddFunction("operator/", WrapOpMatrixDivScalar, compParamTypeList() << vmValType(VTP_REAL, 2, 1, true) << VTP_REAL, true, true, vmValType(VTP_REAL, 2, 1, true), false, true);
	comp.AddFunction("operator*", WrapOpMatrixTimesVec, compParamTypeList() << vmValType(VTP_REAL, 2, 1, true) << vmValType(VTP_REAL, 1, 1, true), true, true, vmValType(VTP_REAL, 1, 1, true), false, true);
	comp.AddFunction("operator*", WrapOpMatrixTimesMatrix, compParamTypeList() << vmValType(VTP_REAL, 2, 1, true) << vmValType(VTP_REAL, 2, 1, true), true, true, vmValType(VTP_REAL, 2, 1, true), false, true);
	comp.AddFunction("operator*", WrapOpVecTimesVec, compParamTypeList() << vmValType(VTP_REAL, 1, 1, true) << vmValType(VTP_REAL, 1, 1, true), true, true, VTP_REAL);
	comp.AddFunction("operator+", WrapOpVecPlusVec, compParamTypeList() << vmValType(VTP_REAL, 1, 1, true) << vmValType(VTP_REAL, 1, 1, true), true, true, vmValType(VTP_REAL, 1, 1, true), false, true);
	comp.AddFunction("operator-", WrapOpVecMinusVec, compParamTypeList() << vmValType(VTP_REAL, 1, 1, true) << vmValType(VTP_REAL, 1, 1, true), true, true, vmValType(VTP_REAL, 1, 1, true), false, true);
	comp.AddFunction("operator+", WrapOpMatrixPlusMatrix, compParamTypeList() << vmValType(VTP_REAL, 2, 1, true) << vmValType(VTP_REAL, 2, 1, true), true, true, vmValType(VTP_REAL, 2, 1, true), false, true);
	comp.AddFunction("operator-", WrapOpMatrixMinusMatrix, compParamTypeList() << vmValType(VTP_REAL, 2, 1, true) << vmValType(VTP_REAL, 2, 1, true), true, true, vmValType(VTP_REAL, 2, 1, true), false, true);
	comp.AddFunction("operator-", WrapOpNegVec, compParamTypeList() << vmValType(VTP_REAL, 1, 1, true), true, true, vmValType(VTP_REAL, 1, 1, true), false, true);
	comp.AddFunction("operator-", WrapOpNegMatrix, compParamTypeList() << vmValType(VTP_REAL, 2, 1, true), true, true, vmValType(VTP_REAL, 2, 1, true), false, true);
}
