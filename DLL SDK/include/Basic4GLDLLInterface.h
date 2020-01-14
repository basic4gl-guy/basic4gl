//---------------------------------------------------------------------------
/*  Created 13-Feb-06: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

    Interface header for writing Basic4GL plug in DLLs.
*/

#ifndef Basic4GLDLLInterfaceH
#define Basic4GLDLLInterfaceH
//---------------------------------------------------------------------------

#include "DLLMisc.h"

// Forward declarations
class IDLL_Basic4GL_ObjectStore;
class IDLL_Basic4GL_ObjectStoreCallback;
class IDLL_Basic4GL_LongRunningFunction;

#define BASIC4GL_DLL_VERSION 4

////////////////////////////////////////////////////////////////////////////////
// Basic type constants
enum DLL_Basic4GL_TypeCode {
    DLL_BASIC4GL_INT    = 0,
    DLL_BASIC4GL_FLOAT  = 1,
    DLL_BASIC4GL_STRING = 2,
	DLL_BASIC4GL_FUNCPTR = 3
};

////////////////////////////////////////////////////////////////////////////////
// Extended type constants
//
// Used when defining structures.
// Basic4GL's data structures don't support all the types defined here. However
// you can use these to define the data structure as it would be stored in C.
// Basic4GL will then use this information when converting data between its own
// format and C structure format.
enum DLL_Basic4GL_Ext_TypeCode {
	DLL_BASIC4GL_EXT_PADDING = -256,	// Empty padding in C structure
	DLL_BASIC4GL_EXT_BYTE,
	DLL_BASIC4GL_EXT_WORD,
	DLL_BASIC4GL_EXT_INT,
	DLL_BASIC4GL_EXT_INT64,			    // 64 bit integer
	DLL_BASIC4GL_EXT_FLOAT,				// 32 bit floating point number
	DLL_BASIC4GL_EXT_DOUBLE,			// 64 bit floating point number
	DLL_BASIC4GL_EXT_STRING 			// Fixed length string
};

////////////////////////////////////////////////////////////////////////////////
//  IDLL_Basic4GL_Runtime
//
/// The main interface to Basic4GL for runtime functions in DLLs. This interface
/// is passed to runtime functions when they are called, and is used to perform
/// necessary services such as fetching function parameters, setting return
/// values and converting between Basic4GL and standard C/C++ formats.
class IDLL_Basic4GL_Runtime {
public:

    /// Fetch function parameter.
    /// Parameters are numbered from right to left, the right most parameter
    /// being index 1.
    /// The request must be correct and correspond to the function definition.
    /// I.e. if you declare paramter 3 as an integer, you MUST request it as
    /// an integer. If you declare a 2 parameter function, you MUST not request
    /// parameter 4.
    /// If you do so, Basic4GL's behaviour will be undefined, and it will likely
    /// crash.
    ///
    /// Basic4GL has just 3 basic types:
    /// int, float (called "real" in Basic4gl documentation) and string (char*)
    virtual int         DLLFUNC GetIntParam(int index) = 0;
    virtual float       DLLFUNC GetFloatParam(int index) = 0;
    virtual const char* DLLFUNC GetStringParam(int index) = 0;

    /// Set function return value.
    /// Be sure to call the appropriate function for your return value type,
    /// as declared in your function declaration.
    /// (If your function doesn't return a value, don't call anything...)
    /// Otherwise behaviour is undefined and Basic4GL could crash.
    virtual void DLLFUNC SetIntResult(int result) = 0;
    virtual void DLLFUNC SetFloatResult(float result) = 0;
    virtual void DLLFUNC SetStringResult(const char *result) = 0;

    /// Get array parameter, and convert to C/C++ style array
    /// Note: Will convert to a fixed size array.
    /// If the array from the Basic4GL program size does not match, it will
    /// simply ignore any overhang from the Basic4GL array, and leave any overhang
    /// from the target array set to 0.
    virtual void DLLFUNC GetIntArrayParam(
        int index,
		int *array,						// Data is returned here. Must be as big as the array params.
        int dimensions,                 // 10 is the maximum # of dimesions supported by Basic4GL
        int dimension0Size,
        ...) = 0;
    virtual void DLLFUNC GetFloatArrayParam(
        int index,
		float *array,					// Data is returned here. Must be as big as the array params.
        int dimensions,                 // 10 is the maximum # of dimesions supported by Basic4GL
        int dimension0Size,
        ...) = 0;

	/// Return the size of an array dimension.
	/// "index" is the parameter index.
	/// "dimension" is the dimension index, 0 origin.
	/// The return value is the actual number of elements in the array (for that dimension).
	/// Need to keep in mind that the BASIC dim command actually allocates one more element
	/// than the number specified (0..N inclusive).
	/// So if an array is DIMmed as: dim a(10)
	/// this method will return 11 as the number of elements.
	virtual int DLLFUNC GetArrayParamDimension(
		int index,
		int dimension) = 0;

    /// Set array parameter result
    virtual void DLLFUNC SetIntArrayResult(
		int *array,
        int dimensions,
        int dimension0Size,
        ...) = 0;
    virtual void DLLFUNC SetFloatArrayResult(
		float *array,
        int dimensions,
        int dimension0Size,
        ...) = 0;

	//---------------------------------------------------------------------------------------------
	// Extended data access methods
	//
	// These methods handle converting data between C variables/structures and Basic4GL parameters
	// and return values.
	//
	// Note: Unlike the above methods, when specifying a data type you must specify the C data type
	//	using the DLL_Basic4GL_Ext_TypeCode codes. Basic4GL will automatically translate between the
	//	specified type and its own data types appropriately.
	//
	// Reading/writing data is a 2 step format:
	//
	//	1. Define the data type using one of the following methods:
	//		* SetType
	//		* SetStringType
	//	   You may also need to use one of the following modifiers:
	//		* ModTypeArray
	//		* ModTypeReference
	//
	//	2. Call the appropriate method to copy and convert the data. One of:
	//		* GetParam
	//		* SetParam
	//		* SetReturnValue

	// Data type definition for data conversion
	virtual void DLLFUNC SetType(int baseType) = 0;					// Can either be a DLL_Basic4GL_Ext_TypeCode, or a structure type handle (returned from RegisterStructure or FetchStructure)
	virtual void DLLFUNC SetStringType(int size) = 0;				// Number of characters (including terminating 0)

	/// Convert a type into an array
	virtual void DLLFUNC ModTypeArray(
		int dimensions,
		int dimension0Size,
		...) = 0;

	/// Convert a type into a reference.
	/// Useful for accessing reference parameters.
	/// Only required if the parameter is a reference to a basic type (int, float or string).
	/// Basic4GL automatically treats arrays and structures as by-reference types, and does
	/// not need to be told!
	virtual void DLLFUNC ModTypeReference() = 0;

	// Reading/writing parameters and return values
	virtual void DLLFUNC GetParam(
		int index,							// Parameter index
		void *dst) = 0;						// Data is copied here

	virtual void DLLFUNC SetParam(
		int index,							// Parameter index
		void *src) = 0;						// Data is copied from here

	virtual void DLLFUNC SetReturnValue(
		void *src) = 0;						// Data is copied from here

	//---------------------------------------------------------------------------------------------
	// Direct data access.
	// These methods allow you to access Basic4GL variable memory directly.
	// To use these methods you must have a good understanding of the underlying
	// Basic4GL data structures, as reading/writing data incorrectly can cause
	// crashes and lockups.
	// These methods should be used as a last resort, if none of the other methods
	// can will achieve the result you need.

	/// Get int value at address
	virtual int DLLFUNC DirectGetInt(int memAddr) = 0;

	/// Get float value at address
	virtual float DLLFUNC DirectGetFloat(int memAddr) = 0;

	/// Get string value at address.
	/// 'str' and 'maxlen' describe the buffer into which the string will be
	/// copied (if it is longer than maxLen-1, it will be truncated).
	/// The method always returns 'buffer'
	virtual char *DLLFUNC DirectGetString(int memAddr, char* str, int maxLen) = 0;

	/// Write int value to address
	virtual void DLLFUNC DirectSetInt(int memAddr, int value) = 0;

	/// Write float value to address
	virtual void DLLFUNC DirectSetFloat(int memAddr, float value) = 0;

	/// Write real value to address
	virtual void DLLFUNC DirectSetString(int memAddr, const char *str) = 0;
	//---------------------------------------------------------------------------------------------

	/// Begin a long running function.
	/// This should be called from a regular BASIC function which should then immediately return.
	/// *** THE FUNCTION MUST ALSO BE REGISTERED WITH A TIMESHARING BREAK (using ModTimeShare()). ***
	/// Basic4GL will not execute any more VM op-codes until the long running function handler
	/// calls EndLongRunningFn().
	/// The main application event loop will continue to run however, so that operating system events
	/// can be processed. Basic4GL can poll the long running function handler if required
	/// (see the IDLL_Basic4GL_LongRunningFunction interface for more info).
	///
	/// An example of a long running function would be stopping and waiting for a user to key in a 
	/// text string.
	/// * The regular BASIC function would create the handler object, hook it up to receive 
	///   keypress events and call BeginLongRunningFn(). 
	/// * Basic4GL will perform a timesharing break after the regular function returns (because it is 
	///   marked timesharing with ModTimeShare()).
	/// * Basic4GL will then process operating system events, but not execute any VM op-codes.
	/// * The handler will process keypress events and update the display as necessary.
	/// * Once the user presses ENTER, the handler will set the result (using SetStringResult())
	///   and call EndLongRunningFn().
	/// * Basic4GL will then resume excuting VM op-codes.
	virtual void DLLFUNC BeginLongRunningFn(IDLL_Basic4GL_LongRunningFunction* handler) = 0;

	/// End a long running function and resume executing BASIC program op-codes.
	virtual void DLLFUNC EndLongRunningFn() = 0;

	/// Indicate an error from a function
	virtual void DLLFUNC FunctionError(const char* text) = 0;

	/// Indicates to the virtual machine that a timesharing break is required.
	/// If the BASIC function has been modified with:
	/// IDLL_Basic4GL_FunctionRegistry.ModTimeshareBreak()
	/// this will trigger a time sharing break after the function returns.
	/// This is useful for creating conditional timesharing breaks, when it 
	/// is not known until runtime whether the break is required.
	virtual void DLLFUNC TimeshareBreakRequired() = 0;
};

////////////////////////////////////////////////////////////////////////////////
//  DLL_Function
//
/// A pointer to a runtime function in a DLL
typedef void (DLLFUNC *DLL_Basic4GL_Function)(IDLL_Basic4GL_Runtime &basic4gl);

////////////////////////////////////////////////////////////////////////////////
//  IDLL_Basic4GL_FunctionRegistry
//
/// Used to register functions to Basic4GL.
class IDLL_Basic4GL_FunctionRegistry {
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Constants

    /// Register a constant
    virtual void DLLFUNC RegisterStringConstant(
        const char *name,
        const char *value) = 0;

    virtual void DLLFUNC RegisterIntConstant(
        const char *name,
        int value) = 0;

    virtual void DLLFUNC RegisterFloatConstant(
        const char *name,
        float value) = 0;

	///////////////////////////////////////////////////////////////////////////
	//	Standard functions

    /// Register function that returns no value
    virtual void DLLFUNC RegisterVoidFunction(
        const char *name,
        DLL_Basic4GL_Function function) = 0;

    /// Register function that returns single value
    virtual void DLLFUNC RegisterFunction(
        const char *name,
        DLL_Basic4GL_Function function,
        DLL_Basic4GL_TypeCode typeCode) = 0;

    /// Register function that returns an array.
	/// Note: This method has been superceeded by ModReturnArray(), and is retained
	/// only for backwards compatibility with older plugins.
    virtual void DLLFUNC RegisterArrayFunction(
        const char *name,
        DLL_Basic4GL_Function function,
        DLL_Basic4GL_TypeCode typeCode,
        int dimensions) = 0;

	/// Register function that returns a structure.
	/// structureTypeHandle is the handle returned from RegisterStructure(), or FetchStructure().
	virtual void DLLFUNC RegisterStructureFunction(
		const char *name,
		DLL_Basic4GL_Function function,
		int structureTypeHandle) = 0;

	///////////////////////////////////////////////////////////////////////////
	//	Function modifiers

	/// Modifies the last registered function to return an array.
	/// E.g. if the function was registered as returning an integer, this will
	/// alter it to return an array of integers.
	virtual void DLLFUNC ModReturnArray(int dimensions) = 0;

	/// Modifies the last registered function to return a pointer.
	///	E.g. if the function was registered as returning a string, this will
	/// alter it to return a pointer to a string.
	///
	/// Note:
	///		Returning pointers is an advanced operation, and you need to be
	///		careful that you understand Basic4GL's data formats and know
	///		exactly what you're doing.
	///		Unless you know exactly why you need to return a pointer from
	///		a DLL function, you should avoid this method.
	///
	/// Level is the level of indirection:
	///		1 = Pointer (most common)
	///		2 = Pointer to pointer
	///	  ...
	virtual void DLLFUNC ModReturnPointer(int level) = 0;

    /// Modifies the last registered function to have no brackets.
    /// Note:
    /// By convention ALL Basic4GL functions have brackets except for a special
    /// subset of historical commands such as PRINT and LOCATE.
    /// Please try to stick to this convention, unless the function really is an
    /// historical BASIC command.
    virtual void DLLFUNC ModNoBrackets() = 0;

    /// Modifies the last registered function to trigger a timesharing break
    /// after it is called.
    /// If your function will (or may) take a reasonable amount of time to
    /// complete (say 0.01 seconds or more), you should use this mod to ensure
    /// Basic4GL forces a timeshare break after it, so that it stays responsive.
    /// Some examples are:
    ///     * File I/O functions
    ///     * Pausing timer based functions
    virtual void DLLFUNC ModTimeshare() = 0;

	///////////////////////////////////////////////////////////////////////////
	//	Standard function parameters

    /// Add standard parameter to the last registered function.
    /// Parameters should be added from left to right.
    /// Please be aware that the RIGHTMOST parameter will have index 1 when the
    /// function is called at runtime, i.e. the LAST parameter added.
    virtual void DLLFUNC AddParam(DLL_Basic4GL_TypeCode typeCode) = 0;

    /// Add an array parameter to the last registered function.
	/// Note: This method has been superceeded by ModParamArray(), and is retained
	/// only for backwards compatibility with older plugins.
    virtual void DLLFUNC AddArrayParam(
        DLL_Basic4GL_TypeCode typeCode,
        int dimensions) = 0;

	/// Add a structure parameter to the last registered function.
	/// 'handle' is the structure type handle returned from "RegisterStructure" or
	/// "FetchStructure"
	virtual void DLLFUNC AddStrucParam(int handle) = 0;

	/// Convert the last parameter added into an array parameter.
	virtual void DLLFUNC ModParamArray(int dimensions) = 0;

	/// Convert the last parameter added into a pointer.
	/// 'level' is the level of indirection:
	/// Using pointers is an advanced function, and you need to be sure you know
	/// what you're doing. If in doubt, avoid...
	///		1 = Pointer (most common)
	///		2 = Pointer to pointer etc
	virtual void DLLFUNC ModParamPointer(int level) = 0;

	/// Convert the last parameter added into a reference parameter.
	/// This is like Pascal's "var" parameters, or C++'s & parameters.
	/// They can be used to return values.
	/// Note: The parameter will appear as a pointer to the plugin function,
	/// and should be accessed like one.
	/// Note 2: Structure and array parameters are automatically passed by
	/// reference, so you don't need to call this method for them.
	virtual void DLLFUNC ModParamReference() = 0;

	///////////////////////////////////////////////////////////////////////////
	//	Structures

	/// Register a new structure.
	/// Returns a handle allowing the structure to be referenced in function
	/// or parameter declarations.
	/// Will return 0 if fails (can fail if the structure has been registered
	/// already). Plugin should set an appropriate error and return false.
	virtual int DLLFUNC RegisterStructure(
        const char *name,
        int versionMajor,
        int versionMinor) = 0;

	///////////////////////////////////////////////////////////////////////////
	// Structure fields

	/// Add empty padding
	virtual void DLLFUNC AddStrucPadding(int numBytes) = 0;

	/// Add regular field.
	/// type can be a DLL_Basic4GL_Ext_TypeCode constant, or structure handle.
	virtual void DLLFUNC AddStrucField(const char *name, int type) = 0;

	/// Add a string field.
	/// Must supply string size in bytes (including 0 terminator)
	virtual void DLLFUNC AddStrucStringField(const char *name, int size) = 0;

	/// Convert the last defined structure field into an array
	virtual void DLLFUNC ModStrucFieldArray(int dimensions, int dimension1Size, ...) = 0;

	/// Convert the last defined structure field into a pointer
	/// Note: 'level' = the level of indirection.
	///	1 = Pointer (most common)
	///	2 = Pointer to pointer
	/// ...
	virtual void DLLFUNC ModStrucFieldPointer(int level) = 0;

    /// Fetch structure (presumably registered by another plugin).
    /// Returns handle, or 0 if not registered.
    virtual int DLLFUNC FetchStructure(
        const char *name,
        int versionMajor,
        int versionMinor) = 0;

	///////////////////////////////////////////////////////////////////////////
	// Object sharing
	//
	// DLLs can register objects to Basic4GL (by text ID), so that other DLLs
	// can fetch and use them. Basic4GL records that the DLL using the object
	// is dependent on the DLL that constructed it, and ensures that DLLs are
	// unloaded in the correct order.
	// The DLL that constructed the object should destroy it in
	// IDLL_Basic4GL_Plugin::Unload() and NOT before.

	///	Register a shared interface to Basic4GL, which can be fetched and used
	/// by other DLLs.
	///	"name", "major" and "minor" are used to identify the object (major.minor
	///	is its version number), so that other DLLs can request it.
	virtual void DLLFUNC RegisterInterface(
		void *object,
		const char *name,
		int major,
		int minor) = 0;

	/// Fetch an interface from another DLL.
	/// Returns the interface if a matching one is found.
	/// Returns NULL otherwise.
	virtual void *DLLFUNC FetchInterface(
		const char *name,
		int major,
		int minor) = 0;

	///////////////////////////////////////////////////////////////////////////
    // Object store
	//
    // These are objects that store miscellaneous objects, and assign them
    // integer IDs (that can be assigned to Basic4GL variables).

    /// Create an object store.
    /// "callback" (can be NULL if not used) is called to notify the DLL whenever
    /// an object is removed.
    /// Note: The DLL should not destroy this object. It will be destroyed
    /// automatically by Basic4GL when the DLL is unloaded.
    virtual IDLL_Basic4GL_ObjectStore *DLLFUNC CreateObjectStore(IDLL_Basic4GL_ObjectStoreCallback *callback) = 0;

	/// Modifies the last registered function to trigger a timesharing break
	/// after it is called, IF the "timeshare" flag has been set in the virtual machine.
	/// See IDLL_Basic4GL_Runtime.TimeshareBreakRequired().
	virtual void DLLFUNC ModConditionalTimeshare() = 0;
};

////////////////////////////////////////////////////////////////////////////////
//  IDLL_Basic4GL_Plugin
//
/// This object must be implemented by the DLL.
/// Basic4GL will ask for it when it loads the DLL (via the DLL_Init function),
/// and will call its methods to extract information about the DLL, such as its
/// function list.
/// It will also call various methods when certain events occur, such as the
/// program starts or stops.
class IDLL_Basic4GL_Plugin {
public:

    /// Called ONCE when the DLL loads.
    /// Perform any initialisation and register any functions to the registery
    /// object passed in.
    /// Return true if the DLL loaded correctly, or false if not (see GetError()
    /// for how to report the error.)
    virtual bool DLLFUNC Load(IDLL_Basic4GL_FunctionRegistry &registry, bool isStandaloneExe) = 0;

    /// Called when the DLL is about to unload.
    /// Perform any final cleanup here.
	/// Note: Basic4GL will not attempt to access the object after this function
	/// is called so the object can safely destroy itself.
    virtual void DLLFUNC Unload() = 0;

    /// Called when a Basic4GL program is about to run.
    /// This is a good place to initialise any data that must be reset each time
    /// the program runs.
    /// Return true if the program can proceed, or false if it cannot (see
    /// GetError() for how to report the error.)
    virtual bool DLLFUNC Start() = 0;

    /// Called when a Basic4GL program stops, either from reaching the end or
    /// if it is interrupted.
    /// This is a good place to clean up any resources used by the DLL while the
    /// program is running.
    /// DO NOT LEAVE IT UP TO THE BASIC4GL PROGRAMMER TO PREVENT RESOURCE AND
    /// MEMORY LEAKS. BASIC4GL IS DESIGNED TO BE A FORGIVING LANGUAGE, THAT
    /// PEOPLE CAN PLAY AND LEARN WITH, WHICH MEANS THE DLLS AND RUNTIME
    /// FUNCTIONS MUST DO A BIT OF EXTRA WORK TO MAKE SURE THEY KEEP TRACK OF
    /// EVERYTHING.
    virtual void DLLFUNC End() = 0;

	/// Called when the program has paused.
	/// This applies to programs running in the Basic4GL IDE only.
	/// DLLs that respond to this event generally do so to make debugging easier.
	/// E.g. a full screen graphics library might switch back to the desktop, so
	/// that the programmer can see the Basic4GL IDE again.
	virtual void DLLFUNC Pause() = 0;

	/// Called when the program resumes after it has been paused.
	virtual void DLLFUNC Resume() = 0;

	/// Called when the program resumes after it has been paused and 1000 VM op-codes
	/// have been executed.
	/// DLLs might respond to this method (instead of Resume()) to avoid switching
	/// between running and paused mode unnecessarily during short executions (e.g.
	/// when the programmer is stepping through the program).
	/// For example, a full screen graphics library might do this so that the monitor
	/// doesn't switch resolutions and back every time the programmer hits the "step"
	/// button.
	virtual void DLLFUNC DelayedResume() = 0;

    /// Return the text of the last error.
    /// Called whenever Load() or Start() returns false to fetch the text of the
    /// error. The method is passed a pointer to a 1024 character array into
    /// which it should write the text of the message (0 terminated).
    /// If GetError() does not modify "error", a generic "DLL could not load/start"
    /// message will be displayed to the user.
    virtual void DLLFUNC GetError(char *error) = 0;

    /// This is called periodically by Basic4GL to keep the application responsive.
    /// If your plugin requires any periodic house-keeping (e.g. if it creates
    /// a window, and needs to respond to windows messages), this is a good place
    /// to put your code. (In fact Basic4GL calls this method immediately after
    /// it processes its own windows messages).
    virtual void DLLFUNC ProcessMessages() = 0;
};

////////////////////////////////////////////////////////////////////////////////
//	DLL functions

/// Called to query the DLL.
/// Basic4GL will pass pointers to:
/// A 256 character array, into which the DLL should write a 0 terminated description string
/// A major and minor version integer, into which the DLL should return version information
/// Function must be named Basic4GL_Query (and declared in an extern "C" block)
/// Function MUST return BASIC4GL_DLL_VERSION.
typedef int (DLLFUNC *DLL_Basic4GL_QueryFunction)(char *details, int *major, int *minor);

/// Called when the DLL is loaded. Function should return an object supporting
/// the IDLL_Basic4GL_Plugin interface.
/// Function must be named Basic4GL_Init (and declared in an extern "C" block)
typedef IDLL_Basic4GL_Plugin *(DLLFUNC *DLL_Basic4GL_InitFunction)();

////////////////////////////////////////////////////////////////////////////////
//  IDLL_Basic4GL_ObjectStore
//
/// These are optional utility objects that your DLL can request from Basic4GL.
/// They are used to associate an object with an integer handle. (Basic4GL
/// variables cannot store object references, but can store integer handles).
/// Basic4GL also tracks all objects allocated with an object store, and will
/// call your DLL to dispose of any that are still active when the Basic4GL
/// program completes.
class IDLL_Basic4GL_ObjectStore {
public:

    /// Add an object and return a new unique integer handle
    virtual int DLLFUNC Add(void *object) = 0;

    /// Remove an object, and free up its handle.
    /// This also calls the dispose-of-object callback.
    virtual void DLLFUNC Remove(int handle) = 0;

    /// Return true if handle is valid.
    virtual bool DLLFUNC HandleIsValid(int handle) = 0;

    /// Return object for given handle, or NULL if none.
    virtual void *DLLFUNC Object(int handle) = 0;

    /// Remove ALL objects and free up handles.
    /// The dispose-of-object callback is called for each object.
    virtual void Clear() = 0;
};

////////////////////////////////////////////////////////////////////////////////
//  IDLL_Basic4GL_ObjectStoreCallback
//
/// Implement this interface to be notified when objects have been removed
/// from an IDLL_Basic4GL_ObjectStore.
class IDLL_Basic4GL_ObjectStoreCallback {
public:
    /// Called when an object is removed, either by:
    /// * An explicit call to IDLL_Basic4GL_ObjectStore::Remove()
    /// * An explicit call to IDLL_Basic4GL_ObjectStore::Clear()
    /// * An implicit clear, such as when a Basic4GL program completes.
    virtual void DLLFUNC ObjectRemoved(IDLL_Basic4GL_ObjectStore *store, void *object) = 0;
};

////////////////////////////////////////////////////////////////////////////////
//	IDLL_Basic4GL_LongRunningFunction
//
/// Interface to a BASIC function that suspends Basic4GL execution until it 
/// completes.
/// See IDLL_Basic4GL_Runtime::BeginLongRunningFn() for more information.
class IDLL_Basic4GL_LongRunningFunction
{
public:
	virtual ~IDLL_Basic4GL_LongRunningFunction() {};
	
	/// Return true to have Basic4GL periodically call "Poll()"
	virtual bool IsPolled() = 0;

	/// Return true to have Basic4GL automatically delete this object after it resumes (or is cancelled).
	/// IMPORTANT: If this returns true, the object should consider itself invalid as soon as it calls
	/// IDLL_Basic4GL_Runtime::Resume().
	virtual bool DeleteWhenDone() = 0;

	/// Periodically called by Basic4GL if "IsPolled()" returns true
	virtual void Poll() = 0;

	/// Called by Basic4GL if execution is cancelled,  e.g. BASIC program has been stopped/restarted
	virtual void Cancel() = 0;
};

#endif
