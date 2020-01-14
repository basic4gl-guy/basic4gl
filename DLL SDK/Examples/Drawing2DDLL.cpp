#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include <math.h>
#include "Basic4GLDLLInterface.h"

///////////////////////////////////////////////////////////////////////////////
//	Globals
char *pluginError = NULL;

///////////////////////////////////////////////////////////////////////////////
//	Plugin
//
///	Main object representing the plugin to Basic4GL
class Plugin : public IDLL_Basic4GL_Plugin {
public:
    virtual bool DLLFUNC Load(IDLL_Basic4GL_FunctionRegistry &registry, bool isStandaloneExe);
    virtual bool DLLFUNC Start();
    virtual void DLLFUNC End();
    virtual void DLLFUNC Unload();
    virtual void DLLFUNC GetError(char *error);
	virtual void DLLFUNC Pause();
	virtual void DLLFUNC Resume();
	virtual void DLLFUNC DelayedResume();
	virtual void DLLFUNC ProcessMessages();
};

///////////////////////////////////////////////////////////////////////////////
// DLL exported functions
extern "C" {

	/// Query function
	DLLExport int DLLFUNC Basic4GL_Query(char *details, int *major, int *minor) {
		char *detailSrc = "2D drawing routines";
		int i;
		for (i = 0; detailSrc[i] != 0; i++)
			details[i] = detailSrc[i];
		*major = 1;
		*minor = 0;
		return BASIC4GL_DLL_VERSION;
	}

	// Main initialisation function
	DLLExport IDLL_Basic4GL_Plugin *DLLFUNC Basic4GL_Init() {
		return new Plugin();
	}
}

///////////////////////////////////////////////////////////////////////////////
// Structures
//
// (These are all imaginary for now)

// Not all compilers define "byte". Add it if not already defined.
#ifndef byte
typedef unsigned char byte;
#endif

// Pack structures by bytes.
// Otherwise the C compiler may pad them out if it thinks it will make a
// faster data structure. This can lead to problems when Basic4GL tries
// to read/write to the structures, as it does not know about padding that
// the compiler might have added.

// Note: If you're using an existing structure type (e.g. defined in a
// 3rd party library), you may not have control over the structure
// definitions. If it looks like the library has been adding padding to
// the structure, you can inform Basic4GL of this with the AddStrucPadding()
// method when describing the structure to Basic4GL.
// If you define the padding in the right places, Basic4GL should be able
// to skip over it and access the fields in their correct places.

#pragma pack(push,1)

#pragma pack(pop)

// Global structure type handles.

// Helper stuff

enum ColorCode {
    BLACK = 0,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    GRAY,
    LIGHTGRAY,
    LIGHTBLUE,
    LIGHTGREEN,
    LIGHTCYAN,
    LIGHTRED,
    LIGHTMAGENTA,
    YELLOW,
    WHITE
};

float colorValues[16][3] = {
    {0, 0, 0},
    {0, 0, 0.5f},
    {0, 0.5f, 0},
    {0, 0.5f, 0.5f},
    {0.5f, 0, 0},
    {0.5f, 0, 0.5f},
    {0.5f, 0.5f, 0},
    {0.5f, 0.5f, 0.5f},
    {0.75f, 0.75f, 0.75f},
    {0, 0, 1},
    {0, 1, 0},
    {0, 1, 1},
    {1, 0, 0},
    {1, 0, 1},
    {1, 1, 0},
    {1, 1, 1}};

float cursorX, cursorY;

void LineTo(float x, float y) {
    glBegin(GL_LINES);
        glVertex2f(cursorX, cursorY);
        glVertex2f(x, y);
    glEnd();
    cursorX = x;
    cursorY = y;
}

void Ellipse(float x, float y, float xRad, float yRad) {
    float factor = M_PI * 2.0f / 100.0f;
    for (int i = 0; i < 100; i++) {
        float ang = i * factor;
        glVertex2f(x + sin(ang) * xRad, y + cos(ang) * yRad);
    }
}

void Circle(float x, float y, float rad) {
    Ellipse(x, y, rad, rad);
}

void Box(float x1, float y1, float x2, float y2) {
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
}

void Mode2D(int xSize, int ySize) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, xSize, ySize, 0, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(false);
}

void SetColor(ColorCode color) {
    if (color >= 0 && color < 16) {
        float* c = colorValues[color];
        glColor3f(c[0], c[1], c[2]);
    }
}

void ClearScreen(ColorCode color) {
    if (color >= 0 && color < 16) {
        float* c = colorValues[color];
        glClearColor(c[0], c[1], c[2], 1);
        glClear(GL_COLOR_BUFFER_BIT);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Runtime functions

void DLLFUNC Func_Mode2D(IDLL_Basic4GL_Runtime &basic4gl) {
    int xSize = basic4gl.GetIntParam(2);
    int ySize = basic4gl.GetIntParam(1);
    Mode2D(xSize, ySize);
    glDrawBuffer(GL_FRONT_AND_BACK);
}

void DLLFUNC Func_Mode2DBuffered(IDLL_Basic4GL_Runtime &basic4gl) {
    int xSize = basic4gl.GetIntParam(2);
    int ySize = basic4gl.GetIntParam(1);
    Mode2D(xSize, ySize);
}

void DLLFUNC Func_ClearScreen(IDLL_Basic4GL_Runtime &basic4gl) {
    int r = basic4gl.GetIntParam(3);
    int g = basic4gl.GetIntParam(2);
    int b = basic4gl.GetIntParam(1);
    glClearColor((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
}

void DLLFUNC Func_ClearScreen2(IDLL_Basic4GL_Runtime &basic4gl) {
    ColorCode color = (ColorCode)basic4gl.GetIntParam(1);
    ClearScreen(color);
}

void DLLFUNC Func_SetColor(IDLL_Basic4GL_Runtime &basic4gl) {
    int r = basic4gl.GetIntParam(3);
    int g = basic4gl.GetIntParam(2);
    int b = basic4gl.GetIntParam(1);
    glColor3f((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f);
}

void DLLFUNC Func_SetColor2(IDLL_Basic4GL_Runtime &basic4gl) {
    ColorCode color = (ColorCode)basic4gl.GetIntParam(1);
    SetColor(color);
}

void DLLFUNC Func_MoveTo(IDLL_Basic4GL_Runtime &basic4gl) {
    cursorX = basic4gl.GetFloatParam(2);
    cursorY = basic4gl.GetFloatParam(1);
}

void DLLFUNC Func_MoveTo2(IDLL_Basic4GL_Runtime &basic4gl) {
    float pos[2];
    basic4gl.SetType(DLL_BASIC4GL_EXT_FLOAT);
    basic4gl.ModTypeArray(1, 2);
    basic4gl.GetParam(1, pos);
    cursorX = pos[0];
    cursorY = pos[1];
}

void DLLFUNC Func_LineTo(IDLL_Basic4GL_Runtime &basic4gl) {
    float newX = basic4gl.GetFloatParam(2);
    float newY = basic4gl.GetFloatParam(1);
    LineTo(newX, newY);
}

void DLLFUNC Func_LineTo2(IDLL_Basic4GL_Runtime &basic4gl) {
    float pos[2];
    basic4gl.SetType(DLL_BASIC4GL_EXT_FLOAT);
    basic4gl.ModTypeArray(1, 2);
    basic4gl.GetParam(1, pos);
    LineTo(pos[0], pos[1]);
}

void DLLFUNC Func_LineWidth(IDLL_Basic4GL_Runtime &basic4gl) {
    float width = basic4gl.GetFloatParam(1);
    glLineWidth(width);
}

void DLLFUNC Func_Plot(IDLL_Basic4GL_Runtime &basic4gl) {
    float x = basic4gl.GetFloatParam(2);
    float y = basic4gl.GetFloatParam(1);
    glBegin(GL_POINTS);
        glVertex2f(x, y);
    glEnd();
}

void DLLFUNC Func_DrawBox(IDLL_Basic4GL_Runtime &basic4gl) {
    float x1 = basic4gl.GetFloatParam(4);
    float y1 = basic4gl.GetFloatParam(3);
    float x2 = basic4gl.GetFloatParam(2);
    float y2 = basic4gl.GetFloatParam(1);
    glBegin(GL_LINE_LOOP);
        Box(x1, y1, x2, y2);
    glEnd();
}

void DLLFUNC Func_FillBox(IDLL_Basic4GL_Runtime &basic4gl) {
    float x1 = basic4gl.GetFloatParam(4);
    float y1 = basic4gl.GetFloatParam(3);
    float x2 = basic4gl.GetFloatParam(2);
    float y2 = basic4gl.GetFloatParam(1);
    glBegin(GL_QUADS);
        Box(x1, y1, x2, y2);
    glEnd();
}

void DLLFUNC Func_DrawEllipse(IDLL_Basic4GL_Runtime &basic4gl) {
    float x = basic4gl.GetFloatParam(4);
    float y = basic4gl.GetFloatParam(3);
    float xRad = basic4gl.GetFloatParam(2);
    float yRad = basic4gl.GetFloatParam(1);
    glBegin(GL_LINE_LOOP);
        Ellipse(x, y, xRad, yRad);
    glEnd();
}

void DLLFUNC Func_FillEllipse(IDLL_Basic4GL_Runtime &basic4gl) {
    float x = basic4gl.GetFloatParam(4);
    float y = basic4gl.GetFloatParam(3);
    float xRad = basic4gl.GetFloatParam(2);
    float yRad = basic4gl.GetFloatParam(1);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
        Ellipse(x, y, xRad, yRad);
        glVertex2f(x, y + yRad);
    glEnd();
}

void DLLFUNC Func_DrawCircle(IDLL_Basic4GL_Runtime &basic4gl) {
    float x = basic4gl.GetFloatParam(3);
    float y = basic4gl.GetFloatParam(2);
    float rad = basic4gl.GetFloatParam(1);
    glBegin(GL_LINE_LOOP);
        Circle(x, y, rad);
    glEnd();
}

void DLLFUNC Func_FillCircle(IDLL_Basic4GL_Runtime &basic4gl) {
    float x = basic4gl.GetFloatParam(3);
    float y = basic4gl.GetFloatParam(2);
    float rad = basic4gl.GetFloatParam(1);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
        Circle(x, y, rad);
        glVertex2f(x, y + rad);
    glEnd();
}

///////////////////////////////////////////////////////////////////////////////
//	Plugin

bool DLLFUNC Plugin::Load(IDLL_Basic4GL_FunctionRegistry &registry, bool isStandaloneExe) {

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Register structures.

	// First we register the structure with RegisterStructure.
	// Then we define the fields using the AddStrucField/AddStrucStringField methods.
	// We can also add array fields using the ModStrucFieldArray method.
	//
	// Note 1:
	//	When setting field types, we use the DLL_Basic4GL_Ext_TypeCode constants.
	//	This allows use to specify C/C++ field type and allows us to specify types that
	//	aren't directly supported by Basic4GL such as 'byte', or 'double'.
	//	Basic4GL will automatically convert data to/from the closest native Basic4GL type.
	//
	// Note 2:
	//	Strings are supported as character arrays (using AddStringField).
	//	Use AddStrucStringField(...) to add the field, specifying the number of characters
	//	in the array.
	//
	// Note 3:
	//	It is good practice to check whether RegisterStructure returns 0, and if so, report an error.
	//	This can happen if another plugin registers a structure with the same name.

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Register constants
	registry.RegisterIntConstant("BLACK", BLACK);
    registry.RegisterIntConstant("BLUE", BLUE);
    registry.RegisterIntConstant("GREEN", GREEN);
    registry.RegisterIntConstant("CYAN", CYAN);
    registry.RegisterIntConstant("RED", RED);
    registry.RegisterIntConstant("MAGENTA", MAGENTA);
    registry.RegisterIntConstant("BROWN", BROWN);
    registry.RegisterIntConstant("GRAY", GRAY);
    registry.RegisterIntConstant("LIGHTGRAY", LIGHTGRAY);
    registry.RegisterIntConstant("LIGHTBLUE", LIGHTBLUE);
    registry.RegisterIntConstant("LIGHTGREEN", LIGHTGREEN);
    registry.RegisterIntConstant("LIGHTCYAN", LIGHTCYAN);
    registry.RegisterIntConstant("LIGHTRED", LIGHTRED);
    registry.RegisterIntConstant("LIGHTMAGENTA", LIGHTMAGENTA);
    registry.RegisterIntConstant("YELLOW", YELLOW);
    registry.RegisterIntConstant("WHITE", WHITE);

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Register functions
	registry.RegisterVoidFunction("Mode2D", Func_Mode2D);
        registry.AddParam(DLL_BASIC4GL_INT);
        registry.AddParam(DLL_BASIC4GL_INT);

	registry.RegisterVoidFunction("Mode2DBuffered", Func_Mode2DBuffered);
        registry.AddParam(DLL_BASIC4GL_INT);
        registry.AddParam(DLL_BASIC4GL_INT);

	registry.RegisterVoidFunction("ClearScreen", Func_ClearScreen);
        registry.AddParam(DLL_BASIC4GL_INT);
        registry.AddParam(DLL_BASIC4GL_INT);
        registry.AddParam(DLL_BASIC4GL_INT);

	registry.RegisterVoidFunction("ClearScreen", Func_ClearScreen2);
        registry.AddParam(DLL_BASIC4GL_INT);

    registry.RegisterVoidFunction("SetColor", Func_SetColor);
        registry.AddParam(DLL_BASIC4GL_INT);
        registry.AddParam(DLL_BASIC4GL_INT);
        registry.AddParam(DLL_BASIC4GL_INT);

    registry.RegisterVoidFunction("SetColor", Func_SetColor2);
        registry.AddParam(DLL_BASIC4GL_INT);

    registry.RegisterVoidFunction("MoveTo", Func_MoveTo);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.AddParam(DLL_BASIC4GL_FLOAT);

    registry.RegisterVoidFunction("MoveTo", Func_MoveTo2);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.ModParamArray(1);

    registry.RegisterVoidFunction("LineTo", Func_LineTo);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.AddParam(DLL_BASIC4GL_FLOAT);

    registry.RegisterVoidFunction("LineTo", Func_LineTo2);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.ModParamArray(1);

    registry.RegisterVoidFunction("LineWidth", Func_LineWidth);
        registry.AddParam(DLL_BASIC4GL_FLOAT);

    registry.RegisterVoidFunction("Plot", Func_Plot);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.AddParam(DLL_BASIC4GL_FLOAT);

    registry.RegisterVoidFunction("DrawBox", Func_DrawBox);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.AddParam(DLL_BASIC4GL_FLOAT);

    registry.RegisterVoidFunction("FillBox", Func_FillBox);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.AddParam(DLL_BASIC4GL_FLOAT);

    registry.RegisterVoidFunction("DrawEllipse", Func_DrawEllipse);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.AddParam(DLL_BASIC4GL_FLOAT);

    registry.RegisterVoidFunction("FillEllipse", Func_FillEllipse);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.AddParam(DLL_BASIC4GL_FLOAT);

    registry.RegisterVoidFunction("DrawCircle", Func_DrawCircle);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.AddParam(DLL_BASIC4GL_FLOAT);

    registry.RegisterVoidFunction("FillCircle", Func_FillCircle);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.AddParam(DLL_BASIC4GL_FLOAT);
        registry.AddParam(DLL_BASIC4GL_FLOAT);

	return true;
}

void DLLFUNC Plugin::Unload() {
	delete this;
}

bool DLLFUNC Plugin::Start() {
    // Reset drawing state
    cursorX = 0;
    cursorY = 0;
	return true;
}

void DLLFUNC Plugin::End() {

}

void DLLFUNC Plugin::GetError(char *error) {

	// Copy error text from our global error variable
	if (pluginError != NULL)
		for (int i = 0; pluginError[i] != 0; i++)
			error[i] = pluginError[i];
}

void DLLFUNC Plugin::Pause() {

}

void DLLFUNC Plugin::Resume() {

}

void DLLFUNC Plugin::DelayedResume() {

}

void DLLFUNC Plugin::ProcessMessages() {

    // Called periodically (in practice, quite frequently).
    // This would be a good place to process windows messages etc.
}
