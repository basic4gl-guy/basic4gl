#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
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
		char *detailSrc = "Structure test DLL";
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

struct ID {
	char name[100];
	int number;
	char address[4][100];
};

struct Entity {
	// Rotation/position matrix
	float m[4][3];
	int idCode;
	byte health;
};

struct AddressBook {
	int count;
	ID entries[100];
};

#pragma pack(pop)

// Global structure type handles.
int idHandle = 0, entityHandle = 0, addressBookHandle = 0;

///////////////////////////////////////////////////////////////////////////////
// Runtime functions

// Note: 
//	These functions are intended to demonstrate the use of the "extended" data 
//	access methods of IDLL_Basic4GL_Runtime. They will use them even when the 
//	same result can be accomplished with the simple data access methods.

void DLLFUNC Func_MakeString(IDLL_Basic4GL_Runtime &basic4gl) {
	
	// First parameter is a character string
	char c[2];			// We must allow for 1 character, and 1 zero byte
	basic4gl.SetStringType(2);
	basic4gl.GetParam(2, c);

	// Second parameter is the count
	int count;
	basic4gl.SetType(DLL_BASIC4GL_EXT_INT);
	basic4gl.GetParam(1, &count);

	// Build return string
	char buffer[1024];
	if (count > 1023)
		count = 1023;
	for (int i = 0; i < count; i++) 
		buffer[i] = c[0];
	buffer[count] = 0;			// Terminate string

	// Return it
	basic4gl.SetStringType(1024);
	basic4gl.SetReturnValue(buffer);
}

void DLLFUNC Func_CountChars(IDLL_Basic4GL_Runtime &basic4gl) {

	// First parameter is string
	char str[1024];
	basic4gl.SetStringType(1024);
	basic4gl.GetParam(2, str);

	// Second parameter is character to count
	char c[2];			// Must allow for 0 terminater
	basic4gl.SetStringType(2);
	basic4gl.GetParam(1, c);

	// Count # characters in string
	int count = 0;
	for (char *p = str; *p != 0; p++) 
		if (*p == c[0])
			count++;

	// Return as integer
	basic4gl.SetType(DLL_BASIC4GL_EXT_INT);
	basic4gl.SetReturnValue(&count);
}

void DLLFUNC Func_SwapInt(IDLL_Basic4GL_Runtime &basic4gl) {

	// Parameters are "by reference". 
	// This means we can return values in them.
	int p1, p2;

	basic4gl.SetType(DLL_BASIC4GL_EXT_INT);
	basic4gl.ModTypeReference();			// Very important!
	basic4gl.GetParam(2, &p1);
	basic4gl.GetParam(1, &p2);

	// Write values back to parameters, but swapped.
	// We don't have to set the type, because it's still set to "int by reference"
	basic4gl.SetParam(1, &p1);
	basic4gl.SetParam(2, &p2);
}

void DLLFUNC Func_SwapString(IDLL_Basic4GL_Runtime &basic4gl) {

	// Parameters are "by reference". 
	// This means we can return values in them.
	char p1[1024], p2[1024];

	basic4gl.SetStringType(1024);
	basic4gl.ModTypeReference();			// Very important!
	basic4gl.GetParam(2, p1);
	basic4gl.GetParam(1, p2);

	// Write values back to parameters, but swapped.
	// We don't have to set the type, because it's still set to "string by reference"
	basic4gl.SetParam(1, p1);
	basic4gl.SetParam(2, p2);
}

void DLLFUNC Func_SwapVector(IDLL_Basic4GL_Runtime &basic4gl) {
	float v1[4];
	float v2[4];

	// Parameters are arrays.
	// Arrays are automatically passed by reference, so we don't need to explicitly
	// call ModTypeReference()
	basic4gl.SetType(DLL_BASIC4GL_EXT_FLOAT);
	basic4gl.ModTypeArray(1, 4);
	basic4gl.GetParam(2, v1);
	basic4gl.GetParam(1, v2);

	// Write values back to parameters, but swapped.
	// We don't have to set the type, because it's still set to "string by reference"
	basic4gl.SetParam(1, v1);
	basic4gl.SetParam(2, v2);
}

void DLLFUNC Func_MakeID(IDLL_Basic4GL_Runtime &basic4gl) {
	ID id;

	// First parameter is name
	basic4gl.SetStringType(100);
	basic4gl.GetParam(6, id.name);

	// Second parameter is number
	basic4gl.SetType(DLL_BASIC4GL_EXT_INT);
	basic4gl.GetParam(5, &id.number);

	// Remaining parameters are address
	basic4gl.SetStringType(100);
	basic4gl.GetParam(4, id.address[0]);
	basic4gl.GetParam(3, id.address[1]);
	basic4gl.GetParam(2, id.address[2]);
	basic4gl.GetParam(1, id.address[3]);

	// Return structure
	basic4gl.SetType(idHandle);
	basic4gl.SetReturnValue(&id);
}

void DLLFUNC Func_AddIDToAddressBook(IDLL_Basic4GL_Runtime &basic4gl) {	
	ID id;
	AddressBook addressBook;

	// Fetch id
	basic4gl.SetType(idHandle);
	basic4gl.GetParam(2, &id);

	// Fetch address book
	basic4gl.SetType(addressBookHandle);
	basic4gl.GetParam(1, &addressBook);

	// Add ID to address book
	if (addressBook.count < 100) {
		addressBook.entries[addressBook.count] = id;
		addressBook.count++;
	}

	// Copy address book back to parameter.
	// Note: We can do this, because structure paramters are automatically
	// passed by reference.
	basic4gl.SetType(addressBookHandle);
	basic4gl.SetParam(1, &addressBook);
}

void DLLFUNC Func_SortEntities(IDLL_Basic4GL_Runtime &basic4gl) {	
	
	// Get array size
	int count = basic4gl.GetArrayParamDimension(1, 0);

	// Allocate array
	Entity *entities = new Entity[count];

	// Read array from parameter
	basic4gl.SetType(entityHandle);
	basic4gl.ModTypeArray(1, count);
	basic4gl.GetParam(1, entities);

	// Sort array.
	// Will use simple insertion sort.
	// Will sort by Z (m[11]) value.
	for (int i = 1; i < count; i++) {
		int j = i;
		while (j > 0 && entities[j].m[3][2] < entities[j - 1].m[3][2]) {
			// Swap elements
			Entity temp = entities[j];
			entities[j] = entities[j - 1];
			entities[j - 1] = temp;

			j--;
		}
	}

	// Copy back to parameter.
	// We can do this because array parameters are automatically passed by 
	// reference.
	// Note: The type is already set to array of entities from before, so we
	// don't need to specify it again.
	basic4gl.SetParam(1, entities);

	// Can delete entities array once finished
	delete[] entities;
}

void DLLFUNC Func_MakeRandomPos(IDLL_Basic4GL_Runtime &basic4gl) {
	float pos[4][3];
	memset(pos, 0, 12 * sizeof(float));
	pos[0][0] = 1;
	pos[1][1] = 1;
	pos[2][2] = 1;
	pos[3][0] = (rand() % 2001) / 100.0f - 10.0f;
	pos[3][1] = (rand() % 2001) / 100.0f - 10.0f;
	pos[3][2] = (rand() % 2001) / 100.0f - 10.0f;

	// Return float array
	basic4gl.SetType(DLL_BASIC4GL_EXT_FLOAT);
	basic4gl.ModTypeArray(2, 4, 3);
	basic4gl.SetReturnValue(pos);
}

void DLLFUNC Func_DescribeID(IDLL_Basic4GL_Runtime &basic4gl) {
	// Check if parameter is null
	// Note: Parameter will be passed as a virtual machine "pointer".
	// In Basic4GL, virtual machine pointers are integers, and 0 corresponds to NULL.
	// So we can fetch the pointer by reading the parameter as an integer.
	int ptr = basic4gl.GetIntParam(1);

	// Check if NULL
	if (ptr == 0) 
		basic4gl.SetStringResult("[NULL]");

	// Not NULL.
	else {

		// Read the structure.
		// Note that we can simply read it as if it were a structure passed by reference,
		// (as by-reference means its passed as a pointer anyway.)
		ID id;
		basic4gl.SetType(idHandle);
		basic4gl.GetParam(1, &id);

		// Describe ID
		basic4gl.SetStringResult(id.name);
	}
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
	idHandle = registry.RegisterStructure("ID", 1, 0);
	if (idHandle == 0) {
		pluginError = "Error registering structure: ID";
		return false;
	}
	registry.AddStrucStringField("name$", 100);					// char[100] field
	registry.AddStrucField("number", DLL_BASIC4GL_EXT_INT);		// int field
	registry.AddStrucStringField("address$", 100);				// Array of char[100] fields.
		registry.ModStrucFieldArray(1, 4);						// Convert into array. 1 dimension, 4 elements.

	entityHandle = registry.RegisterStructure("Entity", 1, 0);
	if (entityHandle == 0) {
		pluginError = "Error registering structure: Entity";
		return false;
	}
	registry.AddStrucField("m#", DLL_BASIC4GL_EXT_FLOAT);		// float[4][3] matrix
		registry.ModStrucFieldArray(2, 4, 3);					// Convert into array. 2 dimensions, 4x3 elements
	registry.AddStrucField("idCode", DLL_BASIC4GL_EXT_INT);
	registry.AddStrucField("health", DLL_BASIC4GL_EXT_BYTE);

	addressBookHandle = registry.RegisterStructure("AddressBook", 1, 0);
	if (addressBookHandle == 0) {
		pluginError = "Error registering structure: AddressBookHandle";
		return false;
	}
	registry.AddStrucField("count", DLL_BASIC4GL_EXT_INT);
	registry.AddStrucField("entries", idHandle);				// An array of 100 ID structures
		registry.ModStrucFieldArray(1, 100);
		
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Register functions
	registry.RegisterFunction("MakeString$", Func_MakeString, DLL_BASIC4GL_STRING);
		registry.AddParam(DLL_BASIC4GL_STRING);
		registry.AddParam(DLL_BASIC4GL_INT);

	registry.RegisterFunction("CountChars", Func_CountChars, DLL_BASIC4GL_INT);
		registry.AddParam(DLL_BASIC4GL_STRING);
		registry.AddParam(DLL_BASIC4GL_STRING);

	registry.RegisterVoidFunction("SwapInt", Func_SwapInt);
		registry.AddParam(DLL_BASIC4GL_INT);	
			registry.ModParamReference();	
		registry.AddParam(DLL_BASIC4GL_INT);
			registry.ModParamReference();

	registry.RegisterVoidFunction("SwapString", Func_SwapString);
		registry.AddParam(DLL_BASIC4GL_STRING);	
			registry.ModParamReference();	
		registry.AddParam(DLL_BASIC4GL_STRING);
			registry.ModParamReference();

	registry.RegisterVoidFunction("SwapVector", Func_SwapVector);
		registry.AddParam(DLL_BASIC4GL_FLOAT);
			registry.ModParamArray(1);
		registry.AddParam(DLL_BASIC4GL_FLOAT);
			registry.ModParamArray(1);

	registry.RegisterStructureFunction("MakeID", Func_MakeID, idHandle);
		registry.AddParam(DLL_BASIC4GL_STRING);
		registry.AddParam(DLL_BASIC4GL_INT);
		registry.AddParam(DLL_BASIC4GL_STRING);
		registry.AddParam(DLL_BASIC4GL_STRING);
		registry.AddParam(DLL_BASIC4GL_STRING);
		registry.AddParam(DLL_BASIC4GL_STRING);

	registry.RegisterVoidFunction("AddIDToAddressBook", Func_AddIDToAddressBook);
		registry.AddStrucParam(idHandle);
		registry.AddStrucParam(addressBookHandle);

	registry.RegisterVoidFunction("SortEntities", Func_SortEntities);
		registry.AddStrucParam(entityHandle);
			registry.ModParamArray(1);

	registry.RegisterFunction("MakeRandomPos#", Func_MakeRandomPos, DLL_BASIC4GL_FLOAT);
		registry.ModReturnArray(2);

	registry.RegisterFunction("DescribeID", Func_DescribeID, DLL_BASIC4GL_STRING);
		registry.AddStrucParam(idHandle);
			registry.ModParamPointer(1);

	return true;
}

void DLLFUNC Plugin::Unload() {
	delete this;
}

bool DLLFUNC Plugin::Start() {
	pluginError = "Ooops";
	return false;
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
