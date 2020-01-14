/*  Created 21-Dec-07: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

    Interfaces for communicating between application and virtual machine view
*/

#ifndef _VMViewInterfaces_h
#define _VMViewInterfaces_h

#include "TomComp.h"
#include "compPreprocessor.h"

////////////////////////////////////////////////////////////////////////////////
//  IVMView
//
/// A virtual machine view
class IVMView {
public:
    /// Display virtual machine view
    virtual void ShowVMView() = 0;

    /// Refresh display
    virtual void RefreshVMView() = 0;

    /// Inform the virtual machine UI that the VM has started or stopped.
    virtual void SetVMIsRunning(bool isRunning) = 0;
};

////////////////////////////////////////////////////////////////////////////////
//  IVMViewCallbacks
//
/// Callback interface to the application controlling the virtual machine view
class IVMViewCallbacks {
public:
    /// Execute a single VM opcode
    virtual void ExecuteSingleOpCode() = 0;
};

////////////////////////////////////////////////////////////////////////////////
//  Global functions
//
/// Get virtual machine view
extern IVMView *GetVMView(
    IVMViewCallbacks* callbacks,
    TomBasicCompiler* compiler,
    compPreprocessor* preprocessor);

#endif
