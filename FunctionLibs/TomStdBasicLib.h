//---------------------------------------------------------------------------
// Created 15-Jun-2003: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
// Copyright (C) Thomas Mulgrew
/*  Standard basic function and constant library.
*/

#ifndef TomStdBasicLibH
#define TomStdBasicLibH
//---------------------------------------------------------------------------
#include "TomComp.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
#ifndef M_E
#define M_E 2.7182818284590452353602874713526
#endif
#define M_RAD2DEG (180/M_PI)
#define M_DEG2RAD (M_PI/180)

void InitTomStdBasicLib (TomBasicCompiler& comp);
void SetProgramArguments(std::vector<std::string> const& arguments);
std::vector<std::string> GetProgramArguments();
#endif
