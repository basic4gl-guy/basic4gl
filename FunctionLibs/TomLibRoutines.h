/*  Created 12-Oct-2003: Thomas Mulgrew

    "Helper" routines for generated Basic4GL library wrapper functions.
*/

#ifndef _TomLibRoutines_h
#define _TomLibRoutines_h

#include "TomVM.h"

inline bool ValidateSizeParam (TomVM& vm, int paramIndex) {
    int size = vm.GetIntParam (paramIndex);
    if (size <= 0)                      // If size is 0, do nothing.
        return false;                   // Program will still proceed.
    if (size > 65536) {                 // If size is greater 64K, this is a run-time error
        vm.FunctionError ("Size must be 0 - 65536 (Basic4GL restriction)");
        return false;
    }

    // Size is valid
    return true;
}

inline int ReadArrayDynamic (   TomVM& vm,
                                int paramIndex,
                                vmValType type,
                                GLenum cType,
                                void *array,
                                int maxSize) {

    // Use the appropriate template function for the given type
    switch (cType) {
        case GL_BYTE:           return ReadAndZero (vm.Data (), vm.GetIntParam (paramIndex), type, (GLbyte        *) array, maxSize);
        case GL_UNSIGNED_BYTE:  return ReadAndZero (vm.Data (), vm.GetIntParam (paramIndex), type, (GLubyte       *) array, maxSize);
        case GL_SHORT:          return ReadAndZero (vm.Data (), vm.GetIntParam (paramIndex), type, (GLshort       *) array, maxSize);
        case GL_UNSIGNED_SHORT: return ReadAndZero (vm.Data (), vm.GetIntParam (paramIndex), type, (GLushort      *) array, maxSize);
        case GL_INT:            return ReadAndZero (vm.Data (), vm.GetIntParam (paramIndex), type, (GLint         *) array, maxSize);
        case GL_UNSIGNED_INT:   return ReadAndZero (vm.Data (), vm.GetIntParam (paramIndex), type, (GLuint        *) array, maxSize);
        case GL_FLOAT:          return ReadAndZero (vm.Data (), vm.GetIntParam (paramIndex), type, (GLfloat       *) array, maxSize);
        case GL_2_BYTES:        return ReadAndZero (vm.Data (), vm.GetIntParam (paramIndex), type, (unsigned short*) array, maxSize);
        case GL_3_BYTES:
            vm.FunctionError ("Data type GL_3_BYTES not supported. (Basic4GL restriction).");
            return 0;
        case GL_4_BYTES:        return ReadAndZero (vm.Data (), vm.GetIntParam (paramIndex), type, (unsigned int  *) array, maxSize);
        case GL_DOUBLE:         return ReadAndZero (vm.Data (), vm.GetIntParam (paramIndex), type, (GLdouble      *) array, maxSize);
        default:
            vm.FunctionError ("Data type must be a GL_xxx data type. (Basic4GL restriction).");
            return 0;
    }
}

inline int WriteArrayDynamic (   TomVM& vm,
                                int paramIndex,
                                vmValType type,
                                GLenum cType,
                                void *array,
                                int maxSize) {

    // Use the appropriate template function for the given type
    switch (cType) {
        case GL_BYTE:           return WriteArray (vm.Data (), vm.GetIntParam (paramIndex), type, (GLbyte        *) array, maxSize);
        case GL_UNSIGNED_BYTE:  return WriteArray (vm.Data (), vm.GetIntParam (paramIndex), type, (GLubyte       *) array, maxSize);
        case GL_SHORT:          return WriteArray (vm.Data (), vm.GetIntParam (paramIndex), type, (GLshort       *) array, maxSize);
        case GL_UNSIGNED_SHORT: return WriteArray (vm.Data (), vm.GetIntParam (paramIndex), type, (GLushort      *) array, maxSize);
        case GL_INT:            return WriteArray (vm.Data (), vm.GetIntParam (paramIndex), type, (GLint         *) array, maxSize);
        case GL_UNSIGNED_INT:   return WriteArray (vm.Data (), vm.GetIntParam (paramIndex), type, (GLuint        *) array, maxSize);
        case GL_FLOAT:          return WriteArray (vm.Data (), vm.GetIntParam (paramIndex), type, (GLfloat       *) array, maxSize);
        case GL_2_BYTES:        return WriteArray (vm.Data (), vm.GetIntParam (paramIndex), type, (unsigned short*) array, maxSize);
        case GL_3_BYTES:
            vm.FunctionError ("Data type GL_3_BYTES not supported. (Basic4GL restriction).");
            return 0;
        case GL_4_BYTES:        return WriteArray (vm.Data (), vm.GetIntParam (paramIndex), type, (unsigned int  *) array, maxSize);
        case GL_DOUBLE:         return WriteArray (vm.Data (), vm.GetIntParam (paramIndex), type, (GLdouble      *) array, maxSize);
        default:
            vm.FunctionError ("Data type must be a GL_xxx data type. (Basic4GL restriction).");
            return 0;
    }
}


#endif
