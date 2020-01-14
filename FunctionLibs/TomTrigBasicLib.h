//---------------------------------------------------------------------------
/*  Created 22-Oct-2003: Thomas Mulgrew

    Matrix and vector routines.
*/

#ifndef TomTrigBasicLibH
#define TomTrigBasicLibH
//---------------------------------------------------------------------------
#include "TomComp.h"
#include "TomStdBasicLib.h"
#include <math.h>

void InitTomTrigBasicLib (TomBasicCompiler& comp);

////////////////////////////////////////////////////////////////////////////////
// Inlined trig functions

#ifndef max
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#endif

// Matrix constructors.
// Note: These all drop their result into the global "matrix" variable (below)
extern vmReal matrix [16];

inline void ClearMatrix () {
    memset (matrix, 0, 16 * sizeof (vmReal));
}
inline void Identity () {
    ClearMatrix ();
    matrix [0]  = 1;
    matrix [5]  = 1;
    matrix [10] = 1;
    matrix [15] = 1;
}
inline void Scale (vmReal scale) {
    ClearMatrix ();
    matrix [0]  = scale;
    matrix [5]  = scale;
    matrix [10] = scale;
    matrix [15] = 1;
}
inline void Scale (vmReal x, vmReal y, vmReal z) {
    ClearMatrix ();
    matrix [0]  = x;
    matrix [5]  = y;
    matrix [10] = z;
    matrix [15] = 1;
}
inline void Translate (vmReal x, vmReal y, vmReal z) {
    Identity ();
    matrix [12] = x;
    matrix [13] = y;
    matrix [14] = z;
}
inline void RotateAxis (vmReal ang, int main, int a1, int a2) {
    ClearMatrix ();
    vmReal  cosa = cos (ang * M_DEG2RAD),
            sina = sin (ang * M_DEG2RAD);
    matrix [15] = 1;
	matrix [main * 4 + main] = 1.0;
	matrix [a1 + a1 * 4] =  cosa;
	matrix [a1 + a2 * 4] =  sina;
	matrix [a2 + a1 * 4] = -sina;
	matrix [a2 + a2 * 4] =  cosa;
}
inline void RotateX (vmReal ang) {  RotateAxis (ang, 0, 2, 1); }
inline void RotateY (vmReal ang) {  RotateAxis (ang, 1, 0, 2); }
inline void RotateZ (vmReal ang) {  RotateAxis (ang, 2, 1, 0); }
inline void CrossProduct (vmReal *vec) {

    // Create a matrix which corresponds to the cross product with vec
    // I.e Mr = vec x r
    ClearMatrix ();
	matrix [1]  =  vec [2];		// Fill in non zero bits
	matrix [2]  = -vec [1];
	matrix [4]  = -vec [2];
	matrix [6]  =  vec [0];
	matrix [8]  =  vec [1];
	matrix [9]  = -vec [0];
	matrix [15] = 1;
}
inline void CopyMatrix (vmReal *dst, vmReal *src) {
    memcpy (dst, src, 16 * sizeof (src [0]));
}

////////////////////////////////////////////////////////////////////////////////
// Basic trig functions
inline vmReal CalcW (vmReal *v1, vmReal *v2) {
    if (v1 [3] == 0 && v2 [3] == 0) return 0;
    else                            return 1;
}

inline void CrossProduct (vmReal *v1, vmReal *v2, vmReal *result) {
    assert (v1 != NULL);
    assert (v1 != NULL);
    assert (result != NULL);
  	result [0]	= v1 [1] * v2 [2] - v1 [2] * v2 [1];
	result [1]	= v1 [2] * v2 [0] - v1 [0] * v2 [2];
	result [2]	= v1 [0] * v2 [1] - v1 [1] * v2 [0];

    // Calculate w
    result [3] = CalcW (v1, v2);
}
inline vmReal DotProduct (vmReal *v1, vmReal *v2) {
    assert (v1 != NULL);
    assert (v2 != NULL);
    return v1 [0] * v2 [0] + v1 [1] * v2 [1] + v1 [2] * v2 [2];
}
inline vmReal Length (vmReal *v) {
    assert (v != NULL);
    vmReal dp = DotProduct(v, v);
    return sqrt (dp);
}
inline void Scale (vmReal *v, vmReal scale) {
    assert (v != NULL);
    v [0] *= scale;
    v [1] *= scale;
    v [2] *= scale;
    if (v [3] != 0)
        v [3] = 1;
}
inline void ScaleMatrix (vmReal *m, vmReal scale) {
    assert (m != NULL);
    for (int i = 0; i < 16; i++)
        m [i] *= scale;
}
inline void Normalize (vmReal *v) {
    vmReal len = Length (v);
    if (len > 0.0001)
        Scale (v, 1.0 / Length (v));
}
inline vmReal Determinant (vmReal *m) {
    assert (m != NULL);

    // Calculate matrix determinant
	vmReal res = 0;
	for (int y = 0; y < 4; y++) {
		res += m [y]
			 * m [4  + ((y + 1) & 3)]
			 * m [8  + ((y + 2) & 3)]
			 * m [12 + ((y + 3) & 3)];
		res -= m [y]
			 * m [4  + ((y - 1) & 3)]
			 * m [8  + ((y - 2) & 3)]
			 * m [12 + ((y - 3) & 3)];
	}
    return res;
}
inline void Transpose (vmReal *src, vmReal *dst) {
    assert (src != NULL);
    assert (dst != NULL);

    // Transpose matrix
    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
            dst [x * 4 + y] = src [y * 4 + x];
}
inline void MatrixTimesVec (vmReal *m, vmReal *v, vmReal *result) {
    assert (m != NULL);
    assert (v != NULL);
    assert (result != NULL);
    for (int y = 0; y < 4; y++) {
        vmReal coord = 0;
        for (int x = 0; x < 4; x++)
            coord += v [x] * m [x * 4 + y];
        result [y] = coord;
    }
}
inline void MatrixTimesMatrix (vmReal *m1, vmReal *m2, vmReal *result) {
    assert (m1 != NULL);
    assert (m2 != NULL);
    assert (result != NULL);
    assert (result != m1);
    assert (result != m2);
    for (int x2 = 0; x2 < 4; x2++) {
        for (int y1 = 0; y1 < 4; y1++) {
            vmReal coord = 0;
            for (int i = 0; i < 4; i++)
                coord += m2 [x2 * 4 + i] * m1 [i * 4 + y1];
            result [x2 * 4 + y1] = coord;
        }
    }
}
inline void VecPlus (vmReal *v1, vmReal *v2, vmReal *result) {
    assert (v1 != NULL);
    assert (v2 != NULL);
    assert (result != NULL);

    // Add vectors
    result [0] = v1 [0] + v2 [0];
    result [1] = v1 [1] + v2 [1];
    result [2] = v1 [2] + v2 [2];

    // Calculate w
    result [3] = CalcW (v1, v2);
}
inline void VecMinus (vmReal *v1, vmReal *v2, vmReal *result) {
    assert (v1 != NULL);
    assert (v2 != NULL);
    assert (result != NULL);

    // Subtract vectors
    result [0] = v1 [0] - v2 [0];
    result [1] = v1 [1] - v2 [1];
    result [2] = v1 [2] - v2 [2];

    // Calculate w
    result [3] = CalcW (v1, v2);
}
inline void MatrixPlus (vmReal *m1, vmReal *m2, vmReal *result) {
    assert (m1 != NULL);
    assert (m2 != NULL);
    assert (result != NULL);

    // Add matrices
    for (int i = 0; i < 16; i++)
        result [i] = m1 [i] + m2 [i];
}
inline void MatrixMinus (vmReal *m1, vmReal *m2, vmReal *result) {
    assert (m1 != NULL);
    assert (m2 != NULL);
    assert (result != NULL);

    // Add matrices
    for (int i = 0; i < 16; i++)
        result [i] = m1 [i] - m2 [i];
}
inline void RTInvert (vmReal *m, vmReal *result) {

    // Invert an "RT" matrix, where "RT" means that the matrix is made from
    // rotations and translations only.

    // Transpose matrix to invert the rotation part
    Transpose (m, result);

    // Clear out the transposed translation component of the original matrix
    result [3]  = 0;
    result [7]  = 0;
    result [11] = 0;

    // Calculate the new translation component
    vmReal t[4];
    MatrixTimesVec (result, &m [12], t);
    result [12] = -t[0];
    result [13] = -t[1];
    result [14] = -t[2];
}
inline void Orthonormalize (vmReal *m) {

    // Ensure rotation component of matrix is orthonormal, via a series of
    // normalizations and cross products.

    // Normalize Z vector
    Normalize (&m [8]);

    // Cross product Y with Z to get X
    // Then normalize resulting X
    CrossProduct (&m [4], &m [8], &m [0]);
    Normalize (&m [0]);

    // Cross product Z with X to get Y
    CrossProduct (&m [8], &m [0], &m [4]);
}

// Arbitrary axis rotation
inline void RotateAxis(vmReal ang, vmReal *v) {

    // Thanks to Satin Hinge for sending me the arbitrary axis maths
    // Normalize vector
    vmReal N[3];
    N[0] = v[0];
    N[1] = v[1];
    N[2] = v[2];
    Normalize(N);

    // Precalc sin/cos
    vmReal c = cos(ang * M_DEG2RAD), s = sin(ang * M_DEG2RAD);

    // Construct matrix
    ClearMatrix();
    matrix[0] = (1-c)*N[0]*N[0]+c;      matrix[4] = (1-c)*N[0]*N[1]-s*N[2]; matrix[8] = (1-c)*N[0]*N[2]+s*N[1];
    matrix[1] = (1-c)*N[0]*N[1]+s*N[2]; matrix[5] = (1-c)*N[1]*N[1]+c;      matrix[9] = (1-c)*N[1]*N[2]-s*N[0];
    matrix[2] = (1-c)*N[0]*N[2]-s*N[1]; matrix[6] = (1-c)*N[1]*N[2]+s*N[0]; matrix[10]= (1-c)*N[2]*N[2]+c;
    matrix[15] = 1;
}


#endif
