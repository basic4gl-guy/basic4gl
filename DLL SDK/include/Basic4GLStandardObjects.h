/*  Basic4GLStandardObjects.h

    Created 5-Aug-06: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

    Interfaces to Basic4GL built in library objects.
    These can be fetched by name from DLLs in IDLL_Plugin::Load() from the
    registry object.

    E.g:
        IB4GLText *textWindow = (IB4GLText *) registry.FetchInterface("B4GLText");
        if (textWindow == NULL) {
            // Error condition here!
            return false;
        }
        ...

*/

#ifndef _Basic4GLStandardObjects_h
#define _Basic4GLStandardObjects_h

#include "DLLMisc.h"

#define IB4GLTEXT_MAJOR 1
#define IB4GLTEXT_MINOR 0
////////////////////////////////////////////////////////////////////////////////
//  IB4GLText
//
/// Interface to the Basic4GL text mechanism.
/// Note:
///     This returns an interface to the OpenGL text mechanism.
///     The interface is deliberately general, and in future versions of Basic4GL
///     might be used to access other text output devices like a console window.
///     Because of this, there are some OpenGL specific commands missing, such
///     as setting fonts. To access these, you should use the IB4GLOpenGLText
///     interface defined in Basic4GLOpenGLObjects.h instead.

class IB4GLText {
public:
    // Standard text output
    virtual void DLLFUNC Print(const char *text, bool newline) = 0;
    virtual void DLLFUNC Locate(int x, int y) = 0;
    virtual void DLLFUNC Cls() = 0;
    virtual void DLLFUNC ClearRegion(int x1, int y1, int x2, int y2) = 0;
    virtual int DLLFUNC TextRows() = 0;
    virtual int DLLFUNC TextCols() = 0;
    virtual void DLLFUNC ResizeText(int cols, int rows) = 0;
    virtual void DLLFUNC SetTextScroll(bool scroll) = 0;
    virtual bool DLLFUNC TextScroll() = 0;
    virtual void DLLFUNC DrawText() = 0;
    virtual char DLLFUNC CharAt(int x, int y) = 0;
};

#define IB4GLFILEACCESSOR_MAJOR 1
#define IB4GLFILEACCESSOR_MINOR 0
////////////////////////////////////////////////////////////////////////////////
//  IB4GLFileAccessor
//
/// Used to access files from a plugin.
/// Plugins can use this object to access files embedded in a standalone exe.
class IB4GLFileAccessor {
public:
    /// Get filename to open for read.
    /// If the file is an embedded file then it will be extracted into a
    /// temporary file whose filename will be returned (in dst parameter).
    /// Otherwise simply returns the filename (in dst parameter).
    /// dst must be at least MAX_PATH characters in length.
    virtual void DLLFUNC GetFilenameForRead(const char* filename, char* dst) = 0;
};

#define IB4GLCOMPILER_MAJOR 1
#define IB4GLCOMPILER_MINOR 0
////////////////////////////////////////////////////////////////////////////////
//  IB4GLCompiler
//
/// Used to compile and execute Basic4GL code at RUNTIME.
/// IMPORTANT:
///  1. Plugins should ONLY compile code while a program is running (i.e. from
///     within a runtime function.
///  2. When the program ends the plugin should consider all handles from
///     compiled blocks of code become INVALID. The plugin should not try to
///     call them.
class IB4GLCompiler {
public:

    /// Compile Basic4GL code and return its handle.
    /// Returns a non-zero handle if compilation succeeded.
    /// Returns 0 if an error occurs.
    virtual int DLLFUNC Compile(const char *sourceText) = 0;

    // Error retrieving methods. Call only if Compile() returns 0.

    /// Get error description.
    /// Error is placed in buffer.
    /// If error is longer than bufferLen, it will be truncated to fit.
    virtual void DLLFUNC GetErrorText(char *buffer, int bufferLen) = 0;

    /// Line number where error occurred
    virtual int DLLFUNC GetErrorLine() = 0;

    /// Column number where error occurred
    virtual int DLLFUNC GetErrorCol() = 0;

    // Execute code block

    /// Execute code.
    /// Returns true if code executed without a runtime error.
    /// Returns false if an error occurred. If an error occurs, the plugin
    /// should return immediately. Basic4GL will then stop and display the error.
    virtual bool DLLFUNC Execute(int codeHandle) = 0;
};

#endif

