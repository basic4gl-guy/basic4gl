//---------------------------------------------------------------------------
// Created 26-Jun-2003: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
// Copyright (C) Thomas Mulgrew
/*  Windows specific function and constant library
*/

#ifndef TomTextBasicLibH
#define TomTextBasicLibH
//---------------------------------------------------------------------------
#include "TomComp.h"
#include "glTextGrid.h"
#include "Content2DManager.h"
#include "GlfwKeyboard.h"

void InitTomTextBasicLib(TomBasicCompiler& comp, glTextGrid *text, OpenGLWindowManager* windowManager, Content2DManager* contentManager, OpenGLKeyboard* keyboard, OpenGLMouse* mouse);


#endif
