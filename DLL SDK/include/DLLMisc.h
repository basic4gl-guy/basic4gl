/*	DLLMisc.h

	Created 12-Mar-06: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

	Miscellaneous DLL types.
*/

#ifndef _DLLMisc_h
#define _DLLMisc_h

// DLL function/method modifiers.
// (This may be different for different compilers)

#ifndef DLLExport
#ifndef __ANDROID__
#define DLLExport __declspec(dllexport)
#else
#define DLLExport
#endif
#endif

#ifndef DLLFUNC
#ifndef __ANDROID__
#define DLLFUNC __cdecl
#else
#define DLLFUNC
#endif
#endif

#endif
