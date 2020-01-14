//---------------------------------------------------------------------------
// Created 22-Jun-2003: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
// Copyright (C) Thomas Mulgrew
/*  Windows specific function and constant library
*/


#ifndef TomWindowsBasicLibH
#define TomWindowsBasicLibH
//---------------------------------------------------------------------------
#include "TomComp.h"
#include "EmbeddedFiles.h"
#include <OpenGLWindowManager.h>

void InitTomWindowsBasicLib (TomBasicCompiler& comp, FileOpener *_files, OpenGLWindowManager* _windowManager);
void ShutDownTomWindowsBasicLib ();
#endif
