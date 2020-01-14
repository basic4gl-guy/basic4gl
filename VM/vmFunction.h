//---------------------------------------------------------------------------
// Created 28-Sep-2003: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
// Copyright (C) Thomas Mulgrew
/*  Function interface.
*/

#ifndef vmFunctionH
#define vmFunctionH
//---------------------------------------------------------------------------

// Function wrapper
class TomVM;
typedef void (*vmFunction)(TomVM& vm);
    // Function wrapper must extract parameters virtual machine's function stack,

#endif
