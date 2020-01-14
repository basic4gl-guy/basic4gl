#define WIN32_LEAN_AND_MEAN
#include <windows.h>
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
		char *detailSrc = "Time functions";
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

struct RandSeed {
    unsigned int x;         // Current value
    byte isHigh;            // Using high word?
};

#pragma pack(pop)

// Global structure type handles.
unsigned int randSeedHandle = 0;

// Constants
// Note: Random implementation is based on the Borland C/C++ linear congurential
// random number generator.
const unsigned int a = 22695477;
const unsigned int c = 1;

// Helper functions
unsigned int GetRand(RandSeed& s) {

    // Calculate next X in sequence
    if (!s.isHigh)
        s.x = s.x * a + c;

    // Return alternating high and low words
    unsigned int result = s.isHigh
        ? (s.x & 0xffff0000) >> 16
        : s.x & 0x0000ffff;
    s.isHigh = !s.isHigh;

    return result;
}

RandSeed GetSeed(unsigned int seed) {
    RandSeed s;
    s.x = seed;
    s.isHigh = false;
    return s;
}

///////////////////////////////////////////////////////////////////////////////
// Runtime functions

void DLLFUNC Func_MakeRandSeed(IDLL_Basic4GL_Runtime &basic4gl) {

    // Get seed value parameter
    unsigned int seedVal = (unsigned int)basic4gl.GetIntParam(1);

    // Initialise seed structure
    RandSeed seed = GetSeed(seedVal);

    // Return to Basic4GL
    basic4gl.SetType(randSeedHandle);
    basic4gl.SetReturnValue(&seed);
}

void DLLFUNC Func_MakeRandSeed2(IDLL_Basic4GL_Runtime &basic4gl) {

    // Initialise seed structure from timer
    RandSeed seed = GetSeed(GetTickCount());

    // Return to Basic4GL
    basic4gl.SetType(randSeedHandle);
    basic4gl.SetReturnValue(&seed);
}

void DLLFUNC Func_Rand(IDLL_Basic4GL_Runtime &basic4gl) {

    // Get random seed
    RandSeed seed;
    basic4gl.SetType(randSeedHandle);
    basic4gl.GetParam(1, &seed);

    // Get next random number
    basic4gl.SetIntResult(GetRand(seed));

    // Copy seed back to parameter
    basic4gl.SetType(randSeedHandle);
    basic4gl.SetParam(1, &seed);
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
	randSeedHandle = registry.RegisterStructure("SRandSeed", 1, 0);
	if (randSeedHandle == 0) {
        pluginError = "Error registering structure: RandSeed";
        return false;
	}
    registry.AddStrucField("x", DLL_BASIC4GL_EXT_INT);
    registry.AddStrucField("isHigh", DLL_BASIC4GL_EXT_BYTE);

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Register functions
	registry.RegisterStructureFunction("RandSeed", Func_MakeRandSeed, randSeedHandle);
        registry.AddParam(DLL_BASIC4GL_INT);

    registry.RegisterStructureFunction("RandSeed", Func_MakeRandSeed2, randSeedHandle);

    registry.RegisterFunction("Rand", Func_Rand, DLL_BASIC4GL_INT);
        registry.AddStrucParam(randSeedHandle);

	return true;
}

void DLLFUNC Plugin::Unload() {
	delete this;
}

bool DLLFUNC Plugin::Start() {
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
