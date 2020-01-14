/*  Basic4GLOpenGLObjects.h

    Created 12-Aug-06: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

    OpenGL specific objects exposed by Basic4GL to plugin DLLs.
*/

#ifndef _Basic4GLOpenGLObjects_h
#define _Basic4GLOpenGLObjects_h

#include "Basic4GLStandardObjects.h"
#include <gl/gl.h>

#ifndef byte
typedef unsigned char byte;
#endif

// Text mode constants/variables
enum TextMode {
    TEXT_SIMPLE = 0,    // (Default). After any print function, text is automatically rendered as follows:
                        //      * Back buffer is cleared
                        //      * Text is rendered
                        //      * Buffers are swapped

    TEXT_BUFFERED,      // Must explicitly call DrawText(), as before:
                        //      * Back buffer is cleared
                        //      * Text is rendered
                        //      * Buffers are swapped

    TEXT_OVERLAID       // Same as TEXT_BUFFERED, except DrawText() only renders the text.
                        // Must explictly clear and swap buffers as necessary.
};

////////////////////////////////////////////////////////////////////////////////
//  IB4GLOpenGLText
//
/// Interface to the Basic4GL OpenGL text mechanism.

class IB4GLOpenGLText : public IB4GLText {
public:
    virtual void DLLFUNC Font(GLuint fontTexture) = 0;
    virtual GLuint DLLFUNC DefaultFont() = 0;
    virtual void DLLFUNC SetTextMode(TextMode mode) = 0;
    virtual void DLLFUNC Color(byte red, byte green, byte blue) = 0;
};

////////////////////////////////////////////////////////////////////////////////
//  IB4GLOpenGLWindow
//
/// Interface to the OpenGL window.

class IB4GLOpenGLWindow {
public:
    virtual int DLLFUNC Width() = 0;
    virtual int DLLFUNC Height() = 0;
    virtual int DLLFUNC BPP() = 0;          // (Bits per pixel)
    virtual bool DLLFUNC Fullscreen() = 0;
    virtual void DLLFUNC SwapBuffers() = 0;
    virtual const char *DLLFUNC Title() = 0;
};

#endif
