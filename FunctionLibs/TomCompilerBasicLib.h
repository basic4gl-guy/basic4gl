/*  Created 17-Apr-06: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
    Copyright (C) Thomas Mulgrew

    Functions for compiling and executing code at runtime.
*/
#ifndef TomCompilerBasicLibH
#define TomCompilerBasicLibH

#include "TomComp.h"
#include "EmbeddedFiles.h"

class VMHostApplication {
public:

    // Process windows messages and stay responsive.
    // Return true if virtual machine can continue, or false if interrupted.
    // (E.g. a "stop" button clicked..)
    virtual bool ProcessMessages() = 0;
};

void InitTomCompilerBasicLib(
    TomBasicCompiler& _comp,
    VMHostApplication *_host,
    FileOpener *_files);

#endif
