//---------------------------------------------------------------------------
// Created 26-Jun-2003: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
// Copyright (C) Thomas Mulgrew
/*  Windows specific function and constant library
*/

#ifndef TomOpenGLBasicLibH
#define TomOpenGLBasicLibH
//---------------------------------------------------------------------------
#include "TomComp.h"
#include "EmbeddedFiles.h"
#include "Corona.h"
#include "OpenGLWindowManager.h"

void InitTomOpenGLBasicLib (TomBasicCompiler& comp, FileOpener *_files, OpenGLWindowManager* windowManager);
corona::Image *LoadImage (std::string filename);

#endif
