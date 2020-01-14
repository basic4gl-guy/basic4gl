/*	Created 29-Feb-2004: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
	Copyright (C) Tom Mulgrew, 2004.

	Structures and routines for standalone Basic4GL programs.

	Standalone programs have all their necessary data patched into the end of the program
	by the standalone .exe generator.
	This module defines the structure of this data, along with the necessary routines to
	unpack it into a useable format.

*/

#ifndef _standalone_h
#define _standalone_h

#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////
// Standalone header

#define STANDALONE_VERSION 4

#ifndef byte
typedef unsigned char byte;
#endif

#pragma pack(push,1)
struct StandaloneHeader {
	int version;
    char title[256];
    byte finishedOption;
    byte errorOption;
	bool useDesktopRes;					// Use desktop resolution (screenWidth/-Height ignored)
	int16_t screenWidth;
	int16_t screenHeight;
    byte colourDepth;
    bool stencil;
    bool fullScreen;
    bool escKeyQuits;
	bool border;
    bool allowResizing;
    byte startupWindowOption;           // 0 = Create window. 1 = Don't create it (yet)
};
#pragma pack(pop)

/*bool UnpackStandaloneData (
    char *rawData,
    StandaloneHeader& header,
    TomBasicCompiler& comp,
    FileOpener& files,
    DLLFileVector& dllFiles);
*/
#endif