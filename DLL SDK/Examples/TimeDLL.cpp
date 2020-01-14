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

struct TimeOfDay {
    byte hour;
    byte minute;
    byte second;
    WORD millisecond;
    byte am;        // True if AM, false if PM
};

#pragma pack(pop)

// Global structure type handles.
int timeOfDayHandle = 0;

// Helper functions
TimeOfDay GetTimeOfDay24Hr() {

    // Get system time from windows
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	// Convert to our own time structure
    TimeOfDay time;
    time.hour = sysTime.wHour;
    time.minute = sysTime.wMinute;
    time.second = sysTime.wSecond;
    time.millisecond = sysTime.wMilliseconds;
    time.am = 0;

    return time;
}

TimeOfDay GetTimeOfDay12Hr() {
    TimeOfDay time = GetTimeOfDay24Hr();

    // Convert to 12 hr time
    time.am = time.hour < 12;
    if (!time.am)
        time.hour -= 12;
    if (time.hour == 0)
        time.hour = 12;

    return time;
}

///////////////////////////////////////////////////////////////////////////////
// Runtime functions

void DLLFUNC Func_Time(IDLL_Basic4GL_Runtime &basic4gl) {

	// Get time of day structure
    TimeOfDay time = GetTimeOfDay12Hr();

    // Return to Basic4GL
    basic4gl.SetType(timeOfDayHandle);
    basic4gl.SetReturnValue(&time);
}

void DLLFUNC Func_Time24Hr(IDLL_Basic4GL_Runtime &basic4gl) {

	// Get time of day structure
    TimeOfDay time = GetTimeOfDay24Hr();

    // Return to Basic4GL
    basic4gl.SetType(timeOfDayHandle);
    basic4gl.SetReturnValue(&time);
}

void DLLFUNC Func_TimeStr(IDLL_Basic4GL_Runtime &basic4gl) {
    TimeOfDay time = GetTimeOfDay12Hr();

    char buffer[32];
    char *c = buffer;
    *c++ = time.hour / 10 + '0';
    *c++ = time.hour % 10 + '0';
    *c++ = ':';
    *c++ = time.minute / 10 + '0';
    *c++ = time.minute % 10 + '0';
    *c++ = ':';
    *c++ = time.second / 10 + '0';
    *c++ = time.second % 10 + '0';
    *c++ = ' ';
    *c++ = time.am ? 'A' : 'P';
    *c++ = 'M';
    *c = 0;

    basic4gl.SetStringType(32);
    basic4gl.SetReturnValue(buffer);
}

void DLLFUNC Func_Time24HrStr(IDLL_Basic4GL_Runtime &basic4gl) {
    TimeOfDay time = GetTimeOfDay24Hr();

    char buffer[32];
    char *c = buffer;
    *c++ = time.hour / 10 + '0';
    *c++ = time.hour % 10 + '0';
    *c++ = ':';
    *c++ = time.minute / 10 + '0';
    *c++ = time.minute % 10 + '0';
    *c++ = ':';
    *c++ = time.second / 10 + '0';
    *c++ = time.second % 10 + '0';
    *c = 0;

    basic4gl.SetStringType(32);
    basic4gl.SetReturnValue(buffer);
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
	timeOfDayHandle = registry.RegisterStructure("TimeOfDay", 1, 0);
	if (timeOfDayHandle == 0) {
		pluginError = "Error registering structure: TimeOfDay";
		return false;
	}
	registry.AddStrucField("hour", DLL_BASIC4GL_EXT_BYTE);
	registry.AddStrucField("min", DLL_BASIC4GL_EXT_BYTE);
	registry.AddStrucField("sec", DLL_BASIC4GL_EXT_BYTE);
	registry.AddStrucField("ms", DLL_BASIC4GL_EXT_WORD);
	registry.AddStrucField("am", DLL_BASIC4GL_EXT_BYTE);

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Register functions
	registry.RegisterStructureFunction("Time", Func_Time, timeOfDayHandle);

	registry.RegisterStructureFunction("Time24Hr", Func_Time24Hr, timeOfDayHandle);

    registry.RegisterFunction("Time$", Func_TimeStr, DLL_BASIC4GL_STRING);

    registry.RegisterFunction("Time24Hr$", Func_Time24HrStr, DLL_BASIC4GL_STRING);


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
